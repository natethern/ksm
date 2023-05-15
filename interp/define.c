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

static ksm_t key_define;   /*** GLOBAL ***/

int ksm_interp_define_args(ksm_t args, ksm_t env, ksm_t *key, ksm_t *val)
     // returns 0 if it is a define form, -1 if not
{
  if( ksm_is_cons(args) ){
    if( ksm_is_key(ksm_car(args)) ){ // args: (key val)
      if( ksm_length(args) != 2 )
	ksm_error("invalid define arg: %k", args);
      if( key ) 
	*key = ksm_car(args);
      if( val )
	*val = ksm_eval(ksm_car(ksm_cdr(args)), env);
      return 0;
    }
    else if( ksm_is_cons(ksm_car(args)) ){
      // args: ((fun x) ...)
      if( !(ksm_is_key(ksm_car(ksm_car(args))) 
	    && ksm_length(args) >= 0) )
	ksm_error("invalid define arg: %k", args);
      if( key )
	*key = ksm_car(ksm_car(args));
      if( val )
	*val = ksm_make_lambda(ksm_cdr(ksm_car(args)), ksm_cdr(args),
			       env);
      return 0;
    }
  }
  return -1;
}

int ksm_interp_define(ksm_t form, ksm_t env, ksm_t *key, ksm_t *val)
     // returns 0 if it is a define form, -1 if not
{
  if( ksm_is_cons(form) && ksm_car(form) == key_define )
    return ksm_interp_define_args(ksm_cdr(form), env, key, val);
  return -1;
}

typedef void (*update_fun)(ksm_t local, ksm_t val);
KSM_INSTALLER(updater,update_fun)       /*** GLOBAL ***/
#define GET_UPDATER(x)  get_updater(ksm_flag(x))

void ksm_update_binding(ksm_t local, ksm_t val)
{
  update_fun f;

  if( val == ksm_unspec_object ){
    ksm_set_local_value(local, val);
    return;
  }

  f = GET_UPDATER(ksm_local_value(local));
  if( f )
    (*f)(local, val);
  else
    ksm_set_local_value(local, val);
}

static ksm_t setter_env;    /*** GLOBAL ***/

void ksm_enter_setter(ksm_t key, ksm_t setter)
{
  ksm_enter_global(key, setter, setter_env, 1);
}

static
ksm_t set(ksm_t var, ksm_t val, ksm_t env)
     // returns val if OK, ksm_except if exception
{
  ksm_t h;

  if( ksm_is_key(var) ){
    if( ksm_update_local(var, val, env, &env) == 0 )
      return val;
    while( ksm_is_global(env) ){
      if( ksm_enter_global(var, val, env, 0) == 0 )
	return val;
      env = ksm_next_global(env);
    }
    ksm_error("can't find variable: %k", var);
  }
  else if( ksm_is_cons(var) ){
    h = ksm_car(var);
    KSM_CONFIRM_KEY(h);
    h = ksm_lookup_global(h, setter_env, 0);
    if( !h )
      ksm_error("can't find setter: %k", ksm_car(var));
    if( ksm_call_setter(h, ksm_cdr(var), val, env) == ksm_except )
      return ksm_except;
  }
  else
    ksm_error("can't find setter: %k", var);
  return val;
}

KSM_DEF_SPC(define)
{
  ksm_t key, val;

  if( !ksm_is_global(KSM_ENV()) )
    ksm_error("DEFINE-FORM out of toplevel: %k", KSM_ARGS());
  if( ksm_interp_define_args(KSM_ARGS(), KSM_ENV(), &key, &val) != 0 )
    ksm_error("invalid define arg: %k", KSM_ARGS());
  if( val == ksm_except )
    ksm_error("can't find exception handler");
  ksm_enter_global(key, val, ksm_interact_env, 1);
  return ksm_unspec_object;
}

KSM_DEF_SPC(set)
{
  ksm_t var, val;

  KSM_SET_ARGS2(var, val);
  val = ksm_eval(val, KSM_ENV());
  if( val == ksm_except )
    return ksm_except;
  if( set(var, val, KSM_ENV()) == ksm_except )
    return ksm_except;
  return ksm_unspec_object;
}

KSM_DEF_SPC(set_value)
{
  ksm_t var, val;

  KSM_SET_ARGS2(var, val);
  val = ksm_eval(val, KSM_ENV());
  if( val == ksm_except )
    return ksm_except;
  if( set(var, val, KSM_ENV()) == ksm_except )
    return ksm_except;
  return val;
}

void ksm_init_define(void)
{
  setter_env = ksm_make_global(32, ksm_nil_object);
  key_define = ksm_make_key("define");
  KSM_ENTER_SPC("define", define);
  KSM_ENTER_SPC("set!", set);
  KSM_ENTER_SPC("set-value!", set_value);
}
