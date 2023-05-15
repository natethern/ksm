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

#include <string.h>
#include <limits.h>
#include "ksm.h"

#define SET_NUMREP(p,x) \
  do{ if( ksm_set_numrep(p,x) != 0 ) \
    ksm_error("number expected: %k", x); \
  } while(0)

static
ksm_t numcmp(ksm_t args, int (*f)(ksm_numrep *, ksm_numrep *))
{
  ksm_t x, y;
  ksm_numrep p, q;

  if( !ksm_is_cons(args) )
    ksm_error("too few args (numcmp): %k", args);
  x = ksm_car(args);
  args = ksm_cdr(args);
  if( !ksm_is_cons(args) )
    ksm_error("too few args (numcmp): %k", args);
  y = ksm_car(args);
  args = ksm_cdr(args);
  while( 1 ){
    SET_NUMREP(&p, x);
    SET_NUMREP(&q, y);
    if( !(*f)(&p, &q) )
      return ksm_false_object;
    if( ksm_is_nil(args) )
      break;
    x = y;
    if( !ksm_is_cons(args) )
      ksm_error("incompatible args (numcmp): %k", args);
    y = ksm_car(args);
    args = ksm_cdr(args);
  }
  return ksm_true_object;
}

KSM_DEF_BLT(numeq)
     // (= z1 z2 z3 ...)
{
  return numcmp(KSM_ARGS(), ksm_numrep_eq);
}

KSM_DEF_BLT(numlt)
     // (< z1 z2 z3 ...)
{
  return numcmp(KSM_ARGS(), ksm_numrep_lt);
}

KSM_DEF_BLT(numgt)
     // (> z1 z2 z3 ...)
{
  return numcmp(KSM_ARGS(), ksm_numrep_gt);
}

KSM_DEF_BLT(numle)
     // (<= z1 z2 z3 ...)
{
  return numcmp(KSM_ARGS(), ksm_numrep_le);
}

KSM_DEF_BLT(numge)
     // (>= z1 z2 z3 ...)
{
  return numcmp(KSM_ARGS(), ksm_numrep_ge);
}

KSM_DEF_BLT(odd_p)
     // (odd? n)
{
  ksm_t x;
  int n;

  KSM_POP_ARG(x);
  n = ksm_int_value(x);
  return KSM_BOOL2OBJ(n&1);
}

KSM_DEF_BLT(even_p)
     // (even? n)
{
  ksm_t x;
  int n;

  KSM_POP_ARG(x);
  n = ksm_int_value(x);
  return KSM_BOOL2OBJ((n&1)==0);
}

KSM_DEF_BLT(max)
     // (max x1 x2 ...)
{
  ksm_t x, y;
  ksm_numrep p, q;

  KSM_POP_ARG(x);
  SET_NUMREP(&p, x);
  while( KSM_MORE_ARGS() ){
    KSM_POP_ARG(y);
    SET_NUMREP(&q, y);
    if( ksm_numrep_lt(&p, &q) ){
      x = y;
      memcpy(&p, &q, sizeof(p));
    }
  }
  return x;
}

KSM_DEF_BLT(min)
     // (min x1 x2 ...)
{
  ksm_t x, y;
  ksm_numrep p, q;

  KSM_POP_ARG(x);
  SET_NUMREP(&p, x);
  while( KSM_MORE_ARGS() ){
    KSM_POP_ARG(y);
    SET_NUMREP(&q, y);
    if( ksm_numrep_gt(&p, &q) ){
      x = y;
      memcpy(&p, &q, sizeof(p));
    }
  }
  return x;
}

KSM_DEF_BLT(plus)
     // (+ z1 ...)
{
  ksm_t x;
  ksm_numrep p, q;

  ksm_set_numrep_0(&p);
  while( KSM_MORE_ARGS() ){
    KSM_POP_ARG(x);
    SET_NUMREP(&q, x);
    ksm_numrep_add(&p, &q);
  }
  return ksm_numrep_to_object(&p);
}

KSM_DEF_BLT(mul)
     // (* z1 ...)
{
  ksm_t x;
  ksm_numrep p, q;

  ksm_set_numrep_1(&p);
  while( KSM_MORE_ARGS() ){
    KSM_POP_ARG(x);
    SET_NUMREP(&q, x);
    ksm_numrep_mul(&p, &q);
  }
  return ksm_numrep_to_object(&p);
}

KSM_DEF_BLT(sub)
     // (- z1 z2 ...)
{
  ksm_t x;
  ksm_numrep p, q;

  KSM_POP_ARG(x);
  SET_NUMREP(&p, x);
  ksm_numrep_clone(&p);
  if( KSM_NO_MORE_ARGS() ){
    ksm_numrep_neg(&p);
    return ksm_numrep_to_object(&p);
  }
  while( KSM_MORE_ARGS() ){
    KSM_POP_ARG(x);
    SET_NUMREP(&q, x);
    ksm_numrep_sub(&p, &q);
  }
  return ksm_numrep_to_object(&p);
}

KSM_DEF_BLT(div)
     // (/ z1 z2 ...)
{
  ksm_t x;
  ksm_numrep p, q;

  KSM_POP_ARG(x);
  SET_NUMREP(&p, x);
  if( KSM_NO_MORE_ARGS() ){
    ksm_set_numrep_1(&q);
    ksm_numrep_div(&q, &p);
    return ksm_numrep_to_object(&q);
  }
  ksm_numrep_clone(&p);
  while( KSM_MORE_ARGS() ){
    KSM_POP_ARG(x);
    SET_NUMREP(&q, x);
    ksm_numrep_div(&p, &q);
  }
  return ksm_numrep_to_object(&p);
}

KSM_DEF_BLT(abs)
     // (abs x)
{
  ksm_t x;
  ksm_numrep p;

  KSM_POP_ARG(x);
  SET_NUMREP(&p, x);
  if( ksm_numrep_negative_p(&p) ){
    ksm_numrep_clone(&p);
    ksm_numrep_neg(&p);
    return ksm_numrep_to_object(&p);
  }
  else
    return x;
}

KSM_DEF_BLT(quotient)
     // (quotient x y)
{
  ksm_t x, y;
  ksm_numrep p, q;

  KSM_SET_ARGS2(x, y);
  SET_NUMREP(&p, x);
  SET_NUMREP(&q, y);
  ksm_numrep_clone(&p);
  ksm_numrep_quotient(&p, &q);
  return ksm_numrep_to_object(&p);
}

KSM_DEF_BLT(remainder)
     // (remainder x y)
{
  ksm_t x, y;
  ksm_numrep p, q;

  KSM_SET_ARGS2(x, y);
  SET_NUMREP(&p, x);
  SET_NUMREP(&q, y);
  ksm_numrep_clone(&p);
  ksm_numrep_remainder(&p, &q);
  return ksm_numrep_to_object(&p);
}

KSM_DEF_BLT(modulo)
     // (modulo x y)
{
  ksm_t x, y;
  ksm_numrep p, q;

  KSM_SET_ARGS2(x, y);
  SET_NUMREP(&p, x);
  SET_NUMREP(&q, y);
  ksm_numrep_clone(&p);
  ksm_numrep_modulo(&p, &q);
  return ksm_numrep_to_object(&p);
}

KSM_DEF_BLT(gcd)
     // (gcd x ...)
{
  ksm_t x;
  ksm_numrep p, q;

  ksm_set_numrep_0(&p);
  while( KSM_MORE_ARGS() ){
    KSM_POP_ARG(x);
    SET_NUMREP(&q, x);
    ksm_numrep_gcd(&p, &q);
  }
  return ksm_numrep_to_object(&p);
}

KSM_DEF_BLT(lcm)
     // (lcm x ...)
{
  ksm_t x;
  ksm_numrep p, q;

  ksm_set_numrep_1(&p);
  while( KSM_MORE_ARGS() ){
    KSM_POP_ARG(x);
    SET_NUMREP(&q, x);
    ksm_numrep_lcm(&p, &q);
  }
  return ksm_numrep_to_object(&p);
}

#define KSM_DEF_UNARY(name) \
KSM_DEF_BLT(name) \
{ \
  ksm_t x; \
  ksm_numrep p; \
 \
  KSM_SET_ARGS1(x); \
  SET_NUMREP(&p, x); \
  ksm_numrep_clone(&p); \
  ksm_numrep_##name(&p); \
  return ksm_numrep_to_object(&p); \
}

#define KSM_DEF_BINARY(name) \
KSM_DEF_BLT(name) \
{ \
  ksm_t x, y; \
  ksm_numrep p, q; \
 \
  KSM_SET_ARGS2(x, y); \
  SET_NUMREP(&p, x); \
  SET_NUMREP(&q, y); \
  ksm_numrep_clone(&p); \
  ksm_numrep_##name(&p, &q); \
  return ksm_numrep_to_object(&p); \
}

KSM_DEF_UNARY(floor)
KSM_DEF_UNARY(ceiling)
KSM_DEF_UNARY(truncate)

KSM_DEF_BLT(round)
{
  ksm_t x;
  ksm_numrep rep;

  KSM_SET_ARGS1(x);
  SET_NUMREP(&rep, x);
#ifdef GMP
  if( rep.kind <= KSM_NUMREP_MPZ )
    return x;
  if( rep.kind == KSM_NUMREP_MPQ ){
    mpz_t r, x;
    mpz_ptr q;
    mpq_ptr xx;

    q = ksm_alloc(sizeof(mpz_t));
    mpz_init(q);
    mpz_init(r);
    mpz_init(x);
    xx = rep.u.pval;
    mpz_tdiv_qr(q, r, mpq_numref(xx), mpq_denref(xx));
    mpz_mul_ui(x, r, 2);
    mpz_tdiv_q(x, x, mpq_denref(xx));
    if( mpz_sgn(x) > 0 )
      mpz_add_ui(q, q, 1);
    else if( mpz_sgn(x) < 0 )
      mpz_sub_ui(q, q, 1);
    mpz_clear(r);
    mpz_clear(x);
    return ksm_make_mpz(q);
  }
#else
  if( rep.kind <= KSM_NUMREP_INT )
    return x;
#endif
  ksm_numrep_round(&rep);
  return ksm_make_double(rep.u.dval);
}

KSM_DEF_UNARY(exp)
KSM_DEF_UNARY(log)
KSM_DEF_UNARY(sin)
KSM_DEF_UNARY(cos)
KSM_DEF_UNARY(tan)
KSM_DEF_UNARY(asin)
KSM_DEF_UNARY(acos)

KSM_DEF_BLT(atan)
{
  ksm_t yz, x;
  ksm_numrep p, q;

  KSM_POP_ARG(yz);
  SET_NUMREP(&p, yz);
  ksm_numrep_clone(&p);
  if( KSM_NO_MORE_ARGS() )
    ksm_numrep_atan(&p);
  else{
    KSM_SET_ARGS1(x);
    SET_NUMREP(&q, x);
    ksm_numrep_atan2(&p, &q);
  }
  return ksm_numrep_to_object(&p);
}

#define ISQRT_MAX  46340    /* calculated from INT_MAX */

static
int isqrt(int x)
     // returns -1 if not found
{
  int left, right, center, dif;

  left = 0;
  right = x;
  if( right > ISQRT_MAX )
    right = ISQRT_MAX;
  while(1){
    if( right - left <= 1 ){
      if( x == left * left )
	return left;
      if( x == right * right )
	return right;
      return -1;
    }
    center = ((unsigned)left+(unsigned)right)>>1;
    dif = x - center * center;
    if( dif == 0 )
      return center;
    if( dif > 0 )
      left = center;
    else 
      right = center;
  }
}

KSM_DEF_BLT(sqrt)
{
  ksm_t z;
  ksm_numrep rep;

  KSM_SET_ARGS1(z);
  SET_NUMREP(&rep, z);
  if( rep.kind == KSM_NUMREP_COMPLEX ){
    ksm_numrep_clone(&rep);
    ksm_numrep_sqrt(&rep);
    return ksm_numrep_to_object(&rep);
  }
  if( ksm_numrep_negative_p(&rep) ){
    ksm_numrep_promote(&rep, KSM_NUMREP_COMPLEX);
    ksm_numrep_sqrt(&rep);
    return ksm_numrep_to_object(&rep);
  }
  if( rep.kind == KSM_NUMREP_INT ){
    int i = isqrt(rep.u.ival);
    
    if( i >= 0 )
      return ksm_make_int(i);
    // fall through
  }
#ifdef GMP
  if( rep.kind == KSM_NUMREP_MPZ ){
    mpz_ptr p;
    mpz_t r;

    p = ksm_alloc(sizeof(mpz_t));
    mpz_init(p);
    mpz_init(r);
    mpz_sqrtrem(p, r, (mpz_ptr)rep.u.pval);
    if( mpz_sgn(r) == 0 ){
      mpz_clear(r);
      return ksm_make_mpz(p);
    }
    mpz_clear(r);
    mpz_clear(p);
    ksm_dispose(p);
    // fall through
  }
#endif
  ksm_numrep_sqrt(&rep);
  return ksm_numrep_to_object(&rep);
}

KSM_DEF_BINARY(expt)
KSM_DEF_UNARY(exact_to_inexact)
KSM_DEF_UNARY(inexact_to_exact)

void ksm_blt_math(void)
{
  KSM_ENTER_BLT("=", numeq);
  KSM_ENTER_BLT("<", numlt);
  KSM_ENTER_BLT(">", numgt);
  KSM_ENTER_BLT("<=", numle);
  KSM_ENTER_BLT(">=", numge);
  KSM_ENTER_BLT("odd?", odd_p);
  KSM_ENTER_BLT("even?", even_p);
  KSM_ENTER_BLT("max", max);
  KSM_ENTER_BLT("min", min);
  KSM_ENTER_BLT("+", plus);
  KSM_ENTER_BLT("*", mul);
  KSM_ENTER_BLT("-", sub);
  KSM_ENTER_BLT("/", div);
  KSM_ENTER_BLT("abs", abs);
  KSM_ENTER_BLT("quotient", quotient);
  KSM_ENTER_BLT("remainder", remainder);
  KSM_ENTER_BLT("modulo", modulo);
  KSM_ENTER_BLT("gcd", gcd);
  KSM_ENTER_BLT("lcm", lcm);
  KSM_ENTER_BLT("floor", floor);
  KSM_ENTER_BLT("ceiling", ceiling);
  KSM_ENTER_BLT("truncate", truncate);
  KSM_ENTER_BLT("round", round);
  KSM_ENTER_BLT("exp", exp);
  KSM_ENTER_BLT("log", log);
  KSM_ENTER_BLT("sin", sin);
  KSM_ENTER_BLT("cos", cos);
  KSM_ENTER_BLT("tan", tan);
  KSM_ENTER_BLT("asin", asin);
  KSM_ENTER_BLT("acos", acos);
  KSM_ENTER_BLT("atan", atan);
  KSM_ENTER_BLT("sqrt", sqrt);
  KSM_ENTER_BLT("expt", expt);
  KSM_ENTER_BLT("exact->inexact", exact_to_inexact);
  KSM_ENTER_BLT("inexact->exact", inexact_to_exact);
}

