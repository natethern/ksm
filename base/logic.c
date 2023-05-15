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

/***
;; LOGICAL-AND : returns logical-and of args
;; (logical-and arg ...) [procedure]
;; Currently, arg's are limited to 32-bit signed integers.
***/

KSM_DEF_BLT(and)
     // (logical-and arg ...)
{
  ksm_t arg;
  ksm_numrep p, q;
  
  KSM_POP_ARG(arg);
  if( ksm_set_numrep(&p, arg) != 0 )
    ksm_error("number expected (LOGICAL-AND): %k", arg);
  if( KSM_NO_MORE_ARGS() )
    return arg;

  ksm_numrep_clone(&p);
  while( KSM_MORE_ARGS() ){
    KSM_POP_ARG(arg);
    if( ksm_set_numrep(&q, arg) != 0 )
      ksm_error("number expected (LOGICAL-AND): %k", arg);
    ksm_numrep_and(&p, &q);
  }
  return ksm_numrep_to_object(&p);
}

/***
;; LOGICAL-OR : returns logical-or of args
;; (logical-or arg ...) [procedure]
;; Currently, arg's are limited to 32-bit signed integers.
***/

KSM_DEF_BLT(or)
     // (logical-or arg ...)
{
  ksm_t arg;
  ksm_numrep p, q;
  
  KSM_POP_ARG(arg);
  if( ksm_set_numrep(&p, arg) != 0 )
    ksm_error("number expected (LOGICAL-OR): %k", arg);
  if( KSM_NO_MORE_ARGS() )
    return arg;

  ksm_numrep_clone(&p);
  while( KSM_MORE_ARGS() ){
    KSM_POP_ARG(arg);
    if( ksm_set_numrep(&q, arg) != 0 )
      ksm_error("number expected (LOGICAL-OR): %k", arg);
    ksm_numrep_or(&p, &q);
  }
  return ksm_numrep_to_object(&p);
}

/***
;; LOGICAL-COMPLEMENT : returns one's complement of arg
;; (logical-complement arg)
;; Currently, arg is limited to 32-bit integer.
***/

KSM_DEF_BLT(complement)
     // (logical-complement arg)
{
  ksm_t arg;
  ksm_numrep rep;

  KSM_SET_ARGS1(arg);
  if( ksm_set_numrep(&rep, arg) != 0 )
    ksm_error("number expected (LOGICAL-COMPLEMENT): %k", arg);
  ksm_numrep_clone(&rep);
  ksm_numrep_complement(&rep);
  return ksm_numrep_to_object(&rep);
}

/***
;; LOGICAL-XOR : returns exclusive-or of args
;; (logical-xor x y)
;; Currently, x and y are limited to 32-bit integers.
***/

KSM_DEF_BLT(xor)
     // (logical-xor x y)
{
  ksm_t arg;
  ksm_numrep p, q;
  
  KSM_POP_ARG(arg);
  if( ksm_set_numrep(&p, arg) != 0 )
    ksm_error("number expected (LOGICAL-XOR): %k", arg);
  if( KSM_NO_MORE_ARGS() )
    return arg;

  ksm_numrep_clone(&p);
  while( KSM_MORE_ARGS() ){
    KSM_POP_ARG(arg);
    if( ksm_set_numrep(&q, arg) != 0 )
      ksm_error("number expected (LOGICAL-XOR): %k", arg);
    ksm_numrep_xor(&p, &q);
  }
  return ksm_numrep_to_object(&p);
}

KSM_DEF_BLT(lshift)
     // (logical-lshift expr n)
{
  ksm_t arg_expr, arg_n;
  ksm_numrep p;
  int n;

  KSM_SET_ARGS2(arg_expr, arg_n);
  if( ksm_set_numrep(&p, arg_expr) != 0 )
    ksm_error("invalid first arg to logical-lshift: %k", arg_expr);
  n = ksm_get_si(arg_n);
  ksm_numrep_clone(&p);
  if( n > 0 )
    ksm_numrep_lshift(&p, n);
  else if( n < 0 )
    ksm_numrep_rshift(&p, -n);
  return ksm_numrep_to_object(&p);
}

KSM_DEF_BLT(rshift)
     // (logical-rshift expr n)
{
  ksm_t arg_expr, arg_n;
  ksm_numrep p;
  int n;

  KSM_SET_ARGS2(arg_expr, arg_n);
  if( ksm_set_numrep(&p, arg_expr) != 0 )
    ksm_error("invalid first arg to logical-rshift: %k", arg_expr);
  n = ksm_get_si(arg_n);
  ksm_numrep_clone(&p);
  if( n > 0 )
    ksm_numrep_rshift(&p, n);
  else if( n < 0 )
    ksm_numrep_lshift(&p, -n);
  return ksm_numrep_to_object(&p);
}

void ksm_init_logic(void)
{
  KSM_ENTER_BLT("logical-and", and);
  KSM_ENTER_BLT("logical-or", or);
  KSM_ENTER_BLT("logical-complement", complement);
  KSM_ENTER_BLT("logical-xor", xor);
  KSM_ENTER_BLT("logical-lshift", lshift);
  KSM_ENTER_BLT("logical-rshift", rshift);
}

