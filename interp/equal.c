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
#include <string.h>

typedef int (*equal_fun)(ksm_t a, ksm_t b);
KSM_INSTALLER(equal,equal_fun)   /*** GLOBAL ***/
#define GET_EQUAL(x)   get_equal(ksm_flag(x))

int ksm_equal(ksm_t a, ksm_t b)
{
  equal_fun f;

  if( ksm_eqv(a, b) )
    return 1;
  f = GET_EQUAL(a);
  return f ? (*f)(a, b) : 0;
}

int ksm_eqv(ksm_t a, ksm_t b)
{
  if( a == b )
    return 1;
  if( ksm_is_number(a) && ksm_is_number(b) )
    return ksm_exact_p(a) == ksm_exact_p(b) && ksm_number_eqv(a, b);
  if( ksm_is_char(a) && ksm_is_char(b) )
    return ksm_fetch_char(a) == ksm_fetch_char(b);
  return 0;
}

int ksm_memq(ksm_t a, ksm_t list)
{
  while( ksm_is_cons(list) ){
    if( a == ksm_car(list) )
      return 1;
    list = ksm_cdr(list);
  }
  return 0;
}

int ksm_memv(ksm_t a, ksm_t list)
{
  while( ksm_is_cons(list) ){
    if( ksm_eqv(a, ksm_car(list)) )
      return 1;
    list = ksm_cdr(list);
  }
  return 0;
}

int ksm_member(ksm_t a, ksm_t list)
{
  while( ksm_is_cons(list) ){
    if( ksm_equal(a, ksm_car(list)) )
      return 1;
    list = ksm_cdr(list);
  }
  return 0;
}

ksm_t ksm_assq(ksm_t key, ksm_t alist)
{
  ksm_t x;

  while( ksm_is_cons(alist) ){
    x = ksm_car(alist);
    alist = ksm_cdr(alist);
    if( ksm_is_cons(x) && key == ksm_car(x) )
      return x;
  }
  return ksm_false_object;
}

ksm_t ksm_assv(ksm_t key, ksm_t alist)
{
  ksm_t x;

  while( ksm_is_cons(alist) ){
    x = ksm_car(alist);
    alist = ksm_cdr(alist);
    if( ksm_is_cons(x) && ksm_eqv(key, ksm_car(x)) )
      return x;
  }
  return ksm_false_object;
}

ksm_t ksm_assoc(ksm_t key, ksm_t alist)
{
  ksm_t x;

  while( ksm_is_cons(alist) ){
    x = ksm_car(alist);
    alist = ksm_cdr(alist);
    if( ksm_is_cons(x) && ksm_equal(key, ksm_car(x)) )
      return x;
  }
  return ksm_false_object;
}

KSM_DEF_BLT(eq)
     // (eq? x y)
{
  ksm_t x, y;

  KSM_POP_ARG(x);
  KSM_POP_ARG(y);
  KSM_SET_ARGS0();
  return KSM_BOOL2OBJ(x==y);
}

KSM_DEF_BLT(eqv)
     // (eqv? x y)
{
  ksm_t x, y;

  KSM_POP_ARG(x);
  KSM_POP_ARG(y);
  KSM_SET_ARGS0();
  return KSM_BOOL2OBJ(ksm_eqv(x,y));
}

KSM_DEF_BLT(equal)
     // (equal? x y)
{
  ksm_t x, y;

  KSM_POP_ARG(x);
  KSM_POP_ARG(y);
  KSM_SET_ARGS0();
  return KSM_BOOL2OBJ(ksm_equal(x,y));
}

void ksm_init_equal(void)
{
  KSM_ENTER_BLT("eq?", eq);
  KSM_ENTER_BLT("eqv?", eqv);
  KSM_ENTER_BLT("equal?", equal);
}

