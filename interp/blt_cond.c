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

KSM_DEF_RSPC(if)
     // (if test conseq [alt])
{
  ksm_t test, conseq, alt, x;

  KSM_POP_ARG(test);
  KSM_POP_ARG(conseq);
  KSM_POP_OPTARG(alt, ksm_unspec_object);
  KSM_SET_ARGS0();
  x = ksm_eval(test, KSM_ENV());
  if( x == ksm_except )
    return ksm_make_eval_done(ksm_except);
  if( ksm_is_false(x) )
    return ksm_make_eval_pair(alt, KSM_ENV());
  else
    return ksm_make_eval_pair(conseq, KSM_ENV());
}

static ksm_t key_arrow; /*** GLOBAL ***/
static ksm_t key_else;  /*** GLOBAL ***/

static
ksm_pair cond_arrow(ksm_t test, ksm_t seq, ksm_t env)
     // (test => fun)
     // seq: (=> fun)
{
  ksm_t fun, val;

  if( ksm_length(seq) != 2 )
    ksm_error("can't eval COND-FORM (cond_arrow): %k", seq);
  fun = ksm_eval(ksm_car(ksm_cdr(seq)), env);
  if( fun == ksm_except )
    return ksm_make_eval_done(ksm_except);
  val = ksm_funcall(fun, ksm_cons(test, ksm_nil_object), env);
  return ksm_make_eval_done(val);
}

KSM_DEF_RSPC(cond)
     // (cond (test seq ...) (test => fun) (else seq ...) ...)
{
  ksm_t s, t, p;

  while( KSM_MORE_ARGS() ){
    KSM_POP_ARG(s);
    if( !ksm_is_cons(s) )
      ksm_error("can't eval COND-FORM: %k", s);
    t = ksm_car(s);
    s = ksm_cdr(s);
    if( t == key_else )
      return ksm_eval_seq(s, KSM_ENV());
    p = ksm_eval(t, KSM_ENV());
    if( p == ksm_except )
      return ksm_make_eval_done(ksm_except);
    if( !ksm_is_false(p) ){
      if( ksm_is_cons(s) && ksm_car(s) == key_arrow )
	return cond_arrow(p, s, KSM_ENV());
      if( ksm_is_nil(s) )
	return ksm_make_eval_done(p);
      return ksm_eval_seq(s, KSM_ENV());
    }
  }
  return ksm_make_eval_done(ksm_unspec_object);
}

KSM_DEF_RSPC(case)
     // (case key clause ...)
{
  ksm_t clause, key, head;
  
  KSM_POP_ARG(key);
  key = ksm_eval(key, KSM_ENV());
  if( key == ksm_except )
    return ksm_make_eval_done(ksm_except);
  while( KSM_MORE_ARGS() ){
    KSM_POP_ARG(clause);
    if( !ksm_is_cons(clause) )
      ksm_error("invalid case clause: %k", clause);
    head = ksm_car(clause);
    if( head == key_else || ksm_memv(key, head) )
      return ksm_eval_seq(ksm_cdr(clause), KSM_ENV());
  }
  return ksm_make_eval_done(ksm_unspec_object);
}

KSM_DEF_RSPC(and)
{
  ksm_t x, y;

  if( KSM_NO_MORE_ARGS() )
    return ksm_make_eval_done(ksm_true_object);
  KSM_POP_ARG(x);
  while( KSM_MORE_ARGS() ){
    y = ksm_eval(x, KSM_ENV());
    if( y == ksm_except )
      return ksm_make_eval_done(ksm_except);
    if( ksm_is_false(y) )
      return ksm_make_eval_done(ksm_false_object);
    KSM_POP_ARG(x);
  }
  return ksm_make_eval_pair(x, KSM_ENV());
}

KSM_DEF_RSPC(or)
{
  ksm_t x, y;

  if( KSM_NO_MORE_ARGS() )
    return ksm_make_eval_done(ksm_false_object);
  KSM_POP_ARG(x);
  while( KSM_MORE_ARGS() ){
    y = ksm_eval(x, KSM_ENV());
    if( y == ksm_except )
      return ksm_make_eval_done(ksm_except);
    if( !ksm_is_false(y) )
      return ksm_make_eval_done(ksm_true_object);
    KSM_POP_ARG(x);
  }
  return ksm_make_eval_pair(x, KSM_ENV());
}

static
ksm_t let_extend(ksm_t vars, ksm_t env, ksm_t environ)
     // vars: ((var val) ...)
     // returns ksm_except if exception encountered
{
  ksm_t x, var, val;

  while( ksm_is_cons(vars) ){
    x = ksm_car(vars);
    vars = ksm_cdr(vars);
    if( ksm_length(x) != 2 )
      ksm_error("invalid let variable definition: %k", vars);
    var = ksm_cvt_key(ksm_first(x));
    val = ksm_eval(ksm_second(x), env);
    if( val == ksm_except )
      return ksm_except;
    environ = ksm_prepend_local(var, val, environ);
  }
  if( !ksm_is_nil(vars) )
    ksm_error("invalid let variable definition: %k", vars);
  return environ;
}

KSM_DEF_RSPC(let)
     // (let ((var val) ...) expr ...)
{
  ksm_t vars, key, fun, environ;

  KSM_POP_ARG(vars);
  environ = KSM_ENV();
  if( ksm_is_key(vars) ){
    key = vars;
    KSM_POP_ARG(vars);
    environ = ksm_prepend_local(key, ksm_unspec_object, environ);
    fun = ksm_make_lambda(ksm_map(ksm_safe_car, vars), KSM_ARGS(), environ);
    ksm_set_local_value(environ, fun);
  }
  environ = let_extend(vars, env, environ);
  if( environ == ksm_except )
    return ksm_make_eval_done(ksm_except);
  return ksm_eval_body(KSM_ARGS(), environ);
}

static
ksm_t letstar_extend(ksm_t vars, ksm_t env)
     // vars: ((var expr) ...)
     // returns ksm_except if exception is encountered
{
  ksm_t var, val, x;

  while( ksm_is_cons(vars) ){
    x = ksm_car(vars);
    vars = ksm_cdr(vars);
    if( ksm_length(x) != 2 )
      ksm_error("invalid let* variable definition: %k", vars);
    var = ksm_cvt_key(ksm_first(x));
    val = ksm_eval(ksm_second(x), env);
    if( val == ksm_except )
      return ksm_except;
    env = ksm_prepend_local(var, val, env);
  }
  if( !ksm_is_nil(vars) )
    ksm_error("invalid let* variable definition: %k", vars);
  return env;
}

KSM_DEF_RSPC(letstar)
     // (let* ((var expr) ...) expr ...)
{
  ksm_t vars, environ;

  KSM_POP_ARG(vars);
  environ = letstar_extend(vars, KSM_ENV());
  if( environ == ksm_except )
    return ksm_make_eval_done(ksm_except);
  return ksm_eval_body(KSM_ARGS(), environ);
}

static
ksm_t letrec_extend1(ksm_t vars, ksm_t env)
     // vars: ((var expr) ...)
{
  ksm_t x, var;

  while( ksm_is_cons(vars) ){
    x = ksm_car(vars);
    vars = ksm_cdr(vars);
    if( ksm_length(x) != 2 )
      ksm_error("invalid letrec variable definition: %k", vars);
    var = ksm_cvt_key(ksm_car(x));
    env = ksm_prepend_local(var, ksm_unspec_object, env);
  }
  if( !ksm_is_nil(vars) )
    ksm_error("invalid letrec variable definition: %k", vars);
  return env;
}

static
ksm_t letrec_extend2(ksm_t vars, ksm_t env)
     // vars: ((var expr) ...)
{
  ksm_t x, var, val;

  while( ksm_is_cons(vars) ){
    x = ksm_car(vars);
    vars = ksm_cdr(vars);
    var = ksm_cvt_key(ksm_car(x));
    val = ksm_eval(ksm_second(x), env);
    if( val == ksm_except )
      return ksm_except;
    if( ksm_update_local(var, val, env, 0) != 0 )
      ksm_error("can't happen in letrec_extend2");
  }
  return env;
}

KSM_DEF_RSPC(letrec)
     // (letrec ((var expr) ...) expr ...)
{
  ksm_t vars, environ;

  KSM_POP_ARG(vars);
  environ = letrec_extend1(vars, KSM_ENV());
  environ = letrec_extend2(vars, environ);
  if( environ == ksm_except )
    return ksm_make_eval_done(ksm_except);
  return ksm_eval_body(KSM_ARGS(), environ);
}

static
int do_init(ksm_t vars, ksm_t env, ksm_t *extend, ksm_t *extend2)
     // vars: ((var init [update]) ...)
     // returns 0 if OK, 1 if exception
{
  int n;
  ksm_t var, val, x;

  while( ksm_is_cons(vars) ){
    x = ksm_car(vars);
    vars = ksm_cdr(vars);
    n = ksm_length(x);
    if( !(n >= 2 && n <= 3) )
      ksm_error("invalid do form: %k", x);
    var = ksm_cvt_key(ksm_first(x));
    val = ksm_eval(ksm_second(x), env);
    if( val == ksm_except )
      return 1;
    *extend = ksm_prepend_local(var, val, *extend);
    *extend2 = ksm_prepend_local(var, val, *extend2);
  }
  if( !ksm_is_nil(vars) )
    ksm_error("invalid do variable definition: %k", vars);
  return 0;
}

static
int do_update(ksm_t vars, ksm_t env, ksm_t update)
     // returns 0 if OK, 1 if exception
{
  ksm_t x, var, val;

  while( ksm_is_cons(vars) ){
    x = ksm_car(vars);
    vars = ksm_cdr(vars);
    var = ksm_cvt_key(ksm_first(x));
    if( ksm_length(x) == 3 ){
      val = ksm_eval(ksm_third(x), env);
      if( val == ksm_except )
	return 1;
    }
    else{
      if( (val = ksm_lookup_local(ksm_car(x), env, 0)) == 0 )
	ksm_error("can't happen (do_update:1): %k", x);
    }
    if( ksm_update_local(var, val, update, 0) != 0 )
      ksm_error("can't happen (do_update:2)");
  }
  return 0;
}

KSM_DEF_RSPC(do)
     // (do ((var init [update]) ...) (pred seq ...) body ...)
{
  ksm_t vars, test, body, environ, environ2, tmp;
  ksm_t pred, seq, s, x;
  ksm_t *frame;
  
  KSM_POP_ARG(vars);
  KSM_POP_ARG(test);
  body = KSM_ARGS();
  environ = environ2 = KSM_ENV();
  if( ksm_length(test) < 1 )
    ksm_error("invalid do form: %k", test);
  pred = ksm_car(test);
  seq = ksm_cdr(test);
  frame = ksm_registry_top();
  if( do_init(vars, KSM_ENV(), &environ, &environ2) != 0 )
    return ksm_make_eval_done(ksm_except);
  ksm_registry_restore_with2(frame, environ, environ2);
  frame = ksm_registry_top();
  while( 1 ){
    x = ksm_eval(pred, environ);
    if( x == ksm_except )
      return ksm_make_eval_done(ksm_except);
    if( !ksm_is_false(x) )
      break;
    ksm_registry_restore(frame);
    s = body;
    while( ksm_is_cons(s) ){
      if( ksm_eval(ksm_car(s), environ) == ksm_except )
	return ksm_make_eval_done(ksm_except);
      ksm_registry_restore(frame);
      s = ksm_cdr(s);
    }
    if( do_update(vars, environ, environ2) != 0 )
      return ksm_make_eval_done(ksm_except);
    tmp = environ;
    environ = environ2;
    environ2 = tmp;
    ksm_registry_restore(frame);
  }
  return ksm_eval_seq(seq, environ);
}

KSM_DEF_RSPC(begin)
     // (begin stmt ...)
{
  return ksm_eval_seq(KSM_ARGS(), KSM_ENV());
}

void ksm_blt_cond(void)
{
  key_arrow = ksm_make_key("=>");
  key_else = ksm_make_key("else");

  KSM_ENTER_RSPC("if", if);
  KSM_ENTER_RSPC("cond", cond);
  KSM_ENTER_RSPC("case", case);
  KSM_ENTER_RSPC("and", and);
  KSM_ENTER_RSPC("or", or);
  KSM_ENTER_RSPC("let", let);
  KSM_ENTER_RSPC("let*", letstar);
  KSM_ENTER_RSPC("letrec", letrec);
  KSM_ENTER_RSPC("do", do);
  KSM_ENTER_RSPC("begin", begin);
}

