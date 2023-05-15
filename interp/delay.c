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

#define MAKE_DELAY(expr,env) \
  ksm_itype(KSM_FLAG_DELAY, (int)(expr), (int)(env), 0)
#define IS_DELAY(x) (ksm_flag(x)==KSM_FLAG_DELAY)
#define DELAY_EXPR(x)   ((ksm_t)ksm_ival(x,0))
#define DELAY_ENV(x)    ((ksm_t)ksm_ival(x,1))
#define DELAY_RESULT(x) ((ksm_t)ksm_ival(x,2))
#define SET_DELAY_RESULT(d,x)  ksm_set_ival(d,2,(int)(x))

#define DELAY_EXPR_NO_LOCK(x)   ((ksm_t)ksm_ival_no_lock(x,0))
#define DELAY_ENV_NO_LOCK(x)    ((ksm_t)ksm_ival_no_lock(x,1))
#define DELAY_RESULT_NO_LOCK(x) ((ksm_t)ksm_ival_no_lock(x,2))

static
ksm_t mark_delay(ksm_t x)
{
  ksm_mark_object(DELAY_EXPR_NO_LOCK(x));
  ksm_mark_object(DELAY_ENV_NO_LOCK(x));
  return DELAY_RESULT_NO_LOCK(x);
}

KSM_DEF_SPC(delay)
     // (delay expr)
{
  ksm_t expr;

  KSM_SET_ARGS1(expr);
  return MAKE_DELAY(expr, KSM_ENV());
}

KSM_DEF_BLT(force)
     // (force delay)
{
  ksm_t delay, val;

  KSM_SET_ARGS1(delay);
  if( !IS_DELAY(delay) )
    ksm_error("incompatible arg to FORCE-FORM: %k", delay);
  if( DELAY_RESULT(delay) )
    return DELAY_RESULT(delay);
  val = ksm_eval(DELAY_EXPR(delay), DELAY_ENV(delay));
  if( val == ksm_except )
    return ksm_except;
  if( DELAY_RESULT(delay) )
    return DELAY_RESULT(delay);
  SET_DELAY_RESULT(delay, val);
  return val;
}

void ksm_init_delay(void)
{
  ksm_install_marker(KSM_FLAG_DELAY, mark_delay);
  KSM_ENTER_SPC("delay", delay);
  KSM_ENTER_BLT("force", force);
}


