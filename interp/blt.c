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

/*** blt/spc ********************************************************/

typedef ksm_t (*blt_t)(ksm_t args, ksm_t env);

#define MAKE_BLT(f)   ksm_itype(KSM_FLAG_BLT,(int)(f),0,0)
#define MAKE_SPC(f)   ksm_itype(KSM_FLAG_SPC,(int)(f),0,0)
#define BLT_FUN(b)    ((blt_t)ksm_ival(b,0))
#define IS_BLT(x)     (ksm_flag(x)==KSM_FLAG_BLT)

ksm_t ksm_make_blt(ksm_t (*f)(ksm_t args, ksm_t env))
{
  return MAKE_BLT(f);
}

ksm_t ksm_make_spc(ksm_t (*f)(ksm_t args, ksm_t env))
{
  return MAKE_SPC(f);
}

static
ksm_pair eval_blt(ksm_t head, ksm_t args, ksm_t env)
{
  blt_t f;

  f = BLT_FUN(head);
  return ksm_make_eval_done((*f)(args, env));
}

/*** rspc ***********************************************************/

typedef ksm_pair (*rspc_t)(ksm_t args, ksm_t env);

#define MAKE_RSPC(f)   ksm_itype(KSM_FLAG_RSPC,(int)(f),0,0)
#define RSPC_FUN(b)    ((rspc_t)ksm_ival(b,0))
#define IS_RSPC(x)     (ksm_flag(x)==KSM_FLAG_RSPC)

ksm_t ksm_make_rspc(ksm_pair (*f)(ksm_t args, ksm_t env))
{
  return MAKE_RSPC(f);
}

static
ksm_pair eval_rspc(ksm_t head, ksm_t args, ksm_t env)
{
  rspc_t f;

  f = RSPC_FUN(head);
  return (*f)(args, env);
}

/*** init ***********************************************************/

void ksm_init_blt(void)
{
  ksm_install_fun_info(KSM_FLAG_BLT, eval_blt, 1);
  ksm_install_fun_info(KSM_FLAG_SPC, eval_blt, 0);
  ksm_install_fun_info(KSM_FLAG_RSPC, eval_rspc, 0);
}


