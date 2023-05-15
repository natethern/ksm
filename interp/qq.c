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

static ksm_t key_qq;   /*** GLOBAL ***/
static ksm_t key_uq;   /*** GLOBAL ***/
static ksm_t key_uqs;  /*** GLOBAL ***/

static
int is_qq_form(ksm_t form)
{
  if( ksm_is_cons(form) && ksm_car(form) == key_qq ){
    if( ksm_length(form) != 2 )
      ksm_error("wrong number of args: %k", form);
    return 1;
  }
  else
    return 0;
}

static
int is_uq_form(ksm_t form)
{
  if( ksm_is_cons(form) && ksm_car(form) == key_uq ){
    if( ksm_length(form) != 2 )
      ksm_error("wrong number of args: %k", form);
    return 1;
  }
  else
    return 0;
}

static
int is_uqs_form(ksm_t form)
{
  if( ksm_is_cons(form) && ksm_car(form) == key_uqs ){
    if( ksm_length(form) != 2 )
      ksm_error("wrong number of args: %k", form);
    return 1;
  }
  else
    return 0;
}

#define SECOND(x)  ksm_car(ksm_cdr(x))

static
ksm_t qq(ksm_t expr, ksm_t env, int level)
{
  ksm_t head, tail, x, y;

  if( is_qq_form(expr) )
    return ksm_list2(key_qq, qq(SECOND(expr), env, level+1));
  if( is_uq_form(expr) || is_uqs_form(expr) ){
    if( level == 0 )
      return ksm_eval(SECOND(expr), env);
    return ksm_list2(ksm_car(expr), qq(SECOND(expr), env, level-1));
  }
  if( ksm_is_cons(expr) ){
    ksm_list_begin(&head, &tail);
    while( ksm_is_cons(expr) ){
      x = ksm_car(expr);
      expr = ksm_cdr(expr);
      y = qq(x, env, level);
      if( is_uqs_form(x) && level == 0 ){
	if( ksm_length(y) < 0 )
	  ksm_error("incompatible arg to QUASIQUOTE-SPLICING: %k", y);
	ksm_list_append(&head, &tail, y);
      }
      else
	ksm_list_extend(&head, &tail, y);
    }
    if( !ksm_is_nil(expr) ){
      if( !ksm_is_cons(tail) )
	ksm_error("can't happen (qq_rewrite): %k", tail);
      if( is_uqs_form(expr) )
	ksm_error("incompatible last arg to QUASIQUOTE: %k", expr);
      ksm_set_cdr(tail, qq(expr, env, level));
    }
    return head;
  }
  else
    return expr;
}

KSM_DEF_SPC(qq)
{
  ksm_t obj;
  int vec;

  KSM_SET_ARGS1(obj);
  vec = ksm_is_vector(obj) ? 1 : 0;
  if( vec )
    obj = ksm_vector_to_list(obj);
  obj = qq(obj, env, 0);
  return vec ? ksm_list_to_vector(obj) : obj;
}

static
ksm_t backquote_reader(ksm_t port)
{
  return ksm_list2(key_qq, ksm_read_object(port));
}

static
ksm_t comma_reader(ksm_t port)
{
  int ch;

  while( ksm_is_ws(ch = ksm_port_getc(port)) )
    ;
  if( ch == EOF )
    ksm_error("unexpected EOF following \",\"");
  if( ch == '@' )
    return ksm_list2(key_uqs, ksm_read_object(port));
  else{
    ksm_port_ungetc(ch, port);
    return ksm_list2(key_uq, ksm_read_object(port));
  }
}

void ksm_init_qq(void)
{
  ksm_backquote_reader = backquote_reader;
  ksm_comma_reader = comma_reader;
  key_qq = ksm_make_key("quasiquote");
  key_uq = ksm_make_key("unquote");
  key_uqs = ksm_make_key("unquote-splicing");
  KSM_ENTER_SPC("quasiquote", qq);
}
