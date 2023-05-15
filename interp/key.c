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

#include <string.h>
#include <stdio.h>
#include "ksm.h"

#define MAKE_KEY(s,h,n)   ksm_itype(KSM_FLAG_KEY,(int)(s),(int)(h),(int)(n))
#define IS_KEY(x)         (ksm_flag(x)==KSM_FLAG_KEY)
#define KEY_PTR(x)        ((char *)ksm_ival(x,0))
#define KEY_PTR_NO_LOCK(x)  ((char *)ksm_ival_no_lock(x,0))
#define KEY_HASH(x)       ((unsigned)ksm_ival(x,1))
#define KEY_HASH_NO_LOCK(x) ((unsigned)ksm_ival_no_lock(x,1))
#define KEY_NEXT(x)       ((ksm_t)ksm_ival(x,2))
#define KEY_NEXT_NO_LOCK(x) ((ksm_t)ksm_ival_no_lock(x,2))
#define SET_KEY_NEXT(x,n) ksm_set_ival(x,2,(int)(n))
#define SET_KEY_NEXT_NO_LOCK(x,n) ksm_set_ival_no_lock(x,2,(int)(n))

#define KEYTAB_BITS   10
#define KEYTAB_SIZE   (1<<KEYTAB_BITS)
#define KEYTAB_MASK   (KEYTAB_SIZE-1)

static ksm_t keytab[KEYTAB_SIZE];  /*** GLOBAL ***/

static
void reset_key(void)
{
  bzero(&keytab[0], sizeof(ksm_t)*KEYTAB_SIZE);
}

static
ksm_t add_key(ksm_t p)
{
  unsigned i;

  i = KEY_HASH_NO_LOCK(p) & KEYTAB_MASK;
  SET_KEY_NEXT_NO_LOCK(p, keytab[i]);
  keytab[i] = p;
  return 0;
}

static
void dispose_key(ksm_t p)
{
  ksm_dispose(KEY_PTR_NO_LOCK(p));
}

unsigned int ksm_hash(char *s)
{
  unsigned int h = 0;

  while(*s){
    h = (h<<1)^(*s++);
  }
  return h;
}

ksm_t ksm_make_key(char *s)
{
  unsigned i, h;
  ksm_t p;
  char *t;

  h = ksm_hash(s);
  i = h & KEYTAB_MASK;
  ksm_lock();
  p = keytab[i];
  while(p){
    if( strcmp(s, KEY_PTR_NO_LOCK(p)) == 0 ){
      ksm_registry_add_no_lock(p);
      ksm_unlock();
      return p;
    }
    p = KEY_NEXT_NO_LOCK(p);
  }
  ksm_unlock();

  t = ksm_alloc(strlen(s)+1);
  strcpy(t,s);
  p = MAKE_KEY(t,h,0);
  ksm_lock();
  SET_KEY_NEXT_NO_LOCK(p, keytab[i]);
  keytab[i] = p;
  ksm_unlock();
  return p;
}

int ksm_is_key(ksm_t x)
{
  return IS_KEY(x);
}

char *ksm_key_value(ksm_t key)
{
  return KEY_PTR(key);
}

unsigned ksm_key_hash(ksm_t key)
{
  return KEY_HASH(key);
}

unsigned ksm_key_hash_no_lock(ksm_t key)
{
  return KEY_HASH_NO_LOCK(key);
}

int ksm_fprintf_key(FILE *fp, ksm_t key, int alt)
{
  return fprintf(fp, "%s", KEY_PTR(key));
}

static
ksm_pair eval_key(ksm_t key, ksm_t env)
{
  ksm_t val;

  val = ksm_lookup_local(key, env, &env);
  if( !val ){
    while( ksm_is_global(env) ){
      val = ksm_lookup_global(key, env, &env);
      if( val )
	break;
    }
  }
  if( !val ){
    if( ksm_is_nil(env) )
      ksm_error("unbound variable: %k", key);
    else
      ksm_error("can't happen in eval_key: %k", env);
  }
  return ksm_make_eval_done(val);
}

char *ksm_fetch_key(ksm_t key)
{
  return KEY_PTR(key);
}

int ksm_cvt_key_x(ksm_t x, ksm_t *result)
     // returns 0 if OK
{
  char *s;

  if( IS_KEY(x) ){
    *result = x;
    return 0;
  }
  if( ksm_string_value_x(x, &s) == 0 ){
    *result = ksm_make_key(s);
    return 0;
  }
  return -1;
}

ksm_t ksm_cvt_key(ksm_t x)
{
  ksm_t y;

  if( IS_KEY(x) )
    return x;
  if( ksm_cvt_key_x(x, &y) != 0 )
    ksm_error("can't convert to key: %k", x);
  return y;
}

void ksm_init_key(void)
{
  ksm_install_marker(KSM_FLAG_KEY, add_key);
  ksm_install_disposer(KSM_FLAG_KEY, dispose_key);
  ksm_add_gc_prolog(reset_key);
  ksm_install_evaluator(KSM_FLAG_KEY, eval_key);
  ksm_install_fprintf(KSM_FLAG_KEY, ksm_fprintf_key);
}

