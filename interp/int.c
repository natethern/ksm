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

#include <limits.h>
#include <stdlib.h>
#include <math.h>
#include "ksm.h"

static void int_clone(ksm_numrep *self) { }

static void int_promote(ksm_numrep *arg) { }

static
void int_exact_to_inexact(ksm_numrep *self)
{
  self->kind = KSM_NUMREP_DOUBLE;
  self->u.dval = self->u.ival;
}

static void int_inexact_to_exact(ksm_numrep *self) { }

static
ksm_t int_object(ksm_numrep *self) 
{ 
  return ksm_make_int(self->u.ival); 
}

static
int int_get_si(ksm_numrep *self)
{
  return self->u.ival;
}

static
double int_get_d(ksm_numrep *self)
{
  return self->u.ival;
}

static
int int_cmp(ksm_numrep *self, ksm_numrep *arg)
{
  return self->u.ival - arg->u.ival;
}

static
int int_sgn(ksm_numrep *self)
{
  return self->u.ival;
}

#ifdef GMP
#define PROMOTE(x)  ksm_numrep_promote(x, KSM_NUMREP_MPZ)
#endif

#define ADD_OVERFLOW(arg1, arg2, result) \
( \
( (arg1) > 0 && (arg2) > 0 && (result) <= 0 ) || \
( (arg1) < 0 && (arg2) < 0 && (result) >= 0 ) \
)

static 
int int_add(ksm_numrep *self, ksm_numrep *arg)
{
  int arg1, arg2, res;

  arg1 = self->u.ival;
  arg2 = arg->u.ival;

#ifdef KSM_LINUXPPC
  {
    int x;

    __asm__ ("addo %0,%2,%3; mfspr %1,1" 
	     : "=r" (res), "=r" (x)
	     : "r" (arg1), "r" (arg2) );
      
    if( !(x & 0x40000000) ){
      self->u.ival = res;
      return 0;
    }
  }
#else
  res = arg1 + arg2;
  if( !ADD_OVERFLOW(arg1, arg2, res) ){
    self->u.ival = res;
    return 0;
  }
#endif // LINUXPPC

  // overflow
#ifdef GMP
  PROMOTE(self);
  return -1;
#endif
  ksm_error("addition overflow");
  return 0;
}

static 
int int_mul(ksm_numrep *self, ksm_numrep *arg)
{
#ifdef KSM_LINUXPPC
  int arg1, arg2, res, x;

  arg1 = self->u.ival;
  arg2 = arg->u.ival;
  __asm__ ("mullwo %0,%2,%3; mfspr %1,1" 
	   : "=r" (res), "=r" (x)
	   : "r" (arg1), "r" (arg2) );
  if( !(x & 0x40000000) ){
    self->u.ival = res;
    return 0;
  }
#else
  long long arg1, arg2, res;

  arg1 = self->u.ival;
  arg2 = arg->u.ival;
  res = arg1 * arg2;
  if( res >= INT_MIN && res <= INT_MAX ){
    self->u.ival = res;
    return 0;
  }
#endif // LINUXPPC

  // overflow
#ifdef GMP
  PROMOTE(self);
  return -1;
#endif
  ksm_error("multiplication overflow");
  return 0;
}

#define SUB_OVERFLOW(arg1, arg2, result) \
( \
( (arg1) > 0 && (arg2) < 0 && (result) <= 0 ) || \
( (arg1) < 0 && (arg2) > 0 && (result) >= 0 ) \
)

static 
int int_sub(ksm_numrep *self, ksm_numrep *arg)
{
  int arg1, arg2, res;

  arg1 = self->u.ival;
  arg2 = arg->u.ival;

#ifdef KSM_LINUXPPC
  {
    int x;

    __asm__ ("subfo %0,%2,%3; mfspr %1,1" 
	     : "=r" (res), "=r" (x)
	     : "r" (arg2), "r" (arg1) );
    
    if( !(x & 0x40000000) ){
      self->u.ival = res;
      return 0;
    }
  }
#else
  res = arg1 - arg2;
  if( !SUB_OVERFLOW(arg1, arg2, res) ){
    self->u.ival = res;
    return 0;
  }
#endif // KSM_LINUXPPC

  // overflow
#ifdef GMP
  PROMOTE(self);
  return -1;
#endif
  ksm_error("subtraction overflow");
  return 0;
}

static 
int int_div(ksm_numrep *self, ksm_numrep *arg)
{
#ifdef GMP
  if( ksm_numrep_exact_p(arg) ){
    ksm_numrep_promote(self, KSM_NUMREP_MPQ);
    return -1;
  }
#endif
  int_exact_to_inexact(self);
  return -1;
}

static
int int_neg(ksm_numrep *self)
{
#ifdef KSM_LINUXPPC
  int arg, res, x;

  arg = self->u.ival;
  __asm__ ("nego %0,%2; mfspr %1,1" :
	   "=r" (res), "=r" (x) :
	   "r" (arg) );
  if( !(x & 0x40000000) ){
    self->u.ival = res;
    return 0;
  }
  ksm_error("neg overflow");
#else  
  if( self->u.ival != INT_MIN ){
    self->u.ival = -self->u.ival;
    return 0;
  }
#endif // KSM_LINUXPPC

  // overflow
#ifdef GMP
  PROMOTE(self);
  return -1;
#endif
  ksm_error("negation overflow (int): %d", self->u.ival);
  return 0;
}

static
int int_quotient(ksm_numrep *self, ksm_numrep *arg)
{
#ifdef KSM_LINUXPPC
  int arg1, arg2, res, x;

  arg1 = self->u.ival;
  arg2 = arg->u.ival;
  __asm__ ("divwo %0,%2,%3; mfspr %1,1" :
	   "=r" (res), "=r" (x) :
	   "r" (arg1), "r" (arg2) );
  if( !(x & 0x40000000) ){
    self->u.ival = res;
    return 0;
  }
#else
  if( !(self->u.ival == INT_MIN && arg->u.ival == -1) ){
    self->u.ival /= arg->u.ival;
    return 0;
  }
#endif // KSM_LINUXPPC

  // overflow
#ifdef GMP
    PROMOTE(self);
    return -1;
#endif    
    ksm_error("quotient overflow (int): %d / %d", self->u.ival, arg->u.ival);
    return 0;
}

static
int int_remainder(ksm_numrep *self, ksm_numrep *arg)
{
  int iq;

  if( abs(arg->u.ival) == 1 ){
    self->u.ival = 0;
    return 0;
  }
  iq = self->u.ival / arg->u.ival;
  self->u.ival -= iq * arg->u.ival;
  return 0;
}

static
int int_modulo(ksm_numrep *self, ksm_numrep *arg)
{
  int_remainder(self, arg);
  if( (arg->u.ival < 0 && self->u.ival > 0) ||
      (arg->u.ival > 0 && self->u.ival < 0) )
    self->u.ival += arg->u.ival;
  return 0;
}

static
int gcd(int p, int q)
     // p > q >= 0
{
  int r;

  while( q ){
    r = p % q;
    p = q;
    q = r;
  }
  return p;
}

static
int int_gcd(ksm_numrep *self, ksm_numrep *arg)
{
  int p, q;

  p = self->u.ival;
  q = arg->u.ival;
  if( p == INT_MIN || q == INT_MIN ){
#ifdef GMP
    PROMOTE(self);
    return -1;
#endif
    ksm_error("integer overflow (int_gcd)");
  }
  p = abs(p);
  q = abs(q);
  self->u.ival = (p>q)?gcd(p,q):gcd(q,p);
  return 0;
}

static
int int_lcm(ksm_numrep *self, ksm_numrep *arg)
{
  int ip, iq, igcd;
  long long res;

  ip = self->u.ival;
  iq = arg->u.ival;
  if( ip == INT_MIN || iq == INT_MIN ){
#ifdef GMP
    PROMOTE(self);
    return -1;
#endif
    ksm_error("integer overflow (int_lcm)");
  }
  ip = abs(ip);
  iq = abs(iq);
  igcd = (ip>iq)?gcd(ip,iq):gcd(iq,ip);
  res = ip/igcd;
  res *= iq;
  if( res >= INT_MIN && res <= INT_MAX ){
    self->u.ival = res;
    return 0;
  }
  // overflow
#ifdef GMP
  PROMOTE(self);
  return -1;
#endif
  ksm_error("lcm overflow");
  return 0;
}

static
int expt_int(int p, int q) // p^q, p!=0, q>0
     // returns non-zero if success, 0 if failure
{
  long long z = p, pp = 1;

  while( q > 0 ){
    if( q & 1 ){
      pp *= z;
      if( !(pp >= INT_MIN && pp <= INT_MAX) )
	return 0;
    }
    z *= z;
    if( !(z >= INT_MIN && z <= INT_MAX) )
      return 0;
    q >>= 1;
  }
  return (int)pp;
}

static
int int_expt(ksm_numrep *self, ksm_numrep *arg)
{
  if( arg->u.ival == 0 ){
    self->u.ival = 1;
    return 0;
  }
  if( self->u.ival == 0 )
    return 0;
  if( arg->u.ival > 0 ){
    int i = expt_int(self->u.ival, arg->u.ival);
    if( i != 0 ){
      self->u.ival = i;
      return 0;
    }
    // overflow
#ifdef GMP
    PROMOTE(self);
    return -1;
#endif
    ksm_error("expt overflow (int): %d^%d", self->u.ival, arg->u.ival);
    return 0;
  }
  // arg->u.ival < 0
  int_exact_to_inexact(self);
  return -1;
}

static
int int_and(ksm_numrep *self, ksm_numrep *arg)
{
  self->u.ival &= arg->u.ival;
  return 0;
}

static
int int_or(ksm_numrep *self, ksm_numrep *arg)
{
  self->u.ival |= arg->u.ival;
  return 0;
}

static
int int_xor(ksm_numrep *self, ksm_numrep *arg)
{
  self->u.ival ^= arg->u.ival;
  return 0;
}

static
void int_complement(ksm_numrep *self)
{
  self->u.ival = ~self->u.ival;
}

#define HEAD     (1<<((sizeof(int)*8)-1))

static
int int_lshift(ksm_numrep *self, int n)
{
  int i;

  i = self->u.ival;
  while( n-- > 0){
#ifdef GMP
    if( i & HEAD ){
      PROMOTE(self);
      return -1;
    }
#endif
    i <<= 1;
  }
  self->u.ival = i;
  return 0;
}

static
int int_rshift(ksm_numrep *self, int n)
{
  self->u.ival >>= n;
  return 0;
}

ksm_numrep_op ksm_int_op = {   /*** GLOBAL ***/
  1,
  int_object,
  int_clone,
  int_promote,
  int_exact_to_inexact,
  int_inexact_to_exact,
  int_get_si,
  int_get_d,
  int_cmp,
  int_sgn,
  int_add,
  int_mul,
  int_sub,
  int_div,
  int_neg,
  int_quotient,
  int_remainder,
  int_modulo,
  int_gcd,
  int_lcm,
  int_expt,
  int_and,
  int_or,
  int_xor,
  int_complement,
  int_lshift,
  int_rshift
};
