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

static
double arg_get_d(ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT: return arg->u.ival;
#ifdef GMP
  case KSM_NUMREP_MPZ: return mpz_get_d(arg->u.zval);
  case KSM_NUMREP_MPQ: return mpq_get_d(arg->u.qval);
#endif
  case KSM_NUMREP_DOUBLE: return arg->u.dval;
  default: ksm_error("can't happen (arg_get_d): %d", arg->kind);
    return 0.0;
  }
}

static
ksm_t double_object(ksm_numrep *self)
{
  return ksm_make_double(self->u.dval);
}

static void double_clone(ksm_numrep *self) { }

static
void double_promote(ksm_numrep *p)
{
  switch(p->kind){
  case KSM_NUMREP_INT:
    p->kind = KSM_NUMREP_DOUBLE;
    p->u.dval = p->u.ival;
    break;
#ifdef GMP
  case KSM_NUMREP_MPZ:
    {
      mpz_ptr zp = p->u.zval;
      p->kind = KSM_NUMREP_DOUBLE;
      p->u.dval = mpz_get_d(zp);
      mpz_clear(zp);
      break;
    }
  case KSM_NUMREP_MPQ:
    {
      mpq_ptr qp = p->u.qval;
      p->kind = KSM_NUMREP_DOUBLE;
      p->u.dval = mpq_get_d(qp);
      mpq_clear(qp);
      break;
    }
#endif
  case KSM_NUMREP_DOUBLE: return;
  default: ksm_error("can't happen (double_promote): %d", p->kind);
  }
}

static void double_exact_to_inexact(ksm_numrep *self) { }

static void double_inexact_to_exact(ksm_numrep *self)
{
  double d;

  d = self->u.dval;
  if( d >= INT_MIN && d <= INT_MAX ){
    self->kind = KSM_NUMREP_INT;
    self->u.ival = d;
    return;
  }
  // overflow
#ifdef GMP
  self->kind = KSM_NUMREP_MPZ;
  self->u.zval = ksm_alloc(sizeof(mpz_t));
  mpz_init_set_d(self->u.zval, d);
  return;
#endif
  ksm_error("inexact->exact overflow: %g", d);
}

static
int double_get_si(ksm_numrep *self)
{
  double d = self->u.dval;

  if( !(d >= INT_MIN && d <= INT_MAX) )
    ksm_error("can't convert double to integer: %g", d);
  return (int)d;
}

static
double double_get_d(ksm_numrep *self)
{
  return self->u.dval;
}

static
int double_cmp(ksm_numrep *self, ksm_numrep *arg)
{
  double x, y;

  x = self->u.dval;
  y = arg_get_d(arg);
  if( x == y )
    return 0;
  if( x < y )
    return -1;
  if( x > y )
    return 1;
  ksm_error("can't compare double: %g, %g", x, y);
  return 0;
}

static
int double_sgn(ksm_numrep *self)
{
  double d;

  d = self->u.dval;
  if( d == 0.0 )
    return 0;
  if( d > 0.0 )
    return 1;
  if( d < 0.0 )
    return -1;
  ksm_error("can't get sign of double: %g", d);
  return 0;
}

static 
int double_add(ksm_numrep *self, ksm_numrep *arg)
{
  self->u.dval += arg_get_d(arg);
  return 0;
}

static 
int double_mul(ksm_numrep *self, ksm_numrep *arg)
{
  self->u.dval *= arg_get_d(arg);
  return 0;
}

static 
int double_sub(ksm_numrep *self, ksm_numrep *arg)
{
  self->u.dval -= arg_get_d(arg);
  return 0;
}

static 
int double_div(ksm_numrep *self, ksm_numrep *arg)
{
  self->u.dval /= arg_get_d(arg);
  return 0;
}

static
int double_neg(ksm_numrep *self)
{
  self->u.dval = -self->u.dval;
  return 0;
}

static
int double_quotient(ksm_numrep *self, ksm_numrep *arg)
{
  double q;

  q = self->u.dval / arg_get_d(arg);
  if( q >= INT_MIN && q <= INT_MAX ){
    self->kind = KSM_NUMREP_INT;
    self->u.ival = q;
    return 0;
  }
  // overflow
#ifdef GMP
  self->kind = KSM_NUMREP_MPZ;
  self->u.zval = ksm_alloc(sizeof(mpz_t));
  mpz_init_set_d(self->u.zval, q);
  return 0;
#endif
  ksm_error("quotient overflow: %d / %d", self->u.dval, arg_get_d(arg));
  return 0;
}

static
int double_remainder(ksm_numrep *self, ksm_numrep *arg)
{
  self->u.dval = fmod(self->u.dval, arg_get_d(arg));
  return 0;
}

static
int double_modulo(ksm_numrep *self, ksm_numrep *arg)
{
  double x, y, r;

  x = self->u.dval;
  y = arg_get_d(arg);
  r = fmod(x, y);
  if( (y < 0.0 && r > 0.0) ||
      (y > 0.0 && r < 0.0) )
    r += y;
  self->u.dval = r;
  return 0;
}

static
int double_gcd(ksm_numrep *self, ksm_numrep *arg)
{
  ksm_error("can't calculate gcd of non-integral number:");
  return 0;
}

static
int double_lcm(ksm_numrep *self, ksm_numrep *arg)
{
  ksm_error("can't calculate lcm of non-integral number:");
  return 0;
}

static
int double_and(ksm_numrep *self, ksm_numrep *arg)
{
  ksm_error("can't happen (double_and)");
  return 0;
}

static
int double_or(ksm_numrep *self, ksm_numrep *arg)
{
  ksm_error("can't happen (double_or)");
  return 0;
}

static
int double_xor(ksm_numrep *self, ksm_numrep *arg)
{
  ksm_error("can't happen (double_xor)");
  return 0;
}

static
void double_complement(ksm_numrep *self)
{
  ksm_error("can't happen (double_complement)");
}

static
int double_lshift(ksm_numrep *self, int n)
{
  ksm_error("can't happen (double_lshift)");
  return 0;
}

static
int double_rshift(ksm_numrep *self, int n)
{
  ksm_error("can't happen (double_rshift)");
  return 0;
}

static
int double_expt(ksm_numrep *self, ksm_numrep *arg)
{
  self->u.dval = pow(self->u.dval, arg_get_d(arg));
  return 0;
}

static
void double_floor(ksm_numrep *self) 
{ 
  self->u.dval = floor(self->u.dval);
}

static
void double_ceiling(ksm_numrep *self)
{ 
  self->u.dval = ceil(self->u.dval);
}

static
void double_truncate(ksm_numrep *self)
{ 
  self->u.dval = (double)((int)self->u.dval);
}

static
void double_round(ksm_numrep *self)
{ 
  self->u.dval = rint(self->u.dval);
}

static
void double_exp(ksm_numrep *self)
{
  self->u.dval = exp(self->u.dval);
}

static
void double_log(ksm_numrep *self)
{
  self->u.dval = log(self->u.dval);
}

static
void double_sin(ksm_numrep *self)
{
  self->u.dval = sin(self->u.dval);
}

static
void double_cos(ksm_numrep *self)
{
  self->u.dval = cos(self->u.dval);
}

static
void double_tan(ksm_numrep *self)
{
  self->u.dval = tan(self->u.dval);
}

static
void double_asin(ksm_numrep *self)
{
  self->u.dval = asin(self->u.dval);
}

static
void double_acos(ksm_numrep *self)
{
  self->u.dval = acos(self->u.dval);
}

static
void double_atan(ksm_numrep *self)
{
  self->u.dval = atan(self->u.dval);
}

static
void double_atan2(ksm_numrep *self, ksm_numrep *arg)
{
  self->u.dval = atan2(self->u.dval, arg_get_d(arg));
}

static
void double_sqrt(ksm_numrep *self)
{
  self->u.dval = sqrt(self->u.dval);
}

ksm_numrep_op ksm_double_op = {   /*** GLOBAL ***/
  0,
  double_object,
  double_clone,
  double_promote,
  double_exact_to_inexact,
  double_inexact_to_exact,
  double_get_si,
  double_get_d,
  double_cmp,
  double_sgn,
  double_add,
  double_mul,
  double_sub,
  double_div,
  double_neg,
  double_quotient,
  double_remainder,
  double_modulo,
  double_gcd,
  double_lcm,
  double_expt,
  double_and,
  double_or,
  double_xor,
  double_complement,
  double_lshift,
  double_rshift,
  double_floor,
  double_ceiling,
  double_truncate,
  double_round,
  double_exp,
  double_log,
  double_sin,
  double_cos,
  double_tan,
  double_asin,
  double_acos,
  double_atan,
  double_atan2,
  double_sqrt
};
