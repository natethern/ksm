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
#include "ksm/clang.h"
#define YYSTYPE ksm_t
#include "parse.tab.h"

extern int ksm_clike_parse(ksm_t *store);
extern int ksm_clike_char;
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern int ksm_clike_yyempty;
extern int ksm_clike_yyeof;
extern char *ksm_clike_text;
extern char ksm_clike_hold_char(void);
extern YY_BUFFER_STATE ksm_clike_current_buffer(void);
extern void ksm_clike__flush_buffer(YY_BUFFER_STATE);

#ifdef PTHREAD
static pthread_mutex_t clike_mutex = PTHREAD_MUTEX_INITIALIZER;
static void lock_clike(void){ pthread_mutex_lock(&clike_mutex); }
static void unlock_clike(void){ pthread_mutex_unlock(&clike_mutex); }
#else
#define lock_clike() 
#define unlock_clike()
#endif

ksm_t ksm_clike_port;
char *ksm_clike_error_string;

static
ksm_t read_clike(ksm_t port)
{
  ksm_t expr;
  int err;
  char hold_char;

  lock_clike();
  ksm_clike_port = port;
  err = ksm_clike_parse(&expr);
  if( err ){
    if( ksm_is_bufport(port) && ksm_bufport_has_filename(port) )
      ksm_error("%k, Line %d: %s", ksm_bufport_filename(port),
		ksm_bufport_line_number(port), ksm_clike_error_string);
    else
      ksm_error("%s", ksm_clike_error_string);
  }
  if( (hold_char = ksm_clike_hold_char()) )
    ksm_port_ungetc(hold_char, port);
  if( ksm_clike_char != ksm_clike_yyeof &&
      ksm_clike_char != ksm_clike_yyempty ){
    if( !ksm_is_bufport(port) )
      ksm_error("can't happen in read_clike");
    ksm_bufport_ungets(port, ksm_clike_text);
  }
  ksm_clike__flush_buffer(ksm_clike_current_buffer());
  unlock_clike();
  return expr;
}

static
ksm_t read_toplevel(ksm_t port)
{
  int ch;

  if( ksm_port_eof_p(port) )
    return ksm_eof_object;
  while( ksm_is_ws(ch=ksm_port_getc(port)) )
    ;
  if( ch == EOF )
    return ksm_eof_object;
  ksm_port_ungetc(ch, port);
  if( ch == '(' )
    return ksm_read_object(port);
  else if( ch == ';' ){
    while( (ch=ksm_port_getc(port)) != EOF )
      if( ch == '\n' )
	return ksm_comment_object;
    return ksm_eof_object;
  }
  else
    return read_clike(port);
}

static
ksm_t add_si(ksm_t x, int i)
{
  ksm_numrep p, q;

  if( ksm_set_numrep(&p, x) != 0 )
    ksm_error("number expected: %k", x);
  ksm_numrep_clone(&p);
  ksm_numrep_set_int(&q, i);
  ksm_numrep_add(&p, &q);
  return ksm_numrep_to_object(&p);
}

static
ksm_t mul(ksm_t x, ksm_t y)
{
  ksm_numrep p, q;

  if( ksm_set_numrep(&p, x) != 0 )
    ksm_error("number expected: %k", x);
  if( ksm_set_numrep(&q, y) != 0 )
    ksm_error("number expected: %k", y);
  ksm_numrep_clone(&p);
  ksm_numrep_mul(&p, &q);
  return ksm_numrep_to_object(&p);
}

static
ksm_t my_div(ksm_t x, ksm_t y)
{
  ksm_numrep p, q;

  if( ksm_set_numrep(&p, x) != 0 )
    ksm_error("number expected: %k", x);
  if( ksm_set_numrep(&q, y) != 0 )
    ksm_error("number expected: %k", y);
  ksm_numrep_clone(&p);
  ksm_numrep_div(&p, &q);
  return ksm_numrep_to_object(&p);
}

static
ksm_t modulo(ksm_t x, ksm_t y)
{
  ksm_numrep p, q;

  if( ksm_set_numrep(&p, x) != 0 )
    ksm_error("number expected: %k", x);
  if( ksm_set_numrep(&q, y) != 0 )
    ksm_error("number expected: %k", y);
  ksm_numrep_clone(&p);
  ksm_numrep_modulo(&p, &q);
  return ksm_numrep_to_object(&p);
}

static
ksm_t add(ksm_t x, ksm_t y)
{
  ksm_numrep p, q;

  if( ksm_set_numrep(&p, x) != 0 )
    ksm_error("number expected: %k", x);
  if( ksm_set_numrep(&q, y) != 0 )
    ksm_error("number expected: %k", y);
  ksm_numrep_clone(&p);
  ksm_numrep_add(&p, &q);
  return ksm_numrep_to_object(&p);
}

static
ksm_t sub(ksm_t x, ksm_t y)
{
  ksm_numrep p, q;

  if( ksm_set_numrep(&p, x) != 0 )
    ksm_error("number expected: %k", x);
  if( ksm_set_numrep(&q, y) != 0 )
    ksm_error("number expected: %k", y);
  ksm_numrep_clone(&p);
  ksm_numrep_sub(&p, &q);
  return ksm_numrep_to_object(&p);
}

static
ksm_t lshift(ksm_t x, ksm_t y)
{
  ksm_numrep p;
  int n;

  if( ksm_set_numrep(&p, x) != 0 )
    ksm_error("number expected: %k", x);
  n = ksm_get_si(y);
  ksm_numrep_clone(&p);
  if( n > 0 )
    ksm_numrep_lshift(&p, n);
  else if( n < 0 )
    ksm_numrep_rshift(&p, -n);
  return ksm_numrep_to_object(&p);
}

static
ksm_t rshift(ksm_t x, ksm_t y)
{
  ksm_numrep p;
  int n;

  if( ksm_set_numrep(&p, x) != 0 )
    ksm_error("number expected: %k", x);
  n = ksm_get_si(y);
  ksm_numrep_clone(&p);
  if( n > 0 )
    ksm_numrep_rshift(&p, n);
  else if( n < 0 )
    ksm_numrep_lshift(&p, -n);
  return ksm_numrep_to_object(&p);
}

static
ksm_t and(ksm_t x, ksm_t y)
{
  int i, j;

  i = ksm_get_si(x);
  j = ksm_get_si(y);
  return ksm_make_int(i & j);
}

static
ksm_t xor(ksm_t x, ksm_t y)
{
  int i, j;

  i = ksm_get_si(x);
  j = ksm_get_si(y);
  return ksm_make_int(i ^ j);
}

static
ksm_t or(ksm_t x, ksm_t y)
{
  int i, j;

  i = ksm_get_si(x);
  j = ksm_get_si(y);
  return ksm_make_int(i | j);
}

#define CHECK_EXCEPT(x)   if( (x) == ksm_except ) return ksm_except

KSM_DEF_SPC(postfix_inc)
     // (postfix-inc expr)
{
  ksm_t expr, val, new_val;

  KSM_POP_ARG(expr);
  KSM_SET_ARGS0();
  if( ksm_is_key(expr) ){
    val = ksm_lookup(expr, KSM_ENV());
    if( !val )
      ksm_error("can't find variable: %k", expr);
    new_val = add_si(val, 1);
    ksm_registry_add(val);
    if( ksm_update(expr, new_val, KSM_ENV()) != 0 )
      ksm_error("can't happen in blt_postfix_inc");
    return val;
  }
  else if( ksm_length(expr) == 3 && 
	   ksm_car(expr) == ksm_make_key("vector-ref") ){
    ksm_t vec, ind;
    int i;

    vec = ksm_eval(ksm_second(expr), KSM_ENV());
    CHECK_EXCEPT(vec);
    ind = ksm_eval(ksm_third(expr), KSM_ENV());
    CHECK_EXCEPT(ind);
    if( !ksm_is_vector(vec) )
      ksm_error("invalid lvalue in increment: %k", expr);
    i = ksm_get_si(ind);
    if( !(i >= 0 && i < ksm_vector_size(vec)) )
      ksm_error("vector index out of range: %d", i);
    val = ksm_vector_ref(vec, i);
    new_val = add_si(val, 1);
    ksm_registry_add(val);
    ksm_vector_set(vec, i, new_val);
    return val;
  }
  else{
    ksm_error("invalid lvalue in increment: %k", expr);
    return ksm_unspec_object;
  }
}

KSM_DEF_SPC(postfix_dec)
     // (postfix-dec expr)
{
  ksm_t expr, val, new_val;

  KSM_POP_ARG(expr);
  KSM_SET_ARGS0();
  if( ksm_is_key(expr) ){
    val = ksm_lookup(expr, KSM_ENV());
    if( !val )
      ksm_error("can't find variable: %k", expr);
    new_val = add_si(val, -1);
    ksm_registry_add(val);
    if( ksm_update(expr, new_val, KSM_ENV()) != 0 )
      ksm_error("can't happen in blt_postfix_dec");
    return val;
  }
  else if( ksm_length(expr) == 3 && 
	   ksm_car(expr) == ksm_make_key("vector-ref") ){
    ksm_t vec, ind;
    int i;

    vec = ksm_eval(ksm_second(expr), KSM_ENV());
    CHECK_EXCEPT(vec);
    ind = ksm_eval(ksm_third(expr), KSM_ENV());
    CHECK_EXCEPT(ind);
    if( !ksm_is_vector(vec) )
      ksm_error("invalid lvalue in decrement: %k", expr);
    i = ksm_get_si(ind);
    if( !(i >= 0 && i < ksm_vector_size(vec)) )
      ksm_error("vector index out of range: %d", i);
    val = ksm_vector_ref(vec, i);
    new_val = add_si(val, -1);
    ksm_registry_add(val);
    ksm_vector_set(vec, i, new_val);
    return val;
  }
  else{
    ksm_error("invalid lvalue in decrement: %k", expr);
    return ksm_unspec_object;
  }
}

KSM_DEF_SPC(prefix_inc)
     // (prefix-inc expr)
{
  ksm_t expr, val, new_val;

  KSM_POP_ARG(expr);
  KSM_SET_ARGS0();
  if( ksm_is_key(expr) ){
    val = ksm_lookup(expr, KSM_ENV());
    if( !val )
      ksm_error("can't find variable: %k", expr);
    new_val = add_si(val, 1);
    if( ksm_update(expr, new_val, KSM_ENV()) != 0 )
      ksm_error("can't happen in blt_prefix_inc");
    return new_val;
  }
  else if( ksm_length(expr) == 3 && 
	   ksm_car(expr) == ksm_make_key("vector-ref") ){
    ksm_t vec, ind;
    int i;

    vec = ksm_eval(ksm_second(expr), KSM_ENV());
    CHECK_EXCEPT(vec);
    ind = ksm_eval(ksm_third(expr), KSM_ENV());
    CHECK_EXCEPT(ind);
    if( !ksm_is_vector(vec) )
      ksm_error("invalid lvalue in increment: %k", expr);
    i = ksm_get_si(ind);
    if( !(i >= 0 && i < ksm_vector_size(vec)) )
      ksm_error("vector index out of range: %d", i);
    val = ksm_vector_ref(vec, i);
    new_val = add_si(val, 1);
    ksm_vector_set(vec, i, new_val);
    return new_val;
  }
  else{
    ksm_error("invalid lvalue in increment: %k", expr);
    return ksm_unspec_object;
  }
}

KSM_DEF_SPC(prefix_dec)
     // (prefix-dec expr)
{
  ksm_t expr, val, new_val;

  KSM_POP_ARG(expr);
  KSM_SET_ARGS0();
  if( ksm_is_key(expr) ){
    val = ksm_lookup(expr, KSM_ENV());
    if( !val )
      ksm_error("can't find variable: %k", expr);
    new_val = add_si(val, -1);
    if( ksm_update(expr, new_val, KSM_ENV()) != 0 )
      ksm_error("can't happen in blt_prefix_dec");
    return new_val;
  }
  else if( ksm_length(expr) == 3 && 
	   ksm_car(expr) == ksm_make_key("vector-ref") ){
    ksm_t vec, ind;
    int i;

    vec = ksm_eval(ksm_second(expr), KSM_ENV());
    CHECK_EXCEPT(vec);
    ind = ksm_eval(ksm_third(expr), KSM_ENV());
    CHECK_EXCEPT(ind);
    if( !ksm_is_vector(vec) )
      ksm_error("invalid lvalue in decrement: %k", expr);
    i = ksm_get_si(ind);
    if( !(i >= 0 && i < ksm_vector_size(vec)) )
      ksm_error("vector index out of range: %d", i);
    val = ksm_vector_ref(vec, i);
    new_val = add_si(val, -1);
    ksm_vector_set(vec, i, new_val);
    return new_val;
  }
  else{
    ksm_error("invalid lvalue in decrement: %k", expr);
    return ksm_unspec_object;
  }
}

KSM_DEF_SPC(mul_assign)
     // (clike:mul-assign expr arg)
{
  ksm_t expr, arg, val, new_val;

  KSM_SET_ARGS2(expr, arg);
  arg = ksm_eval(arg, KSM_ENV());
  CHECK_EXCEPT(arg);
  if( ksm_is_key(expr) ){
    val = ksm_lookup(expr, KSM_ENV());
    if( !val )
      ksm_error("can't find variable: %k", expr);
    new_val = mul(val, arg);
    if( ksm_update(expr, new_val, KSM_ENV()) != 0 )
      ksm_error("can't happen in blt_mul_assign");
    return new_val;
  }
  else if( ksm_length(expr) == 3 && 
	   ksm_car(expr) == ksm_make_key("vector-ref") ){
    ksm_t vec, ind;
    int i;

    vec = ksm_eval(ksm_second(expr), KSM_ENV());
    CHECK_EXCEPT(vec);
    ind = ksm_eval(ksm_third(expr), KSM_ENV());
    CHECK_EXCEPT(ind);
    if( !ksm_is_vector(vec) )
      ksm_error("invalid lvalue in assign: %k", expr);
    i = ksm_get_si(ind);
    if( !(i >= 0 && i < ksm_vector_size(vec)) )
      ksm_error("vector index out of range: %d", i);
    val = ksm_vector_ref(vec, i);
    new_val = mul(val, arg);
    ksm_vector_set(vec, i, new_val);
    return new_val;
  }
  else{
    ksm_error("invalid lvalue in assign: %k", expr);
    return ksm_unspec_object;
  }
}

KSM_DEF_SPC(div_assign)
     // (clike:div-assign expr arg)
{
  ksm_t expr, arg, val, new_val;

  KSM_SET_ARGS2(expr, arg);
  arg = ksm_eval(arg, KSM_ENV());
  CHECK_EXCEPT(arg);
  if( ksm_is_key(expr) ){
    val = ksm_lookup(expr, KSM_ENV());
    if( !val )
      ksm_error("can't find variable: %k", expr);
    new_val = my_div(val, arg);
    if( ksm_update(expr, new_val, KSM_ENV()) != 0 )
      ksm_error("can't happen in blt_div_assign");
    return new_val;
  }
  else if( ksm_length(expr) == 3 && 
	   ksm_car(expr) == ksm_make_key("vector-ref") ){
    ksm_t vec, ind;
    int i;

    vec = ksm_eval(ksm_second(expr), KSM_ENV());
    CHECK_EXCEPT(vec);
    ind = ksm_eval(ksm_third(expr), KSM_ENV());
    CHECK_EXCEPT(ind);
    if( !ksm_is_vector(vec) )
      ksm_error("invalid lvalue in assign: %k", expr);
    i = ksm_get_si(ind);
    if( !(i >= 0 && i < ksm_vector_size(vec)) )
      ksm_error("vector index out of range: %d", i);
    val = ksm_vector_ref(vec, i);
    new_val = my_div(val, arg);
    ksm_vector_set(vec, i, new_val);
    return new_val;
  }
  else{
    ksm_error("invalid lvalue in assign: %k", expr);
    return ksm_unspec_object;
  }
}

KSM_DEF_SPC(rem_assign)
     // (clike:rem-assign expr arg)
{
  ksm_t expr, arg, val, new_val;

  KSM_SET_ARGS2(expr, arg);
  arg = ksm_eval(arg, KSM_ENV());
  CHECK_EXCEPT(arg);
  if( ksm_is_key(expr) ){
    val = ksm_lookup(expr, KSM_ENV());
    if( !val )
      ksm_error("can't find variable: %k", expr);
    new_val = modulo(val, arg);
    if( ksm_update(expr, new_val, KSM_ENV()) != 0 )
      ksm_error("can't happen in blt_rem_assign");
    return new_val;
  }
  else if( ksm_length(expr) == 3 && 
	   ksm_car(expr) == ksm_make_key("vector-ref") ){
    ksm_t vec, ind;
    int i;

    vec = ksm_eval(ksm_second(expr), KSM_ENV());
    CHECK_EXCEPT(vec);
    ind = ksm_eval(ksm_third(expr), KSM_ENV());
    CHECK_EXCEPT(ind);
    if( !ksm_is_vector(vec) )
      ksm_error("invalid lvalue in assign: %k", expr);
    i = ksm_get_si(ind);
    if( !(i >= 0 && i < ksm_vector_size(vec)) )
      ksm_error("vector index out of range: %d", i);
    val = ksm_vector_ref(vec, i);
    new_val = modulo(val, arg);
    ksm_vector_set(vec, i, new_val);
    return new_val;
  }
  else{
    ksm_error("invalid lvalue in assign: %k", expr);
    return ksm_unspec_object;
  }
}

KSM_DEF_SPC(add_assign)
     // (clike:add-assign expr arg)
{
  ksm_t expr, arg, val, new_val;

  KSM_SET_ARGS2(expr, arg);
  arg = ksm_eval(arg, KSM_ENV());
  CHECK_EXCEPT(arg);
  if( ksm_is_key(expr) ){
    val = ksm_lookup(expr, KSM_ENV());
    if( !val )
      ksm_error("can't find variable: %k", expr);
    new_val = add(val, arg);
    if( ksm_update(expr, new_val, KSM_ENV()) != 0 )
      ksm_error("can't happen in blt_add_assign");
    return new_val;
  }
  else if( ksm_length(expr) == 3 && 
	   ksm_car(expr) == ksm_make_key("vector-ref") ){
    ksm_t vec, ind;
    int i;

    vec = ksm_eval(ksm_second(expr), KSM_ENV());
    CHECK_EXCEPT(vec);
    ind = ksm_eval(ksm_third(expr), KSM_ENV());
    CHECK_EXCEPT(ind);
    if( !ksm_is_vector(vec) )
      ksm_error("invalid lvalue in assign: %k", expr);
    i = ksm_get_si(ind);
    if( !(i >= 0 && i < ksm_vector_size(vec)) )
      ksm_error("vector index out of range: %d", i);
    val = ksm_vector_ref(vec, i);
    new_val = add(val, arg);
    ksm_vector_set(vec, i, new_val);
    return new_val;
  }
  else{
    ksm_error("invalid lvalue in assign: %k", expr);
    return ksm_unspec_object;
  }
}

KSM_DEF_SPC(sub_assign)
     // (clike:sub-assign expr arg)
{
  ksm_t expr, arg, val, new_val;

  KSM_SET_ARGS2(expr, arg);
  arg = ksm_eval(arg, KSM_ENV());
  CHECK_EXCEPT(arg);
  if( ksm_is_key(expr) ){
    val = ksm_lookup(expr, KSM_ENV());
    if( !val )
      ksm_error("can't find variable: %k", expr);
    new_val = sub(val, arg);
    if( ksm_update(expr, new_val, KSM_ENV()) != 0 )
      ksm_error("can't happen in blt_sub_assign");
    return new_val;
  }
  else if( ksm_length(expr) == 3 && 
	   ksm_car(expr) == ksm_make_key("vector-ref") ){
    ksm_t vec, ind;
    int i;

    vec = ksm_eval(ksm_second(expr), KSM_ENV());
    CHECK_EXCEPT(vec);
    ind = ksm_eval(ksm_third(expr), KSM_ENV());
    CHECK_EXCEPT(ind);
    if( !ksm_is_vector(vec) )
      ksm_error("invalid lvalue in assign: %k", expr);
    i = ksm_get_si(ind);
    if( !(i >= 0 && i < ksm_vector_size(vec)) )
      ksm_error("vector index out of range: %d", i);
    val = ksm_vector_ref(vec, i);
    new_val = sub(val, arg);
    ksm_vector_set(vec, i, new_val);
    return new_val;
  }
  else{
    ksm_error("invalid lvalue in assign: %k", expr);
    return ksm_unspec_object;
  }
}

KSM_DEF_SPC(lshift_assign)
     // (clike:lshift-assign expr arg)
{
  ksm_t expr, arg, val, new_val;

  KSM_SET_ARGS2(expr, arg);
  arg = ksm_eval(arg, KSM_ENV());
  CHECK_EXCEPT(arg);
  if( ksm_is_key(expr) ){
    val = ksm_lookup(expr, KSM_ENV());
    if( !val )
      ksm_error("can't find variable: %k", expr);
    new_val = lshift(val, arg);
    if( ksm_update(expr, new_val, KSM_ENV()) != 0 )
      ksm_error("can't happen in blt_lshift_assign");
    return new_val;
  }
  else if( ksm_length(expr) == 3 && 
	   ksm_car(expr) == ksm_make_key("vector-ref") ){
    ksm_t vec, ind;
    int i;

    vec = ksm_eval(ksm_second(expr), KSM_ENV());
    CHECK_EXCEPT(vec);
    ind = ksm_eval(ksm_third(expr), KSM_ENV());
    CHECK_EXCEPT(ind);
    if( !ksm_is_vector(vec) )
      ksm_error("invalid lvalue in assign: %k", expr);
    i = ksm_get_si(ind);
    if( !(i >= 0 && i < ksm_vector_size(vec)) )
      ksm_error("vector index out of range: %d", i);
    val = ksm_vector_ref(vec, i);
    new_val = lshift(val, arg);
    ksm_vector_set(vec, i, new_val);
    return new_val;
  }
  else{
    ksm_error("invalid lvalue in assign: %k", expr);
    return ksm_unspec_object;
  }
}

KSM_DEF_SPC(rshift_assign)
     // (clike:rshift-assign expr arg)
{
  ksm_t expr, arg, val, new_val;

  KSM_SET_ARGS2(expr, arg);
  arg = ksm_eval(arg, KSM_ENV());
  CHECK_EXCEPT(arg);
  if( ksm_is_key(expr) ){
    val = ksm_lookup(expr, KSM_ENV());
    if( !val )
      ksm_error("can't find variable: %k", expr);
    new_val = rshift(val, arg);
    if( ksm_update(expr, new_val, KSM_ENV()) != 0 )
      ksm_error("can't happen in blt_rshift_assign");
    return new_val;
  }
  else if( ksm_length(expr) == 3 && 
	   ksm_car(expr) == ksm_make_key("vector-ref") ){
    ksm_t vec, ind;
    int i;

    vec = ksm_eval(ksm_second(expr), KSM_ENV());
    CHECK_EXCEPT(vec);
    ind = ksm_eval(ksm_third(expr), KSM_ENV());
    CHECK_EXCEPT(ind);
    if( !ksm_is_vector(vec) )
      ksm_error("invalid lvalue in assign: %k", expr);
    i = ksm_get_si(ind);
    if( !(i >= 0 && i < ksm_vector_size(vec)) )
      ksm_error("vector index out of range: %d", i);
    val = ksm_vector_ref(vec, i);
    new_val = rshift(val, arg);
    ksm_vector_set(vec, i, new_val);
    return new_val;
  }
  else{
    ksm_error("invalid lvalue in assign: %k", expr);
    return ksm_unspec_object;
  }
}

KSM_DEF_SPC(and_assign)
     // (clike:and-assign expr arg)
{
  ksm_t expr, arg, val, new_val;

  KSM_SET_ARGS2(expr, arg);
  arg = ksm_eval(arg, KSM_ENV());
  CHECK_EXCEPT(arg);
  if( ksm_is_key(expr) ){
    val = ksm_lookup(expr, KSM_ENV());
    if( !val )
      ksm_error("can't find variable: %k", expr);
    new_val = and(val, arg);
    if( ksm_update(expr, new_val, KSM_ENV()) != 0 )
      ksm_error("can't happen in blt_and_assign");
    return new_val;
  }
  else if( ksm_length(expr) == 3 && 
	   ksm_car(expr) == ksm_make_key("vector-ref") ){
    ksm_t vec, ind;
    int i;

    vec = ksm_eval(ksm_second(expr), KSM_ENV());
    CHECK_EXCEPT(vec);
    ind = ksm_eval(ksm_third(expr), KSM_ENV());
    CHECK_EXCEPT(ind);
    if( !ksm_is_vector(vec) )
      ksm_error("invalid lvalue in assign: %k", expr);
    i = ksm_get_si(ind);
    if( !(i >= 0 && i < ksm_vector_size(vec)) )
      ksm_error("vector index out of range: %d", i);
    val = ksm_vector_ref(vec, i);
    new_val = and(val, arg);
    ksm_vector_set(vec, i, new_val);
    return new_val;
  }
  else{
    ksm_error("invalid lvalue in assign: %k", expr);
    return ksm_unspec_object;
  }
}

KSM_DEF_SPC(xor_assign)
     // (clike:xor-assign expr arg)
{
  ksm_t expr, arg, val, new_val;

  KSM_SET_ARGS2(expr, arg);
  arg = ksm_eval(arg, KSM_ENV());
  CHECK_EXCEPT(arg);
  if( ksm_is_key(expr) ){
    val = ksm_lookup(expr, KSM_ENV());
    if( !val )
      ksm_error("can't find variable: %k", expr);
    new_val = xor(val, arg);
    if( ksm_update(expr, new_val, KSM_ENV()) != 0 )
      ksm_error("can't happen in blt_xor_assign");
    return new_val;
  }
  else if( ksm_length(expr) == 3 && 
	   ksm_car(expr) == ksm_make_key("vector-ref") ){
    ksm_t vec, ind;
    int i;

    vec = ksm_eval(ksm_second(expr), KSM_ENV());
    CHECK_EXCEPT(vec);
    ind = ksm_eval(ksm_third(expr), KSM_ENV());
    CHECK_EXCEPT(ind);
    if( !ksm_is_vector(vec) )
      ksm_error("invalid lvalue in assign: %k", expr);
    i = ksm_get_si(ind);
    if( !(i >= 0 && i < ksm_vector_size(vec)) )
      ksm_error("vector index out of range: %d", i);
    val = ksm_vector_ref(vec, i);
    new_val = xor(val, arg);
    ksm_vector_set(vec, i, new_val);
    return new_val;
  }
  else{
    ksm_error("invalid lvalue in assign: %k", expr);
    return ksm_unspec_object;
  }
}

KSM_DEF_SPC(or_assign)
     // (clike:or-assign expr arg)
{
  ksm_t expr, arg, val, new_val;

  KSM_SET_ARGS2(expr, arg);
  arg = ksm_eval(arg, KSM_ENV());
  CHECK_EXCEPT(arg);
  if( ksm_is_key(expr) ){
    val = ksm_lookup(expr, KSM_ENV());
    if( !val )
      ksm_error("can't find variable: %k", expr);
    new_val = or(val, arg);
    if( ksm_update(expr, new_val, KSM_ENV()) != 0 )
      ksm_error("can't happen in blt_or_assign");
    return new_val;
  }
  else if( ksm_length(expr) == 3 && 
	   ksm_car(expr) == ksm_make_key("vector-ref") ){
    ksm_t vec, ind;
    int i;

    vec = ksm_eval(ksm_second(expr), KSM_ENV());
    CHECK_EXCEPT(vec);
    ind = ksm_eval(ksm_third(expr), KSM_ENV());
    CHECK_EXCEPT(ind);
    if( !ksm_is_vector(vec) )
      ksm_error("invalid lvalue in assign: %k", expr);
    i = ksm_get_si(ind);
    if( !(i >= 0 && i < ksm_vector_size(vec)) )
      ksm_error("vector index out of range: %d", i);
    val = ksm_vector_ref(vec, i);
    new_val = or(val, arg);
    ksm_vector_set(vec, i, new_val);
    return new_val;
  }
  else{
    ksm_error("invalid lvalue in assign: %k", expr);
    return ksm_unspec_object;
  }
}

KSM_DEF_SPC(define)
     // (clike:define [(var .val)]...)
{
  int n;
  ksm_t arg, var, val;

  n = KSM_NARG();
  if( n < 0 )
    ksm_error("invalid arg (clike:define): %k", KSM_ARGS());
  while( n-- > 0 ){
    KSM_POP_ARG(arg);
    var = ksm_safe_car(arg);
    val = ksm_cdr(arg);
    val = ksm_eval(val, KSM_ENV());
    CHECK_EXCEPT(val);
    var = ksm_cvt_key(var);
    ksm_enter_global(var, val, ksm_interact_env, 1);
  }
  return ksm_unspec_object;
}

static ksm_t ksm_return_tag;
static ksm_t ksm_break_tag;

static
ksm_t return_handler(ksm_t ex, ksm_t args, void *aux)
{
  if( ex == ksm_return_tag ){
    if( ksm_length(args) != 1 )
      ksm_error("can't happen (return_handler): %k", args);
    return ksm_first(args);
  }
  else
    return ksm_raise_except(ex, args);
}

KSM_DEF_BLT(with_return)
     // (clike:with-return fun)
     // fun: (lambda () ...)
{
  ksm_t fun;

  KSM_SET_ARGS1(fun);
  return ksm_with_handler(return_handler, fun, 0);
}

KSM_DEF_BLT(return)
     // (clike:return val)
{
  ksm_t val;

  KSM_SET_ARGS1(val);
  return ksm_raise_except(ksm_return_tag, ksm_cons(val, ksm_nil_object));
}

static
ksm_t break_handler(ksm_t ex, ksm_t args, void *aux)
{
  if( ex == ksm_break_tag ){
    if( ksm_length(args) != 0 )
      ksm_error("can't happen (break_handler): %k", args);
    return ksm_unspec_object;
  }
  else{
    return ksm_raise_except(ex, args);
  }
}

KSM_DEF_BLT(with_break)
     // (clike:with-break fun)
{
  ksm_t fun;

  KSM_SET_ARGS1(fun);
  return ksm_with_handler(break_handler, fun, 0);
}

KSM_DEF_BLT(break)
     // (clike:break)
{
  KSM_SET_ARGS0();
  return ksm_raise_except(ksm_break_tag, ksm_nil_object);
}

KSM_DEF_BLT(aref)
     // (clike:aref obj index)
{
  ksm_t obj, index;
  int i;

  KSM_SET_ARGS2(obj, index);
  i = ksm_get_si(index);
  if( ksm_is_vector(obj) ){
    if( !(i >= 0 && i < ksm_vector_size(obj)) )
      ksm_error("index out of range: %d", i);
    return ksm_vector_ref(obj, i);
  }
  else if( ksm_clang_is_csym(obj) )
    return ksm_clang_aref(obj, i);
  else{
    ksm_error("invalid array access: %k", obj);
    return ksm_unspec_object;
  }
}

KSM_DEF_BLT(aset)
     // (clike:aset (obj index) val)
{
  ksm_t pair, obj, index, val;
  int i;

  KSM_SET_ARGS2(pair, val);
  if( ksm_length(pair) != 2 )
    ksm_error("can't happen in blt_aset (clike.c)");
  obj = ksm_first(pair);
  index = ksm_second(pair);
  i = ksm_get_si(index);
  if( ksm_is_vector(obj) ){
    if( !(i >= 0 && i < ksm_vector_size(obj)) )
      ksm_error("index out of range: %d", i);
    ksm_vector_set(obj, i, val);
  }
  else if( ksm_clang_is_csym(obj) )
    ksm_clang_aset(obj, i, val);
  else
    ksm_error("invalid array access: %k", obj);
  return ksm_unspec_object;
}

KSM_DEF_BLT(sizeof)
     // (clike:sizeof arg)
{
  ksm_t x;

  KSM_POP_ARG(x);
  KSM_SET_ARGS0();
  if( ksm_clang_is_type(x) )
    return ksm_make_int(ksm_clang_type_size(x));
  else if( ksm_clang_is_csym(x) )
    return ksm_make_int(ksm_clang_type_size(ksm_clang_csym_type(x)));
  else{
    ksm_error("invalid arg to sizeof: %k", x);
    return ksm_unspec_object;
  }
}

static int clike_false(ksm_t x)
{
  return (x == ksm_false_object) ||
    (ksm_is_int(x) && (ksm_fetch_int(x) == 0)) ||
    (ksm_is_double(x) && (ksm_fetch_double(x) == 0.0)) ||
    (ksm_clang_is_csym(x) && 
     ksm_clang_type_is_pointer(ksm_clang_csym_type(x)) &&
     (ksm_clang_csym_adr(x) == 0));
}

KSM_DEF_BLT(true)
     // (clike:true x)
{
  ksm_t x;

  KSM_SET_ARGS1(x);
  return KSM_BOOL2OBJ(!clike_false(x));
}

KSM_DEF_BLT(false)
     // (clike:false x)
{
  ksm_t x;

  KSM_SET_ARGS1(x);
  return KSM_BOOL2OBJ(clike_false(x));
}

void ksm_init_clike_package(void)
{
  ksm_return_tag = ksm_make_imm("#<clike:return>");
  ksm_permanent(&ksm_return_tag);
  ksm_break_tag = ksm_make_imm("#<clike:break>");
  ksm_permanent(&ksm_break_tag);
  ksm_read_toplevel_fun = read_toplevel;
  KSM_ENTER_SPC("clike:postfix-inc", postfix_inc);
  KSM_ENTER_SPC("clike:postfix-dec", postfix_dec);
  KSM_ENTER_SPC("clike:prefix-inc", prefix_inc);
  KSM_ENTER_SPC("clike:prefix-dec", prefix_dec);
  KSM_ENTER_SPC("clike:mul-assign", mul_assign);
  KSM_ENTER_SPC("clike:div-assign", div_assign);
  KSM_ENTER_SPC("clike:rem-assign", rem_assign);
  KSM_ENTER_SPC("clike:add-assign", add_assign);
  KSM_ENTER_SPC("clike:sub-assign", sub_assign);
  KSM_ENTER_SPC("clike:lshift-assign", lshift_assign);
  KSM_ENTER_SPC("clike:rshift-assign", rshift_assign);
  KSM_ENTER_SPC("clike:and-assign", and_assign);
  KSM_ENTER_SPC("clike:xor-assign", xor_assign);
  KSM_ENTER_SPC("clike:or-assign", or_assign);
  KSM_ENTER_SPC("clike:define", define);
  KSM_ENTER_BLT("clike:with-return", with_return);
  KSM_ENTER_BLT("clike:return", return);
  KSM_ENTER_BLT("clike:with-break", with_break);
  KSM_ENTER_BLT("clike:break", break);
  KSM_ENTER_BLT("clike:aref", aref);
  KSM_ENTER_BLT_SETTER("clike:aref", aset);
  KSM_ENTER_BLT("clike:sizeof", sizeof);
  KSM_ENTER_BLT("clike:true", true);
  KSM_ENTER_BLT("clike:false", false);

  /***
  {
    extern int ksm_clike_debug;

    ksm_clike_debug = 1;
  }
  ***/
}

