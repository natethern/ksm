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

#define MAKE_LAMBDA(v,b,e) \
  ksm_itype(KSM_FLAG_LAMBDA,(int)(v),(int)(b),(int)(e))
#define LAMBDA_VARS(lambda)  ((ksm_t)ksm_ival(lambda,0))
#define LAMBDA_BODY(lambda)  ((ksm_t)ksm_ival(lambda,1))
#define LAMBDA_ENV(lambda)   ((ksm_t)ksm_ival(lambda,2))
#define IS_LAMBDA(x)         (ksm_flag(x)==KSM_FLAG_LAMBDA)

#define LAMBDA_VARS_NO_LOCK(lambda)  ((ksm_t)ksm_ival_no_lock(lambda,0))
#define LAMBDA_BODY_NO_LOCK(lambda)  ((ksm_t)ksm_ival_no_lock(lambda,1))
#define LAMBDA_ENV_NO_LOCK(lambda)   ((ksm_t)ksm_ival_no_lock(lambda,2))

static
ksm_t mark_lambda(ksm_t lambda)
{
  ksm_mark_object(LAMBDA_VARS_NO_LOCK(lambda));
  ksm_mark_object(LAMBDA_BODY_NO_LOCK(lambda));
  ksm_mark_object(LAMBDA_ENV_NO_LOCK(lambda));
  return 0;
}

ksm_t ksm_make_lambda(ksm_t vars, ksm_t body, ksm_t env)
{
  return MAKE_LAMBDA(vars, body, env);
}

KSM_DEF_SPC(lambda)
     // (lambda vars body)
{
  ksm_t vars;

  KSM_POP_ARG(vars);
  return MAKE_LAMBDA(vars, KSM_ARGS(), KSM_ENV());
}

/*** eval ***********************************************************/

static
ksm_t lambda_extend(ksm_t vars, ksm_t args, ksm_t local)
     // args should be a proper list returned from arglist()
{
  ksm_t v, args_save, vars_save;

  args_save = args;
  vars_save = vars;
  while( 1 ){   // usual parameters
    if( ksm_is_nil(vars) )
      break;
    else if( ksm_is_key(vars) ){
      local = ksm_prepend_local(vars, args, local);
      goto out;
    }
    else if( ksm_is_cons(vars) ){
      v = ksm_car(vars);
      KSM_CONFIRM_KEY(v);
      if( ksm_is_nil(args) )
	ksm_error("too few args: %k", args_save);
      local = ksm_prepend_local(v, ksm_car(args), local);
      vars = ksm_cdr(vars);
      args = ksm_cdr(args);
    }
    else
      ksm_error("invalid lambda vars: %k", vars_save);
  }
  if( !ksm_is_nil(args) )
    ksm_error("too many args: %k", args);
 out:
  return local;
}

ksm_t (*ksm_lambda_extender)(ksm_t vars, ksm_t args, ksm_t env)/*** GLOBAL ***/
  = lambda_extend;

ksm_pair ksm_eval_body(ksm_t seq, ksm_t env)
{
  ksm_t save = seq, key, val;

  while( ksm_is_cons(seq) ){
    if( ksm_interp_define(ksm_car(seq), env, &key, 0) != 0 )
      break;
    seq = ksm_cdr(seq);
    env = ksm_prepend_local(key, ksm_unspec_object, env);
  }
  seq = save;
  while( ksm_is_cons(seq) ){
    if( ksm_interp_define(ksm_car(seq), env, &key, &val) != 0 )
      break;
    if( val == ksm_except )
      return ksm_make_eval_done(ksm_except);
    seq = ksm_cdr(seq);
    if( ksm_update_local(key, val, env, 0) != 0 )
      ksm_error("can't happen in ksm_eval_body");
  }
  return ksm_eval_seq(seq, env);
}

static
ksm_pair eval_lambda(ksm_t lambda, ksm_t args, ksm_t env)
{
  env = (*ksm_lambda_extender)(LAMBDA_VARS(lambda), args, LAMBDA_ENV(lambda));
  return ksm_eval_body(LAMBDA_BODY(lambda), env);
}

/*** init ***********************************************************/

void ksm_init_lambda(void)
{
  ksm_install_marker(KSM_FLAG_LAMBDA, mark_lambda);
  ksm_install_fun_info(KSM_FLAG_LAMBDA, eval_lambda, 1);
  KSM_ENTER_SPC("lambda", lambda);
}
