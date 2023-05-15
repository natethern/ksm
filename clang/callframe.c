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
#include "ksm/callframe.h"

#ifdef KSM_LINUXPPC
#include <string.h>

/***
# struct ret_storage {
#   double dval;
#   union {
#     long long llval;
#     int ival;
#   } u;
# };
#
# struct call_data {
#   void (*fun)(void);    // 0
#   int *gpr;             // 4
#   int ngpr;             // 8
#   double *fpr;          // 12
#   int nfpr;             // 16
#   int *spill;           // 20
#   int nspill;           // 24
#   struct ret_storage *store; // 28
#   int vararg;           // 32
# };
#
# void call_glue(struct call_data *data);
***/

extern void ksm_clang_call_glue(struct ksm_clang_callframe *);   // call.S

void ksm_clang_init_callframe(struct ksm_clang_callframe *p, void (*f)())
{
  p->data.fun    = f;
  p->data.gpr    = p->gpr_buf;
  p->data.ngpr   = 0;
  p->data.fpr    = p->fpr_buf;
  p->data.nfpr   = 0;
  p->data.spill  = p->spill_buf;
  p->data.nspill = 0;
  p->data.store  = &p->store_buf;
  p->data.vararg = 0;
}

void ksm_clang_arg_int(struct ksm_clang_callframe *p, int i)
{
  struct ksm_clang_calldata *q = &p->data;

  if( q->ngpr < 8 )
    q->gpr[q->ngpr++] = i;
  else{
    if( q->nspill >= MAX_NSPILL )
      ksm_error("too many args to C-call");
    q->spill[q->nspill++] = i;
  }
}

void ksm_clang_arg_double(struct ksm_clang_callframe *p, double r)
{
  struct ksm_clang_calldata *q = &p->data;

  if( q->nfpr < 8 )
    q->fpr[q->nfpr++] = r;
  else{
    if( q->nspill >= (MAX_NSPILL-1) )
      ksm_error("too many args to C-call");
    memcpy(&q->spill[q->nspill], &r, sizeof(double));
    q->nspill += 2;
  }
}

void ksm_clang_arg_float(struct ksm_clang_callframe *p, double f)
{
  struct ksm_clang_calldata *q = &p->data;
  float val;
  
  if( q->nfpr < 8 )
    q->fpr[q->nfpr++] = f;
  else{
    if( q->nspill >= MAX_NSPILL )
      ksm_error("too many args to C-call");
    val = f;
    memcpy(&q->spill[q->nspill++], &val, sizeof(float));
  }
}

void ksm_clang_set_vararg(struct ksm_clang_callframe *p)
{
  p->data.vararg = 1;
}

int ksm_clang_ret_int(struct ksm_clang_callframe *p)
{
  return p->store_buf.u.ival;
}

double ksm_clang_ret_double(struct ksm_clang_callframe *p)
{
  return p->store_buf.dval;
}

void ksm_clang_call(struct ksm_clang_callframe *p)
{
  ksm_clang_call_glue(p);
}

#elif defined KSM_I386

extern void ksm_clang_call_glue(struct ksm_clang_callframe *p);  // call.S

void ksm_clang_init_callframe(struct ksm_clang_callframe *p, void (*f)())
{
  p->fun = f;
  p->narg = 0;
}

void ksm_clang_arg_int(struct ksm_clang_callframe *p, int i)
{
  struct ksm_clang_arg_unit *q;

  if( p->narg >= MAX_NARG )
    ksm_error("too many C-fun args");
  q = &p->args[p->narg++];
  q->flag = 0;
  q->u.ival = i;
}

void ksm_clang_arg_double(struct ksm_clang_callframe *p, double r)
{
  struct ksm_clang_arg_unit *q;

  if( p->narg >= MAX_NARG )
    ksm_error("too many C-fun args");
  q = &p->args[p->narg++];
  q->flag = 1;
  q->u.dval = r;
}

void ksm_clang_arg_float(struct ksm_clang_callframe *p, double f)
{
  struct ksm_clang_arg_unit *q;

  if( p->narg >= MAX_NARG )
    ksm_error("too many C-fun args");
  q = &p->args[p->narg++];
  q->flag = 2;
  q->u.fval = f;
}

void ksm_clang_set_vararg(struct ksm_clang_callframe *p) { }

int ksm_clang_ret_int(struct ksm_clang_callframe *p)
{
  return p->iret;
}

double ksm_clang_ret_double(struct ksm_clang_callframe *p)
{
  return p->dret;
}

void ksm_clang_call(struct ksm_clang_callframe *p)
{
  ksm_clang_call_glue(p);
}

#else

void ksm_call_data_init(struct ksm_clang_callframe *p, void (*f)()) { }
void ksm_call_data_push_int(struct ksm_clang_callframe *p, int i) { }
void ksm_call_data_push_double(struct ksm_clang_callframe *p, double r) { }
void ksm_call_data_set_vararg(struct ksm_clang_callframe *p) { }
int ksm_call_data_get_int(struct ksm_clang_callframe *p) { return 0; }
double ksm_call_data_get_double(struct ksm_clang_callframe *p) { return 0.0; }
void ksm_call_data_call(struct ksm_clang_callframe *p) { }

#endif
