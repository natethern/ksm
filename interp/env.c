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

/*** local **********************************************************/

#define MAKE_LOCAL(k,v,n) ksm_itype(KSM_FLAG_LOCAL,(int)(k),(int)(v),(int)(n))
#define MAKE_LOCAL_NO_LOCK(k,v,n) \
          ksm_itype_no_lock(KSM_FLAG_LOCAL,(int)(k),(int)(v),(int)(n))
#define IS_LOCAL(x)       (ksm_flag(x)==KSM_FLAG_LOCAL)
#define IS_LOCAL_NO_LOCK(x) (ksm_flag_no_lock(x)==KSM_FLAG_LOCAL)
#define LOCAL_KEY(local)  ((ksm_t)ksm_ival(local,0))
#define LOCAL_KEY_NO_LOCK(local)  ((ksm_t)ksm_ival_no_lock(local,0))
#define LOCAL_VAL(local)  ((ksm_t)ksm_ival(local,1))
#define LOCAL_VAL_NO_LOCK(local)  ((ksm_t)ksm_ival_no_lock(local,1))
#define LOCAL_NEXT(local) ((ksm_t)ksm_ival(local,2))
#define LOCAL_NEXT_NO_LOCK(local) ((ksm_t)ksm_ival_no_lock(local,2))
#define SET_LOCAL_VAL(local,v) ksm_set_ival(local,1,(int)(v))
#define SET_LOCAL_NEXT(local,n) ksm_set_ival(local,2,(int)(n))
#define SET_LOCAL_VAL_NO_LOCK(local,v) ksm_set_ival_no_lock(local,1,(int)(v))
#define SET_LOCAL_NEXT_NO_LOCK(local,n) ksm_set_ival_no_lock(local,2,(int)(n))

static
ksm_t mark_local(ksm_t local)
{
  ksm_mark_object(LOCAL_KEY_NO_LOCK(local));
  ksm_mark_object(LOCAL_VAL_NO_LOCK(local));
  return LOCAL_NEXT_NO_LOCK(local);
}

static 
void init_local(void)
{
  ksm_install_marker(KSM_FLAG_LOCAL, mark_local);
}

ksm_t ksm_prepend_local(ksm_t key, ksm_t val, ksm_t local)
{
  return MAKE_LOCAL(key, val, local);
}

int ksm_is_local(ksm_t x)
{ 
  return IS_LOCAL(x);
}

ksm_t ksm_local_key(ksm_t local)
{ 
  return LOCAL_KEY(local); 
}

ksm_t ksm_local_value(ksm_t local)
{ 
  return LOCAL_VAL(local); 
}

ksm_t ksm_local_next(ksm_t local)
{ 
  return LOCAL_NEXT(local); 
}

void ksm_set_local_value(ksm_t local, ksm_t val)
{ 
  SET_LOCAL_VAL(local, val); 
}

void ksm_set_local_value_no_lock(ksm_t local, ksm_t val)
{ 
  SET_LOCAL_VAL_NO_LOCK(local, val); 
}

int ksm_update_local(ksm_t key, ksm_t val, ksm_t local, ksm_t *next)
     // returns 0 if success, otherwise returns -1
{
  while( IS_LOCAL(local) ){
    if( key == LOCAL_KEY(local) ){
      ksm_update_binding(local, val);
      return 0;
    }
    local = LOCAL_NEXT(local);
  }
  if( next )
    *next = local;
  return -1;
}

ksm_t ksm_lookup_local(ksm_t key, ksm_t local, ksm_t *next)
     // returns 0 if not found
{
  while( IS_LOCAL(local) ){
    if( key == LOCAL_KEY(local) ){
      return LOCAL_VAL(local);
    }
    local = LOCAL_NEXT(local);
  }
  if( next )
    *next = local;
  return 0;
}

ksm_t ksm_search_local(ksm_t key, ksm_t local, ksm_t *last)
{
  while( IS_LOCAL(local) ){
    if( key == LOCAL_KEY(local) )
      return local;
    local = LOCAL_NEXT(local);
  }
  if( last )
    *last = local;
  return 0;
}

/*** dict ***********************************************************/

dict_impl *ksm_make_dict_impl(int size)
{
  dict_impl *p;
  ksm_t *q;
  int n;

  p = ksm_alloc(sizeof(*p));
  p->size = n = size;
  p->tab = q = ksm_alloc(sizeof(ksm_t)*size);
  while( n-- > 0 )
    *q++ = ksm_nil_object;
  p->entries = 0;
  return p;
}

void ksm_dispose_dict_impl(dict_impl *p)
{
  ksm_dispose(p->tab);
  ksm_dispose(p);
}

void ksm_mark_dict_impl(dict_impl *p)
{
  int n;
  ksm_t *q;

  n = p->size;
  q = p->tab;
  while( n-- > 0 )
    ksm_mark_object(*q++);
}

ksm_t ksm_search_dict_impl(ksm_t key, dict_impl *p)
     // returns a local, or 0 if no such entry and !create
     // should be called with ksm_lock()-ed
{
  int index;
  ksm_t q;

  index = ksm_key_hash_no_lock(key) % p->size;
  q = p->tab[index];
  while( IS_LOCAL_NO_LOCK(q) ){
    if( LOCAL_KEY_NO_LOCK(q) == key )
      return q;
    q = LOCAL_NEXT_NO_LOCK(q);
  }
  return 0;
}  

void ksm_enter_dict_impl(ksm_t key, ksm_t val, dict_impl *p)
     // should be called with ksm_lock()-ed
{
  int index;
  ksm_t bind;

  index = ksm_key_hash_no_lock(key) % p->size;
  bind = MAKE_LOCAL_NO_LOCK(key, val, p->tab[index]);
  p->tab[index] = bind;
}

/*** global *********************************************************/

#define MAKE_GLOBAL(p,n) \
  ksm_itype(KSM_FLAG_GLOBAL,(int)(p),(int)(n),0)
#define IS_GLOBAL(x)    (ksm_flag(x)==KSM_FLAG_GLOBAL)
#define GLOBAL_TAB(g)   ((ksm_dict_impl *)ksm_ival(g,0))
#define GLOBAL_TAB_NO_LOCK(g)   ((ksm_dict_impl *)ksm_ival_no_lock(g,0))
#define GLOBAL_NEXT(g)  ((ksm_t)ksm_ival(g,1))
#define GLOBAL_NEXT_NO_LOCK(g)  ((ksm_t)ksm_ival_no_lock(g,1))

ksm_t ksm_make_global(size_t size, ksm_t next_global)
{
  ksm_dict_impl *p;

  p = ksm_make_dict_impl(size);
  return MAKE_GLOBAL(p, next_global);
}

static
ksm_t mark_global(ksm_t g)
{
  ksm_dict_impl *p;

  p = GLOBAL_TAB_NO_LOCK(g);
  ksm_mark_dict_impl(p);
  return GLOBAL_NEXT_NO_LOCK(g);
}

static
void dispose_global(ksm_t g)
{
  ksm_dispose_dict_impl(GLOBAL_TAB_NO_LOCK(g));
}

static
void init_global(void)
{
  ksm_install_marker(KSM_FLAG_GLOBAL, mark_global);
  ksm_install_disposer(KSM_FLAG_GLOBAL, dispose_global);
}

int ksm_enter_global(ksm_t key, ksm_t val, ksm_t g, int create)
     // returns 0 if success
{
  ksm_dict_impl *p;
  ksm_t bind;

  ksm_lock();
  p = GLOBAL_TAB_NO_LOCK(g);
  bind = ksm_search_dict_impl(key, p);
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
}

int ksm_is_global(ksm_t x)
{
  return IS_GLOBAL(x);
}

ksm_t ksm_next_global(ksm_t global)
{
  return GLOBAL_NEXT(global);
}

ksm_t ksm_lookup_global(ksm_t key, ksm_t global, ksm_t *next)
{
  ksm_t bind;
  ksm_dict_impl *p;

  ksm_lock();
  p = GLOBAL_TAB_NO_LOCK(global);
  bind = ksm_search_dict_impl(key, p);
  if( bind ){
    ksm_unlock();
    return LOCAL_VAL(bind);
  }
  ksm_unlock();
  if( next )
    *next = GLOBAL_NEXT(global);
  return 0;
}

/*** init ***********************************************************/

ksm_t ksm_report_env;
ksm_t ksm_interact_env;

void ksm_start_interact_env(void)
{
  ksm_interact_env = ksm_make_global(1000, ksm_report_env);
}

ksm_t ksm_lookup(ksm_t key, ksm_t env)
{
  ksm_t x;

  if( (x = ksm_lookup_local(key, env, &env)) )
    return x;
  while( IS_GLOBAL(env) )
    if( (x = ksm_lookup_global(key, env, &env)) )
      return x;
  if( !ksm_is_nil(env) )
    ksm_error("can't happen in ksm_search: %k", env);
  return 0;
}

int ksm_update(ksm_t key, ksm_t val, ksm_t env)
     // returns 0 if success
{
  if( ksm_update_local(key, val, env, &env) == 0 )
    return 0;
  while( IS_GLOBAL(env) ){
    if( ksm_enter_global(key, val, env, 0) == 0 )
      return 0;
    env = GLOBAL_NEXT(env);
  }
  if( !ksm_is_nil(env) )
    ksm_error("can't happen in ksm_update");
  return -1;
}

void ksm_init_env(void)
{
  ksm_t global;

  init_local();
  init_global();
  global = ksm_make_global(500, ksm_nil_object);
  ksm_report_env = global;
  ksm_interact_env = global;
  ksm_permanent(&ksm_report_env);
  ksm_permanent(&ksm_interact_env);
}
