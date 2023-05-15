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

//static ksm_t jumps;
#define JUMPS   (ksm_get_ctx()->jumps)

ksm_t ksm_search_jumps(int (*pred)(ksm_t))
{
  ksm_t list;

  list = JUMPS;
  while( ksm_is_cons(list) ){
    if( (*pred)(ksm_car(list)) )
      return ksm_car(list);
    list = ksm_cdr(list);
  }
  return ksm_nil_object;
}

static
void sweep(ksm_t list)
{
  ksm_t x;

  while( ksm_is_cons(list) ){
    x = ksm_car(list);
    if( ksm_is_dywin(x) ){
      sweep(ksm_cdr(list));
      ksm_funcall(ksm_dywin_before(x), ksm_nil_object, ksm_nil_object);
      break;
    }
    list = ksm_cdr(list);
  }
}

void ksm_sweep_jumps(void)
{
  sweep(JUMPS);
}

ksm_t ksm_rewind_jumps(ksm_t target)
{
  ksm_t j;

  while( ksm_is_cons(JUMPS) ){
    j = ksm_car(JUMPS);
    if( ksm_is_dywin(j) )
      ksm_funcall(ksm_dywin_after(j), ksm_nil_object, ksm_nil_object);
    JUMPS = ksm_cdr(JUMPS);
    if( j == target )
      return target;
  }
  return ksm_nil_object;
}

void ksm_push_jump(ksm_t jump)
{
  JUMPS = ksm_cons(jump, JUMPS);
}

void ksm_pop_jump(void)
{
  if( !ksm_is_cons(JUMPS) )
    ksm_error("jump stack underflow (ksm_pop_jump)");
  JUMPS = ksm_cdr(JUMPS);
}

void ksm_reset_jumps(void)
{
  JUMPS = ksm_nil_object;
}

ksm_t ksm_fetch_jumps(void)
{
  return JUMPS;
}

void ksm_set_jumps(ksm_t jump_list)
{
  JUMPS = jump_list;
}

void ksm_init_jumps(void)
{
  JUMPS = ksm_nil_object;
  //ksm_permanent(&jumps);
}
