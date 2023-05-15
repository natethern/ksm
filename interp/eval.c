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

typedef ksm_pair (*evaluator_t)(ksm_t expr, ksm_t env);
KSM_INSTALLER(evaluator,evaluator_t)    /*** GLOBAL ***/
#define GET_EVALFUN(obj)  get_evaluator(ksm_flag(obj))

#define EVAL_DONE(pair)  ksm_is_unspec(pair.env)

ksm_t ksm_eval(ksm_t expr, ksm_t env)
{
  ksm_t *frame;
  evaluator_t efun;
  ksm_pair pair;

  frame = ksm_registry_top();
 again:
  efun = GET_EVALFUN(expr);
  if( efun ){
    pair = (*efun)(expr, env);
    if( ksm_is_eval_done(pair) ){
      ksm_registry_restore_with(frame, pair.expr);
      return pair.expr;
    }
    else{
      ksm_registry_restore_with2(frame, pair.expr, pair.env);
      expr = pair.expr;
      env = pair.env;
      goto again;
    }
  }
  else
    return expr;
}

ksm_pair ksm_make_eval_pair(ksm_t expr, ksm_t env)
{
  ksm_pair pair;

  pair.expr = expr;
  pair.env  = env;
  return pair;
}

ksm_pair ksm_make_eval_done(ksm_t expr)
{
  ksm_pair pair;

  pair.expr = expr;
  pair.env  = ksm_unspec_object;
  return pair;
}

int ksm_is_eval_done(ksm_pair pair)
{
  return EVAL_DONE(pair);
}

ksm_t ksm_eval_args(ksm_t args, ksm_t env)
{
  ksm_t head, tail, val;

  ksm_list_begin(&head, &tail);
  while( ksm_is_cons(args) ){
    val = ksm_eval(ksm_car(args), env);
    if( val == ksm_except )
      return ksm_except;
    ksm_list_extend(&head, &tail, val);
    args = ksm_cdr(args);
  }
  if( !ksm_is_nil(args) )
    ksm_error("improper arglist: %k", args);
  return head;
}

ksm_pair ksm_eval_seq(ksm_t seq, ksm_t env)
{
  ksm_t *frame;

  if( ksm_is_nil(seq) )
    return ksm_make_eval_done(ksm_unspec_object);
  if( !ksm_is_cons(seq) )
    ksm_error("can't eval sequence: %k", seq);
  frame = ksm_registry_top();
  while( !ksm_is_nil(ksm_cdr(seq)) ){
    if( ksm_eval(ksm_car(seq), env) == ksm_except )
      return ksm_make_eval_done(ksm_except);
    seq = ksm_cdr(seq);
    if( !ksm_is_cons(seq) )
      ksm_error("can't eval sequence: %k", seq);
    ksm_registry_restore(frame);
  }
  return ksm_make_eval_pair(ksm_car(seq), env);
}

ksm_t ksm_eval_pair(ksm_pair pair)
{
  return EVAL_DONE(pair) ? pair.expr : ksm_eval(pair.expr, pair.env);
}
