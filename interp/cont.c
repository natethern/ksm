/* Copyright (C) 2000 - 2001 Hangil Chang

   This file is part of KSM-Scheme, Kit for Scheme Manipulation Scheme.
   
   KSM-Scheme is free software; you can distribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
  
   KSM-Scheme is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with KSM-Scheme; see the file COPYING. If not, write to the
   Free Software Foundation, 59 Temple Place, Suite 330, Boston,
   MA 02111 USA.                                                   */

#include <setjmp.h>
#include <string.h>
#include "ksm.h"

typedef struct ksm_cont_image {
  void (*mark)(void *);
  void (*free)(void *);
  void (*restore)(void *);
  void *data;
  struct ksm_cont_image *next;
} ksm_cont_image_t;

typedef struct {
  jmp_buf regs;
  ksm_t ret_value;
  ksm_t jumps;
  unsigned stack_size;  // number of int's
  int *stack_buf;
  ksm_cont_image_t *image;
  int thread_serial;    // owner thread
} ksm_cont_t;

#define MAKE_CONT(p)   ksm_itype(KSM_FLAG_CONT, (int)(p),0,0)
#define IS_CONT(x)     (ksm_flag(x)==KSM_FLAG_CONT)
#define CONT_PTR(cont) ((ksm_cont_t *)ksm_ival(cont,0))

#define CONT_PTR_NO_LOCK(cont) ((ksm_cont_t *)ksm_ival_no_lock(cont,0))

static
ksm_t mark_cont(ksm_t cont)
{
  ksm_cont_t *cp;
  ksm_cont_image_t *ip;

  cp = CONT_PTR_NO_LOCK(cont);
  ksm_mark_object(cp->ret_value);
  ksm_mark_object(cp->jumps);
  ip = cp->image;
  while(ip){
    (*ip->mark)(ip->data);
    ip = ip->next;
  }
  return 0;
}

static
void dispose_cont(ksm_t cont)
{
  ksm_cont_t *cp;
  ksm_cont_image_t *ip, *iq;

  cp = CONT_PTR_NO_LOCK(cont);
  if( cp->stack_buf )
    ksm_dispose(cp->stack_buf);
  ip = cp->image;
  while(ip){
    iq = ip->next;
    (*ip->free)(ip->data);
    ksm_dispose(ip);
    ip = iq;
  }
}

typedef struct {
  void *(*create)(void);
  void (*mark)(void *);
  void (*free)(void *);
  void (*restore)(void *);
} cont_op_t;

#define MAX_IMAGE  10
static cont_op_t ops[MAX_IMAGE];   /*** GLOBAL ***/
static int n_ops;                  /*** GLOBAL ***/

void ksm_install_cont_op(void *(*fun_create)(void), void (*fun_mark)(void *),
	 void (*fun_free)(void *), void (*fun_restore)(void *))
{
  cont_op_t *p;

  if( n_ops >= MAX_IMAGE )
    ksm_fatal("too many cont ops (ksm_install_cont_op)");
  p = &ops[n_ops++];
  p->create = fun_create;
  p->mark = fun_mark;
  p->free = fun_free;
  p->restore = fun_restore;
}

static
ksm_cont_image_t *save_image(void)
{
  ksm_cont_image_t *p = 0, *t;
  cont_op_t *q;
  int i;

  for(i=0;i<n_ops;++i){
    q = &ops[i];
    t = ksm_alloc(sizeof(*t));
    t->mark = q->mark;
    t->free = q->free;
    t->restore = q->restore;
    t->data = (*q->create)();
    t->next = p;
    p = t;
  }
  return p;
}

/*** stack image ****************************************************/

#define KSM_STACK_BASE  (ksm_get_ctx()->stack_base)

static
int *stack_top(int arg)
{
  return &arg;
}

static
int *stack_image(unsigned *size, ksm_t *adr, ksm_t **image_adr)
{
  int *p, *q;
  unsigned n;

  p = stack_top(0);
  n = KSM_STACK_BASE - p;
  q = ksm_alloc(sizeof(int)*n);
  memcpy(q, p, sizeof(int)*n);
  *size = n;
  *image_adr = (ksm_t *)(q + ((int *)adr - p));
  return q;
}

/*** interface ******************************************************/

static
void set_cont(ksm_t *adr)
{
  ksm_cont_t *cp;
  ksm_t cont, *image_adr;

  cp = ksm_alloc(sizeof(*cp));
  cp->ret_value = ksm_unspec_object;
  cp->jumps = ksm_nil_object;
  cp->stack_buf = stack_image(&cp->stack_size, adr, &image_adr);
  cp->image = save_image();
  cp->thread_serial = ksm_get_ctx()->serial;
  cont = MAKE_CONT(cp);
  ksm_push_jump(cont);
  cp->jumps = ksm_fetch_jumps();
  *adr = *image_adr = cont;
}

int ksm_is_cont(ksm_t x)
{
  return ksm_flag(x) == KSM_FLAG_CONT;
}

static
ksm_t ksm_cont_ret(ksm_t cont)
{
  return CONT_PTR(cont)->ret_value;
}

static
void ksm_cont_set_ret(ksm_t cont, ksm_t val)
{
  CONT_PTR(cont)->ret_value = val;
}

static
ksm_t ksm_cont_jumps(ksm_t cont)
{
  return CONT_PTR(cont)->jumps;
}

static
jmp_buf *ksm_cont_regs(ksm_t cont)
{
  return &CONT_PTR(cont)->regs;
}

static
void ksm_cont_restore(ksm_t cont)
{
  ksm_cont_image_t *ip;

  ip = CONT_PTR(cont)->image;
  while(ip){
    (*ip->restore)(ip->data);
    ip = ip->next;
  }
}

/*** eval ***********************************************************/

static
void restore_full_stack(ksm_t cont, int no_more)
{
  int cont_n;
  int *cont_p, *cont_q;
  ksm_cont_t *cp;

  cp = CONT_PTR(cont);
  if( no_more )
    goto L1;
  if( cp->stack_size >= (KSM_STACK_BASE - (int *)&cont) )
    restore_full_stack(cont, 0);
  else
    restore_full_stack(cont, 1);
 L1:
  cont_n = cp->stack_size;
  cont_p = cp->stack_buf;
  cont_q = KSM_STACK_BASE - cont_n;
  ksm_cont_restore(cont);
  while( cont_n-- > 0 )
    *cont_q++ = *cont_p++;
  longjmp(*ksm_cont_regs(cont), 2);
}

#ifdef PTHREAD
static
void check_owner(ksm_t cont)
{
  if( CONT_PTR(cont)->thread_serial != ksm_get_ctx()->serial )
    ksm_error("can't restore continuation of other thread");
}
#endif

static
ksm_pair call_cont(ksm_t cont, ksm_t args, ksm_t env)
{
  if( ksm_length(args) != 1 )
    ksm_error("invalid number of args: %k", args);
  ksm_cont_set_ret(cont, ksm_car(args));
  if( !ksm_is_nil(ksm_rewind_jumps(cont)) ) // cheap continuation
    longjmp(*ksm_cont_regs(cont), 1);
  else{                                     // full continuation
#ifdef PTHREAD    
    check_owner(cont);
#endif
    ksm_set_jumps(ksm_cont_jumps(cont));
    ksm_pop_jump();
    ksm_sweep_jumps();
    restore_full_stack(cont, 0);
  }
  return ksm_make_eval_done(ksm_unspec_object);
}

/*** registry image *************************************************/

typedef struct {
  unsigned size;
  ksm_t *buf;
} registry_image_t;

static
void *create_registry_image(void)
{
  unsigned size;
  ksm_t *buf;
  registry_image_t *p;
  
  size = ksm_registry_size();
  buf = ksm_alloc(sizeof(*buf)*size);
  ksm_registry_save_image(buf);
  p = ksm_alloc(sizeof(*p));
  p->size = size;
  p->buf = buf;
  return p;
}

static
void mark_registry_image(void *data)
{
  registry_image_t *image = data;
  unsigned i;
  ksm_t *p;
  
  p = image->buf;
  for(i=0;i<image->size;++i)
    ksm_mark_object(*p++);
}

static
void free_registry_image(void *data)
{
  registry_image_t *image = data;

  ksm_dispose(image->buf);
  ksm_dispose(image);
}

static
void restore_registry_image(void *data)
{
  registry_image_t *image = data;

  ksm_registry_restore_image(image->buf, image->size);
}

/*** blt ************************************************************/

KSM_DEF_BLT(callcc)
     // (call/cc (lambda (cc) ...))
{
  ksm_t fun, cont, val;

  KSM_SET_ARGS1(fun);
  set_cont(&cont);
  if( setjmp(*ksm_cont_regs(cont)) == 0 ){
    val = ksm_funcall(fun, ksm_list1(cont), env);
    ksm_pop_jump();
    return val;
  }
  else
    return ksm_cont_ret(cont);
}

/*** init ***********************************************************/

void ksm_init_cont(void)
{
  ksm_install_marker(KSM_FLAG_CONT, mark_cont);
  ksm_install_disposer(KSM_FLAG_CONT, dispose_cont);
  ksm_install_cont_op(create_registry_image, mark_registry_image,
		      free_registry_image, restore_registry_image);
  ksm_install_fun_info(KSM_FLAG_CONT, call_cont, 1);
  KSM_ENTER_BLT("call-with-current-continuation", callcc);
  KSM_ENTER_BLT("call/cc", callcc);
}
