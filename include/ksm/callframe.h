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

#ifndef CALLFRAME_H
#define CALLFRAME_H

#include "ksm.h"

#ifdef KSM_LINUXPPC

#define MAX_NSPILL 16

struct ksm_clang_ret_storage {
  double dval;
  union {
    long long llval;
    int ival;
  } u;
};

struct ksm_clang_calldata {
  void (*fun)();
  int   *gpr;
  int   ngpr;   // ngpr <= 8
  double *fpr;
  int    nfpr;  // nfpr <= 8
  int    *spill;
  int     nspill;
  struct ksm_clang_ret_storage *store;
  int    vararg;
};

struct ksm_clang_callframe {
  struct ksm_clang_calldata data;   // should be the first
  int gpr_buf[8];
  double fpr_buf[8];
  int spill_buf[MAX_NSPILL];
  struct ksm_clang_ret_storage store_buf;
};

#elif defined KSM_I386

#define MAX_NARG 24

struct ksm_clang_arg_unit {  // size: 16
  int flag;   // [0] 0: integer, 1: double, 2: float
  int rsrv;   // [4] reserved
  union {     // [8]
    double dval;  
    float  fval;
    int    ival;
  } u;
};

struct ksm_clang_callframe {
  double dret;    // [0]
  int    iret;    // [8]
  void (*fun)();  // [12]
  int    narg;    // [16]
  struct ksm_clang_arg_unit args[MAX_NARG];  // [20]
};

#endif  // KSM_I386

#endif  // CALLFRAME_H
