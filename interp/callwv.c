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

#include <setjmp.h>
#include "ksm.h"

#define MAKE_CALLWV() \
  ksm_itype(KSM_FLAG_CALLWV,(int)ksm_alloc(sizeof(jmp_buf)), \
    (int)(ksm_nil_object), 0)
#define IS_CALLWV(x) (ksm_flag(x)==KSM_FLAG_CALLWV)
#define CALLWV_REGS(x)  ((jmp_buf *)ksm_ival(x,0))
#define CALLWV_ARGS(x)  ((ksm_t)ksm_ival(x,1))
#define SET_CALLWV_ARGS(x,args) ksm_set_ival(x,1,(int)(args))

#define CALLWV_REGS_NO_LOCK(x)  ((jmp_buf *)ksm_ival_no_lock(x,0))
#define CALLWV_ARGS_NO_LOCK(x)  ((ksm_t)ksm_ival_no_lock(x,1))

static
ksm_t mark_callwv(ksm_t x)
{
  ksm_mark_object(CALLWV_ARGS_NO_LOCK(x));
  return 0;
}

static
void dispose_callwv(ksm_t x)
{
  ksm_dispose(CALLWV_REGS_NO_LOCK(x));
}

int ksm_is_callwv(ksm_t x)
{
  return IS_CALLWV(x);
}

KSM_DEF_BLT(callwv)
     // (call/wv producer consumer)
{
  ksm_t prod, thunk, obj, val, funargs;

  KSM_SET_ARGS2(prod, thunk);
  obj = MAKE_CALLWV();
  if( setjmp(*CALLWV_REGS(obj)) == 0 ){
    ksm_push_jump(obj);
    val = ksm_funcall(prod, ksm_nil_object, KSM_ENV());
    funargs = ksm_cons(val, ksm_nil_object);
    ksm_pop_jump();
  }
  else
    funargs = CALLWV_ARGS(obj);
  return ksm_funcall(thunk, funargs, KSM_ENV());
}

KSM_DEF_BLT(values)
     // (values arg ...)
{
  ksm_t target, arg;

  target = ksm_search_jumps(ksm_is_callwv);
  if( ksm_is_nil(target) ){
    KSM_SET_ARGS1(arg);
    return arg;
  }
  else{
    ksm_rewind_jumps(target);
    SET_CALLWV_ARGS(target, KSM_ARGS());
    longjmp(*CALLWV_REGS(target), 1);
    return ksm_unspec_object;
  }
}

void ksm_init_callwv(void)
{
  ksm_install_marker(KSM_FLAG_CALLWV, mark_callwv);
  ksm_install_disposer(KSM_FLAG_CALLWV, dispose_callwv);
  KSM_ENTER_BLT("call-with-values", callwv);
  KSM_ENTER_BLT("call/wv", callwv);
  KSM_ENTER_BLT("values", values);
}
