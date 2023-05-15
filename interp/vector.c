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

#define MAKE_VEC(size,buf) ksm_itype(KSM_FLAG_VECTOR,(int)size,(int)(buf),0)
#define IS_VEC(x)          (ksm_flag(x)==KSM_FLAG_VECTOR)
#define VEC_SIZE(v)        ksm_ival(v,0)
#define VEC_BUF(v)         ((ksm_t *)ksm_ival(v,1))

#define VEC_SIZE_NO_LOCK(v)        ksm_ival_no_lock(v,0)
#define VEC_BUF_NO_LOCK(v)         ((ksm_t *)ksm_ival_no_lock(v,1))

ksm_t ksm_make_vector(int size, ksm_t fill)
{
  ksm_t *p, *q;
  int i;

  p = q = ksm_alloc(sizeof(ksm_t)*size);
  for(i=0;i<size;++i)
    *q++ = fill;
  return MAKE_VEC(size, p);
}

int ksm_is_vector(ksm_t x)
{
  return IS_VEC(x);
}

int ksm_vector_size(ksm_t vec)
{
  return  VEC_SIZE(vec);
}

ksm_t ksm_vector_ref(ksm_t vec, int i)
{
  ksm_t x, *v;

  ksm_lock();
  v = VEC_BUF_NO_LOCK(vec);
  x = v[i];
  ksm_unlock();
  return x;
}

void ksm_vector_set(ksm_t vec, int i, ksm_t val)
{
  ksm_t *v;

  ksm_lock();
  v = VEC_BUF_NO_LOCK(vec);
  v[i] = val;
  ksm_unlock();
}

static
ksm_t mark_vec(ksm_t vec)
{
  int size;
  ksm_t *p;

  size = VEC_SIZE_NO_LOCK(vec);
  p = VEC_BUF_NO_LOCK(vec);
  while( size-- > 0 )
    ksm_mark_object(*p++);
  return 0;
}

static
void dispose_vec(ksm_t vec)
{
  ksm_dispose(VEC_BUF_NO_LOCK(vec));
}

static
ksm_t read_vector(char lead, ksm_t port)
{
  ksm_t list;

  ksm_port_ungetc(lead, port);
  list = ksm_read_object(port);
  if( ksm_length(list) < 0 )
    ksm_error("can't read vector: %k", list);
  return ksm_list_to_vector(list);
}

static
int fprint_vec(FILE *fp, ksm_t vec, int alt)
{
  int size, n;
  ksm_t *p;

  size = VEC_SIZE(vec);
  p = VEC_BUF(vec);
  n = fprintf(fp, "#(");
  while( size-- > 0 )
    n += fprintf(fp, alt?"%#k%s":"%k%s", *p++, (size>0)?" ":"");
  return n + fprintf(fp, ")");
}

ksm_t ksm_vector_to_list(ksm_t vec)
{
  int size, i;
  ksm_t *p, head, tail;

  size = VEC_SIZE(vec);
  p = VEC_BUF(vec);
  ksm_list_begin(&head, &tail);
  for(i=0;i<size;++i)
    ksm_list_extend(&head, &tail, *p++);
  return head;
}

ksm_t ksm_list_to_vector(ksm_t list)
{
  int n;
  ksm_t vec, *p;

  n = ksm_length(list);
  if( n < 0 )
    ksm_error("proper list expected: %k", list);
  vec = ksm_make_vector(n, ksm_unspec_object);
  p = VEC_BUF(vec);
  while( n-- > 0 ){
    *p++ = ksm_car(list);
    list = ksm_cdr(list);
  }
  return vec;
}

void ksm_vector_fill(ksm_t vec, ksm_t fill)
{
  int size;
  ksm_t *p;

  size = VEC_SIZE(vec);
  p = VEC_BUF(vec);
  while( size-- > 0 )
    *p++ = fill;
}

static
int equal_vector(ksm_t a, ksm_t b)
{
  int na, nb, i;

  if( !IS_VEC(b) )
    return 0;
  na = ksm_vector_size(a);
  nb = ksm_vector_size(b);
  if( na != nb )
    return 0;
  for(i=0;i<na;++i)
    if( !ksm_equal(ksm_vector_ref(a, i), ksm_vector_ref(b, i)) )
      return 0;
  return 1;
}

KSM_DEF_BLT(vector_p)
     // (vector? x)
{
  ksm_t x;

  KSM_SET_ARGS1(x);
  return KSM_BOOL2OBJ(IS_VEC(x));
}

KSM_DEF_BLT(make_vector)
     // (make-vector k [fill]
{
  ksm_t k, fill;
  int size;

  KSM_POP_ARG(k);
  size = ksm_int_value(k);
  if( KSM_MORE_ARGS() ){
    KSM_POP_ARG(fill);
    KSM_SET_ARGS0();
  }
  else
    fill = ksm_unspec_object;
  return ksm_make_vector(size, fill);
}

KSM_DEF_BLT(vector)
     // (vector obj ...)
{
  int n;
  ksm_t v, *p, seq;

  seq = KSM_ARGS();
  n = ksm_length(seq);
  if( n < 0 )
    ksm_error("incompatible args to VECTOR: %k", KSM_ARGS());
  v = ksm_make_vector(n, ksm_unspec_object);
  p = VEC_BUF(v);
  while( n-- > 0 ){
    *p++ = ksm_car(seq);
    seq = ksm_cdr(seq);
  }
  return v;
}

KSM_DEF_BLT(vector_length)
     // (vector-length vector)
{
  ksm_t vec;

  KSM_SET_ARGS1(vec);
  if( !IS_VEC(vec) )
    ksm_error("incompatible arg to VECTOR-LENGTH: %k", vec);
  return ksm_make_int(VEC_SIZE(vec));
}

KSM_DEF_BLT(vector_ref)
     // (vector-ref vector k)
{
  ksm_t vec, k;
  int i;

  KSM_SET_ARGS2(vec, k);
  if( !IS_VEC(vec) )
    ksm_error("incompatible first arg to VECTOR-REF: %k", vec);
  i = ksm_int_value(k);
  if( !(i >= 0 && i < VEC_SIZE(vec)) )
    ksm_error("vector index out of range: %d", i);
  return VEC_BUF(vec)[i];
}

KSM_DEF_BLT(vector_set)
     // (vector-set! vector k val)
{
  ksm_t vec, k, val;
  int i;

  KSM_SET_ARGS3(vec, k, val);
  if( !IS_VEC(vec) )
    ksm_error("incompatible first arg to VECTOR-SET!: %k", vec);
  i = ksm_int_value(k);
  if( !(i >= 0 && i < VEC_SIZE(vec)) )
    ksm_error("vector index out of range: %d", i);
  VEC_BUF(vec)[i] = val;
  return ksm_unspec_object;
}

KSM_DEF_BLT(vector_to_list)
     // (vector->list vector)
{
  ksm_t vec;

  KSM_SET_ARGS1(vec);
  if( !IS_VEC(vec) )
    ksm_error("incompatible arg to VECTOR->LIST: %k", vec);
  return ksm_vector_to_list(vec);
}

KSM_DEF_BLT(list_to_vector)
     // (list->vector list)
{
  ksm_t list;

  KSM_SET_ARGS1(list);
  return ksm_list_to_vector(list);
}

KSM_DEF_BLT(vector_fill)
     // (vector-fill! vector fill)
{
  ksm_t vec, fill;
  

  KSM_SET_ARGS2(vec, fill);
  if( !IS_VEC(vec) )
    ksm_error("incompatible first arg to VECTOR-FILL!: %k", vec);
  ksm_vector_fill(vec, fill);
  return ksm_unspec_object;
}

KSM_DEF_BLT(set_vect)
     // (set_vect args val)
{
  ksm_t my_args, val, vec, idx;
  int i;

  KSM_SET_ARGS2(my_args, val);
  if( ksm_length(my_args) != 2 )
    ksm_error("invalid arg to vector-setter: %k", my_args);
  vec = ksm_first(my_args);
  idx = ksm_second(my_args);
  if( !IS_VEC(vec) )
    ksm_error("vector expected: %k", vec);
  i = ksm_get_si(idx);
  if( !(i >= 0 && i < VEC_SIZE(vec)) )
    ksm_error("index out of range: %d", i);
  ksm_vector_set(vec, i, val);
  return ksm_unspec_object;
}

void ksm_init_vector(void)
{
  ksm_install_marker(KSM_FLAG_VECTOR, mark_vec);
  ksm_install_disposer(KSM_FLAG_VECTOR, dispose_vec);
  ksm_install_sharp_reader('(', read_vector);
  ksm_install_fprintf(KSM_FLAG_VECTOR, fprint_vec);
  ksm_install_equal(KSM_FLAG_VECTOR, equal_vector);
  KSM_ENTER_BLT("vector?", vector_p);
  KSM_ENTER_BLT("make-vector", make_vector);
  KSM_ENTER_BLT("vector", vector);
  KSM_ENTER_BLT("vector-length", vector_length);
  KSM_ENTER_BLT("vector-ref", vector_ref);
  KSM_ENTER_BLT("vector-set!", vector_set);
  KSM_ENTER_BLT("vector->list", vector_to_list);
  KSM_ENTER_BLT("list->vector", list_to_vector);
  KSM_ENTER_BLT("vector-fill!", vector_fill);
  KSM_ENTER_BLT_SETTER("vector-ref", set_vect);
}
