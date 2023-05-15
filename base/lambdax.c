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
#include "ksmext.h"

static ksm_t key_optional;
static ksm_t key_rest;
static ksm_t key_key;

static
void gc_prolog(void)
{
  if( key_optional ) ksm_mark_object(key_optional);
  if( key_rest ) ksm_mark_object(key_rest);
  if( key_key ) ksm_mark_object(key_key);
}

static
void init_keys(void)
{
  key_optional = ksm_make_key("&optional");
  key_rest = ksm_make_key("&rest");
  key_key = ksm_make_key("&key");
}

static
ksm_t find_keyword(ksm_t key, ksm_t args)
     // returns 0 if not found
{
  ksm_t arg;

  while( ksm_is_cons(args) ){
    arg = ksm_car(args);
    args = ksm_cdr(args);
    if( !ksm_is_keyword(arg) )
      ksm_error("can't handle function arguments: %k", args);
    if( key == ksm_keyword_key(arg) ){
      if( ksm_is_cons(args) && !ksm_is_keyword(ksm_car(args)) )
	return ksm_car(args);
      else
	return ksm_true_object;
    }
    if( ksm_is_cons(args) && !ksm_is_keyword(ksm_car(args)) )
      args = ksm_cdr(args);
  }
  return 0;
}

static
ksm_t extract_args(ksm_t args, ksm_t *remaining)
{
  ksm_t head, tail, x;

  ksm_list_begin(&head, &tail);
  while( ksm_is_cons(args) ){
    x = ksm_car(args);
    if( ksm_is_keyword(x) )
      break;
    ksm_list_extend(&head, &tail, x);
    args = ksm_cdr(args);
  }
  *remaining = args;
  return head;
}

// extended lambda params
// (symbol ... [&optional opt-param ...] [&args symbol] [&rest symbol]
//   [&key param ...])
// opt-param: symbol or (symbol expr) or (symbol)
//   in the last case, remaining list of remaining args up to 
//   keyword arguments is bound to symbol
//   e.g., (define f (lambda (a &optional (args) &key x) args))
//         > (f 1 2 3 :x 100) ==> (1 2 3)
// param: symbol or (symbol expr)

static ksm_t lambda_extend_x(ksm_t vars, ksm_t args, ksm_t local)
     // returns a local-env
{
  ksm_t v, args_save, vars_save;
  ksm_t name, val, val2;

  args_save = args;
  vars_save = vars;
  while( 1 ){   // usual parameters
    if( ksm_is_nil(vars) ){
      if( !ksm_is_nil(args) )
	ksm_error("too many args: %k", args);
      goto out;
    }
    else if( ksm_is_key(vars) ){
      local = ksm_prepend_local(vars, args, local);
      goto out;
    }
    else if( ksm_is_cons(vars) ){
      v = ksm_car(vars);
      vars = ksm_cdr(vars);
      KSM_CONFIRM_KEY(v);
      if( v == key_optional )
	goto optional;
      if( v == key_rest )
	goto rest;
      if( v == key_key )
	goto key;
      if( ksm_is_nil(args) )
	ksm_error("too few args: %k", args_save);
      local = ksm_prepend_local(v, ksm_car(args), local);
      args = ksm_cdr(args);
    }
    else
      ksm_error("invalid lambda vars: %k", vars_save);
  }
 optional:
  while( 1 ){  // optional parameters
    if( ksm_is_nil(vars) ){
      if( !ksm_is_nil(args) )
	ksm_error("too many args: %k", args);
      goto out;
    }
    else if( ksm_is_cons(vars) ){
      v = ksm_car(vars);
      vars = ksm_cdr(vars);
      if( v == key_rest )
	goto rest;
      if( v == key_key )
	goto key;
      if( ksm_is_key(v) ){
	name = v;
	val = ksm_nil_object;
      }
      else if( ksm_length(v) == 2 ){
	name = ksm_car(v);
	val = ksm_car(ksm_cdr(v));
      }
      else if( ksm_length(v) == 1 ){
	name = ksm_car(v);
	val = extract_args(args, &args);
	local = ksm_prepend_local(name, val, local);
	continue;
      }
      else
	ksm_error("can't handle optional parameter: %k", v);
      if( ksm_is_cons(args) ){
	val = ksm_car(args);
	args = ksm_cdr(args);
      }
      else
	val = ksm_eval(val, local);
      local = ksm_prepend_local(name, val, local);
    }
    else
      ksm_error("can't handle lambda variables: %k", vars);
  }
 rest:
  if( !(ksm_length(vars) >= 1) )
    ksm_error("can't handle lambda variables: %k", vars);
  v = ksm_car(vars);
  vars = ksm_cdr(vars);
  KSM_CONFIRM_KEY(v);
  local = ksm_prepend_local(v, args, local);
  if( ksm_is_nil(vars) )
    goto out;
  if( ksm_is_cons(vars) && ksm_car(vars) == key_key ){
    vars = ksm_cdr(vars);
    goto key;
  }
  else
    ksm_error("can't handle lambda variables: %k", vars);
 key:
  while( 1 ){
    if( ksm_is_nil(vars) )
      goto out;
    else if( ksm_is_cons(vars) ){
      v = ksm_car(vars);
      vars = ksm_cdr(vars);
      if( ksm_is_key(v) ){
	name = v;
	val = ksm_nil_object;
      }
      else if( ksm_length(v) == 2 ){
	name = ksm_car(v);
	val = ksm_car(ksm_cdr(v));
      }
      else
	ksm_error("can't handle lambda keyword: %k", v);
      if( (val2 = find_keyword(name, args)) )
	val = val2;
      else
	val = ksm_eval(val, local);
      local = ksm_prepend_local(name, val, local);
    }
    else
      ksm_error("can't handle lambda variables: %k", vars);
  }
 out:
  return local;
}

void ksm_init_lambdax(void)
{
  ksm_add_gc_prolog(gc_prolog);
  init_keys();
  ksm_lambda_extender = lambda_extend_x;
}
