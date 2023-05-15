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

#include <string.h>
#include <errno.h>
#include "ksm.h"

union ksm_object{
  struct {
    int flag;
    int ivals[3];
  } u_int;
  struct {
    int flag;
    int aux;
    double dval;
  } u_double;
};

int ksm_gc_shower = 0; /*** GLOBAL ***/
static void gc(void);
static void registry_add(ksm_t x);

#define OBJ_FLAG(obj)    ((obj)->u_int.flag)
#define MARKBIT          (1<<31)
#define MARK_OBJ(obj)    (OBJ_FLAG(obj))|=MARKBIT
#define UNMARK_OBJ(obj)  (OBJ_FLAG(obj))&=~MARKBIT
#define IS_MARKED(obj)   (OBJ_FLAG(obj)&MARKBIT)

#define IS_FREE(obj)     (OBJ_FLAG(obj)==KSM_FLAG_FREE)
#define SET_FREE(obj)    OBJ_FLAG(obj)=KSM_FLAG_FREE
#define NEXT_FREE(obj)   ((ksm_t)(obj)->u_int.ivals[0])

/*** marker *********************************************************/

typedef ksm_t (*marker_t)(ksm_t);
KSM_INSTALLER(marker,marker_t)    /*** GLOBAL ***/
#define GET_MARKER(obj) get_marker(OBJ_FLAG(obj))

/*** disposer *******************************************************/

typedef void (*disposer_t)(ksm_t);
KSM_INSTALLER(disposer, disposer_t) /*** GLOBAL ***/
#define GET_DISPOSER(obj) get_disposer(OBJ_FLAG(obj))

/*** objtab *********************************************************/

#define OBJTAB_SIZE  10000
static union ksm_object objtab[OBJTAB_SIZE];   /*** GLOBAL ***/
static ksm_t objtab_top = objtab;              /*** GLOBAL ***/
static ksm_t objtab_last = objtab+OBJTAB_SIZE; /*** GLOBAL ***/
static ksm_t free_obj;                         /*** GLOBAL ***/

#ifdef PTHREAD
static pthread_mutex_t objtab_mutex = PTHREAD_MUTEX_INITIALIZER;

void ksm_lock(void)
{
  pthread_mutex_lock(&objtab_mutex);
}

void ksm_unlock(void)
{
  pthread_mutex_unlock(&objtab_mutex);
}
#endif  /* PTHREAD */

static
ksm_t find_object(void)
     // returns 0 if not found
{
  ksm_t p;

  if( free_obj ){
    p = free_obj;
    free_obj = NEXT_FREE(free_obj);
    return p;
  }
  else
    return 0;
}

static
void recycle_object(ksm_t p)
{
  SET_FREE(p);
  NEXT_FREE(p) = free_obj;
  free_obj = p;
}

static
ksm_t new_object(void)
{
  ksm_t p;
  
  if( ksm_gc_shower )
    gc();

  if( objtab_top == objtab_last ){
    p = find_object();
    if( p == 0 ){
      gc();
      p = find_object();
      if( p == 0 )
	ksm_fatal("out of memory");
    }
  }
  else
    p = objtab_top++;
  return p;
}

void ksm_mark_object(ksm_t p)
{
  ksm_t (*m)(ksm_t);

 again:
  if( IS_MARKED(p) )
    return;

  MARK_OBJ(p);
  m = GET_MARKER(p);
  if( m ){
    p = (*m)(p);
    if( p )
      goto again;
  }
}

static
void sweep(void)
{
  ksm_t p;
  void (*d)(ksm_t);

  for(p=objtab;p<objtab_top;++p){
    if( IS_MARKED(p) ){
      UNMARK_OBJ(p);
    }
    else{
      if( IS_FREE(p) )
	continue;
      d = GET_DISPOSER(p);
      if( d )
	(*d)(p);
      recycle_object(p);
    }
  }
}

/*** registry *******************************************************/

#define MAXREG  5000

static ksm_t objreg[MAXREG];

struct ksm_registry {
  ksm_t *objreg;
  ksm_t *objreg_last;
  ksm_t *objreg_next;
};

#ifdef PTHREAD
static
struct ksm_registry *copy_registry(struct ksm_registry *p)
{
  struct ksm_registry *q;
  int bytes;

  q = ksm_alloc(sizeof(*q));
  bytes = ((char *)p->objreg_last)-((char *)p->objreg);
  q->objreg = ksm_alloc(bytes);
  memcpy(q->objreg, p->objreg, bytes);
  q->objreg_last = q->objreg + (p->objreg_last-p->objreg);
  q->objreg_next = q->objreg + (p->objreg_next-p->objreg);
  return q;
}

static
void dispose_registry(struct ksm_registry *p)
{
  ksm_dispose(p->objreg);
  ksm_dispose(p);
}
#endif    // PTHREAD

static 
struct ksm_registry main_registry = {
  &objreg[0], 
  &objreg[0]+MAXREG, 
  &objreg[0]
};

/*** THREAD SPECIFIC DATA ***/
#ifdef PTHREAD
#define CTX_MAX   20
#else
#define CTX_MAX   1
#endif

static int ctx_count = 1;
#ifdef PTHREAD
ksm_ctx *free_ctx;
pthread_key_t ksm_ctx_key;

static
#endif
ksm_ctx ksm_main_ctx[CTX_MAX] = {
  {&main_registry, KSM_THREAD_RUNNING}
};

#ifdef PTHREAD
ksm_ctx *ksm_get_ctx(void)
{
  return pthread_getspecific(ksm_ctx_key);
}

static int serial = 0;

static int get_serial(void)
{
  if( ++serial == 0 )
    ksm_fatal("too many threads");
  return serial;
}

ksm_ctx *ksm_new_ctx(ksm_t fun)
{
  ksm_ctx *cur, *ctx;

  ksm_lock();

  if( free_ctx ){
    ctx = free_ctx;
    free_ctx = ctx->next_free;
  }
  else{
    if( ctx_count >= CTX_MAX )
      ksm_fatal("too many threads");
    ctx = &ksm_main_ctx[ctx_count];
  }

  cur = pthread_getspecific(ksm_ctx_key);
  ctx->registry = copy_registry(cur->registry);
  ctx->state = KSM_THREAD_CREATED;
  ctx->detached = 0;
  ctx->fun = fun;
  ctx->result = ksm_unspec_object;
  ctx->input = cur->input;
  ctx->output = cur->output;
  ctx->jumps = ksm_nil_object;
  ctx->private_global = cur->private_global;
  ctx->specific = 0;
  ctx->trace_on = cur->trace_on;
  ctx->serial = get_serial();
  ctx->load_file_name = cur->load_file_name;
  ctx->load_shared_file_name = cur->load_shared_file_name;
  ++ctx_count;
  ksm_unlock();
  return ctx;
}

static
void recycle_ctx(ksm_ctx *ctx)
     // to be called while ctx-locked
{
  ctx->state = KSM_THREAD_EMPTY;
  ctx->next_free = free_ctx;
  free_ctx = ctx;
}

static
void release_ctx_resource(void *arg)
     // releases ctx resources except ctx->result
{
  ksm_ctx *ctx = arg;

  ksm_lock();
  if( ctx->registry != &main_registry )
    dispose_registry(ctx->registry);
  ctx->registry = 0;
  ctx->fun    = ksm_unspec_object;
  ctx->input  = ksm_unspec_object;
  ctx->output = ksm_unspec_object;
  ctx->jumps  = ksm_unspec_object;
  ctx->private_global  = ksm_unspec_object;
  ctx->specific        = 0;
  ksm_free_strbuf(&ctx->read_buf);
  ksm_free_strbuf(&ctx->readline_buf);
  ksm_free_strbuf(&ctx->readstr_buf);
  if( ctx->detached )
    recycle_ctx(ctx);
  ksm_unlock();
}

void ksm_start_ctx(ksm_ctx *ctx, int *stack_base)
{
  ctx->state = KSM_THREAD_RUNNING;
  ctx->stack_base = stack_base;
}

void ksm_finish_ctx(ksm_ctx *ctx, ksm_t result)
{
  ctx->state = KSM_THREAD_FINISHED;
  ctx->result = result;
}

void ksm_exit_ctx(ksm_ctx *ctx, ksm_t result)
{
  ctx->state = KSM_THREAD_EXITED;
  ctx->result = result;
}

void ksm_cancel_ctx(ksm_ctx *ctx)
{
  ctx->state = KSM_THREAD_CANCELED;
}

ksm_t ksm_join_ctx(ksm_ctx *ctx, pthread_t thread_id)
{
  int err;
  ksm_t ret;

  err = pthread_join(thread_id, 0);
  switch(err){
  case ESRCH: ksm_error("no such thread (PTHREAD_JOIN)"); break;
  case EINVAL: ksm_error("can't join thread (PTHREAD_JOIN)"); break;
  case EDEADLK: ksm_error("dead lock (PTHREAD_JOIN)"); break;
  }
  ksm_lock();
  ret = ctx->result;
  recycle_ctx(ctx);
  registry_add(ret);
  ksm_unlock();
  return ret;
}

static
ksm_t specific_tab(void)
{
  ksm_t tab;

  tab = ksm_get_ctx()->specific;
  if( !tab ){
    tab = ksm_get_ctx()->specific = ksm_make_global(20, ksm_nil_object);
  }
  return tab;
}

ksm_t ksm_get_specific(ksm_t key)
{
  ksm_t tab, bind;

  tab = specific_tab();
  bind = ksm_lookup_global(key, tab, 0);
  if( !bind )
    ksm_error("can't find specifid: %k", key);
  return bind;
}

void ksm_set_specific(ksm_t key, ksm_t value)
{
  ksm_t tab;

  tab = specific_tab();
  ksm_enter_global(key, value, tab, 1);
}

#endif  /* PTHREAD */

void ksm_init_object(void)
{
  ksm_main_ctx[0].stack_base = ksm_stack_base;
#ifdef PTHREAD
  pthread_key_create(&ksm_ctx_key, release_ctx_resource);
  pthread_setspecific(ksm_ctx_key, &ksm_main_ctx[0]);
#endif
}

#define OBJREG      (ksm_get_ctx()->registry->objreg)
#define OBJREG_NEXT (ksm_get_ctx()->registry->objreg_next)
#define OBJREG_LAST (ksm_get_ctx()->registry->objreg_last)

ksm_t *ksm_registry_top(void)
{
  return OBJREG_NEXT;
}

static
void registry_add(ksm_t obj)
{
  if( OBJREG_NEXT >= OBJREG_LAST )
    ksm_fatal("too many objects");
  *OBJREG_NEXT++ = obj;
}

void ksm_registry_add(ksm_t obj)
{
  ksm_lock();
  registry_add(obj);
  ksm_unlock();
}

void ksm_registry_add_no_lock(ksm_t obj)
{
  registry_add(obj);
}

void ksm_registry_restore(ksm_t *p)
{
  ksm_lock();
  OBJREG_NEXT = p;
  ksm_unlock();
}

ksm_t ksm_registry_restore_with(ksm_t *p, ksm_t obj)
{
  ksm_lock();
  OBJREG_NEXT = p;
  registry_add(obj);
  ksm_unlock();
  return obj;
}

void ksm_registry_restore_with2(ksm_t *p, ksm_t x, ksm_t y)
{
  ksm_lock();
  OBJREG_NEXT = p;
  registry_add(x);
  registry_add(y);
  ksm_unlock();
}

unsigned ksm_registry_size(void)
{
  return OBJREG_NEXT - OBJREG;
}

void ksm_registry_save_image(ksm_t *buf)
     // size of buf should be at least ksm_registry_size()
{
  unsigned n;

  n = sizeof(ksm_t)*ksm_registry_size();
  memcpy(buf, OBJREG, n);
}

void ksm_registry_restore_image(ksm_t *buf, unsigned size)
{
  unsigned n;

  if( size > MAXREG )
    ksm_fatal("can't happen (ksm_registry_restore_image): %u", size);
  n = sizeof(ksm_t)*size;
  memcpy(OBJREG, buf, n);
  OBJREG_NEXT = OBJREG + size;
}

static
ksm_t make_object(void)
{
  ksm_t x;

  x = new_object();
  registry_add(x);
  return x;
}

/*** interface ******************************************************/

ksm_t ksm_itype(int flag, int a, int b, int c)
{
  ksm_t p;

  ksm_lock();
  p = make_object();
  p->u_int.flag = flag;
  p->u_int.ivals[0] = a;
  p->u_int.ivals[1] = b;
  p->u_int.ivals[2] = c;
  ksm_unlock();
  return p;
}

ksm_t ksm_itype_no_lock(int flag, int a, int b, int c)
{
  ksm_t p;

  p = make_object();
  p->u_int.flag = flag;
  p->u_int.ivals[0] = a;
  p->u_int.ivals[1] = b;
  p->u_int.ivals[2] = c;
  return p;
}

ksm_t ksm_dtype(int flag, double d)
{
  ksm_t p;

  ksm_lock();
  p = make_object();
  p->u_double.flag = flag;
  p->u_double.dval = d;
  p->u_double.aux = 0;
  ksm_unlock();
  return p;
}

int ksm_flag(ksm_t p)
{ 
  int flag;

  ksm_lock();
  flag = p->u_int.flag; 
  ksm_unlock();
  return flag;
}

int ksm_flag_no_lock(ksm_t p)
{ 
  return p->u_int.flag; 
}

int ksm_ival(ksm_t p, int i)
{ 
  int ival;
  
  ksm_lock();
  ival = p->u_int.ivals[i]; 
  ksm_unlock();
  return ival;
}

int ksm_ival_no_lock(ksm_t p, int i)
{ 
  return p->u_int.ivals[i]; 
}

int ksm_ival_reg(ksm_t p, int i)
{
  int ival;
  
  ksm_lock();
  ival = p->u_int.ivals[i];
  registry_add((ksm_t)ival);
  ksm_unlock();
  return ival;
}

void ksm_set_ival(ksm_t p, int i, int val)
{ 
  ksm_lock();
  p->u_int.ivals[i]=val; 
  ksm_unlock();
}

void ksm_set_ival_no_lock(ksm_t p, int i, int val)
{ 
  p->u_int.ivals[i]=val; 
}

double ksm_dval(ksm_t p)
{ 
  double d;

  ksm_lock();
  d = p->u_double.dval; 
  ksm_unlock();
  return d;
}

void ksm_set_address_of_ival2_in_ival1(ksm_t x)
{
  ksm_lock();
  x->u_int.ivals[1] = (int)&x->u_int.ivals[2];
  ksm_unlock();
}

/*** gc *************************************************************/

#define GC_PROLOG_MAX 20
typedef void (*gc_prolog_t)(void);
static gc_prolog_t gc_prologs[GC_PROLOG_MAX];  /*** GLOBAL ***/
static int n_gc_prolog;                        /*** GLOBAL ***/

void ksm_add_gc_prolog(void (*fun)(void))
{
  if( n_gc_prolog >= GC_PROLOG_MAX )
    ksm_fatal("too many GC prologs");
  gc_prologs[n_gc_prolog++] = fun;
}

static void do_gc_prolog(void)
{
  int i;

  for(i=0;i<n_gc_prolog;++i)
    (*gc_prologs[i])();
}

static
void mark_registry(struct ksm_registry *r)
{
  ksm_t *p;

  for(p=r->objreg;p<r->objreg_next;++p){
    ksm_mark_object(*p);
  }
}

#define MARK_CTX(field) \
if( (x = ctx->field) ) ksm_mark_object(ctx->field)

static void mark_ctx(ksm_ctx *ctx)
{
  ksm_t x;  // used by MARK_CTX

  if( ctx->state == KSM_THREAD_EMPTY )
    return;
  if( ctx->registry )
    mark_registry(ctx->registry);
  MARK_CTX(fun);
  MARK_CTX(result);
  MARK_CTX(input);
  MARK_CTX(output);
  MARK_CTX(jumps);
  MARK_CTX(private_global);
  MARK_CTX(specific);
}

static
void mark_ctxtab(void)
{
  int i;

  for(i=0;i<ctx_count;++i)
    mark_ctx(&ksm_main_ctx[i]);
}

static
void gc(void)
{
  do_gc_prolog();
  mark_ctxtab();
  sweep();
}

void ksm_gc(void)
{
  ksm_lock();
  gc();
  ksm_unlock();
}

/*** alloc/dispose **************************************************/

void *ksm_alloc(unsigned size)
{
  void *p;

  p = malloc(size);
  if( p == 0 ){
    ksm_gc();
    p = malloc(size);
  }
  if( p == 0 )
    ksm_fatal("out of memory");
  return p;
}

void *ksm_calloc(size_t nmemb, size_t size)
{
  void *p;

  p = calloc(nmemb, size);
  if( p == 0 ){
    ksm_gc();
    p = calloc(nmemb, size);
  }
  if( p == 0 )
    ksm_fatal("out of memory");
  return p;
}

void *ksm_realloc(void *ptr, size_t size)
{
  void *p;

  p = realloc(ptr, size);
  if( p == 0 ){
    ksm_gc();
    p = realloc(ptr, size);
  }
  if( p == 0 )
    ksm_fatal("out of memory");
  return p;
}

void ksm_dispose(void *p)
{
  if( p )
    free(p);
}

