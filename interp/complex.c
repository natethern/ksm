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

#include <complex.h>
#include <math.h>
#include "ksm.h"

typedef ksm_numrep numrep;

#define CX(rep)   (*(__complex__ double *)(rep)->u.pval)

static
void init_cx(numrep *p)
{
  p->kind = KSM_NUMREP_COMPLEX;
  p->u.pval = ksm_alloc(sizeof(__complex__ double));
}

static
__complex__ double get_cx(numrep *arg)
{
  switch(arg->kind){
  case KSM_NUMREP_INT: return arg->u.ival+0i;
#ifdef GMP
  case KSM_NUMREP_MPZ: return mpz_get_d(arg->u.zval)+0i;
  case KSM_NUMREP_MPQ: return mpq_get_d(arg->u.pval)+0i;
#endif
  case KSM_NUMREP_DOUBLE: return arg->u.dval+0i;
  case KSM_NUMREP_COMPLEX: return CX(arg);
  default: ksm_error("can't happen (get_cx): %d", arg->kind);
    return 0+0i;
  }
}

static
ksm_t cx_object(numrep *self)
{
  return ksm_make_complex(__real__ CX(self), __imag__ CX(self));
}

static
void cx_clone(numrep *self)
{
  void *save = self->u.pval;

  self->u.pval = ksm_alloc(sizeof(__complex__ double));
  memcpy(self->u.pval, save, sizeof(__complex__ double));
}

static
void cx_promote(numrep *p)
{
  double r;

  switch(p->kind){
  case KSM_NUMREP_INT:
    r = p->u.ival;
    break;
#ifdef GMP
  case KSM_NUMREP_MPZ:
    r = mpz_get_d(p->u.zval);
    mpz_clear(p->u.zval);
    break;
  case KSM_NUMREP_MPQ:
    r = mpq_get_d(p->u.pval);
    mpq_clear(p->u.pval);
    break;
#endif
  case KSM_NUMREP_DOUBLE:
    r = p->u.dval;
    break;
  default:
    r = 0.0;
    ksm_error("can't happen (cx_promote): %d", p->kind);
  }
  init_cx(p);
  CX(p) = r+0i;
}

static void cx_exact_to_inexact(numrep *self) { }

static
void cx_inexact_to_exact(numrep *self)
{
  ksm_error("can't convert complex number to exact number");
}

static
int cx_get_si(numrep *self)
{
  ksm_error("can't convert complex to int");
  return 0;
}

static
double cx_get_d(numrep *self)
{
  ksm_error("can't convert complex to double");
  return 0;
}

static
int cx_cmp(numrep *self, numrep *arg)
{
  ksm_error("can't compare complex number(s)");
  return 0;
}

static
int cx_sgn(numrep *self)
{
  ksm_error("can't get sign of complex number");
  return 0;
}

static
int cx_add(numrep *self, numrep *arg)
{
  CX(self) += get_cx(arg);
  return 0;
}

static
int cx_mul(numrep *self, numrep *arg)
{
  CX(self) *= get_cx(arg);
  return 0;
}

static
int cx_sub(numrep *self, numrep *arg)
{
  CX(self) -= get_cx(arg);
  return 0;
}

static
int cx_div(numrep *self, numrep *arg)
{
  CX(self) /= get_cx(arg);
  return 0;
}

static
int cx_neg(numrep *self)
{
  CX(self) = -CX(self);
  return 0;
}

static
int cx_quotient(numrep *self, numrep *arg)
{
  ksm_error("can't calculate quotient of complex number(s)");
  return 0;
}

static
int cx_remainder(numrep *self, numrep *arg)
{
  ksm_error("can't calculate remainder of complex number(s)");
  return 0;
}

static
int cx_modulo(numrep *self, numrep *arg)
{
  ksm_error("can't calculate modulo of complex number(s)");
  return 0;
}

static
int cx_gcd(numrep *self, numrep *arg)
{
  ksm_error("can't calculate gcd of complex number(s)");
  return 0;
}

static
int cx_lcm(numrep *self, numrep *arg)
{
  ksm_error("can't calculate lcm of complex number(s)");
  return 0;
}

static
int cx_expt(numrep *self, numrep *arg)
{
  CX(self) = cpow(CX(self), get_cx(arg));
  return 0;
}

static
int cx_and(numrep *self, numrep *arg)
{
  ksm_error("can't happen (cx_and)");
  return 0;
}

static
int cx_or(numrep *self, numrep *arg)
{
  ksm_error("can't happen (cx_or)");
  return 0;
}

static
int cx_xor(numrep *self, numrep *arg)
{
  ksm_error("can't happen (cx_xor)");
  return 0;
}

static
void cx_complement(numrep *self)
{
  ksm_error("can't happen (cx_complement)");
}

static
int cx_lshift(numrep *self, int n)
{
  ksm_error("can't happen (cx_lshift)");
  return 0;
}

static
int cx_rshift(numrep *self, int n)
{
  ksm_error("can't happen (cx_rshift)");
  return 0;
}

static
void cx_floor(numrep *self)
{
  ksm_error("can't calculate floor of complex number");
}

static
void cx_ceiling(numrep *self)
{
  ksm_error("can't calculate ceiling of complex number");
}

static
void cx_truncate(numrep *self)
{
  ksm_error("can't calculate truncate of complex number");
}

static
void cx_round(numrep *self)
{
  ksm_error("can't calculate round of complex number");
}

static
void cx_exp(numrep *self)
{
  CX(self) = cexp(CX(self));
}

static
void cx_log(numrep *self)
{
  CX(self) = clog(CX(self));
}

static
void cx_sin(numrep *self)
{
  CX(self) = csin(CX(self));
}

static
void cx_cos(numrep *self)
{
  CX(self) = ccos(CX(self));
}

static
void cx_tan(numrep *self)
{
  CX(self) = ctan(CX(self));
}

static
void cx_asin(numrep *self)
{
  CX(self) = casin(CX(self));
}

static
void cx_acos(numrep *self)
{
  CX(self) = cacos(CX(self));
}

static
void cx_atan(numrep *self)
{
  CX(self) = catan(CX(self));
}

static
void cx_atan2(numrep *self, numrep *arg)
{
  ksm_error("can't calculate atan2 of complex number(s)");
}

static
void cx_sqrt(numrep *self)
{
  CX(self) = csqrt(CX(self));
}

ksm_numrep_op ksm_complex_op = {  /*** GLOBAL ***/
  0,
  cx_object,
  cx_clone,
  cx_promote,
  cx_exact_to_inexact,
  cx_inexact_to_exact,
  cx_get_si,
  cx_get_d,
  cx_cmp,
  cx_sgn,
  cx_add,
  cx_mul,
  cx_sub,
  cx_div,
  cx_neg,
  cx_quotient,
  cx_remainder,
  cx_modulo,
  cx_gcd,
  cx_lcm,
  cx_expt,
  cx_and,
  cx_or,
  cx_xor,
  cx_complement,
  cx_lshift,
  cx_rshift,
  cx_floor,
  cx_ceiling,
  cx_truncate,
  cx_round,
  cx_exp,
  cx_log,
  cx_sin,
  cx_cos,
  cx_tan,
  cx_asin,
  cx_acos,
  cx_atan,
  cx_atan2,
  cx_sqrt
};
