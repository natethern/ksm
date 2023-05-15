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

#include "ksm.h"

#define MAKE_CONS(car,cdr) ksm_itype(KSM_FLAG_CONS,(int)(car),(int)(cdr),0)
#define CAR(cons)          ((ksm_t)ksm_ival(cons,0))
#define CDR(cons)          ((ksm_t)ksm_ival(cons,1))
#define IS_CONS(x)         (ksm_flag(x)==KSM_FLAG_CONS)
#define SET_CAR(cons,car)  ksm_set_ival(cons,0,(int)(car))
#define SET_CDR(cons,cdr)  ksm_set_ival(cons,1,(int)(cdr))

#define CAR_NO_LOCK(cons)          ((ksm_t)ksm_ival_no_lock(cons,0))
#define CDR_NO_LOCK(cons)          ((ksm_t)ksm_ival_no_lock(cons,1))

static
ksm_t mark_cons(ksm_t cons)
{
  ksm_mark_object(CAR_NO_LOCK(cons));
  return CDR_NO_LOCK(cons);
}

ksm_t ksm_cons(ksm_t car, ksm_t cdr)
{
  return MAKE_CONS(car, cdr);
}

ksm_t ksm_car(ksm_t cons)
{ 
  return CAR(cons); 
}

ksm_t ksm_cdr(ksm_t cons)
{ 
  return CDR(cons); 
}

int ksm_is_cons(ksm_t x)
{ 
  return IS_CONS(x); 
}

void ksm_set_car(ksm_t cons, ksm_t car)
{ 
  SET_CAR(cons, car); 
}

void ksm_set_cdr(ksm_t cons, ksm_t cdr)
{ 
  SET_CDR(cons, cdr); 
}

void ksm_list_begin(ksm_t *head, ksm_t *tail)
{
  *head = *tail = ksm_nil_object;
}

void ksm_list_extend(ksm_t *head, ksm_t *tail, ksm_t item)
{
  if( ksm_is_nil(*tail) )
    *head = *tail = MAKE_CONS(item, ksm_nil_object);
  else{
    SET_CDR(*tail, MAKE_CONS(item, ksm_nil_object));
    *tail = CDR(*tail);
  }
}

void ksm_list_append(ksm_t *head, ksm_t *tail, ksm_t list)
{
  ksm_t save;

  save = list;
  while( ksm_is_cons(list) ){
    ksm_list_extend(head, tail, CAR(list));
    list = CDR(list);
  }
  if( !ksm_is_nil(list) )
    ksm_error("proper list expected: %k", save);
}

ksm_t ksm_append(ksm_t list1, ksm_t list2)
{
  ksm_t head, tail;

  head = tail = ksm_nil_object;
  ksm_list_append(&head, &tail, list1);
  if( ksm_is_nil(tail) )
    return list2;
  else{
    ksm_set_cdr(tail, list2);
    return head;
  }
}

int ksm_length(ksm_t x)
{
  int len = 0;

  while( IS_CONS(x) ){
    ++len;
    x = CDR(x);
  }
  if( ksm_is_nil(x) )
    return len;
  else
    return -1;
}

ksm_t ksm_map_x(ksm_t (*f)(), ksm_t list, void *aux1, void *aux2)
{
  ksm_t head, tail, x, save = list;

  ksm_list_begin(&head, &tail);
  while( ksm_is_cons(list) ){
    x = (*f)(ksm_car(list), aux1, aux2);
    ksm_list_extend(&head, &tail, x);
    list = ksm_cdr(list);
  }
  if( !ksm_is_nil(list) )
    ksm_error("proper list expected: %k", save);
  return head;
}

void ksm_foreach_x(void (*f)(), ksm_t list, void *aux1, void *aux2, 
		   void *aux3)
{
  ksm_t save = list;

  while( ksm_is_cons(list) ){
    (*f)(ksm_car(list), aux1, aux2, aux3);
    list = ksm_cdr(list);
  }
  if( !ksm_is_nil(list) )
    ksm_error("proper list expected: %k", save);
}

ksm_t ksm_safe_car(ksm_t x)
{
  if( !ksm_is_cons(x) )
    ksm_error("cons expected: %k", x);
  return ksm_car(x);
}

ksm_t ksm_safe_cdr(ksm_t x)
{
  if( !ksm_is_cons(x) )
    ksm_error("cons expected: %k", x);
  return ksm_cdr(x);
}

ksm_t ksm_last_cdr(ksm_t list)
{
  ksm_t x;

  while( ksm_is_cons(list) ){
    x = ksm_cdr(list);
    if( ksm_is_nil(x) )
      return list;
    list = x;
  }
  ksm_error("proper list expected: %k", list);
  return ksm_unspec_object;
}

ksm_t ksm_list_ref(ksm_t list, int i)
{
  while( i-- > 0 )
    list = ksm_safe_cdr(list);
  return ksm_safe_car(list);
}

ksm_t ksm_list1(ksm_t a)
{
  return ksm_cons(a, ksm_nil_object);
}

ksm_t ksm_list2(ksm_t a, ksm_t b)
{
  return ksm_cons(a, ksm_cons(b, ksm_nil_object));
}

ksm_t ksm_list3(ksm_t a, ksm_t b, ksm_t c)
{
  return ksm_cons(a, ksm_cons(b, ksm_cons(c, ksm_nil_object)));
}

ksm_t ksm_list4(ksm_t a, ksm_t b, ksm_t c, ksm_t d)
{
  return ksm_cons(a, ksm_cons(b, ksm_cons(c, ksm_cons(d, ksm_nil_object))));
}

void ksm_extend_list(ksm_t cons, ksm_t ele)
{
  cons = ksm_last_cdr(cons);
  ksm_set_cdr(cons, ksm_cons(ele, ksm_nil_object));
}

/*** read *********************************************************/

static ksm_t key_period;  /*** GLOBAL ***/

static
ksm_t read_cons(ksm_t port)
{
  ksm_t head, tail, obj;

  ksm_list_begin(&head, &tail);
  while(1){
    obj = ksm_read_object(port);
    if( ksm_is_eof(obj) )
      ksm_error("unexpected EOF");
    if( ksm_is_rpar(obj) )
      break;
    if( obj == key_period ){
      if( ksm_is_nil(tail) )
	ksm_error("can't read cons");
      obj = ksm_read_object(port);
      if( ksm_is_eof(obj) )
	ksm_error("unexpected EOF");
      ksm_set_cdr(tail, obj);
      if( !ksm_is_rpar(ksm_read_object(port)) )
	ksm_error("can't read cons");
      break;
    }
    ksm_list_extend(&head, &tail, obj);
  }
  return head;
}

/*** eval ***********************************************************/

typedef ksm_pair (*cons_evaluator_t)(ksm_t head, ksm_t args, ksm_t env);

typedef struct {
  cons_evaluator_t fun;
  int eval_args;
} fun_info;

KSM_INSTALLER_X(fun_info, fun_info)   /*** GLOBAL ***/

void ksm_install_fun_info(int flag, 
  ksm_pair (*fun)(ksm_t head, ksm_t args, ksm_t env), int eval_args)
{
  fun_info info;

  info.fun = fun;
  info.eval_args = eval_args;
  install_fun_info(flag, &info);
}

#define GET_FUN_INFO(obj)  get_fun_info(ksm_flag(obj))
#define GET_EVALUATOR(obj) (GET_FUN_INFO(obj)->fun)
#define GET_EVAL_ARGS(obj) (GET_FUN_INFO(obj)->eval_args)

#define KSM_TRACE_ON   (ksm_get_ctx()->trace_on)

static
ksm_pair eval_cons(ksm_t cons, ksm_t env)
{
  cons_evaluator_t f;
  ksm_t head, args;

  if( ksm_length(cons) < 0 )
    ksm_error("can't handle cons expression: %k", cons);
  head = ksm_eval(CAR(cons), env);
  if( head == ksm_except )
    return ksm_make_eval_done(ksm_except);
  f = GET_EVALUATOR(head);
  if( !f )
    ksm_error("invalid expression: %k", cons);
  args = CDR(cons);
  if( GET_EVAL_ARGS(head) ){
    args = ksm_eval_args(args, env);
    if( args == ksm_except )
      return ksm_make_eval_done(ksm_except);
  }
  if( KSM_TRACE_ON ){
    char buf[80];
    snprintf(buf, 78, "%k : %k", ksm_car(cons), args);
    fprintf(stderr, "%s\n", buf);
  }
  return (*f)(head, args, env);
}

ksm_t ksm_funcall(ksm_t fun, ksm_t args, ksm_t env)
{
  cons_evaluator_t f;

  f = GET_EVALUATOR(fun);
  if( !(f && GET_EVAL_ARGS(fun)) )
    ksm_error("invalid function: %k", fun);
  return ksm_eval_pair((*f)(fun, args, env));
}

ksm_t ksm_call_setter(ksm_t fun, ksm_t args, ksm_t val, ksm_t env)
{
  fun_info *info;

  info = GET_FUN_INFO(fun);
  if( !info->fun )
    ksm_error("can't find setter: %k", fun);
  if( info->eval_args ){
    args = ksm_eval_args(args, env);
    if( args == ksm_except )
      return ksm_except;
  }
  return ksm_eval_pair((*info->fun)(fun, ksm_list2(args, val), env));
}

int ksm_procedure_p(ksm_t x)
{
  return GET_EVALUATOR(x) && GET_EVAL_ARGS(x);
}

/*** print ********************************************************/

static
int fprintf_cons(FILE *fp, ksm_t cons, int alt)
{
  int len;

  len = fprintf(fp, "(");
  while( 1 ){
    len += ksm_fprintf(fp, ksm_car(cons), alt);
    cons = ksm_cdr(cons);
    if( ksm_is_nil(cons) )
      break;
    else if( ksm_is_cons(cons) )
      len += fprintf(fp, " ");
    else{
      len += fprintf(fp, " . ");
      len += ksm_fprintf(fp, cons, alt);
      break;
    }
  }
  return len + fprintf(fp, ")");
}

/*** equal **********************************************************/

static
int equal_cons(ksm_t a, ksm_t b)
{
  if( !ksm_is_cons(b) )
    return 0;
  while( ksm_is_cons(a) && ksm_is_cons(b) ){
    if( !ksm_equal(ksm_car(a), ksm_car(b)) )
      return 0;
    a = ksm_cdr(a);
    b = ksm_cdr(b);
  }
  return ksm_equal(a, b);
}

/*** init ***********************************************************/

void ksm_init_cons(void)
{
  ksm_install_marker(KSM_FLAG_CONS, mark_cons);
  key_period = ksm_make_key(".");
  ksm_list_reader = read_cons;
  ksm_install_evaluator(KSM_FLAG_CONS, eval_cons);
  ksm_install_fprintf(KSM_FLAG_CONS, fprintf_cons);
  ksm_install_equal(KSM_FLAG_CONS, equal_cons);
}
