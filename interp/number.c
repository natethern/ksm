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

#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include "ksm.h"

/*** numrep *********************************************************/

typedef int (*numcvt_fun)(ksm_numrep *, ksm_t);  // returns 0 if success
KSM_INSTALLER(numcvter, numcvt_fun)    /*** GLOBAL ***/
#define GET_NUMCVTER(x)   get_numcvter(ksm_flag(x))

int ksm_set_numrep(ksm_numrep *p, ksm_t x)
     // returns 0 if success, -1 if failure
{
  numcvt_fun f;

  f = GET_NUMCVTER(x);
  return f ? (*f)(p, x) : -1;
}

void ksm_numrep_set_int(ksm_numrep *p, int i)
{
  p->kind = KSM_NUMREP_INT;
  p->u.ival = i;
}

void ksm_numrep_set_longlong(ksm_numrep *p, long long ll)
{
  ksm_error("long long type not supported");
}

void ksm_numrep_set_double(ksm_numrep *p, double d)
{
  p->kind = KSM_NUMREP_DOUBLE;
  p->u.dval = d;
}

/*** op *************************************************************/

static ksm_numrep_op *ops[KSM_NUMREP_LAST] = { /*** GLOBAL ***/
  &ksm_int_op,
#ifdef GMP
  &ksm_mpz_op,
  &ksm_mpq_op,
#endif
  &ksm_double_op,
  &ksm_complex_op
};

int ksm_numrep_exact_p(ksm_numrep *p)
{
  return ops[p->kind]->exact_p;
}

void ksm_numrep_clone(ksm_numrep *p)
{
  (*ops[p->kind]->clone)(p);
}

void ksm_numrep_promote(ksm_numrep *p, int kind)
{
  (*ops[kind]->promote)(p);
}

#define PROMOTE(p,q) \
  do{ if( p->kind < q->kind ) \
        (*ops[q->kind]->promote)(p); \
  } while(0)

#define PROMOTE_BOTH(p,q) \
  do{ if( p->kind < q->kind ) \
        (*ops[q->kind]->promote)(p); \
      else if( p->kind > q->kind ) \
        (*ops[p->kind]->promote)(q); \
  } while(0)


int ksm_numrep_eq(ksm_numrep *p, ksm_numrep *q)
{
  PROMOTE(p,q);
  return (*ops[p->kind]->cmp)(p, q) == 0;
}

int ksm_numrep_lt(ksm_numrep *p, ksm_numrep *q)
{
  PROMOTE(p,q);
  return (*ops[p->kind]->cmp)(p, q) < 0;
}

int ksm_numrep_gt(ksm_numrep *p, ksm_numrep *q)
{
  PROMOTE(p,q);
  return (*ops[p->kind]->cmp)(p, q) > 0;
}

int ksm_numrep_le(ksm_numrep *p, ksm_numrep *q)
{
  PROMOTE(p,q);
  return (*ops[p->kind]->cmp)(p, q) <= 0;
}

int ksm_numrep_ge(ksm_numrep *p, ksm_numrep *q)
{
  PROMOTE(p,q);
  return (*ops[p->kind]->cmp)(p, q) >= 0;
}

int ksm_numrep_zero_p(ksm_numrep *p)
{
  return (*ops[p->kind]->sgn)(p) == 0;
}

int ksm_numrep_positive_p(ksm_numrep *p)
{
  return (*ops[p->kind]->sgn)(p) > 0;
}

int ksm_numrep_negative_p(ksm_numrep *p)
{
  return (*ops[p->kind]->sgn)(p) < 0;
}

void ksm_numrep_add(ksm_numrep *accum, ksm_numrep *arg)
{
  PROMOTE(accum,arg);
  while( (*ops[accum->kind]->add)(accum, arg) != 0 )
    ;
}

void ksm_numrep_mul(ksm_numrep *accum, ksm_numrep *arg)
{
  PROMOTE(accum,arg);
  while( (*ops[accum->kind]->mul)(accum, arg) != 0 )
     ;
}

void ksm_numrep_sub(ksm_numrep *accum, ksm_numrep *arg)
{
  PROMOTE(accum,arg);
  while( (*ops[accum->kind]->sub)(accum, arg) != 0 )
     ;
}

void ksm_numrep_div(ksm_numrep *accum, ksm_numrep *arg)
{
  PROMOTE(accum,arg);
  while( (*ops[accum->kind]->div)(accum, arg) != 0 )
    ;
}

void ksm_numrep_neg(ksm_numrep *p)
{
  while( (*ops[p->kind]->neg)(p) != 0 )
    ;
}

// n1 = [n1/n2]*n2 + r
// n1 > 0, n2 > 0;  r > 0
// n1 < 0, n2 < 0;  r < 0
// n1 > 0, n2 < 0;  r > 0
// n1 < 0, n2 > 0;  r < 0

void ksm_numrep_quotient(ksm_numrep *p, ksm_numrep *q)
{
  PROMOTE(p,q);
  while( (*ops[p->kind]->quotient)(p,q) != 0 )
    ;
}

void ksm_numrep_remainder(ksm_numrep *p, ksm_numrep *q)
{
  PROMOTE(p,q);
  while( (*ops[p->kind]->remainder)(p,q) != 0 )
    ;
}

void ksm_numrep_modulo(ksm_numrep *p, ksm_numrep *q)
{
  PROMOTE(p,q);
  while( (*ops[p->kind]->modulo)(p,q) != 0 )
    ;
}

void ksm_numrep_gcd(ksm_numrep *p, ksm_numrep *q)
{
  PROMOTE(p,q);
  while( (*ops[p->kind]->gcd)(p,q) != 0 )
    ;
}

void ksm_numrep_lcm(ksm_numrep *p, ksm_numrep *q)
{
  PROMOTE(p,q);
  while( (*ops[p->kind]->lcm)(p,q) != 0 )
    ;
}

void ksm_numrep_expt(ksm_numrep *p, ksm_numrep *q)
{
  PROMOTE(p,q);
  while( (*ops[p->kind]->expt)(p,q) != 0 )
    ;
}

void ksm_numrep_and(ksm_numrep *accum, ksm_numrep *arg)
{
  PROMOTE_BOTH(accum,arg);
  while( (*ops[accum->kind]->and)(accum, arg) != 0 )
    ;
}

void ksm_numrep_or(ksm_numrep *accum, ksm_numrep *arg)
{
  PROMOTE_BOTH(accum,arg);
  while( (*ops[accum->kind]->or)(accum, arg) != 0 )
    ;
}

void ksm_numrep_xor(ksm_numrep *accum, ksm_numrep *arg)
{
  PROMOTE_BOTH(accum,arg);
  while( (*ops[accum->kind]->xor)(accum, arg) != 0 )
    ;
}

void ksm_numrep_complement(ksm_numrep *accum)
{
  (*ops[accum->kind]->complement)(accum);
}

void ksm_numrep_lshift(ksm_numrep *p, int n)
{
  while( (*ops[p->kind]->lshift)(p,n) != 0 )
    ;
}

void ksm_numrep_rshift(ksm_numrep *p, int n)
{
  while( (*ops[p->kind]->rshift)(p,n) != 0 )
    ;
}

void ksm_numrep_exact_to_inexact(ksm_numrep *p)
{
  (*ops[p->kind]->exact_to_inexact)(p);
}

void ksm_numrep_inexact_to_exact(ksm_numrep *p)
{
  (*ops[p->kind]->inexact_to_exact)(p);
}

ksm_t ksm_numrep_to_object(ksm_numrep *p)
{
  return (*ops[p->kind]->object)(p);
}

void ksm_numrep_floor(ksm_numrep *p)
{
  if( ops[p->kind]->exact_p )
    (*ops[p->kind]->exact_to_inexact)(p);
  (*ops[p->kind]->floor)(p);
}

void ksm_numrep_ceiling(ksm_numrep *p)
{
  if( ops[p->kind]->exact_p )
    (*ops[p->kind]->exact_to_inexact)(p);
  (*ops[p->kind]->ceiling)(p);
}

void ksm_numrep_truncate(ksm_numrep *p)
{
  if( ops[p->kind]->exact_p )
    (*ops[p->kind]->exact_to_inexact)(p);
  (*ops[p->kind]->truncate)(p);
}

void ksm_numrep_round(ksm_numrep *p)
{
  if( ops[p->kind]->exact_p )
    (*ops[p->kind]->exact_to_inexact)(p);
  (*ops[p->kind]->round)(p);
}

void ksm_numrep_exp(ksm_numrep *p)
{
  if( ops[p->kind]->exact_p )
    (*ops[p->kind]->exact_to_inexact)(p);
  (*ops[p->kind]->exp)(p);
}

void ksm_numrep_log(ksm_numrep *p)
{
  if( ops[p->kind]->exact_p )
    (*ops[p->kind]->exact_to_inexact)(p);
  (*ops[p->kind]->log)(p);
}

void ksm_numrep_sin(ksm_numrep *p)
{
  if( ops[p->kind]->exact_p )
    (*ops[p->kind]->exact_to_inexact)(p);
  (*ops[p->kind]->sin)(p);
}

void ksm_numrep_cos(ksm_numrep *p)
{
  if( ops[p->kind]->exact_p )
    (*ops[p->kind]->exact_to_inexact)(p);
  (*ops[p->kind]->cos)(p);
}

void ksm_numrep_tan(ksm_numrep *p)
{
  if( ops[p->kind]->exact_p )
    (*ops[p->kind]->exact_to_inexact)(p);
  (*ops[p->kind]->tan)(p);
}

void ksm_numrep_asin(ksm_numrep *p)
{
  if( ops[p->kind]->exact_p )
    (*ops[p->kind]->exact_to_inexact)(p);
  (*ops[p->kind]->asin)(p);
}

void ksm_numrep_acos(ksm_numrep *p)
{
  if( ops[p->kind]->exact_p )
    (*ops[p->kind]->exact_to_inexact)(p);
  (*ops[p->kind]->acos)(p);
}

void ksm_numrep_atan(ksm_numrep *p)
{
  if( ops[p->kind]->exact_p )
    (*ops[p->kind]->exact_to_inexact)(p);
  (*ops[p->kind]->atan)(p);
}

void ksm_numrep_atan2(ksm_numrep *y, ksm_numrep *x)
{
  if( ops[y->kind]->exact_p )
    (*ops[y->kind]->exact_to_inexact)(y);
  if( ops[x->kind]->exact_p )
    (*ops[x->kind]->exact_to_inexact)(x);
  PROMOTE(y,x);
  (*ops[y->kind]->atan2)(y,x);
}

void ksm_numrep_sqrt(ksm_numrep *p)
{
  if( ops[p->kind]->exact_p )
    (*ops[p->kind]->exact_to_inexact)(p);
  (*ops[p->kind]->sqrt)(p);
}

/*** interface ******************************************************/

int ksm_get_si(ksm_t x)
{
  ksm_numrep rep;

  if( ksm_set_numrep(&rep, x) != 0 ){
    int ch;

    if( ksm_char_value_x(x, &ch) == 0 )
      return ch;
    ksm_error("can't convert to int: %k", x);
  }
  return (*ops[rep.kind]->get_si)(&rep);
}

double ksm_get_d(ksm_t x)
{
  ksm_numrep rep;

  if( ksm_set_numrep(&rep, x) != 0 ){
    int ch;
    
    if( ksm_char_value_x(x, &ch) == 0 )
      return ch;
    ksm_error("can't convert to int: %k", x);
  }
  return (*ops[rep.kind]->get_d)(&rep);
}

int ksm_exact_p(ksm_t x)
{
  ksm_numrep rep;

  return (ksm_set_numrep(&rep, x) == 0 && ksm_numrep_exact_p(&rep))
    ? 1 : 0;
}

int ksm_inexact_p(ksm_t x)
{
  ksm_numrep rep;

  return ksm_set_numrep(&rep, x) == 0 && !ksm_numrep_exact_p(&rep);
}

ksm_t ksm_exact_to_inexact(ksm_t x)
{
  ksm_numrep rep;

  if( !(ksm_set_numrep(&rep, x) == 0 && ksm_numrep_exact_p(&rep)) )
    ksm_error("exact number expected: %k", x);
  ksm_numrep_clone(&rep);
  ksm_numrep_exact_to_inexact(&rep);
  return ksm_numrep_to_object(&rep);
}

ksm_t ksm_inexact_to_exact(ksm_t x)
{
  ksm_numrep rep;

  if( !(ksm_set_numrep(&rep, x) == 0 && !ksm_numrep_exact_p(&rep)) )
    ksm_error("inexact number expected: %k", x);
  ksm_numrep_clone(&rep);
  ksm_numrep_inexact_to_exact(&rep);
  return ksm_numrep_to_object(&rep);
}

/*** integer ********************************************************/

#define MAKE_INT(i)   ksm_itype(KSM_FLAG_INT, i, 0, 0)
#define IS_INT(x)     (ksm_flag(x)==KSM_FLAG_INT)
#define INT_VALUE(x)  ksm_ival(x, 0)

ksm_t ksm_make_int(int i)
{
  return MAKE_INT(i);
}

int ksm_is_int(ksm_t x)
{
  return IS_INT(x);
}

int ksm_fetch_int(ksm_t x)
{
  return INT_VALUE(x);
}

static int int_intvalue(ksm_t x, int *result)
{
  *result = INT_VALUE(x);
  return 0;
}

typedef int (*intvalue_fun)(ksm_t x, int *result);
KSM_INSTALLER(intvalue,intvalue_fun)   /*** GLOBAL ***/
#define GET_INTVALUE(x)   get_intvalue(ksm_flag(x))

int ksm_int_value_x(ksm_t x, int *result)
     // returns 0 if OK
{
  intvalue_fun f;

  f = GET_INTVALUE(x);
  return f ? (*f)(x, result) : -1;
}

int ksm_int_value(ksm_t x)
{
  int i;

  if( ksm_int_value_x(x, &i) != 0 )
    ksm_error("can't convert to int: %k", x);
  return i;
}

int ksm_fprintf_int(FILE *fp, int i, int alt)
{
  return fprintf(fp, "%d", i);
}

static
int fprint_int(FILE *fp, ksm_t x, int alt)
{
  return ksm_fprintf_int(fp, ksm_fetch_int(x), alt);
}

static
int int_to_numrep(ksm_numrep *p, ksm_t x)
{
  p->kind = KSM_NUMREP_INT;
  p->u.ival = ksm_fetch_int(x);
  return 0;
}

ksm_t ksm_integer_maker(char *s, int radix)
{
  long i;
  char *tail;

  errno = 0;
  i = strtol(s, &tail, radix);
  if( s == tail || *tail )
    return 0;
  if( errno == ERANGE ){
#ifdef GMP
    return 0;
#endif    
    ksm_error("integral number out of range: %s", s);
  }
  if( errno )
    return 0;
  return ksm_make_int(i);
}

/*** double *********************************************************/

#define MAKE_DOUBLE(d)   ksm_dtype(KSM_FLAG_DOUBLE, d)
#define IS_DOUBLE(x)     (ksm_flag(x)==KSM_FLAG_DOUBLE)
#define DOUBLE_VALUE(x)  ksm_dval(x)

ksm_t ksm_make_double(double r)
{
  return MAKE_DOUBLE(r);
}

int ksm_is_double(ksm_t x)
{
  return IS_DOUBLE(x);
}

double ksm_fetch_double(ksm_t x)
{
  return DOUBLE_VALUE(x);
}

static
int double_doublevalue(ksm_t x, double *result)
{
  *result = DOUBLE_VALUE(x);
  return 0;
}

typedef int (*doublevalue_fun)(ksm_t x, double *result);
KSM_INSTALLER(doublevalue,doublevalue_fun)   /*** GLOBAL ***/
#define GET_DOUBLEVALUE(x)    get_doublevalue(ksm_flag(x))

int ksm_double_value_x(ksm_t x, double *result)
     // returns 0 if OK
{
  doublevalue_fun f;

  f = GET_DOUBLEVALUE(x);
  return f ? (*f)(x, result) : -1;
}

double ksm_double_value(ksm_t x)
{
  double d;

  if( ksm_double_value_x(x, &d) != 0 )
    ksm_error("can't convert to double: %k", x);
  return d;
}

int ksm_fprintf_double(FILE *fp, double d, int alt)
{
  return fprintf(fp, "%#g", d);
}

static
int fprint_double(FILE *fp, ksm_t x, int alt)
{
  return ksm_fprintf_double(fp, ksm_fetch_double(x), alt);
}

static
int double_to_numrep(ksm_numrep *p, ksm_t x)
{
  p->kind = KSM_NUMREP_DOUBLE;
  p->u.dval = ksm_fetch_double(x);
  return 0;
}

ksm_t ksm_double_maker(char *s)
{
  double d;
  char *tail;

  errno = 0;
  d = strtod(s, &tail);
  if( s == tail || *tail )
    return 0;
  if( errno == ERANGE )
    ksm_error("floating point number out of range: %s", s);
  if( errno )
    return 0;
  return ksm_make_double(d);
}

#ifdef GMP
/*** mpz ************************************************************/

#define MAKE_MPZ(zp)  \
  ksm_itype(KSM_FLAG_MPZ, (int)(zp), 0, 0)
#define IS_MPZ(x)       (ksm_flag(x)==KSM_FLAG_MPZ)
#define MPZ_VALUE(mpz)  ((mpz_ptr)ksm_ival(mpz,0))
#define MPZ_VALUE_NO_LOCK(mpz)  ((mpz_ptr)ksm_ival_no_lock(mpz,0))

static
void dispose_mpz(ksm_t mpz)
{
  mpz_clear(MPZ_VALUE_NO_LOCK(mpz));
}

ksm_t ksm_make_mpz(mpz_ptr zp) { return MAKE_MPZ(zp); }

int ksm_is_mpz(ksm_t x) { return IS_MPZ(x); }

static
int mpz_to_numrep(ksm_numrep *p, ksm_t mpz)
{
  p->kind = KSM_NUMREP_MPZ;
  p->u.zval = MPZ_VALUE(mpz);
  return 0;
}

static
int fprintf_mpz(FILE *fp, ksm_t x, int alt)
{
  int n;

  n = mpz_out_str(fp, 10, MPZ_VALUE(x));
  return (n==0) ? -1 : n;
}

ksm_t ksm_mpz_maker(char *str, int radix)
{
  mpz_ptr zp;

  zp = ksm_alloc(sizeof(mpz_t));
  mpz_init(zp);
  if( mpz_set_str(zp, str, radix) == 0 )
    return ksm_make_mpz(zp);
  else{
    mpz_clear(zp);
    return 0;
  }
}

/*** mpq ************************************************************/

#define MAKE_MPQ(qp)  \
  ksm_itype(KSM_FLAG_MPQ, (int)(qp), 0, 0)
#define IS_MPQ(x)       (ksm_flag(x)==KSM_FLAG_MPQ)
#define MPQ_VALUE(mpq)  ((mpq_ptr)ksm_ival(mpq,0))
#define MPQ_VALUE_NO_LOCK(mpq)  ((mpq_ptr)ksm_ival_no_lock(mpq,0))

static
void dispose_mpq(ksm_t mpq)
{
  mpq_clear(MPQ_VALUE_NO_LOCK(mpq));
}

ksm_t ksm_make_mpq(mpq_ptr qp) { return MAKE_MPQ(qp); }

int ksm_is_mpq(ksm_t x) { return IS_MPQ(x); }

static
int mpq_to_numrep(ksm_numrep *p, ksm_t mpq)
{
  p->kind = KSM_NUMREP_MPQ;
  p->u.qval = MPQ_VALUE(mpq);
  return 0;
}

static
int fprintf_mpq(FILE *fp, ksm_t x, int alt)
{
  int n, i;
  mpq_ptr qp = MPQ_VALUE(x);

  i = mpz_out_str(fp, 10, mpq_numref(qp));
  if( i == 0 )
    return -1;
  n = i;
  i = fprintf(fp, "/");
  if( i == 0 )
    return -1;
  n += i;
  i = mpz_out_str(fp, 10, mpq_denref(qp));
  if( i == 0 )
    return -1;
  return n + i;
}

ksm_t ksm_mpq_maker(char *str, int radix)
{
  mpz_t num, den;
  mpq_ptr qp;
  char *p;

  if( !((p = strchr(str, '/')) && !strchr(p+1, '/') && 
	isdigit(*(p+1))) )
      return 0;
  mpz_init(num);
  mpz_init(den);
  *p = '\0';
  if( mpz_set_str(num, str, radix) == 0 &&
      mpz_set_str(den, p+1, radix) == 0 ){
    qp = ksm_alloc(sizeof(mpq_t));
    mpq_init(qp);
    mpq_set_num(qp, num);
    mpq_set_den(qp, den);
    mpq_canonicalize(qp);
    mpz_clear(num);
    mpz_clear(den);
    *p = '/';
    return ksm_make_mpq(qp);
  }
  else{
    mpz_clear(num);
    mpz_clear(den);
    *p = '/';
    return 0;
  }
}

#endif // GMP (mpz/mpq)

/*** complex ********************************************************/

#define MAKE_COMPLEX(cp) \
 ksm_itype(KSM_FLAG_COMPLEX, (int)(cp), 0, 0)
#define IS_COMPLEX(x)      (ksm_flag(x)==KSM_FLAG_COMPLEX)
#define COMPLEX_VALUE(x)   ((__complex__ double *)ksm_ival(x,0))
#define COMPLEX_REAL_PART(x)  (__real__ *COMPLEX_VALUE(x))
#define COMPLEX_IMAG_PART(x)  (__imag__ *COMPLEX_VALUE(x))

#define COMPLEX_VALUE_NO_LOCK(x) ((__complex__ double *)ksm_ival_no_lock(x,0))

static
void dispose_complex(ksm_t c)
{
  ksm_dispose(COMPLEX_VALUE_NO_LOCK(c));
}

ksm_t ksm_make_complex(double real, double imag)
{
  __complex__ double *p;

  p = ksm_alloc(sizeof(__complex__ double));
  *p = real + imag * 1i;
  return MAKE_COMPLEX(p);
}

int ksm_is_complex(ksm_t x)
{
  return IS_COMPLEX(x);
}

static
int complex_to_numrep(ksm_numrep *p, ksm_t x)
{
  p->kind = KSM_NUMREP_COMPLEX;
  p->u.pval = COMPLEX_VALUE(x);
  return 0;
}

static
double numrep_complex_imag_part(ksm_numrep *p)
{
  __complex__ double cx;

  cx = *(__complex__ double *)p->u.pval;
  return __imag__ cx;
}

static
double numrep_complex_real_part(ksm_numrep *p)
{
  __complex__ double cx;

  cx = *(__complex__ double *)p->u.pval;
  return __real__ cx;
}

static
int fprintf_complex(FILE *fp, ksm_t x, int alt)
{
  __complex__ double *p;

  p = COMPLEX_VALUE(x);
  return fprintf(fp, "%g+%gi", __real__ *p, __imag__ *p);
}

ksm_t ksm_complex_maker(char *str)
{
  char *p, *q;
  ksm_t real, imag;
  int len;
  
  len = strlen(str);
  q = str + len - 1;
  if( !(len > 0 && *q == 'i') )
    return 0;
  if( !(p = strchr(str, '+')) ){
    *q = '\0';
    imag = ksm_double_maker(str);
    *q = 'i';
    if( imag )
      return ksm_make_complex(0.0, DOUBLE_VALUE(imag));
    else
      return 0;
  }
  if( strchr(p+1, '+') )
    return 0;

  *p = '\0';
  *q = '\0';
  real = ksm_double_maker(str);
  imag = ksm_double_maker(p+1);
  *p = '+';
  *q = 'i';
  if( real && imag )
    return ksm_make_complex(DOUBLE_VALUE(real), DOUBLE_VALUE(imag));
  return 0;
}

/*** number *********************************************************/

int ksm_is_number(ksm_t x)
{
  ksm_numrep rep;

  return ksm_set_numrep(&rep, x) == 0;
}

int ksm_number_eqv(ksm_t x, ksm_t y)
{
  ksm_numrep p, q;

  if( !(ksm_set_numrep(&p, x) == 0 && ksm_set_numrep(&q, y) == 0) )
    return 0;
  return ksm_numrep_eq(&p, &q);
}

/*** procedures *****************************************************/

KSM_DEF_BLT(number_p)
     // (number? obj)
{
  ksm_t obj;

  KSM_SET_ARGS1(obj);
  return KSM_BOOL2OBJ(ksm_is_number(obj));
}

KSM_DEF_BLT(complex_p)
     // (complex? obj)
{
  ksm_t obj;
  ksm_numrep rep;

  KSM_SET_ARGS1(obj);
  return KSM_BOOL2OBJ(ksm_set_numrep(&rep,obj) == 0 &&
		      rep.kind <= KSM_NUMREP_COMPLEX);
}

KSM_DEF_BLT(real_p)
     // (real? obj)
{
  ksm_t obj;
  ksm_numrep rep;

  KSM_SET_ARGS1(obj);
  if( !(ksm_set_numrep(&rep, obj) == 0) )
    return ksm_false_object;
  if( rep.kind == KSM_NUMREP_COMPLEX )
    return KSM_BOOL2OBJ(numrep_complex_imag_part(&rep)==0.0);
  else
    return KSM_BOOL2OBJ(ksm_set_numrep(&rep,obj) == 0 &&
			rep.kind <= KSM_NUMREP_DOUBLE);
}

KSM_DEF_BLT(rational_p)
     // (rational? obj)
{
  ksm_t obj;
  ksm_numrep rep;
  int kind;

  KSM_SET_ARGS1(obj);
#ifdef GMP
  kind = KSM_NUMREP_MPQ;
#else
  kind = KSM_NUMREP_INT;
#endif
  return KSM_BOOL2OBJ(ksm_set_numrep(&rep,obj) == 0 &&
		      rep.kind <= kind);
}

KSM_DEF_BLT(integer_p)
     // (integer? obj)
{
  ksm_t obj;
  ksm_numrep rep;
  int kind;

  KSM_SET_ARGS1(obj);
  if( !(ksm_set_numrep(&rep, obj) == 0) )
    return ksm_false_object;
  if( rep.kind == KSM_NUMREP_COMPLEX )
    return KSM_BOOL2OBJ(numrep_complex_imag_part(&rep)==0.0 &&
			numrep_complex_real_part(&rep)==
			rint(numrep_complex_real_part(&rep)));
  if( rep.kind == KSM_NUMREP_DOUBLE )
    return KSM_BOOL2OBJ(rep.u.dval == rint(rep.u.dval));
#ifdef GMP
  if( rep.kind == KSM_NUMREP_MPQ )
    return KSM_BOOL2OBJ(mpz_cmp_si(mpq_denref(rep.u.qval), 1) == 0);
  kind = KSM_NUMREP_MPZ;
#else
  kind = KSM_NUMREP_INT;
#endif
  return KSM_BOOL2OBJ(ksm_set_numrep(&rep,obj) == 0 &&
		      rep.kind <= kind);
}

KSM_DEF_BLT(exact_p)
     // (exact? obj)
{
  ksm_t obj;
  ksm_numrep rep;

  KSM_SET_ARGS1(obj);
  return KSM_BOOL2OBJ( ksm_set_numrep(&rep, obj) == 0 &&
		       ksm_numrep_exact_p(&rep) );
}

KSM_DEF_BLT(inexact_p)
     // (inexact? obj)
{
  ksm_t obj;
  ksm_numrep rep;

  KSM_SET_ARGS1(obj);
  return KSM_BOOL2OBJ( ksm_set_numrep(&rep, obj) == 0 &&
		       !ksm_numrep_exact_p(&rep) );
}

KSM_DEF_BLT(zero_p)
     // (zero? obj)
{
  ksm_t obj;
  ksm_numrep rep;

  KSM_SET_ARGS1(obj);
  return KSM_BOOL2OBJ( ksm_set_numrep(&rep, obj) == 0 &&
		       ksm_numrep_zero_p(&rep) );
}

KSM_DEF_BLT(positive_p)
     // (positive? obj)
{
  ksm_t obj;
  ksm_numrep rep;

  KSM_SET_ARGS1(obj);
  return KSM_BOOL2OBJ( ksm_set_numrep(&rep, obj) == 0 &&
		       ksm_numrep_positive_p(&rep) );
}

KSM_DEF_BLT(negative_p)
     // (negative? obj)
{
  ksm_t obj;
  ksm_numrep rep;

  KSM_SET_ARGS1(obj);
  if( ksm_set_numrep(&rep, obj) != 0 )
    return ksm_false_object;
  return KSM_BOOL2OBJ(ksm_numrep_negative_p(&rep));
}

KSM_DEF_BLT(numerator)
     // (numerator q)
{
  ksm_numrep rep;
  ksm_t obj;
  int kind;
#ifdef GMP
  mpz_ptr zp;
#endif GMP

  KSM_SET_ARGS1(obj);
  if( !(ksm_set_numrep(&rep, obj) == 0) )
    ksm_error("number expected (NUMERATOR): %k", obj);
#ifdef GMP
  kind = KSM_NUMREP_MPZ;
#else
  kind = KSM_NUMREP_INT;
#endif
  if( rep.kind <= kind )
    return obj;
#ifdef GMP
  if( rep.kind == KSM_NUMREP_MPQ ){
    zp = ksm_alloc(sizeof(mpz_t));
    mpz_init(zp);
    mpz_set(zp, mpq_numref(rep.u.qval));
    return ksm_make_mpz(zp);
  }
#endif
  ksm_error("can't handle arg (NUMERATOR): %k", obj);
  return ksm_unspec_object;
}

KSM_DEF_BLT(denominator)
     // (denominator q)
{
  ksm_numrep rep;
  ksm_t obj;
  int kind;
#ifdef GMP
  mpz_ptr zp;
#endif

  KSM_SET_ARGS1(obj);
  if( !(ksm_set_numrep(&rep, obj) == 0) )
    ksm_error("number expected (DENOMINATOR): %k", obj);
#ifdef GMP
  kind = KSM_NUMREP_MPZ;
#else
  kind = KSM_NUMREP_INT;
#endif
  if( rep.kind <= kind )
    return ksm_make_int(1);
#ifdef GMP
  if( rep.kind == KSM_NUMREP_MPQ ){
    zp = ksm_alloc(sizeof(mpz_t));
    mpz_init(zp);
    mpz_set(zp, mpq_denref(rep.u.qval));
    return ksm_make_mpz(zp);
  }
#endif
  ksm_error("incompatible arg (DENOMINATOR): %k", obj);
  return ksm_unspec_object;
}

KSM_DEF_BLT(rationalize)
     // (rationalize x y)
{
  ksm_error("not supported (RATIONALIZE)");
  return ksm_unspec_object;
}

KSM_DEF_BLT(make_rectangular)
     // (make-rectangular x y)
{
  ksm_numrep xrep, yrep;
  ksm_t x, y;

  KSM_SET_ARGS2(x, y);
  if( !(ksm_set_numrep(&xrep, x) == 0 && xrep.kind <= KSM_NUMREP_DOUBLE) )
    ksm_error("incompatible first arg to MAKE-RECTANGULAR: %k", x);
  if( !(ksm_set_numrep(&yrep, y) == 0 && yrep.kind <= KSM_NUMREP_DOUBLE) )
    ksm_error("incompatible first arg to MAKE-RECTANGULAR: %k", y);
  ksm_numrep_promote(&xrep, KSM_NUMREP_DOUBLE);
  ksm_numrep_promote(&yrep, KSM_NUMREP_DOUBLE);
  return ksm_make_complex(xrep.u.dval, yrep.u.dval);
}

KSM_DEF_BLT(make_polar)
     // (make-polar x y)
{
  ksm_numrep xrep, yrep;
  ksm_t x, y;
  __complex__ double cx;

  KSM_SET_ARGS2(x, y);
  if( !(ksm_set_numrep(&xrep, x) == 0 && xrep.kind <= KSM_NUMREP_DOUBLE) )
    ksm_error("incompatible first arg to MAKE-POLAR: %k", x);
  if( !(ksm_set_numrep(&yrep, y) == 0 && yrep.kind <= KSM_NUMREP_DOUBLE) )
    ksm_error("incompatible first arg to MAKE-POLAR: %k", y);
  ksm_numrep_promote(&xrep, KSM_NUMREP_DOUBLE);
  ksm_numrep_promote(&yrep, KSM_NUMREP_DOUBLE);
  cx = xrep.u.dval * cexp(yrep.u.dval*1i);
  return ksm_make_complex(__real__ cx, __imag__ cx);
}

KSM_DEF_BLT(real_part)
     // (real-part z)
{
  ksm_t z;
  ksm_numrep rep;
  
  KSM_SET_ARGS1(z);
  if( !(ksm_set_numrep(&rep, z) == 0 && rep.kind == KSM_NUMREP_COMPLEX) )
    ksm_error("incompatible arg to REAL-PART: %k", z);
  return ksm_make_double(numrep_complex_real_part(&rep));
}

KSM_DEF_BLT(imag_part)
     // (imag-part z)
{
  ksm_t z;
  ksm_numrep rep;
  
  KSM_SET_ARGS1(z);
  if( !(ksm_set_numrep(&rep, z) == 0 && rep.kind == KSM_NUMREP_COMPLEX) )
    ksm_error("incompatible arg to REAL-PART: %k", z);
  return ksm_make_double(numrep_complex_imag_part(&rep));
}

KSM_DEF_BLT(magnitude)
     // (magnitude z)
{
  ksm_t z;
  ksm_numrep rep;
  __complex__ double cx;
  double x, y;
  
  KSM_SET_ARGS1(z);
  if( !(ksm_set_numrep(&rep, z) == 0 && rep.kind == KSM_NUMREP_COMPLEX) )
    ksm_error("incompatible arg to REAL-PART: %k", z);
  cx = *(__complex__ double *)rep.u.pval;
  x = __real__ cx;
  y = __imag__ cx;
  return ksm_make_double(sqrt(x*x+y*y));
}

KSM_DEF_BLT(angle)
     // (angle z)
{
  ksm_t z;
  ksm_numrep rep;
  __complex__ double cx;
  
  KSM_SET_ARGS1(z);
  if( !(ksm_set_numrep(&rep, z) == 0 && rep.kind == KSM_NUMREP_COMPLEX) )
    ksm_error("incompatible arg to REAL-PART: %k", z);
  cx = *(__complex__ double *)rep.u.pval;
  return ksm_make_double(carg(cx));
}

/*** init ***********************************************************/

void ksm_init_number(void)
{
  ksm_install_fprintf(KSM_FLAG_INT, fprint_int);
  ksm_install_numcvter(KSM_FLAG_INT, int_to_numrep);
  ksm_install_intvalue(KSM_FLAG_INT, int_intvalue);
  ksm_install_fprintf(KSM_FLAG_DOUBLE, fprint_double);
  ksm_install_numcvter(KSM_FLAG_DOUBLE, double_to_numrep);
  ksm_install_doublevalue(KSM_FLAG_DOUBLE, double_doublevalue);
  ksm_install_disposer(KSM_FLAG_COMPLEX, dispose_complex);
  ksm_install_fprintf(KSM_FLAG_COMPLEX, fprintf_complex);
  ksm_install_numcvter(KSM_FLAG_COMPLEX, complex_to_numrep);

#ifdef GMP
  ksm_install_disposer(KSM_FLAG_MPZ, dispose_mpz);
  ksm_install_fprintf(KSM_FLAG_MPZ, fprintf_mpz);
  ksm_install_numcvter(KSM_FLAG_MPZ, mpz_to_numrep);
  ksm_install_disposer(KSM_FLAG_MPQ, dispose_mpq);
  ksm_install_fprintf(KSM_FLAG_MPQ, fprintf_mpq);
  ksm_install_numcvter(KSM_FLAG_MPQ, mpq_to_numrep);
#endif // GMP

  KSM_ENTER_BLT("number?", number_p);
  KSM_ENTER_BLT("complex?", complex_p);
  KSM_ENTER_BLT("real?", real_p);
  KSM_ENTER_BLT("rational?", rational_p);
  KSM_ENTER_BLT("integer?", integer_p);
  KSM_ENTER_BLT("exact?", exact_p);
  KSM_ENTER_BLT("inexact?", inexact_p);
  KSM_ENTER_BLT("zero?", zero_p);
  KSM_ENTER_BLT("positive?", positive_p);
  KSM_ENTER_BLT("negative?", negative_p);
  KSM_ENTER_BLT("numerator", numerator);
  KSM_ENTER_BLT("denominator", denominator);
  KSM_ENTER_BLT("rationalize", rationalize);
  KSM_ENTER_BLT("make-rectangular", make_rectangular);
  KSM_ENTER_BLT("make-polar", make_polar);
  KSM_ENTER_BLT("real-part", real_part);
  KSM_ENTER_BLT("imag-part", imag_part);
  KSM_ENTER_BLT("magnitude", magnitude);
  KSM_ENTER_BLT("angle", angle);
}

