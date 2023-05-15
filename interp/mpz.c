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
#include <math.h>
#include "ksm.h"

#ifdef GMP

static
ksm_t mpzop_object(ksm_numrep *self)
{
  if( mpz_cmp_si(self->u.zval, INT_MIN) >= 0 &&
      mpz_cmp_si(self->u.zval, INT_MAX) <= 0 ){
    int i = mpz_get_si(self->u.zval);
    mpz_clear(self->u.zval);
    return ksm_make_int(i);
  }
  return ksm_make_mpz(self->u.zval);
}

static
void mpzop_clone(ksm_numrep *self)
{
  mpz_ptr p;

  p = self->u.zval;
  self->u.zval = ksm_alloc(sizeof(mpz_t));
  mpz_init_set(self->u.zval, p);
}

static
void mpzop_promote(ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      int i = arg->u.ival;
      arg->kind = KSM_NUMREP_MPZ;
      arg->u.zval = ksm_alloc(sizeof(mpz_t));
      mpz_init_set_si(arg->u.zval, i);
      return;
    }
  default: ksm_error("can't happen (mpzop_promote): %d", arg->kind);
  }
}

static
void mpzop_exact_to_inexact(ksm_numrep *self)
{
  mpz_ptr zp = self->u.zval;

  self->kind = KSM_NUMREP_DOUBLE;
  self->u.dval = mpz_get_d(zp);
  mpz_clear(zp);
}

static
void mpzop_inexact_to_exact(ksm_numrep *self) { }

static
int mpzop_get_si(ksm_numrep *self)
{
  if( !(mpz_cmp_si(self->u.zval, INT_MIN) >= 0 &&
	mpz_cmp_si(self->u.zval, INT_MAX) <= 0) )
    ksm_error("integer overflow");
  return mpz_get_si(self->u.zval);
}

static
double mpzop_get_d(ksm_numrep *self)
{
  return mpz_get_d(self->u.zval);
}

static
int mpzop_cmp(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT: return mpz_cmp_si(self->u.zval, arg->u.ival);
  case KSM_NUMREP_MPZ:
    return mpz_cmp(self->u.zval, arg->u.zval) == 0;
  default:
    ksm_error("can't handle eq (mpz): %d", arg->kind);
    return 0;
  }
}

static
int mpzop_sgn(ksm_numrep *self)
{
  return mpz_sgn(self->u.zval);
}

static
int mpzop_add(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpz_t z;

      mpz_init_set_si(z, arg->u.ival);
      mpz_add(self->u.zval, self->u.zval, z);
      mpz_clear(z);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    mpz_add(self->u.zval, self->u.zval, arg->u.zval);
    return 0;
  default:
    ksm_error("can't handle addition (mpz): %d", arg->kind);
    return 0;
  }
}

static
int mpzop_mul(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpz_t z;

      mpz_init_set_si(z, arg->u.ival);
      mpz_mul(self->u.zval, self->u.zval, z);
      mpz_clear(z);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    mpz_mul(self->u.zval, self->u.zval, arg->u.zval);
    return 0;
  default:
    ksm_error("can't handle multiplication (mpz): %d", arg->kind);
    return 0;
  }
}

static
int mpzop_sub(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpz_t z;

      mpz_init_set_si(z, arg->u.ival);
      mpz_sub(self->u.zval, self->u.zval, z);
      mpz_clear(z);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    mpz_sub(self->u.zval, self->u.zval, arg->u.zval);
    return 0;
  default:
    ksm_error("can't handle subtraction (mpz): %d", arg->kind);
    return 0;
  }
}

static
int mpzop_div(ksm_numrep *self, ksm_numrep *arg)
{
#ifdef GMP
  if( ksm_numrep_exact_p(arg) ){
    ksm_numrep_promote(self, KSM_NUMREP_MPQ);
    return -1;
  }
#endif
  mpzop_exact_to_inexact(self);
  return -1;
}

static
int mpzop_neg(ksm_numrep *self)
{
  mpz_neg(self->u.zval, self->u.zval);
  return 0;
}

static
int mpzop_quotient(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpz_t z;

      mpz_init_set_si(z, arg->u.ival);
      mpz_tdiv_q(self->u.zval, self->u.zval, z);
      mpz_clear(z);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    mpz_tdiv_q(self->u.zval, self->u.zval, arg->u.zval);
    return 0;
  default: ksm_error("can't handle quotient (mpz): %d", arg->kind);
    return 0;
  }
}

static
int mpzop_remainder(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpz_t z;

      mpz_init_set_si(z, arg->u.ival);
      mpz_tdiv_r(self->u.zval, self->u.zval, z);
      mpz_clear(z);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    mpz_tdiv_r(self->u.zval, self->u.zval, arg->u.zval);
    return 0;
  default: ksm_error("can't handle remainder (mpz): %d", arg->kind);
    return 0;
  }
}

static
int mpzop_modulo(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpz_t z;

      mpz_init_set_si(z, arg->u.ival);
      mpz_fdiv_r(self->u.zval, self->u.zval, z);
      mpz_clear(z);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    mpz_fdiv_r(self->u.zval, self->u.zval, arg->u.zval);
    return 0;
  default: ksm_error("can't handle modulo (mpz): %d", arg->kind);
    return 0;
  }
}

static
int mpzop_gcd(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpz_t z;

      mpz_init_set_si(z, arg->u.ival);
      mpz_gcd(self->u.zval, self->u.zval, z);
      mpz_clear(z);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    mpz_gcd(self->u.zval, self->u.zval, arg->u.zval);
    return 0;
  default:
    ksm_error("can't handle gcd (mpz): %d", arg->kind);
    return 0;
  }
}

static
void lcm_mpz(mpz_ptr ROP, mpz_ptr OP1, mpz_ptr OP2)
{
  mpz_t gcd;

  mpz_init(gcd);
  mpz_gcd(gcd, OP1, OP2);
  mpz_set(ROP, OP1);
  mpz_divexact(ROP, ROP, gcd);
  mpz_mul(ROP, ROP, OP2);
  mpz_clear(gcd);
}

static
int mpzop_lcm(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpz_t z;

      mpz_init_set_si(z, arg->u.ival);
      lcm_mpz(self->u.zval, self->u.zval, z);
      mpz_clear(z);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    lcm_mpz(self->u.zval, self->u.zval, arg->u.zval);
    return 0;
  default:
    ksm_error("can't handle lcm (mpz): %d", arg->kind);
    return 0;
  }
}

static
void expt_mpz(mpz_ptr p, mpz_ptr q) // p^q   (q >= 0)
{
  mpz_t z;

  mpz_init_set(z, p);
  mpz_set_si(p, 1);
  while( mpz_sgn(q) > 0 ){
    if( mpz_get_ui(q) & 1 )
      mpz_mul(p, p, z);
    mpz_mul(z, z, z);
    mpz_tdiv_q_2exp(q, q, 1);
  }
  mpz_clear(z);
}

static
int mpzop_expt(ksm_numrep *self, ksm_numrep *arg)
{
  if( mpz_sgn(self->u.zval) == 0 ){
    mpz_set_si(self->u.zval, ksm_numrep_zero_p(arg)?1:0);
    return 0;
  }
  if( ksm_numrep_negative_p(arg) ){
    mpzop_exact_to_inexact(self);
    return -1;
  }
  // both self and arg are positive
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpz_t z;

      mpz_init_set_si(z, arg->u.ival);
      expt_mpz(self->u.zval, z);
      mpz_clear(z);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    expt_mpz(self->u.zval, arg->u.zval);
    return 0;
  default: ksm_error("can't handle expt (mpz): %d", arg->kind);
    return 0;
  }
}

static
int mpzop_and(ksm_numrep *self, ksm_numrep *arg)
{
  if( !(self->kind == arg->kind) )
    ksm_error("can't happen in mpzop_and");
  mpz_and(self->u.zval, self->u.zval, arg->u.zval);
  return 0;
}

static
int mpzop_or(ksm_numrep *self, ksm_numrep *arg)
{
  if( !(self->kind == arg->kind) )
    ksm_error("can't happen in mpzop_or");
  mpz_ior(self->u.zval, self->u.zval, arg->u.zval);
  return 0;
}

static
int mpzop_xor(ksm_numrep *self, ksm_numrep *arg)
{
  mpz_t x, y, not_x, not_y;

  if( !(self->kind == arg->kind) )
    ksm_error("can't happen in mpzop_xor");
  mpz_init_set(x, self->u.zval);
  mpz_init_set(y, arg->u.zval);
  mpz_init(not_x);
  mpz_com(not_x, x);
  mpz_init(not_y);
  mpz_com(not_y, y);
  mpz_and(x, x, y);
  mpz_and(not_x, not_x, not_y);
  mpz_ior(self->u.zval, x, not_x);
  mpz_clear(x);
  mpz_clear(y);
  mpz_clear(not_x);
  mpz_clear(not_y);
  return 0;
}

static
void mpzop_complement(ksm_numrep *self)
{
  mpz_com(self->u.zval, self->u.zval);
}

static
int mpzop_lshift(ksm_numrep *self, int n)
{
  mpz_mul_2exp(self->u.zval, self->u.zval, n);
  return 0;
}

static
int mpzop_rshift(ksm_numrep *self, int n)
{
  mpz_tdiv_q_2exp(self->u.zval, self->u.zval, n);
  return 0;
}

ksm_numrep_op ksm_mpz_op = {   /*** GLOBAL ***/
  1,
  mpzop_object,
  mpzop_clone,
  mpzop_promote,
  mpzop_exact_to_inexact,
  mpzop_inexact_to_exact,
  mpzop_get_si,
  mpzop_get_d,
  mpzop_cmp,
  mpzop_sgn,
  mpzop_add,
  mpzop_mul,
  mpzop_sub,
  mpzop_div,
  mpzop_neg,
  mpzop_quotient,
  mpzop_remainder,
  mpzop_modulo,
  mpzop_gcd,
  mpzop_lcm,
  mpzop_expt,
  mpzop_and,
  mpzop_or,
  mpzop_xor,
  mpzop_complement,
  mpzop_lshift,
  mpzop_rshift
};

#endif   // GMP

