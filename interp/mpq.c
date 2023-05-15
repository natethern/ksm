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
#include "ksm.h"

#ifdef GMP

static
ksm_t mpqop_object(ksm_numrep *self)
{
  if( mpz_cmp_si(mpq_denref(self->u.qval), 1) == 0 ){
    mpz_ptr zp;

    zp = ksm_alloc(sizeof(mpz_t));
    mpz_init(zp);
    mpz_set(zp, mpq_numref(self->u.qval));
    mpq_clear(self->u.qval);
    self->kind = KSM_NUMREP_MPZ;
    self->u.zval = zp;
    return ksm_numrep_to_object(self);
  }
  return ksm_make_mpq(self->u.qval);
}

static
void mpqop_clone(ksm_numrep *self)
{
  mpq_ptr p;

  p = self->u.qval;
  self->u.qval = ksm_alloc(sizeof(mpq_t));
  mpq_init(self->u.qval);
  mpq_set(self->u.qval, p);
}

static
void mpqop_promote(ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      int i = arg->u.ival;
      arg->kind = KSM_NUMREP_MPQ;
      arg->u.qval = ksm_alloc(sizeof(mpq_t));
      mpq_init(arg->u.qval);
      mpq_set_si(arg->u.qval, i, 1);
      return;
    }
  case KSM_NUMREP_MPZ:
    {
      mpz_ptr z = arg->u.zval;
      arg->kind = KSM_NUMREP_MPQ;
      arg->u.qval = ksm_alloc(sizeof(mpq_t));
      mpq_init(arg->u.qval);
      mpq_set_z(arg->u.qval, z);
      mpz_clear(z);
      return;
    }
  default:
    ksm_error("can't happen (mpqop_promote): %d", arg->kind);
  }
}

static
void mpqop_exact_to_inexact(ksm_numrep *self)
{
  mpq_ptr qp = self->u.qval;

  self->kind = KSM_NUMREP_DOUBLE;
  self->u.dval = mpq_get_d(qp);
  mpq_clear(qp);
}

static void mpqop_inexact_to_exact(ksm_numrep *self) { }

static
int mpqop_get_si(ksm_numrep *self)
{
  mpz_t p;
  int i;

  mpz_init(p);
  mpz_tdiv_q(p, mpq_numref(self->u.qval), mpq_denref(self->u.qval));
  if( !(mpz_cmp_si(p, INT_MIN) >= 0 &&
	mpz_cmp_si(p, INT_MAX) <= 0) )
    ksm_error("integer overflow");
  i = mpz_get_si(p);
  mpz_clear(p);
  return i;
}

static
double mpqop_get_d(ksm_numrep *self)
{
  return mpq_get_d(self->u.qval);
}

static
int mpqop_cmp(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT: 
    {
      mpq_t q;
      int cmp;

      mpq_init(q);
      mpq_set_si(q, arg->u.ival, 1);
      cmp = mpq_cmp(self->u.qval, q);
      mpq_clear(q);
      return cmp;
    }
  case KSM_NUMREP_MPZ:
    {
      mpq_t q;
      int cmp;

      mpq_init(q);
      mpq_set_z(q, arg->u.zval);
      cmp = mpq_cmp(self->u.qval, q);
      mpq_clear(q);
      return cmp;
    }
  case KSM_NUMREP_MPQ:
    return mpq_cmp(self->u.qval, arg->u.qval);
  default:
    ksm_error("can't handle eq (mpz): %d", arg->kind);
    return 0;
  }
}

static
int mpqop_sgn(ksm_numrep *self)
{
  return mpq_sgn(self->u.qval);
}

static
int mpqop_add(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpq_t q;

      mpq_init(q);
      mpq_set_si(q, arg->u.ival, 1);
      mpq_add(self->u.qval, self->u.qval, q);
      mpq_clear(q);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    {
      mpq_t q;

      mpq_init(q);
      mpq_set_z(q, arg->u.zval);
      mpq_add(self->u.qval, self->u.qval, q);
      mpq_clear(q);
      return 0;
    }
  case KSM_NUMREP_MPQ:
    mpq_add(self->u.qval, self->u.qval, arg->u.qval);
    return 0;
  default:
    ksm_error("can't handle addition (mpq): %d", arg->kind);
    return 0;
  }
}

static
int mpqop_mul(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpq_t q;

      mpq_init(q);
      mpq_set_si(q, arg->u.ival, 1);
      mpq_mul(self->u.qval, self->u.qval, q);
      mpq_clear(q);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    {
      mpq_t q;

      mpq_init(q);
      mpq_set_z(q, arg->u.zval);
      mpq_mul(self->u.qval, self->u.qval, q);
      mpq_clear(q);
      return 0;
    }
  case KSM_NUMREP_MPQ:
    mpq_mul(self->u.qval, self->u.qval, arg->u.qval);
    return 0;
  default:
    ksm_error("can't handle multiplication (mpq): %d", arg->kind);
    return 0;
  }
}

static
int mpqop_sub(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpq_t q;

      mpq_init(q);
      mpq_set_si(q, arg->u.ival, 1);
      mpq_sub(self->u.qval, self->u.qval, q);
      mpq_clear(q);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    {
      mpq_t q;

      mpq_init(q);
      mpq_set_z(q, arg->u.zval);
      mpq_sub(self->u.qval, self->u.qval, q);
      mpq_clear(q);
      return 0;
    }
  case KSM_NUMREP_MPQ:
    mpq_sub(self->u.qval, self->u.qval, arg->u.qval);
    return 0;
  default:
    ksm_error("can't handle subtraction (mpq): %d", arg->kind);
    return 0;
  }
}

static
int mpqop_div(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpq_t q;

      mpq_init(q);
      mpq_set_si(q, arg->u.ival, 1);
      mpq_div(self->u.qval, self->u.qval, q);
      mpq_clear(q);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    {
      mpq_t q;

      mpq_init(q);
      mpq_set_z(q, arg->u.zval);
      mpq_div(self->u.qval, self->u.qval, q);
      mpq_clear(q);
      return 0;
    }
  case KSM_NUMREP_MPQ:
    mpq_div(self->u.qval, self->u.qval, arg->u.qval);
    return 0;
  default:
    ksm_error("can't handle subtraction (mpq): %d", arg->kind);
    return 0;
  }
}

static
int mpqop_neg(ksm_numrep *self)
{
  mpq_neg(self->u.qval, self->u.qval);
  return 0;
}

static
void do_quotient(ksm_numrep *self, mpq_ptr qp)
     // self is MPQ
{
  mpz_ptr zp;

  mpq_div(self->u.qval, self->u.qval, qp);
  zp = ksm_alloc(sizeof(mpz_t));
  mpz_init(zp);
  mpz_tdiv_q(zp, mpq_numref(self->u.qval), mpq_denref(self->u.qval));
  mpq_clear(self->u.qval);
  self->kind = KSM_NUMREP_MPZ;
  self->u.zval = zp;
}

static
int mpqop_quotient(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpq_t q;

      mpq_init(q);
      mpq_set_si(q, arg->u.ival, 1);
      do_quotient(self, q);
      mpq_clear(q);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    {
      mpq_t q;

      mpq_init(q);
      mpq_set_z(q, arg->u.zval);
      do_quotient(self, q);
      mpq_clear(q);
      return 0;
    }
  case KSM_NUMREP_MPQ:
    do_quotient(self, arg->u.qval);
    return 0;
  default: ksm_error("can't handle quotient (mpq): %d", arg->kind);
    return 0;
  }
}

static
void do_remainder(ksm_numrep *self, mpq_ptr qp)
{
  mpq_div(self->u.qval, self->u.qval, qp);
  mpz_tdiv_r(mpq_numref(self->u.qval), mpq_numref(self->u.qval), 
	     mpq_denref(self->u.qval));
}

static
int mpqop_remainder(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpq_t q;

      mpq_init(q);
      mpq_set_si(q, arg->u.ival, 1);
      do_remainder(self, q);
      mpq_clear(q);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    {
      mpq_t q;

      mpq_init(q);
      mpq_set_z(q, arg->u.zval);
      do_remainder(self, q);
      mpq_clear(q);
      return 0;
    }
  case KSM_NUMREP_MPQ:
    do_remainder(self, arg->u.qval);
    return 0;
  default:
    ksm_error("can't handle remainder (mpq): %d", arg->kind);
    return 0;
  }
}

static
void do_modulo(ksm_numrep *self, mpq_ptr qp)
{
  mpq_div(self->u.qval, self->u.qval, qp);
  mpz_fdiv_r(mpq_numref(self->u.qval), mpq_numref(self->u.qval), 
	     mpq_denref(self->u.qval));
}

static
int mpqop_modulo(ksm_numrep *self, ksm_numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpq_t q;

      mpq_init(q);
      mpq_set_si(q, arg->u.ival, 1);
      do_modulo(self, q);
      mpq_clear(q);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    {
      mpq_t q;

      mpq_init(q);
      mpq_set_z(q, arg->u.zval);
      do_modulo(self, q);
      mpq_clear(q);
      return 0;
    }
  case KSM_NUMREP_MPQ:
    do_modulo(self, arg->u.qval);
    return 0;
  default:
    ksm_error("can't handle remainder (mpq): %d", arg->kind);
    return 0;
  }
}

static
int mpqop_gcd(ksm_numrep *self, ksm_numrep *arg)
{
  ksm_error("can't calculate gcd of non-integral number(s)");
  return 0;
}

static
int mpqop_lcm(ksm_numrep *self, ksm_numrep *arg)
{
  ksm_error("can't calculate lcm of non-integral number(s)");
  return 0;
}

static
void expt_mpq(mpq_ptr p, mpz_ptr e) // p^q   (q >= 0)
{
  mpq_t pp;

  mpq_init(pp);
  mpq_set(pp, p);
  mpq_set_si(p, 1, 1);
  while( mpz_sgn(e) > 0 ){
    if( mpz_get_ui(e) & 1 )
      mpq_mul(p, p, pp);
    mpq_mul(pp, pp, pp);
    mpz_tdiv_q_2exp(e, e, 1);
  }
  mpq_clear(pp);
}

static
int mpqop_expt(ksm_numrep *self, ksm_numrep *arg)
{
  if( mpz_sgn(mpq_numref(self->u.qval)) == 0 ){
    mpq_set_si(self->u.qval, ksm_numrep_zero_p(arg)?1:0, 1);
    return 0;
  }
  if( ksm_numrep_negative_p(arg) ){
    mpqop_exact_to_inexact(self);
    return -1;
  }
  switch(arg->kind){
  case KSM_NUMREP_INT:
    {
      mpz_t e;

      mpz_init_set_si(e, arg->u.ival);
      expt_mpq(self->u.qval, e);
      mpz_clear(e);
      return 0;
    }
  case KSM_NUMREP_MPZ:
    expt_mpq(self->u.qval, arg->u.zval);
    return 0;
  case KSM_NUMREP_MPQ:
    mpqop_exact_to_inexact(self);
    return -1;
  default:
    ksm_error("can't handle expt (mpq): %d", arg->kind);
    return 0;
  }
}

static
int mpqop_and(ksm_numrep *self, ksm_numrep *arg)
{
  ksm_error("can't happen (mpqop_and)");
  return 0;
}

static
int mpqop_or(ksm_numrep *self, ksm_numrep *arg)
{
  ksm_error("can't happen (mpqop_or)");
  return 0;
}

static
int mpqop_xor(ksm_numrep *self, ksm_numrep *arg)
{
  ksm_error("can't happen (mpqop_xor)");
  return 0;
}

static
void mpqop_complement(ksm_numrep *self)
{
  ksm_error("can't happen (mpqop_complement)");
}

static
int mpqop_lshift(ksm_numrep *self, int n)
{
  ksm_error("can't happen (mpqop_lshift)");
  return 0;
}

static
int mpqop_rshift(ksm_numrep *self, int n)
{
  ksm_error("can't happen (mpqop_rshift)");
  return 0;
}

ksm_numrep_op ksm_mpq_op = {  /*** GLOBAL ***/
  1,
  mpqop_object,
  mpqop_clone,
  mpqop_promote,
  mpqop_exact_to_inexact,
  mpqop_inexact_to_exact,
  mpqop_get_si,
  mpqop_get_d,
  mpqop_cmp,
  mpqop_sgn,
  mpqop_add,
  mpqop_mul,
  mpqop_sub,
  mpqop_div,
  mpqop_neg,
  mpqop_quotient,
  mpqop_remainder,
  mpqop_modulo,
  mpqop_gcd,
  mpqop_lcm,
  mpqop_expt,
  mpqop_and,
  mpqop_or,
  mpqop_xor,
  mpqop_complement,
  mpqop_lshift,
  mpqop_rshift
};

#endif // GMP
