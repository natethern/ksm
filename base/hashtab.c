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
#include "ksmext.h"

static int hashtab_flag;

#define MAKE_HASHTAB(p) ksm_itype(hashtab_flag, (int)p, 0, 0)
#define HASHTAB_TAB(x)  ((ksm_dict_impl *)ksm_ival(x, 0))
#define HASHTAB_TAB_NO_LOCK(x)  ((ksm_dict_impl *)ksm_ival_no_lock(x, 0))
#define IS_HASHTAB(x)   (ksm_flag(x)==hashtab_flag)

static
ksm_t mark_hashtab(ksm_t tab)
{
  ksm_dict_impl *p;

  p = HASHTAB_TAB_NO_LOCK(tab);
  ksm_mark_dict_impl(p);
  return 0;
}

static
void dispose_hashtab(ksm_t h)
{
  ksm_dispose_dict_impl(HASHTAB_TAB_NO_LOCK(h));
}

ksm_t ksm_make_hashtab(int size)
{
  ksm_dict_impl *p;

  p = ksm_make_dict_impl(size);
  return MAKE_HASHTAB(p);
}

int ksm_is_hashtab(ksm_t x)
{
  return IS_HASHTAB(x);
}

int ksm_hashtab_enter(ksm_t key, ksm_t val, ksm_t tab, int create)
{
  ksm_dict_impl *p;
  int ret;
  ksm_t bind;

  p = HASHTAB_TAB(tab);
  ksm_lock();
  bind= ksm_search_dict_impl(key, p);
  if( bind ){
    ksm_unlock();
    ksm_update_binding(bind, val);
    return 0;
  }
  else{
    if( create ){
      ksm_enter_dict_impl(key, val, p);
      ksm_unlock();
      return 0;
    }
    else{
      ksm_unlock();
      return -1;
    }
  }
  return ret;
}

ksm_t ksm_hashtab_lookup(ksm_t key, ksm_t tab)
{
  ksm_dict_impl *p;
  ksm_t bind;

  p = HASHTAB_TAB(tab);
  ksm_lock();
  bind = ksm_search_dict_impl(key, p);
  ksm_unlock();
  return bind ? ksm_local_value(bind) : 0;
}

KSM_DEF_BLT(make_hashtab)
     // (make-hashtab size)
{
  ksm_t size;
  int i;

  KSM_POP_ARG(size);
  KSM_SET_ARGS0();
  i = ksm_get_si(size);
  return ksm_make_hashtab(i);
}

KSM_DEF_BLT(hashtab_p)
     // (hashtab? x)
{
  ksm_t x;

  KSM_SET_ARGS1(x);
  return KSM_BOOL2OBJ(IS_HASHTAB(x));
}

KSM_DEF_BLT(hashtab_enter)
     // (hashtab-enter tab key val)
{
  ksm_t key, val, tab;

  KSM_SET_ARGS3(tab, key, val);
  if( !IS_HASHTAB(tab) )
    ksm_error("hashtab expected (HASHTAB-ENTER): %k", tab);
  if( ksm_hashtab_enter(key, val, tab, 1) != 0 )
    ksm_error("can't happen in blt_hashtab_enter");
  return ksm_unspec_object;
}

KSM_DEF_BLT(hashtab_lookup)
     // (hashtab-lookup tab key)
{
  ksm_t key, tab, x;

  KSM_SET_ARGS2(tab, key);
  if( !IS_HASHTAB(tab) )
    ksm_error("hashtab expected (HASHTAB-LOOKUP): %k", tab);
  x = ksm_hashtab_lookup(key, tab);
  if( !x )
    ksm_error("can't find binding (HASHTAB-LOOKUP): %k", key);
  return x;
}

KSM_DEF_BLT(hashtab_update)
     // (hashtab-update! tab key val)
{
  ksm_t key, val, tab;

  KSM_SET_ARGS3(tab, key, val);
  if( !IS_HASHTAB(tab) )
    ksm_error("hashtab expected (HASHTAB-UPDATE): %k", tab);
  if( ksm_hashtab_enter(key, val, tab, 0) != 0 )
    ksm_error("can't find binding (HASHTAB-UPDATE): %k", tab);
  return ksm_unspec_object;
}

KSM_DEF_BLT(hashtab_includes_p)
     // (hashtab-includes? tab key)
{
  ksm_t key, tab;

  KSM_SET_ARGS2(tab, key);
  if( !IS_HASHTAB(tab) )
    ksm_error("hashtab expected (HASHTAB-INCLUDES?): %k", tab);
  return KSM_BOOL2OBJ( ksm_hashtab_lookup(key, tab) != 0 );
}

void ksm_init_hashtab(void)
{
  hashtab_flag = ksm_new_flag();
  ksm_install_marker(hashtab_flag, mark_hashtab);
  ksm_install_disposer(hashtab_flag, dispose_hashtab);
  KSM_ENTER_BLT("make-hashtab", make_hashtab);
  KSM_ENTER_BLT("hashtab?", hashtab_p);
  KSM_ENTER_BLT("hashtab-enter!", hashtab_enter);
  KSM_ENTER_BLT("hashtab-lookup", hashtab_lookup);
  KSM_ENTER_BLT("hashtab-update!", hashtab_update);
  KSM_ENTER_BLT("hashtab-includes?", hashtab_includes_p);
}

