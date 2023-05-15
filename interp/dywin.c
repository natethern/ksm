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

#define MAKE_DYWIN(b,t,a) \
  ksm_itype(KSM_FLAG_DYWIN,(int)(b),(int)(t),(int)(a))
#define IS_DYWIN(x) (ksm_flag(x)==KSM_FLAG_DYWIN)
#define DYWIN_BEFORE(d)  ((ksm_t)ksm_ival(d,0))
#define DYWIN_THUNK(d)   ((ksm_t)ksm_ival(d,1))
#define DYWIN_AFTER(d)   ((ksm_t)ksm_ival(d,2))

#define DYWIN_BEFORE_NO_LOCK(d)  ((ksm_t)ksm_ival_no_lock(d,0))
#define DYWIN_THUNK_NO_LOCK(d)   ((ksm_t)ksm_ival_no_lock(d,1))
#define DYWIN_AFTER_NO_LOCK(d)   ((ksm_t)ksm_ival_no_lock(d,2))

static
ksm_t mark_dywin(ksm_t x)
{
  ksm_mark_object(DYWIN_BEFORE_NO_LOCK(x));
  ksm_mark_object(DYWIN_THUNK_NO_LOCK(x));
  ksm_mark_object(DYWIN_AFTER_NO_LOCK(x));
  return 0;
}

ksm_t ksm_make_dywin(ksm_t before, ksm_t thunk, ksm_t after)
{
  return MAKE_DYWIN(before, thunk, after);
}

int ksm_is_dywin(ksm_t x)
{
  return IS_DYWIN(x);
}

ksm_t ksm_dywin_before(ksm_t dywin)
{
  return DYWIN_BEFORE(dywin);
}

ksm_t ksm_dywin_thunk(ksm_t dywin)
{
  return DYWIN_THUNK(dywin);
}

ksm_t ksm_dywin_after(ksm_t dywin)
{
  return DYWIN_AFTER(dywin);
}

KSM_DEF_BLT(dynamic_wind)
     // (dynamic-wind before thunk after)
{
  ksm_t before, thunk, after, obj;

  KSM_SET_ARGS3(before, thunk, after);
  obj = MAKE_DYWIN(before, thunk, after);
  ksm_push_jump(obj);
  ksm_funcall(before, ksm_nil_object, KSM_ENV());
  obj = ksm_funcall(thunk, ksm_nil_object, KSM_ENV());
  ksm_pop_jump();
  ksm_funcall(after, ksm_nil_object, KSM_ENV());
  return obj;
}

void ksm_init_dywin(void)
{
  ksm_install_marker(KSM_FLAG_DYWIN, mark_dywin);
  KSM_ENTER_BLT("dynamic-wind", dynamic_wind);
}
