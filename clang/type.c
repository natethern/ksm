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
#include "ksm/clang.h"
#include "ksm/abbrev.h"

#define ALIGN_BITS    4
#define SIZE_BITS    22
#define KIND_BITS     6

#define ALIGN_MASK  ((1<<4)-1)
#define SIZE_MASK   ((1<<22)-1)
#define KIND_MASK   ((1<<6)-1)

#define ALIGN_MAX  ALIGN_MASK
#define SIZE_MAX   SIZE_MASK
#define KIND_MAX   KIND_MASK

#define MAKE_INFO(kind,size,align) \
  (((align)<<(SIZE_BITS+KIND_BITS))|((size)<<KIND_BITS)|(kind))
#define INFO_ALIGN(info)    (((info)>>(SIZE_BITS+KIND_BITS))&ALIGN_MASK)
#define INFO_SIZE(info)     (((info)>>KIND_BITS)&SIZE_MASK)
#define INFO_KIND(info)     ((info)&KIND_MASK)

static int ksm_type_flag;

#define MAKE_TYPE(kind,size,align) \
  ksm_itype(ksm_type_flag,MAKE_INFO(kind,size,align),(int)ksm_unspec_object,0)
#define IS_TYPE(x)     (ksm_flag(x)==ksm_type_flag)
#define TYPE_INFO(t)   ksm_ival(t,0)
#define TYPE_KIND(t)   INFO_KIND(ksm_ival(t,0))
#define TYPE_SIZE(t)   INFO_SIZE(ksm_ival(t,0))
#define TYPE_ALIGN(t)  INFO_ALIGN(ksm_ival(t,0))
#define TYPE_OBJECT(t) ((ksm_t)ksm_ival(t,1))
#define TYPE_RAW(t)    ksm_ival(t,2)
#define SET_TYPE_INFO(t,i)    ksm_set_ival(t,0,i)
#define SET_TYPE_OBJECT(t,x)  ksm_set_ival(t,1,(int)(x))
#define SET_TYPE_RAW(t,i)     ksm_set_ival(t,2,(int)(i))

#define TYPE_OBJECT_NO_LOCK(t) ((ksm_t)ksm_ival_no_lock(t,1))

static int alignment_pad(int offset, int align);

static
ksm_t mark_type(ksm_t type)
{
  ksm_mark_object(TYPE_OBJECT_NO_LOCK(type));
  return 0;
}

static
ksm_t ksm_make_type(int kind, int size, int align)
{
  if( kind > KIND_MAX )
    ksm_error("too many types (clang)");
  if( size > SIZE_MAX )
    ksm_error("too large type size (clang)");
  if( align > ALIGN_MAX )
    ksm_error("too large alignment (clang)");
  return MAKE_TYPE(kind, size, align);
}

static
void set_type_size(ksm_t t, int size)
{
  int info;

  if( size > SIZE_MAX )
    ksm_error("too large struct/union size: %d", size);
  info = TYPE_INFO(t);
  SET_TYPE_INFO(t, (info&(~(SIZE_MASK<<KIND_BITS)))|(size<<KIND_BITS));
}

static
void set_type_align(ksm_t t, int align)
{
  int info, bits;

  if( align > ALIGN_MAX )
    ksm_error("too large alignment: %d", align);
  info = TYPE_INFO(t);
  bits = SIZE_BITS+KIND_BITS;
  SET_TYPE_INFO(t, (info&(~(ALIGN_MASK<<bits)))|(align<<bits));
}

int ksm_clang_is_type(ksm_t x)
{
  return IS_TYPE(x);
}

int ksm_clang_type_size(ksm_t t)
{
  return TYPE_SIZE(t);
}

int ksm_clang_type_align(ksm_t t)
{
  return TYPE_ALIGN(t);
}

int ksm_clang_type_kind(ksm_t t)
{
  return TYPE_KIND(t);
}

ksm_t ksm_clang_type_int;
ksm_t ksm_clang_type_sint;
ksm_t ksm_clang_type_uint;
ksm_t ksm_clang_type_char;
ksm_t ksm_clang_type_schar;
ksm_t ksm_clang_type_uchar;
ksm_t ksm_clang_type_short;
ksm_t ksm_clang_type_sshort;
ksm_t ksm_clang_type_ushort;
ksm_t ksm_clang_type_float;
ksm_t ksm_clang_type_double;
ksm_t ksm_clang_type_longlong;
ksm_t ksm_clang_type_slonglong;
ksm_t ksm_clang_type_ulonglong;
ksm_t ksm_clang_type_void;
ksm_t ksm_clang_type_dots;

#define MTYPE(name,size,align) \
  ksm_make_type(KSM_CLANG_TYPE_##name,size,align)

static
void init_static_type(void)
{
  ksm_clang_type_int = MTYPE(INT, 4, 4);
  ksm_clang_type_sint = MTYPE(SINT, 4, 4);
  ksm_clang_type_uint = MTYPE(UINT, 4, 4);
  ksm_clang_type_char = MTYPE(CHAR, 1, 1);
  ksm_clang_type_schar = MTYPE(SCHAR, 1, 1);
  ksm_clang_type_uchar = MTYPE(UCHAR, 1, 1);
  ksm_clang_type_short = MTYPE(SHORT, 2, 2);
  ksm_clang_type_sshort = MTYPE(SSHORT, 2, 2);
  ksm_clang_type_ushort = MTYPE(USHORT, 2, 2);
  ksm_clang_type_float = MTYPE(FLOAT, 4, 4);
  ksm_clang_type_double = MTYPE(DOUBLE, 8, 8);
  ksm_clang_type_longlong = MTYPE(LONGLONG, 2, 2);
  ksm_clang_type_slonglong = MTYPE(SLONGLONG, 2, 2);
  ksm_clang_type_ulonglong = MTYPE(ULONGLONG, 2, 2);
  ksm_clang_type_void = MTYPE(VOID, 0, 1);
  ksm_clang_type_dots = MTYPE(DOTS, 0, 1);
}

#define MARK_STATIC_TYPE(x) \
  if( x ) ksm_mark_object(x)

static
void gc_prolog(void)
{
  MARK_STATIC_TYPE(ksm_clang_type_int);
  MARK_STATIC_TYPE(ksm_clang_type_sint);
  MARK_STATIC_TYPE(ksm_clang_type_uint);
  MARK_STATIC_TYPE(ksm_clang_type_char);
  MARK_STATIC_TYPE(ksm_clang_type_schar);
  MARK_STATIC_TYPE(ksm_clang_type_uchar);
  MARK_STATIC_TYPE(ksm_clang_type_short);
  MARK_STATIC_TYPE(ksm_clang_type_sshort);
  MARK_STATIC_TYPE(ksm_clang_type_ushort);
  MARK_STATIC_TYPE(ksm_clang_type_float);
  MARK_STATIC_TYPE(ksm_clang_type_double);
  MARK_STATIC_TYPE(ksm_clang_type_longlong);
  MARK_STATIC_TYPE(ksm_clang_type_slonglong);
  MARK_STATIC_TYPE(ksm_clang_type_ulonglong);
  MARK_STATIC_TYPE(ksm_clang_type_void);
  MARK_STATIC_TYPE(ksm_clang_type_dots);
}

/*** fun ************************************************************/

// FUN_OBJECT: (ret_type (type . name) ...)
//         name is #unspec if not specified
// FUN_RAW: flag

#define FUN_OBJECT(x)     TYPE_OBJECT(x)
#define FUN_RET_TYPE(x)   ksm_car(TYPE_OBJECT(x))
#define FUN_PARA(x)       ksm_cdr(TYPE_OBJECT(x))
#define FUN_DATA(x)       TYPE_RAW(x)
#define SET_FUN_DATA(x,d) SET_TYPE_RAW(x,d)

#define FUN_DOTS_MASK     1
#define FUN_UPAR_MASK     2

ksm_t ksm_clang_type_fun(ksm_t ret_type)
{
  ksm_t t;

  t = ksm_make_type(KSM_CLANG_TYPE_FUN, 4, 4);
  SET_TYPE_OBJECT(t, ksm_cons(ret_type, ksm_nil_object));
  return t;
}

int ksm_clang_type_is_fun(ksm_t t)
{
  return IS_TYPE(t) && TYPE_KIND(t) == KSM_CLANG_TYPE_FUN;
}

void ksm_clang_type_fun_para(ksm_t t, ksm_t name, ksm_t type)
{
  ksm_t x;

  if( ksm_clang_type_is_array(type) )
    type = ksm_clang_type_pointer(ksm_clang_type_array_ele(type));
  x = ksm_cons(type, name);
  ksm_set_cdr(ksm_last_cdr(FUN_OBJECT(t)), ksm_cons(x, ksm_nil_object));
}

int ksm_clang_type_fun_npara(ksm_t t)
{
  return ksm_length(FUN_PARA(t));
}

void ksm_clang_type_fun_ipara(ksm_t t, int i, ksm_t *name, ksm_t *type)
{
  ksm_t x;

  x = ksm_list_ref(FUN_PARA(t), i);
  *type = ksm_car(x);
  *name = ksm_cdr(x);
}

void ksm_clang_type_fun_set_ptype(ksm_t t, ksm_t name, ksm_t type)
{
  ksm_t x;

  if( !ksm_is_cons(x = ksm_assq(name, FUN_PARA(t))) )
    ksm_error("can't find parameter name: %k", name);
  ksm_set_car(x, type);
}

static
void sweep_para(ksm_t x)
{
  ksm_t type, name;

  type = ksm_car(x);
  name = ksm_cdr(x);
  if( ksm_is_unspec(type) )
    ksm_set_car(x, ksm_clang_type_int);
}

void ksm_clang_type_fun_sweep(ksm_t t)
{
  ksm_foreach(sweep_para, FUN_PARA(t));
}

ksm_t ksm_clang_type_fun_ret(ksm_t t)
{
  return FUN_RET_TYPE(t);
}

void ksm_clang_type_fun_set_dots(ksm_t t)
{
  int flag;

  flag = FUN_DATA(t);
  flag |= FUN_DOTS_MASK;
  SET_FUN_DATA(t, flag);
}

int ksm_clang_type_fun_has_dots(ksm_t t)
{
  return FUN_DATA(t) & FUN_DOTS_MASK;
}

void ksm_clang_type_fun_set_upar(ksm_t t)
     // unspecified formal parameters (e.g., int f();)
{
  int flag;

  flag = FUN_DATA(t);
  flag |= FUN_UPAR_MASK;
  SET_FUN_DATA(t, flag);
}

int ksm_clang_type_fun_has_upar(ksm_t t)
{
  return FUN_DATA(t) & FUN_UPAR_MASK;
}

int ksm_clang_type_fun_eq(ksm_t a, ksm_t b)
{
  ksm_t pa, pb;

  if( a == b )
    return 1;
  if( FUN_DATA(a) != FUN_DATA(b) )
    return 0;
  pa = FUN_PARA(a);
  pb = FUN_PARA(b);
  while( ksm_is_cons(pa) ){
    if( !ksm_is_cons(pb) )
      return 0;
    if( !ksm_clang_type_equal(ksm_caar(pa), ksm_caar(pb)) )
      return 0;
    pa = ksm_cdr(pa);
    pb = ksm_cdr(pb);
  }
  return ksm_is_nil(pb);
}

/*** pointer ********************************************************/

// POINTER_OBJECT: pointee-type
// POINTER_DATA: (not used)

#define POINTER_POINTEE(x)    TYPE_OBJECT(x)

ksm_t ksm_clang_type_pointer(ksm_t ele_type)
{
  ksm_t t;

  t = ksm_make_type(KSM_CLANG_TYPE_POINTER, 4, 4);
  SET_TYPE_OBJECT(t, ele_type);
  return t;
}

int ksm_clang_type_is_pointer(ksm_t t)
{
  return IS_TYPE(t) && TYPE_KIND(t) == KSM_CLANG_TYPE_POINTER;
}

ksm_t ksm_clang_type_pointer_pointee(ksm_t t)
{
  return POINTER_POINTEE(t);
}

int ksm_clang_type_pointer_eq(ksm_t p1, ksm_t p2)
{
  if( p1 == p2 )
    return 1;
  return ksm_clang_type_equal(POINTER_POINTEE(p1),
			      POINTER_POINTEE(p2));
}

/*** array **********************************************************/

// ARRAY_OBJECT: ele-type
// ARRAY_RAW: array-size

#define ARRAY_ELE_TYPE(x)    TYPE_OBJECT(x)
#define ARRAY_SIZE(x)        TYPE_RAW(x)

ksm_t ksm_clang_type_array(ksm_t ele_type, int size)
{
  ksm_t t;

  t = ksm_make_type(KSM_CLANG_TYPE_ARRAY, 
		    ksm_clang_type_size(ele_type)*size,
		    ksm_clang_type_align(ele_type));
  SET_TYPE_OBJECT(t, ele_type);
  SET_TYPE_RAW(t, size);
  return t;
}

int ksm_clang_type_is_array(ksm_t t)
{
  return IS_TYPE(t) && TYPE_KIND(t) == KSM_CLANG_TYPE_ARRAY;
}

int ksm_clang_type_array_size(ksm_t t)
{
  return ARRAY_SIZE(t);
}

ksm_t ksm_clang_type_array_ele(ksm_t array_type)
{
  return ARRAY_ELE_TYPE(array_type);
}

int ksm_clang_type_array_eq(ksm_t a, ksm_t b)
{
  if( a == b )
    return 1;
  return ksm_clang_type_equal(ARRAY_ELE_TYPE(a), ARRAY_ELE_TYPE(b)) &&
    ARRAY_SIZE(a) == ARRAY_SIZE(b);
}

/*** struct/union ***************************************************/

// STRUCT_OBJECT: (name (name type offset bits) ...)
// name is a key or #unspec
#define STRUCT_NAME(x)      ksm_car(TYPE_OBJECT(x))
#define STRUCT_FIELDS(x)    ksm_cdr(TYPE_OBJECT(x))
#define STRUCT_SET_FIELDS(x, fields) ksm_set_cdr(TYPE_OBJECT(x), fields)
#define MAKE_FIELD(type,name,offset,bits) \
  ksm_cons(name, ksm_list3(type,ksm_make_int(offset),ksm_make_int(bits)))
#define ADD_FIELD(x,type,name,offset,bits) \
  ksm_extend_list(TYPE_OBJECT(x),MAKE_FIELD(type,name,offset,bits))
#define NFIELDS(x)          ksm_length(STRUCT_FIELDS(x))
#define NTH_FIELD(x,n)      ksm_list_ref(STRUCT_FIELDS(x), n)
#define SEARCH_FIELD(x,name)  ksm_assq(name,STRUCT_FIELDS(x))
#define IS_FIELD(x)         ksm_is_cons(x)
#define FIELD_NAME(f)       first(f)
#define FIELD_TYPE(f)       second(f)
#define FIELD_OFFSET(f)     ksm_int_value(ksm_list_ref(f,2))
#define FIELD_BITS(f)       ksm_int_value(ksm_list_ref(f,3))
#define MAKE_BITS(lshift, rshift) ((lshift)<<16|(rshift))
#define BITS_LSHIFT(bits)         ((bits)>>16)
#define BITS_RSHIFT(bits)         ((bits)&0xFFFF)

ksm_t ksm_clang_type_struct(ksm_t name)
{
  ksm_t t;

  t = ksm_make_type(KSM_CLANG_TYPE_STRUCT, 0, 1);
  SET_TYPE_OBJECT(t, cons(name, nil));
  return t;
}

ksm_t ksm_clang_type_union(ksm_t name)
{
  ksm_t t;

  t = ksm_make_type(KSM_CLANG_TYPE_UNION, 0, 1);
  SET_TYPE_OBJECT(t, cons(name, nil));
  return t;
}

int ksm_clang_type_is_struct(ksm_t t)
{
  return IS_TYPE(t) && TYPE_KIND(t) == KSM_CLANG_TYPE_STRUCT;
}

int ksm_clang_type_is_union(ksm_t t)
{
  return IS_TYPE(t) && TYPE_KIND(t) == KSM_CLANG_TYPE_UNION;
}

int ksm_clang_type_struct_is_incomplete(ksm_t t)
{
  return TYPE_SIZE(t) == 0;
}

int ksm_clang_type_struct_add_field(ksm_t t, ksm_t name, ksm_t type, 
				    int cur_ofs)
     // returns next offset
{
  int size, align, pad, ofs;

  size = ksm_clang_type_size(type);
  align = ksm_clang_type_align(type);
  pad = alignment_pad(cur_ofs, align);
  ofs = cur_ofs + pad;
  ADD_FIELD(t, type, name, ofs, 0);
  ofs += size;
  if( TYPE_SIZE(t) < ofs )
    set_type_size(t, ofs);
  if( TYPE_ALIGN(t) < align )
    set_type_align(t, align);
  return ofs;
}

void ksm_clang_type_struct_add_bit_field(ksm_t t, ksm_t name, 
	ksm_t field_type, int offset, int bit_offset, int bit_length)
{
  int left, right;

  left = bit_offset;
  right = 32 - bit_length;
  ADD_FIELD(t, field_type, name, offset, MAKE_BITS(left, right));
  ++offset;
  if( ksm_clang_type_size(t) < offset )
    set_type_size(t, offset);
  if( ksm_clang_type_align(t) < 4 )
    set_type_align(t, 4);
}

void ksm_clang_type_struct_end_pad(ksm_t t)
{
  int pad;

  pad = alignment_pad(TYPE_SIZE(t), TYPE_ALIGN(t));
  set_type_size(t, TYPE_SIZE(t)+pad);
}

int ksm_clang_type_struct_nfield(ksm_t t)
{
  return NFIELDS(t);
}

void ksm_clang_type_struct_ifield(ksm_t t, int i, ksm_t *name, ksm_t *type, 
				  int *offset)
{
  ksm_t p;

  p = NTH_FIELD(t, i);
  *name = FIELD_NAME(p);
  *type = FIELD_TYPE(p);
  *offset = FIELD_OFFSET(p);
}

int ksm_clang_type_struct_search(ksm_t t, ksm_t name, ksm_t *type, 
    int *offset, int *left_shift, int *right_shift)
     /* returns 0 if found, -1 if not found */
{
  ksm_t p;
  int bits;

  p = SEARCH_FIELD(t, name);
  if( IS_FIELD(p) ){
    *type = FIELD_TYPE(p);
    *offset = FIELD_OFFSET(p);
    bits = FIELD_BITS(p);
    *left_shift = BITS_LSHIFT(bits);
    *right_shift = BITS_RSHIFT(bits);
    return 0;
  }
  else
    return -1;
}

void ksm_clang_type_struct_set_fields(ksm_t struct_type_dst, 
				      ksm_t struct_type_src)
{
  STRUCT_SET_FIELDS(struct_type_dst, STRUCT_FIELDS(struct_type_src));
}

int ksm_clang_type_struct_eq(ksm_t a, ksm_t b)
{
  ksm_t namea, nameb;

  namea = STRUCT_NAME(a);
  nameb = STRUCT_NAME(b);
  return ((ksm_is_unspec(namea) && ksm_is_unspec(nameb)) || 
    (namea == nameb)) &&
    ksm_equal(STRUCT_FIELDS(a), STRUCT_FIELDS(b));
}

/*** enum ***********************************************************/

// ENUM_OBJECT: ((name . value) ...)

#define ENUM_LIST(x)           TYPE_OBJECT(x)
#define SET_ENUM_LIST(x,list)  SET_TYPE_OBJECT(x,list)
#define ENUM_ENTRY(name,value) ksm_cons(name,ksm_make_int(value))
#define ENUM_ENTRY_NAME(e)     ksm_car(e)
#define ENUM_ENTRY_VALUE(e)    ksm_int_value(ksm_cdr(e))

ksm_t ksm_clang_type_enum(void)
{
  return ksm_make_type(KSM_CLANG_TYPE_ENUM, 4, 4);
}

int ksm_clang_type_is_enum(ksm_t x)
{
  return IS_TYPE(x) && TYPE_KIND(x) == KSM_CLANG_TYPE_ENUM;
}

static
int enum_add(ksm_t t, ksm_t key, int value, int supplied_p)
     // returns enum value
{
  ksm_t q, x;

  q = ENUM_LIST(t);
  if( !ksm_is_cons(q) ){
    SET_TYPE_OBJECT(t, ksm_list1(ENUM_ENTRY(key, supplied_p?value:0)));
    return 0;
  }
  while( !ksm_is_nil(ksm_cdr(q)) ){
    x = ksm_car(q);
    q = ksm_cdr(q);
    if( key == ENUM_ENTRY_NAME(x) )
      ksm_error("duplicate ENUM definition: %k", key);
  }
  if( !supplied_p )
    value = ENUM_ENTRY_VALUE(ksm_car(q)) + 1;
  ksm_set_cdr(q, ksm_list1(ENUM_ENTRY(key, value)));
  return value;
}

int ksm_clang_type_enum_add(ksm_t t, ksm_t key)
{
  return enum_add(t, key, 0, 0);
}

void ksm_clang_type_enum_addx(ksm_t t, ksm_t key, int value)
{
  enum_add(t, key, value, 1);
}

int ksm_clang_type_enum_includes(ksm_t t, ksm_t key)
{
  ksm_t x;

  x = ksm_assq(key, ENUM_LIST(t));
  return ksm_is_cons(x);
}

/*** equal **********************************************************/

int ksm_clang_type_equal(ksm_t a, ksm_t b)
{
  if( a == b )
    return 1;
  if( !(IS_TYPE(a) && IS_TYPE(b)) )
    return 0;
  if( TYPE_KIND(a) != TYPE_KIND(b) ){
    if( ksm_clang_type_is_array(a) && ksm_clang_type_is_pointer(b) )
      return ksm_clang_type_equal(ksm_clang_type_array_ele(a), 
				  ksm_clang_type_pointer_pointee(b));
    if( ksm_clang_type_is_array(b) && ksm_clang_type_is_pointer(a) )
      return ksm_clang_type_equal(ksm_clang_type_array_ele(b), 
				  ksm_clang_type_pointer_pointee(a));
    return 0;
  }
  switch(TYPE_KIND(a)){
  case KSM_CLANG_TYPE_STRUCT: return ksm_clang_type_struct_eq(a, b);
  case KSM_CLANG_TYPE_POINTER: return ksm_clang_type_pointer_eq(a, b);
  case KSM_CLANG_TYPE_ARRAY: return ksm_clang_type_array_eq(a, b);
  case KSM_CLANG_TYPE_FUN: return ksm_clang_type_fun_eq(a, b);
  case KSM_CLANG_TYPE_ENUM: return 0;
  default: return 1;
  }
}

/*** pad ************************************************************/

static
int alignment_pad(int offset, int align)
{
  int r;

  r = offset % align;
  return r ? align - r : r;
}

/*** init ***********************************************************/

void ksm_clang_init_type(void)
{
  ksm_type_flag = ksm_new_flag();
  ksm_install_marker(ksm_type_flag, mark_type);
  ksm_add_gc_prolog(gc_prolog);
  ksm_install_equal(ksm_type_flag, ksm_clang_type_equal);
  init_static_type();
}
