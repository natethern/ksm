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

#include <dlfcn.h>
#include <limits.h>
#include <printf.h>
#include "ksm.h"
#include "ksmext.h"
#include "ksm/abbrev.h"
#include "ksm/clang.h"
#include "ksm/callframe.h"

#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT 0
#endif

static ksm_t Ksm_t;   // type ksm_t

void ksm_clang_register_types(void)
{
  Ksm_t = ksm_clang_find_type(ksm_make_key("clang:ksm_t"));
}

static ksm_t key_bra;
static ksm_t key_star;
static ksm_t key_struct;
static ksm_t key_type;
static ksm_t key_union;

#define MARK(x) if(x) ksm_mark_object(x)

static void mark_keys(void)
{
  MARK(key_bra);
  MARK(key_struct);
  MARK(key_star);
  MARK(key_type);
  MARK(key_union);
}

static void init_keys(void)
{
  key_bra = ksm_make_key("[]");
  key_star = ksm_make_key("*");
  key_struct = ksm_make_key("struct");
  key_type = ksm_make_key("type");
  key_union = ksm_make_key("union");
}

/*** type-tab *******************************************************/

static ksm_t type_tab;

// type_tab entry: ((TYPE . type) (STRUCT . type) (UNION . type) ...)

static
void type_tab_enter(ksm_t key, ksm_t category, ksm_t type)
{
  ksm_t x;

  x = ksm_hashtab_lookup(key, type_tab);
  if( !x )
    ksm_hashtab_enter(key, ksm_list1(ksm_cons(category, type)), type_tab, 1);
  else
    ksm_hashtab_enter(key, ksm_cons(ksm_cons(category, type), x),
		      type_tab, 0);
}

static
ksm_t type_tab_lookup(ksm_t key, ksm_t category)
     // returns a cons (category . type)  if success, #f otherwise
{
  ksm_t x;

  x = ksm_hashtab_lookup(key, type_tab);
  return x ? ksm_assq(category, x) : ksm_false_object;
}

ksm_t ksm_clang_find_type(ksm_t name)
{
  ksm_t bind;

  bind = type_tab_lookup(name, key_type);
  if( !ksm_is_cons(bind) )
    ksm_error("can't find type: %k", name);
  return ksm_cdr(bind);
}

static
void install_type(ksm_t key, ksm_t type)
{
  ksm_t pair;
  
  pair = type_tab_lookup(key, key_type);
  if( is_cons(pair) ){
    if( !ksm_clang_type_equal(cdr(pair), type) )
      ksm_error("inconsistent type definition (clang): %k", type);
  }
  else  // no previous entry
    type_tab_enter(key, key_type, type);
}

static
ksm_t find_type(ksm_t key, int invoke_error)
     // return #unspec if not found and invoke_error is zero
{
  ksm_t pair;

  pair = type_tab_lookup(key, key_type);
  if( !is_cons(pair) ){
    if( invoke_error )
      ksm_error("can't find type (clang): %k", key);
    return unspec;
  }
  return cdr(pair);
}

static
ksm_t install_struct(ksm_t key, ksm_t type)
{
  ksm_t pair, tt;

  pair = type_tab_lookup(key, key_struct);
  if( !is_cons(pair) ){
    type_tab_enter(key, key_struct, type);
    return type;
  }
  else{
    tt = cdr(pair);
    if( tt == type || ksm_clang_type_struct_is_incomplete(type) ||
	ksm_clang_type_struct_eq(tt, type) )
      return tt;
    if( ksm_clang_type_struct_is_incomplete(tt) ){
      ksm_clang_type_struct_set_fields(tt, type);
      return type;
    }
    ksm_error("inconsistent struct definition: %k", type);
    return type;
  }
}

#define INSTALL_TYPE(s,t) \
  install_type(ksm_make_key(s), t)

static
void init_type_tab(void)
{
  type_tab = ksm_make_hashtab(128);
  ksm_permanent(&type_tab);
  INSTALL_TYPE("int", ksm_clang_type_int);
  INSTALL_TYPE("signed-int", ksm_clang_type_sint);
  INSTALL_TYPE("unsigned-int", ksm_clang_type_uint);
  INSTALL_TYPE("char", ksm_clang_type_char);
  INSTALL_TYPE("signed-char", ksm_clang_type_schar);
  INSTALL_TYPE("unsigned-char", ksm_clang_type_uchar);
  INSTALL_TYPE("short", ksm_clang_type_short);
  INSTALL_TYPE("signed-short", ksm_clang_type_sshort);
  INSTALL_TYPE("unsigned-short", ksm_clang_type_ushort);
  INSTALL_TYPE("float", ksm_clang_type_float);
  INSTALL_TYPE("double", ksm_clang_type_double);
  INSTALL_TYPE("long-long", ksm_clang_type_longlong);
  INSTALL_TYPE("signed-long-long", ksm_clang_type_slonglong);
  INSTALL_TYPE("unsigned-long-long", ksm_clang_type_ulonglong);
  INSTALL_TYPE("long", ksm_clang_type_int);
  INSTALL_TYPE("signed-long", ksm_clang_type_sint);
  INSTALL_TYPE("unsigned-long", ksm_clang_type_uint);
  INSTALL_TYPE("void", ksm_clang_type_void);
  INSTALL_TYPE("...", ksm_clang_type_dots);
}

/*** csym ***********************************************************/

static int csym_flag;

#define MAKE_CSYM(type,adr) \
  ksm_itype(csym_flag, (int)(type), (int)(adr), 0)
#define IS_CSYM(x)   (ksm_flag(x)==csym_flag)
#define CSYM_TYPE(x) ((ksm_t)ksm_ival(x,0))
#define CSYM_ADR(x)  ((void *)ksm_ival(x,1))
#define CSYM_AUX(x)  ((void *)ksm_ival(x,2))
#define SET_CSYM_TYPE(x,t) ksm_set_ival(x,0,(int)(t))
#define SET_CSYM_AUX(x,p)  ksm_set_ival(x,2,(int)(p))
#define CSYM_TYPE_KIND(x)  ksm_clang_type_kind(CSYM_TYPE(x))

#define CSYM_TYPE_NO_LOCK(x) ((ksm_t)ksm_ival_no_lock(x,0))

ksm_t ksm_clang_make_csym(ksm_t type, void *adr)
{
  return MAKE_CSYM(type, adr);
}

ksm_t ksm_clang_resolve_csym(ksm_t type, ksm_t ident)
{
  char *s;
  void *p;

  s = ksm_string_value(ident);
  p = dlsym(RTLD_DEFAULT, s);
  if( p == 0 )
    ksm_error("can't find C symbol: %s", s);
  return MAKE_CSYM(type, p);
}

int ksm_clang_is_csym(ksm_t x)
{
  return IS_CSYM(x);
}

ksm_t ksm_clang_csym_type(ksm_t x)
{
  return CSYM_TYPE(x);
}

void *ksm_clang_csym_adr(ksm_t x)
{
  return CSYM_ADR(x);
}

static
ksm_t mark_csym(ksm_t x)
{
  ksm_mark_object(CSYM_TYPE_NO_LOCK(x));
  return 0;
}

static ksm_t parse_type(ksm_t spec, ksm_t env);
static ksm_t parse_fun_type(ksm_t ret_type, ksm_t arg_types, ksm_t env);
static ksm_t parse_struct_x(ksm_t spec, ksm_t env, int union_p);
#define parse_struct(spec, env) parse_struct_x(spec, env, 0)
#define parse_union(spec, env) parse_struct_x(spec, env, 1)

static
ksm_t find_csym(ksm_t spec, ksm_t env)
     // returns CSYM
     // spec: (type symbol)
     // spec: (type (symbol type ...))
{
  ksm_t type, symbol, args;
  void *p;
  char *s;

  if( ksm_length(spec) != 2 )
    ksm_error("incompatible C-type spec: %k", spec);
  type = parse_type(car(spec), env);
  if( is_cons(second(spec)) ){
    symbol = caadr(spec);
    args = cdadr(spec);
    type = parse_fun_type(type, args, env);
  }
  else
    symbol = cadr(spec);
  s = ksm_string_value(symbol);
  p = dlsym(RTLD_DEFAULT, s);
  if( !p )
    ksm_error("can't find C symbol: %s", s);
  return MAKE_CSYM(type, p);
}

/*** print **********************************************************/

static
int fprintf_csym(FILE *fp, ksm_t x, int alt)
{
  switch(CSYM_TYPE_KIND(x)){
  case KSM_CLANG_TYPE_INT:
    return ksm_fprintf_int(fp, *(int *)CSYM_ADR(x), alt);
  case KSM_CLANG_TYPE_SINT:
    return fprintf(fp, "%d", *(signed int *)CSYM_ADR(x));
  case KSM_CLANG_TYPE_UINT:
    return fprintf(fp, "%u", *(unsigned int *)CSYM_ADR(x));
  case KSM_CLANG_TYPE_CHAR:
  case KSM_CLANG_TYPE_SCHAR:
  case KSM_CLANG_TYPE_UCHAR:
    return ksm_fprintf_char(fp, *(char *)CSYM_ADR(x), alt);
  case KSM_CLANG_TYPE_SHORT:
  case KSM_CLANG_TYPE_SSHORT:
    return ksm_fprintf_int(fp, *(short *)CSYM_ADR(x), alt);
  case KSM_CLANG_TYPE_USHORT:
    return ksm_fprintf_int(fp, *(unsigned short *)CSYM_ADR(x), alt);
  case KSM_CLANG_TYPE_FLOAT:
    return ksm_fprintf_double(fp, *(float *)CSYM_ADR(x), alt);
  case KSM_CLANG_TYPE_DOUBLE:
    return ksm_fprintf_double(fp, *(double *)CSYM_ADR(x), alt);
  case KSM_CLANG_TYPE_LONGLONG:
  case KSM_CLANG_TYPE_SLONGLONG:
    return fprintf(fp, "%Ld", *(long long *)CSYM_ADR(x), alt);
  case KSM_CLANG_TYPE_ULONGLONG:
    return fprintf(fp, "%Lu", *(unsigned long long *)CSYM_ADR(x), alt);
  default:
    return ksm_default_fprintf(fp, x, alt);
  }
}

/*** set ************************************************************/

static
int set_csym(void *adr, ksm_t type, ksm_t val)
     // returns 0 if OK
{
  switch(ksm_clang_type_kind(type)){
  case KSM_CLANG_TYPE_INT:
  case KSM_CLANG_TYPE_SINT:
  case KSM_CLANG_TYPE_UINT:
    *(int *)adr = ksm_get_si(val);
    break;
  case KSM_CLANG_TYPE_CHAR:
  case KSM_CLANG_TYPE_SCHAR:
  case KSM_CLANG_TYPE_UCHAR:
    *(char *)adr = ksm_get_si(val);
    break;
  case KSM_CLANG_TYPE_SHORT:
  case KSM_CLANG_TYPE_SSHORT:
  case KSM_CLANG_TYPE_USHORT:
    *(short *)adr = ksm_get_si(val);
    break;
  case KSM_CLANG_TYPE_FLOAT:
    *(float *)adr = ksm_get_d(val);
    break;
  case KSM_CLANG_TYPE_DOUBLE:
    *(double *)adr = ksm_get_d(val);
    break;
  case KSM_CLANG_TYPE_POINTER:
    {
      ksm_t pointee;

      if( ksm_is_int(val) && ksm_int_value(val) == 0 ){
	*(void **)adr = 0;
	break;
      }
      if( ksm_clang_is_csym(val) ){
	if( ksm_clang_type_equal(type, ksm_clang_csym_type(val)) ){
	  *(void **)adr = *(void **)CSYM_ADR(val);
	  break;
	}
	pointee = ksm_clang_type_pointer_pointee(type);
	if( ksm_clang_type_is_fun(pointee) ){
	  if( ksm_clang_type_equal(pointee, ksm_clang_csym_type(val)) ){
	    *(void **)adr = CSYM_ADR(val);
	    break;
	  }
	}
      }
      return -1;
    }
  default:
    if( ksm_clang_is_csym(val) &&
	ksm_clang_type_equal(type, ksm_clang_csym_type(val)) )
      memcpy(adr, CSYM_ADR(val), ksm_clang_type_size(type));
    else
      return -1;
  }
  return 0;
}

static
void update_csym(ksm_t local, ksm_t val)
{
  ksm_t csym;

  csym = ksm_local_value(local);
  if( set_csym(CSYM_ADR(csym), CSYM_TYPE(csym), val) != 0 )
    ksm_error("can't update value in UPDATE_CSYM: %k", val);
}

KSM_DEF_SPC(set_csym)
     // (set_csym args val)
{
  ksm_t my_args, val, csym;

  KSM_SET_ARGS2(my_args, val);
  csym = find_csym(my_args, KSM_ENV());
  if( set_csym(CSYM_ADR(csym), CSYM_TYPE(csym), val) != 0 )
    ksm_error("can't set value in SET_CSYM: %k", val);
  return ksm_unspec_object;
}

KSM_DEF_BLT(set_deref)
     // (set_deref (csym) val)
{
  ksm_t my_args, val, csym, type;
  void *adr;

  KSM_SET_ARGS2(my_args, val);
  if( ksm_length(my_args) != 1 )
    ksm_error("can't handle SET_DEREF: %k", my_args);
  csym = ksm_car(my_args);
  if( !IS_CSYM(csym) )
    ksm_error("csym expected in SET_DEREF: %k", csym);
  type = CSYM_TYPE(csym);
  if( !ksm_clang_type_is_pointer(type) )
    ksm_error("pointer type expected in SET_DEREF: %k", type);
  adr = *(void **)CSYM_ADR(csym);
  type = ksm_clang_type_pointer_pointee(type);
  if( set_csym(adr, type, val) != 0 )
    ksm_error("can't set value in SET_DEREF: %k", val);
  return ksm_unspec_object;
}

void ksm_clang_aset(ksm_t csym, int i, ksm_t val)
{
  ksm_t type;
  char *p;

  if( !IS_CSYM(csym) )
    ksm_error("csym expected: %k", csym);
  type = CSYM_TYPE(csym);
  p = CSYM_ADR(csym);
  if( ksm_clang_type_is_array(type) ){
    type = ksm_clang_type_array_ele(CSYM_TYPE(csym));
    p = CSYM_ADR(csym);
  }
  else if( ksm_clang_type_is_pointer(type) ){
    type = ksm_clang_type_pointer_pointee(type);
    p = *(void **)CSYM_ADR(csym);
  }
  else
    ksm_error("clang pointer/array expected: %k", csym);
  p += ksm_clang_type_size(type) * i;
  if( set_csym(p, type, val) != 0 )
    ksm_error("can't set value in SET_AREF: %k", val);
}

KSM_DEF_BLT(set_aref)
     // (set_aref args val)
     // args: (csym index)
{
  ksm_t my_args, val, csym;
  int i;

  KSM_SET_ARGS2(my_args, val);
  if( length(my_args) != 2 )
    ksm_error("can't handle SET_AREF: %k", my_args);
  csym = car(my_args);
  if( !IS_CSYM(csym) )
    ksm_error("csym expected: %k", csym);
  i = ksm_int_value(second(my_args));
  ksm_clang_aset(csym, i, val);
  return unspec;
}

/***
KSM_DEF_BLT(set_aref)
     // (set_aref args val)
     // args: (csym index)
{
  ksm_t my_args, val, csym, type;
  int i;
  char *p;

  KSM_SET_ARGS2(my_args, val);
  if( length(my_args) != 2 )
    ksm_error("can't handle SET_AREF: %k", my_args);
  csym = car(my_args);
  if( !IS_CSYM(csym) )
    ksm_error("csym expected: %k", csym);
  type = CSYM_TYPE(csym);
  p = CSYM_ADR(csym);
  i = ksm_int_value(second(my_args));
  if( ksm_clang_type_is_array(type) ){
    type = ksm_clang_type_array_ele(CSYM_TYPE(csym));
    p = CSYM_ADR(csym);
  }
  else if( ksm_clang_type_is_pointer(type) ){
    type = ksm_clang_type_pointer_pointee(type);
    p = *(void **)CSYM_ADR(csym);
  }
  else
    ksm_error("clang pointer/array expected: %k", csym);
  p += ksm_clang_type_size(type) * i;
  if( set_csym(p, type, val) != 0 )
    ksm_error("can't set value in SET_AREF: %k", val);
  return unspec;
}
***/

static
int set_fieldref(void *p, ksm_t stype, ksm_t field, ksm_t val)
     // returns 0 if success
{
  ksm_t t, tt;
  int offset, left_shift, right_shift, ival, mask, i;

  field = ksm_cvt_key(field);
  t = stype;
  if( !(ksm_clang_type_is_struct(t) || ksm_clang_type_is_union(t)) )
    return -1;
  if( !ksm_clang_type_struct_search(t, field, &tt,
	&offset, &left_shift, &right_shift) == 0 )
    return -1;
  if( left_shift || right_shift ){
    ival = *(int *)(p + offset);
    mask = ((~0)>>right_shift)<<left_shift;
    ival &= ~mask;
    i = ksm_get_si(val)<<right_shift;
    ival |= (i & mask);
    *(int *)(p+offset) = ival;
  }
  else{
    p += offset;
    if( set_csym(p, tt, val) != 0 )
      return -1;
  }
  return 0;
}

KSM_DEF_BLT(set_dotref)
     // (set_dotref (csym field) val)
{
  ksm_t my_args, val, csym, field;
  ksm_t t;

  KSM_SET_ARGS2(my_args, val);
  if( ksm_length(my_args) != 2 )
    ksm_error("can't handle SET_DOTREF: %k", my_args);
  csym = ksm_car(my_args);
  field = ksm_car(ksm_cdr(my_args));
  if( !IS_CSYM(csym) )
    ksm_error("C object expected: %k", csym);
  t = CSYM_TYPE(csym);
  if( set_fieldref(CSYM_ADR(csym), t, field, val) != 0 )
    ksm_error("can't set field value: %k in %k", field, csym);
  return ksm_unspec_object;
}

KSM_DEF_BLT(set_arrowref)
     // (set_arrowref (csym field) val)
{
  ksm_t my_args, val, csym, field;
  ksm_t t;

  KSM_SET_ARGS2(my_args, val);
  if( ksm_length(my_args) != 2 )
    ksm_error("can't handle SET_ARROWREF: %k", my_args);
  csym = ksm_car(my_args);
  field = ksm_car(ksm_cdr(my_args));
  if( !IS_CSYM(csym) )
    ksm_error("C object expected: %k", csym);
  t = CSYM_TYPE(csym);
  if( !ksm_clang_type_is_pointer(t) )
    ksm_error("pointer type expected in SET_ARROWREF: %k", csym);
  t = ksm_clang_type_pointer_pointee(t);
  if( set_fieldref(*(void **)CSYM_ADR(csym), t, field, val) != 0 )
    ksm_error("can't set field value: %k in %k", field, csym);
  return ksm_unspec_object;
}

/*** cvt ************************************************************/

static
int csym_charvalue(ksm_t csym, int *result)
{
  switch(CSYM_TYPE_KIND(csym)){
  case KSM_CLANG_TYPE_CHAR:
    *result = *(char *)CSYM_ADR(csym);
    return 0;
  case KSM_CLANG_TYPE_SCHAR:
    {
      signed char ch = *(signed char *)CSYM_ADR(csym);
      if( ch >= CHAR_MIN ){
	*result = ch;
	return 0;
      }
      return -1;
    }
  case KSM_CLANG_TYPE_UCHAR:
    {
      unsigned char ch = *(unsigned char *)CSYM_ADR(csym);
      if( ch <= CHAR_MAX ){
	*result = ch;
	return 0;
      }
      return -1;
    }
  }
  return -1;
}

static
int csym_intvalue(ksm_t csym, int *result)
{
  switch(CSYM_TYPE_KIND(csym)){
  case KSM_CLANG_TYPE_INT:
    *result = *(int *)CSYM_ADR(csym);
    return 0;
  case KSM_CLANG_TYPE_SINT:
    {
      signed int i = *(signed int *)CSYM_ADR(csym);
      if( i >= INT_MIN && i <= INT_MAX ){
	*result = i;
	return 0;
      }
      return -1;
    }
  case KSM_CLANG_TYPE_UINT:
    {
      unsigned int i = *(unsigned int *)CSYM_ADR(csym);
      if( i <= INT_MAX ){
	*result = i;
	return 0;
      }
      return -1;
    }
  case KSM_CLANG_TYPE_SHORT:
    *result = *(short *)CSYM_ADR(csym);
    return 0;
  case KSM_CLANG_TYPE_SSHORT:
    *result = *(signed short *)CSYM_ADR(csym);
    return 0;
  case KSM_CLANG_TYPE_USHORT:
    *result = *(unsigned short *)CSYM_ADR(csym);
    return 0;
  }
  return -1;
}

static
int csym_doublevalue(ksm_t csym, double *result)
{
  switch(CSYM_TYPE_KIND(csym)){
  case KSM_CLANG_TYPE_DOUBLE:
    *result = *(double *)CSYM_ADR(csym);
    return 0;
  case KSM_CLANG_TYPE_FLOAT:
    *result = *(float *)CSYM_ADR(csym);
    return 0;
  }
  return -1;
}

static
int set_numrep(ksm_numrep *p, ksm_t csym)
{
  switch(CSYM_TYPE_KIND(csym)){
  case KSM_CLANG_TYPE_INT:
  case KSM_CLANG_TYPE_SINT:
    ksm_numrep_set_int(p, *(int *)CSYM_ADR(csym));
    return 0;
  case KSM_CLANG_TYPE_UINT:
    {
      unsigned int i = *(unsigned int *)CSYM_ADR(csym);
      if( i <= INT_MAX )
	ksm_numrep_set_int(p, i);
      else
	ksm_numrep_set_longlong(p, i);
      return 0;
    }
  case KSM_CLANG_TYPE_CHAR:
    ksm_numrep_set_int(p, *(char *)CSYM_ADR(csym));
    return 0;
  case KSM_CLANG_TYPE_SCHAR:
    ksm_numrep_set_int(p, *(signed char *)CSYM_ADR(csym));
    return 0;
  case KSM_CLANG_TYPE_UCHAR:
    ksm_numrep_set_int(p, *(unsigned char *)CSYM_ADR(csym));
    return 0;
  case KSM_CLANG_TYPE_SHORT:
    ksm_numrep_set_int(p, *(short *)CSYM_ADR(csym));
    return 0;
  case KSM_CLANG_TYPE_SSHORT:
    ksm_numrep_set_int(p, *(signed short *)CSYM_ADR(csym));
    return 0;
  case KSM_CLANG_TYPE_USHORT:
    ksm_numrep_set_int(p, *(unsigned short *)CSYM_ADR(csym));
    return 0;
  case KSM_CLANG_TYPE_DOUBLE:
    ksm_numrep_set_double(p, *(double *)CSYM_ADR(csym));
    return 0;
  case KSM_CLANG_TYPE_FLOAT:
    ksm_numrep_set_double(p, *(float *)CSYM_ADR(csym));
    return 0;
  default:
    return -1;
  }
}

/*** parse-type *****************************************************/

static
ksm_t parse_type(ksm_t spec, ksm_t env)
     // spec: key  ---> named type
     // spec: (struct [ident] (type name) ...)  ---> struct
     // spec: (union [ident] (type name) ...)  ---> union
     // spec: (type *)  ---> pointer
     // spec: (type [] size)  ---> array
     // spec: (type (type ...)) ---> fun
{
  int len;
  ksm_t head;

  if( ksm_is_key(spec) )
    return find_type(spec, 1);
  if( is_cons(spec) ){
    head = car(spec);
    if( head == key_struct )
      return parse_struct(cdr(spec), env);
    if( head == key_union )
      return parse_union(cdr(spec), env);
    len = length(spec);
    if( len == 2 && second(spec) == key_star )
      return ksm_clang_type_pointer(parse_type(head, env));
    if( (len == 2 || len == 3) && second(spec) == key_bra ){
      len = (len==3)?ksm_int_value(third(spec)):0;
      return ksm_clang_type_array(parse_type(head, env), len);
    }
    if( len == 2 && 
	(ksm_is_cons(second(spec)) || ksm_is_nil(second(spec)))){
      return parse_fun_type(parse_type(ksm_car(spec), env),
			    second(spec), env);
    }
  }
  ksm_error("can't parse type (clang): %k", spec);
  return ksm_unspec_object;
}

static
ksm_t parse_fun_type(ksm_t ret_type, ksm_t arg_spec, ksm_t env)
     // arg_spec: (type-spec ...)
{
  ksm_t t, x;

  t = ksm_clang_type_fun(ret_type);
  while( is_cons(arg_spec) ){
    x = car(arg_spec);
    ksm_clang_type_fun_para(t, unspec, parse_type(x, env));
    arg_spec = cdr(arg_spec);
  }
  if( !is_nil(arg_spec) )
    ksm_error("proper list expected: %k", arg_spec);
  ksm_clang_type_fun_sweep(t);
  return t;
}

static
ksm_t parse_struct_x(ksm_t spec, ksm_t env, int union_p)
     // spec: ([ident] (type name) (type name bits) ...)
{
  ksm_t t, ident, x, type, name;
  int offset = 0, len, bits;
  int bit_offset = 0;

  if( !ksm_is_cons(spec) )
    ksm_error("incomplete struct spec");
  if( ksm_cvt_key_x(car(spec), &ident) == 0 )
    spec = cdr(spec);
  else
    ident = unspec;
  t = ksm_clang_type_struct(ident);
  while( is_cons(spec) ){
    x = car(spec);
    spec = cdr(spec);
    len = length(x);
    if( len == 2 ){
      type = parse_type(first(x), env);
      name = second(x);
      if( bit_offset != 0 ){
	++offset;
	bit_offset = 0;
      }
      offset = ksm_clang_type_struct_add_field(t, name, type, offset);
      if( union_p )
	offset = 0;
    }
    else if( len == 3 ){
      type = parse_type(first(x), env);
      name = second(x);
      bits = ksm_int_value(third(x));
      if( !(ksm_clang_type_is_int(type) || 
	    ksm_clang_type_is_sint(type) ||
	    ksm_clang_type_is_uint(type)) )
	ksm_error("incompatible bit field type: %k %k", name, type);
      if( bits > 32 )
	ksm_error("too large bit field: %k %d", name, bits);
      if( bit_offset == 0 && bits == 32 ){
	offset = ksm_clang_type_struct_add_field(t, name, 
			 ksm_clang_type_int, offset);
      }
      else if( bit_offset + bits <= 32 ){
	ksm_clang_type_struct_add_bit_field(t, name, type, offset,
					    bit_offset, bits);
	bit_offset += bits;
	if( bit_offset == 32 )
	  ++offset;
      }
      else{
	ksm_clang_type_struct_add_bit_field(t, name, type, ++offset,
					    0, bits);
	bit_offset = bits;
      }
    }
    else
      ksm_error("can't handle struct element: %k", x);
  }
  if( !is_nil(spec) )
    ksm_error("proper list expected: %k", spec);
  ksm_clang_type_struct_end_pad(t);
  if( ksm_is_key(ident) )
    t = install_struct(ident, t);
  return t;
}

/*** modify_type ****************************************************/

ksm_t ksm_clang_modify_type(ksm_t base, ksm_t modifiers)
     // base: base-type
     // modifiers: (modifier ...)
     // modifier: * ([] size) (type ...)
{
  ksm_t m;
  int n;
  
  while( ksm_is_cons(modifiers) ){
    m = ksm_car(modifiers);
    modifiers = ksm_cdr(modifiers);
    if( m == key_star )
      base = ksm_clang_type_pointer(base);
    else{
      n = ksm_length(m);
      if( n == 2 && ksm_car(m) == key_bra ){
        int size = ksm_get_si(ksm_eval(ksm_second(m),
				       ksm_get_ctx()->private_global));
	base = ksm_clang_type_array(base, size);
      }
      else if( n >= 0 ){
	base = ksm_clang_type_fun(base);
	if( n == 0 )
	  ksm_clang_type_fun_set_upar(base);
	else if( n == 1 && ksm_clang_type_is_void(ksm_car(m)) )
	    ;  // do nothing
	else{
	  while( n-- > 0 ){
	    ksm_t t = ksm_car(m);
	    if( ksm_clang_type_is_void(t) )
	      ksm_error("invalid void para: %k", m);
	    if( !ksm_clang_is_type(t) )
	      ksm_error("can't happen in modify_type");
	    ksm_clang_type_fun_para(base, unspec, t);
	    m = ksm_cdr(m);
	  }
	}
      }
      else
	ksm_error("unknown type modifier: %k", m);
    }
  }
  if( !ksm_is_nil(modifiers) )
    ksm_error("can't happen in ksm_clang_modify_type");
  return base;
}

ksm_t ksm_clang_def_struct(ksm_t ident, ksm_t spec, int union_p)
     // ident: struct name, or #<unspec> if anonymous
     // spec: ((type name) (type name bits) ...)
{
  ksm_t t, x, type, name;
  int offset = 0, len, bits;
  int bit_offset = 0;

  t = ksm_clang_type_struct(ident);
  while( is_cons(spec) ){
    x = car(spec);
    spec = cdr(spec);
    len = length(x);
    if( len == 2 ){
      type = first(x);
      name = second(x);
      if( bit_offset != 0 ){
	++offset;
	bit_offset = 0;
      }
      offset = ksm_clang_type_struct_add_field(t, name, type, offset);
      if( union_p )
	offset = 0;
    }
    else if( len == 3 ){
      type = first(x);
      name = second(x);
      bits = ksm_int_value(third(x));
      if( !(ksm_clang_type_is_int(type) || 
	    ksm_clang_type_is_sint(type) ||
	    ksm_clang_type_is_uint(type)) )
	ksm_error("incompatible bit field type: %k %k", name, type);
      if( bits > 32 )
	ksm_error("too large bit field: %k %d", name, bits);
      if( bit_offset == 0 && bits == 32 ){
	offset = ksm_clang_type_struct_add_field(t, name, 
			 ksm_clang_type_int, offset);
      }
      else if( bit_offset + bits <= 32 ){
	ksm_clang_type_struct_add_bit_field(t, name, type, offset,
					    bit_offset, bits);
	bit_offset += bits;
	if( bit_offset == 32 )
	  ++offset;
      }
      else{
	ksm_clang_type_struct_add_bit_field(t, name, type, ++offset,
					    0, bits);
	bit_offset = bits;
      }
    }
    else
      ksm_error("can't handle struct element: %k", x);
  }
  if( !is_nil(spec) )
    ksm_error("proper list expected: %k", spec);
  ksm_clang_type_struct_end_pad(t);
  if( ksm_is_key(ident) )
    t = install_struct(ident, t);
  return t;
}

/*** arg_rep ********************************************************/

typedef struct {
  ksm_t type;  // be careful!  argrep type should not be used for 
               // global variables
  union {
    int ival;
    double dval;
    void *pval;
  } u;
} argrep;

static
void set_argrep(argrep *rep, ksm_t x)
{
  union { int i; double d; char *s; } u;

  if( ksm_int_value_x(x, &u.i) == 0 ){
    rep->type = ksm_clang_type_int;
    rep->u.ival = u.i;
    return;
  }
  if( ksm_double_value_x(x, &u.d) == 0 ){
    rep->type = ksm_clang_type_double;
    rep->u.dval = u.d;
    return;
  }
  if( ksm_char_value_x(x, &u.i) == 0 ){
    rep->type = ksm_clang_type_char;
    rep->u.ival = u.i;
    return;
  }
  if( ksm_string_value_x(x, &u.s) == 0 ){
    rep->type = ksm_clang_type_pointer(ksm_clang_type_char);
    rep->u.pval = u.s;
    return;
  }
  if( ksm_is_true(x) ){
    rep->type = ksm_clang_type_int;
    rep->u.ival = 1;
    return;
  }
  if( ksm_is_false(x) ){
    rep->type = ksm_clang_type_int;
    rep->u.ival = 0;
    return;
  }
  if( IS_CSYM(x) ){
    switch(CSYM_TYPE_KIND(x)){
    case KSM_CLANG_TYPE_CHAR:
      rep->type = ksm_clang_type_char;
      rep->u.ival = *(char *)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_SCHAR:
      rep->type = ksm_clang_type_char;
      rep->u.ival = *(signed char *)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_UCHAR:
      rep->type = ksm_clang_type_char;
      rep->u.ival = *(unsigned char *)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_SHORT:
      rep->type = ksm_clang_type_short;
      rep->u.ival = *(short *)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_SSHORT:
      rep->type = ksm_clang_type_short;
      rep->u.ival = *(signed short *)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_USHORT:
      rep->type = ksm_clang_type_short;
      rep->u.ival = *(unsigned short *)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_INT:
      rep->type = ksm_clang_type_int;
      rep->u.ival = *(int *)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_SINT:
      rep->type = ksm_clang_type_int;
      rep->u.ival = *(signed int *)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_UINT:
      rep->type = ksm_clang_type_int;
      rep->u.ival = *(unsigned int *)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_LONGLONG:
      rep->type = ksm_clang_type_longlong;
      rep->u.ival = *(long long *)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_SLONGLONG:
      rep->type = ksm_clang_type_longlong;
      rep->u.ival = *(signed long long *)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_ULONGLONG:
      rep->type = ksm_clang_type_longlong;
      rep->u.ival = *(unsigned long long *)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_FLOAT:
      rep->type = ksm_clang_type_float;
      rep->u.dval = *(float *)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_DOUBLE:
      rep->type = ksm_clang_type_double;
      rep->u.dval = *(double *)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_POINTER:
      rep->type = CSYM_TYPE(x);
      rep->u.pval = *(void **)CSYM_ADR(x);
      return;
    case KSM_CLANG_TYPE_FUN:
      rep->type = CSYM_TYPE(x);
      rep->u.pval = CSYM_ADR(x);
      return;
    }
  }
  ksm_error("can't convert to funcall arg (clang): %k", x);
}

/*** funcall ********************************************************/

static
int push_arg_int(struct ksm_clang_callframe *p, argrep *q)
     // return 0 if OK
{
  switch(ksm_clang_type_kind(q->type)){
  case KSM_CLANG_TYPE_CHAR:
  case KSM_CLANG_TYPE_SCHAR:
  case KSM_CLANG_TYPE_UCHAR:
  case KSM_CLANG_TYPE_SHORT:
  case KSM_CLANG_TYPE_SSHORT:
  case KSM_CLANG_TYPE_USHORT:
  case KSM_CLANG_TYPE_INT:
  case KSM_CLANG_TYPE_SINT:
  case KSM_CLANG_TYPE_UINT:
  case KSM_CLANG_TYPE_LONGLONG:
  case KSM_CLANG_TYPE_SLONGLONG:
  case KSM_CLANG_TYPE_ULONGLONG:
    ksm_clang_arg_int(p, q->u.ival);
    return 0;
  case KSM_CLANG_TYPE_DOUBLE:
  case KSM_CLANG_TYPE_FLOAT:
    ksm_clang_arg_int(p, q->u.dval);
    return 0;
  default:
    return -1;
  }
}

static
int push_arg_double(struct ksm_clang_callframe *p, argrep *q)
     // return 0 if OK
{
  switch(ksm_clang_type_kind(q->type)){
  case KSM_CLANG_TYPE_CHAR:
  case KSM_CLANG_TYPE_SCHAR:
  case KSM_CLANG_TYPE_UCHAR:
  case KSM_CLANG_TYPE_SHORT:
  case KSM_CLANG_TYPE_SSHORT:
  case KSM_CLANG_TYPE_USHORT:
  case KSM_CLANG_TYPE_INT:
  case KSM_CLANG_TYPE_SINT:
  case KSM_CLANG_TYPE_UINT:
  case KSM_CLANG_TYPE_LONGLONG:
  case KSM_CLANG_TYPE_SLONGLONG:
  case KSM_CLANG_TYPE_ULONGLONG:
    ksm_clang_arg_double(p, q->u.ival);
    return 0;
  case KSM_CLANG_TYPE_DOUBLE:
  case KSM_CLANG_TYPE_FLOAT:
    ksm_clang_arg_double(p, q->u.dval);
    return 0;
  default:
    return -1;
  }
}

static
int push_arg_float(struct ksm_clang_callframe *p, argrep *q)
     // return 0 if OK
{
  switch(ksm_clang_type_kind(q->type)){
  case KSM_CLANG_TYPE_CHAR:
  case KSM_CLANG_TYPE_SCHAR:
  case KSM_CLANG_TYPE_UCHAR:
  case KSM_CLANG_TYPE_SHORT:
  case KSM_CLANG_TYPE_SSHORT:
  case KSM_CLANG_TYPE_USHORT:
  case KSM_CLANG_TYPE_INT:
  case KSM_CLANG_TYPE_SINT:
  case KSM_CLANG_TYPE_UINT:
  case KSM_CLANG_TYPE_LONGLONG:
  case KSM_CLANG_TYPE_SLONGLONG:
  case KSM_CLANG_TYPE_ULONGLONG:
    ksm_clang_arg_float(p, q->u.ival);
    return 0;
  case KSM_CLANG_TYPE_DOUBLE:
  case KSM_CLANG_TYPE_FLOAT:
    ksm_clang_arg_float(p, q->u.dval);
    return 0;
  default:
    return -1;
  }
}

static
int push_arg_pointer(struct ksm_clang_callframe *p, argrep *q, ksm_t type)
     // returns 0 if OK
{
  ksm_t pa, pb;

  if( !ksm_clang_type_is_pointer(q->type) ){
    if( ksm_clang_type_is_int(q->type) && q->u.ival == 0 ){
      ksm_clang_arg_int(p, 0);
      return 0;
    }
    pb = ksm_clang_type_pointer_pointee(type);
    if( ksm_clang_type_is_fun(pb) && ksm_clang_type_equal(pb, q->type) ){
      ksm_clang_arg_int(p, (int)q->u.pval);
      return 0;
    }
    return -1;
  }
  pa = ksm_clang_type_pointer_pointee(q->type);
  pb = ksm_clang_type_pointer_pointee(type);

  if( (ksm_clang_type_is_void(pa) || ksm_clang_type_is_void(pb)) ||
      ksm_clang_type_equal(pa, pb) ){
    ksm_clang_arg_int(p, (int)q->u.pval);
    return 0;
  }

  return -1;
}

static
int push_arg_fun(struct ksm_clang_callframe *p, argrep *q, ksm_t type)
{
  if( !ksm_clang_type_is_fun(q->type) ){
    if( ksm_clang_type_is_int(q->type) && q->u.ival == 0 ){
      ksm_clang_arg_int(p, 0);
      return 0;
    }
    return -1;
  }
  if( ksm_clang_type_equal(q->type, type) ){
    ksm_clang_arg_int(p, (int)q->u.pval);
    return 0;
  }
  return -1;
}


static
void push_arg_dots(struct ksm_clang_callframe *p, argrep *q)
{
  switch(ksm_clang_type_kind(q->type)){
  case KSM_CLANG_TYPE_DOUBLE:
  case KSM_CLANG_TYPE_FLOAT:
    ksm_clang_arg_double(p, q->u.dval);
    return;
  case KSM_CLANG_TYPE_POINTER:
    ksm_clang_arg_int(p, (int)q->u.pval);
    return;
  default:
    ksm_clang_arg_int(p, q->u.ival);
    return;
  }
}

static
void push_arg(struct ksm_clang_callframe *p, ksm_t type, ksm_t arg)
{
  argrep rep;

  if( type == Ksm_t ){
    ksm_clang_arg_int(p, (int)arg);
    return;
  }
    
  set_argrep(&rep, arg);
  switch(ksm_clang_type_kind(type)){
  case KSM_CLANG_TYPE_CHAR:
  case KSM_CLANG_TYPE_SCHAR:
  case KSM_CLANG_TYPE_UCHAR:
  case KSM_CLANG_TYPE_SHORT:
  case KSM_CLANG_TYPE_SSHORT:
  case KSM_CLANG_TYPE_USHORT:
  case KSM_CLANG_TYPE_INT:
  case KSM_CLANG_TYPE_SINT:
  case KSM_CLANG_TYPE_UINT:
    if( push_arg_int(p, &rep) == 0 )
      return;
    break;
  case KSM_CLANG_TYPE_DOUBLE:
    if( push_arg_double(p, &rep) == 0 )
      return;
    break;
  case KSM_CLANG_TYPE_FLOAT:
    if( push_arg_float(p, &rep) == 0 )
      return;
    break;
  case KSM_CLANG_TYPE_POINTER:
    if( push_arg_pointer(p, &rep, type) == 0 )
      return;
    break;
  case KSM_CLANG_TYPE_FUN:
    if( push_arg_fun(p, &rep, type) == 0 )
      return;
    break;
  case KSM_CLANG_TYPE_DOTS:
    push_arg_dots(p, &rep);
    return;
  }
  ksm_error("incompatible function arg (clang): %k", arg);
}

ksm_t ksm_clang_make_pointer(ksm_t pointer_type, void *pointer_value)
{
  ksm_t x = MAKE_CSYM(pointer_type, 0);

  ksm_set_ival(x, 2, (int)pointer_value);
  ksm_set_address_of_ival2_in_ival1(x);
  return x;
}

static
ksm_t ret_value(struct ksm_clang_callframe *p, ksm_t ret_type)
{
  if( ret_type == Ksm_t )
    return (ksm_t)ksm_clang_ret_int(p);

  switch(ksm_clang_type_kind(ret_type)){
  case KSM_CLANG_TYPE_INT:
  case KSM_CLANG_TYPE_SINT:
  case KSM_CLANG_TYPE_UINT:
    return ksm_make_int(ksm_clang_ret_int(p));
  case KSM_CLANG_TYPE_DOUBLE:
  case KSM_CLANG_TYPE_FLOAT:
    return ksm_make_double(ksm_clang_ret_double(p));
  case KSM_CLANG_TYPE_POINTER:
    return ksm_clang_make_pointer(ret_type, (void *)ksm_clang_ret_int(p));
  case KSM_CLANG_TYPE_VOID:
    return unspec;
  default:
    ksm_error("can't handle C function return type: %k", ret_type);
    return unspec;
  }
}

static
ksm_pair cfuncall(ksm_t head, ksm_t args, ksm_t env)
{
  struct ksm_clang_callframe frame;
  ksm_t t, ret, name, type, save = args;
  int i, n;
  
  t = CSYM_TYPE(head);
  if( !ksm_clang_type_is_fun(t) )
    ksm_error("function expected: %k", t);
  ret = ksm_clang_type_fun_ret(t);
  ksm_clang_init_callframe(&frame, CSYM_ADR(head));
  n = ksm_clang_type_fun_npara(t);
  for(i=0;i<n;++i){
    ksm_clang_type_fun_ipara(t, i, &name, &type);
    if( ksm_clang_type_is_dots(type) ){
      while( is_cons(args) ){
	push_arg(&frame, type, car(args));
	args = cdr(args);
      }
      ksm_clang_set_vararg(&frame);
      break;
    }
    if( !is_cons(args) )
      ksm_error("too few args: %k", save);
    push_arg(&frame, type, car(args));
    args = cdr(args);
  }
  if( !is_nil(args) )
    ksm_error("too many args: %k", args);
  ksm_clang_call(&frame);
  return ksm_make_eval_done(ret_value(&frame, ret));
}

/*** printf *********************************************************/

static
int ksm_clang_fprintf(ksm_t port, char *fmt, ksm_t args)
{
  ksm_t arg;
  int argtypes[50], n, i, size;
  struct ksm_clang_callframe frame;
  char buf[100], *cp;
  
  n = parse_printf_format(fmt, 50, argtypes);
  if( n > 50 )
    ksm_error("too many args for printf format", fmt);
  size = 100;
  cp = &buf[0];
 again:
  ksm_clang_init_callframe(&frame, (void (*)())snprintf);
  ksm_clang_arg_int(&frame, (int)cp);
  ksm_clang_arg_int(&frame, size);
  ksm_clang_arg_int(&frame, (int)fmt);
  for(i=0;i<n;++i){
    if( !is_cons(args) )
      ksm_error("too few args (CLANG:FPRINTF): %s", fmt);
    arg = car(args);
    args = cdr(args);
    if( (argtypes[i] & ~PA_FLAG_MASK) == ksm_PA_OBJECT )
      ksm_clang_arg_int(&frame, (int)arg);
    else
      push_arg(&frame, ksm_clang_type_dots, arg);
  }
  ksm_clang_set_vararg(&frame);
  ksm_clang_call(&frame);
  n = ksm_clang_ret_int(&frame);
  if( n >= size ){
    size = n+1;
    cp = ksm_alloc(size);
    goto again;
  }
  if( cp != &buf[0] )
    ksm_dispose(cp);
  if( ksm_port_puts(cp, port) < 0 )
    ksm_error("write error: %s", cp);
  return n;
}

/*** get-value ******************************************************/

static
ksm_t get_value(ksm_t type, void *adr)
     // returns 0 if it can't be converted to Scheme object
{
  if( type == Ksm_t )
    return (ksm_t)adr;
  switch(ksm_clang_type_kind(type)){
  case KSM_CLANG_TYPE_INT:
  case KSM_CLANG_TYPE_SINT:
    return ksm_make_int(*(int *)adr);
  case KSM_CLANG_TYPE_DOUBLE:
    return ksm_make_double(*(double *)adr);
  }
  return 0;
}

/***
static
ksm_t get_value(ksm_t csym)
{
  switch(CSYM_TYPE_KIND(csym)){
  case KSM_CLANG_TYPE_INT:
  case KSM_CLANG_TYPE_SINT:
    return ksm_make_int(*(int *)CSYM_ADR(csym));
  case KSM_CLANG_TYPE_DOUBLE:
    return ksm_make_double(*(double *)CSYM_ADR(csym));
  case KSM_CLANG_TYPE_POINTER:
    if( ksm_clang_type_pointer_pointee(CSYM_TYPE(csym)) ==
	ksm_clang_type_char ){
      return ksm_make_string(*(char **)CSYM_ADR(csym));
    }
  }
  ksm_error("can't convert to object: %k", csym);
  return unspec;
}
***/

/*** interface ******************************************************/

KSM_DEF_SPC(sym)
     // (clang:sym type key)
{
  return find_csym(KSM_ARGS(), KSM_ENV());
}

KSM_DEF_SPC(struct)
     // (clang:struct [ident] (type name) ...)
{
  parse_struct(KSM_ARGS(), KSM_ENV());
  return ksm_unspec_object;
}

KSM_DEF_SPC(union)
     // (clang:union [ident] (type name) ...)
{
  return parse_union(KSM_ARGS(), KSM_ENV());
}

KSM_DEF_SPC(sizeof)
     // (clang:sizeof type)
{
  ksm_t type;

  KSM_POP_ARG(type);
  KSM_SET_ARGS0();
  type = parse_type(type, KSM_ENV());
  return ksm_make_int(ksm_clang_type_size(type));
}

KSM_DEF_SPC(align)
     // (clang:align type)
{
  ksm_t type;

  KSM_POP_ARG(type);
  KSM_SET_ARGS0();
  type = parse_type(type, KSM_ENV());
  return ksm_make_int(ksm_clang_type_align(type));
}

KSM_DEF_SPC(typedef)
     // (clang:typedef type identifier)
     // (clang:typedef type (identifier arg-type ...))
{
  ksm_t type, x, name;

  KSM_SET_ARGS2(type, x);
  if( !ksm_clang_is_type(type) )
    type = parse_type(type, KSM_ENV());
  if( ksm_cvt_key_x(x, &name) != 0 ){
    if( !(is_cons(x) && ksm_cvt_key_x(car(x), &name) == 0) )
      ksm_error("can't handle typedef (clang): %k", x);
    type = parse_fun_type(type, cdr(x), KSM_ENV());
  }
  install_type(name, type);
  return unspec;
}

KSM_DEF_BLT(deref)
     // (clang:* csym)
{
  ksm_t csym, t;

  KSM_SET_ARGS1(csym);
  if( !IS_CSYM(csym) )
    ksm_error("C symbol expected: %k", csym);
  t = CSYM_TYPE(csym);
  if( ksm_clang_type_is_pointer(t) )
    return MAKE_CSYM(ksm_clang_type_pointer_pointee(t),
		     *(void **)CSYM_ADR(csym));
  if( ksm_clang_type_is_array(t) )
    return MAKE_CSYM(ksm_clang_type_array_ele(t), CSYM_ADR(csym));
  ksm_error("inconpatible arg (CLANG:*): %k", csym);
  return unspec;
}


KSM_DEF_BLT(adr)
     // (clang:& csym)
{
  ksm_t csym, t, x;

  KSM_SET_ARGS1(csym);
  if( !IS_CSYM(csym) )
    ksm_error("C symbol expected: %k", csym);
  t = CSYM_TYPE(csym);
  x = MAKE_CSYM(ksm_clang_type_pointer(t), 0);
  ksm_set_ival(x, 2, (int)CSYM_ADR(csym));
  ksm_set_address_of_ival2_in_ival1(x);
  return x;
}

ksm_t ksm_clang_aref(ksm_t csym, int i)
{
  ksm_t t, tt;
  char *p;

  t = CSYM_TYPE(csym);
  if( ksm_clang_type_is_array(t) ){
    tt = ksm_clang_type_array_ele(t);
    p = CSYM_ADR(csym);
    p += ksm_clang_type_size(tt) * i;
    return MAKE_CSYM(tt, p);
  }
  if( ksm_clang_type_is_pointer(t) ){
    tt = ksm_clang_type_pointer_pointee(t);
    p = *(void **)CSYM_ADR(csym);
    p += ksm_clang_type_size(tt) * i;
    return MAKE_CSYM(tt, p);
  }
  ksm_error("incompatible first arg (CLANG:[]): %k", csym);
  return unspec;
}

KSM_DEF_BLT(aref)
     // (clang:[] csym index)
{
  ksm_t csym, index;
  int i;

  KSM_SET_ARGS2(csym, index);
  if( !IS_CSYM(csym) )
    ksm_error("C symbol expected: %k", csym);
  i = ksm_int_value(index);
  return ksm_clang_aref(csym, i);
}

/***
KSM_DEF_BLT(aref)
     // (clang:[] csym index)
{
  ksm_t csym, index, t, tt;
  int i;
  char *p;

  KSM_SET_ARGS2(csym, index);
  if( !IS_CSYM(csym) )
    ksm_error("C symbol expected: %k", csym);
  i = ksm_int_value(index);
  t = CSYM_TYPE(csym);
  if( ksm_clang_type_is_array(t) ){
    tt = ksm_clang_type_array_ele(t);
    p = CSYM_ADR(csym);
    p += ksm_clang_type_size(tt) * i;
    return MAKE_CSYM(tt, p);
  }
  if( ksm_clang_type_is_pointer(t) ){
    tt = ksm_clang_type_pointer_pointee(t);
    p = *(void **)CSYM_ADR(csym);
    p += ksm_clang_type_size(tt) * i;
    return MAKE_CSYM(tt, p);
  }
  ksm_error("incompatible first arg (CLANG:[]): %k", csym);
  return unspec;
}
***/

static
ksm_t field_ref(void *adr, ksm_t stype, ksm_t field)
     // returns 0 if not found
{
  ksm_t t, tt, x;
  char *p = adr;
  int offset, left_shift, right_shift, ival;

  field = ksm_cvt_key(field);
  t = stype;
  if( !(ksm_clang_type_is_struct(t) || ksm_clang_type_is_union(t)) )
    return 0;
  if( !ksm_clang_type_struct_search(t, field, &tt,
	&offset, &left_shift, &right_shift) == 0 )
    return 0;
  if( left_shift || right_shift ){
    ival = *(int *)(p + offset);
    if( ksm_clang_type_is_uint(tt) )
      ival = ((unsigned int)ival<<left_shift)>>right_shift;
    else if( ksm_clang_type_is_int(tt) )
      ival = ((int)ival<<left_shift)>>right_shift;
    else
      ival = ((signed int)ival<<left_shift)>>right_shift;
    return ksm_make_int(ival);
  }
  p += offset;
  x = get_value(tt, p);
  return x ? x : MAKE_CSYM(tt, p);
}

KSM_DEF_BLT(dot)
     // (clang:. csym field)
{
  ksm_t csym, field, x;

  KSM_SET_ARGS2(csym, field);
  if( !IS_CSYM(csym) )
    ksm_error("C symbol expected: %k", csym);
  if( !(x = field_ref(CSYM_ADR(csym), CSYM_TYPE(csym), field)) )
    ksm_error("can't reference field: %k in %k", field, csym);
  return x;
}

KSM_DEF_BLT(arrow)
     // (clang:-> csym field)
{
  ksm_t csym, field, t, x;

  KSM_SET_ARGS2(csym, field);
  csym = ksm_eval(csym, KSM_ENV());
  if( csym == ksm_except )
    return ksm_except;
  if( !IS_CSYM(csym) )
    ksm_error("C symbol expected: %k", csym);
  t = CSYM_TYPE(csym);
  if( !ksm_clang_type_is_pointer(t) )
    ksm_error("pointer expected in ARROW_REF: %k", csym);
  t = ksm_clang_type_pointer_pointee(t);
  if( !(x = field_ref(*(void **)CSYM_ADR(csym), t, field)) )
    ksm_error("can't reference field: %k in %k", field, csym);
  return x;
}

KSM_DEF_SPC(new)
     // (clang:new type)
{
  ksm_t type;
  void *p;

  KSM_SET_ARGS1(type);
  type = parse_type(type, KSM_ENV());
  p = ksm_alloc(ksm_clang_type_size(type));
  return MAKE_CSYM(type, p);
}

KSM_DEF_BLT(dispose)
     // (clang:dispose csym)
{
  ksm_t x;

  KSM_SET_ARGS1(x);
  if( !IS_CSYM(x) )
    ksm_error("allocated csym expected: %k", x);
  ksm_dispose(CSYM_ADR(x));
  return ksm_unspec_object;
}

KSM_DEF_BLT(get_value)
     // (clang:get-value csym)
{
  ksm_t x, y, t;

  KSM_SET_ARGS1(x);
  if( !IS_CSYM(x) )
    ksm_error("csym expected: %k", x);
  t = CSYM_TYPE(x);
  if( (y = get_value(t, CSYM_ADR(x))) )
    return y;
  if( ksm_clang_type_is_pointer(t) &&
      ksm_clang_type_pointer_pointee(t) == ksm_clang_type_char )
    return ksm_make_string(*(char **)CSYM_ADR(x));
  return x;
}

KSM_DEF_BLT(printf)
     // (printf fmt arg ...)
{
  ksm_t fmt;
  char *s;

  KSM_POP_ARG(fmt);
  s = ksm_string_value(fmt);
  return ksm_make_int(ksm_clang_fprintf(ksm_output(), s, KSM_ARGS()));
}

KSM_DEF_BLT(fprintf)
     // (fprintf port fmt arg ...)
{
  ksm_t port, fmt;
  char *s;

  KSM_POP_ARG(port);
  if( !ksm_is_output_port(port) )
    ksm_error("output port expected (FPRINTF): %k", port);
  KSM_POP_ARG(fmt);
  s = ksm_string_value(fmt);
  return ksm_make_int(ksm_clang_fprintf(port, s, KSM_ARGS()));
}

KSM_DEF_BLT(sprintf)
     // (sprintf fmt ...) --> string
{
  ksm_t fmt, arg, x;
  char *s, *bp;
  int argtypes[50], n, i;
  struct ksm_clang_callframe frame;
  
  KSM_POP_ARG(fmt);
  s = ksm_string_value(fmt);
  n = parse_printf_format(s, 50, argtypes);
  if( n > 50 )
    ksm_error("too many args for printf format", s);
  ksm_clang_init_callframe(&frame, (void (*)())asprintf);
  ksm_clang_arg_int(&frame, (int)&bp);
  ksm_clang_arg_int(&frame, (int)s);
  for(i=0;i<n;++i){
    KSM_POP_ARG(arg);
    if( (argtypes[i] & ~PA_FLAG_MASK) == ksm_PA_OBJECT )
      ksm_clang_arg_int(&frame, (int)arg);
    else
      push_arg(&frame, ksm_clang_type_dots, arg);
  }
  ksm_clang_set_vararg(&frame);
  ksm_clang_call(&frame);
  x = ksm_make_string(bp);
  free(bp);
  return x;
}

KSM_DEF_SPC(variable_unassign)
     // (clang:variable-unassign var)
{
  ksm_t var, my_env;

  KSM_SET_ARGS1(var);
  var = ksm_cvt_key(var);
  my_env = KSM_ENV();
  if( ksm_update_local(var, ksm_unspec_object, my_env, &my_env) == 0 )
    return ksm_unspec_object;
  while( ksm_is_global(my_env) ){
    if( ksm_enter_global(var, ksm_unspec_object, my_env, 0) == 0 )
      return ksm_unspec_object;
    my_env = ksm_next_global(my_env);
  }
  if( !ksm_is_nil(my_env) )
    ksm_error("can't happen in blt_variable_unassign");
  ksm_error("can't find variable: %k", var);
  return ksm_unspec_object;
}

void ksm_init_clang(void)
{
  ksm_add_gc_prolog(mark_keys);
  init_keys();
  ksm_clang_init_type();
  init_type_tab();
  csym_flag = ksm_new_flag();
  ksm_install_marker(csym_flag, mark_csym);
  ksm_install_fprintf(csym_flag, fprintf_csym);
  ksm_install_updater(csym_flag, update_csym);
  ksm_install_numcvter(csym_flag, set_numrep);
  ksm_install_charvalue(csym_flag, csym_charvalue);
  ksm_install_intvalue(csym_flag, csym_intvalue);
  ksm_install_doublevalue(csym_flag, csym_doublevalue);
  ksm_install_fun_info(csym_flag, cfuncall, 1);
  KSM_ENTER_SPC("clang:sym", sym);
  KSM_ENTER_SPC_SETTER("clang:sym", set_csym);
  KSM_ENTER_SPC("clang:struct", struct);
  KSM_ENTER_SPC("clang:union", union);
  KSM_ENTER_SPC("clang:sizeof", sizeof);
  KSM_ENTER_SPC("clang:align", align);
  KSM_ENTER_SPC("clang:typedef", typedef);
  KSM_ENTER_BLT("clang:*", deref);
  KSM_ENTER_BLT_SETTER("clang:*", set_deref);
  KSM_ENTER_BLT("clang:&", adr);
  KSM_ENTER_BLT("clang:[]", aref);
  KSM_ENTER_BLT_SETTER("clang:[]", set_aref);
  KSM_ENTER_BLT("clang:.", dot);
  KSM_ENTER_BLT_SETTER("clang:.", set_dotref);
  KSM_ENTER_BLT("clang:->", arrow);
  KSM_ENTER_BLT_SETTER("clang:->", set_arrowref);
  KSM_ENTER_SPC("clang:new", new);
  KSM_ENTER_BLT("clang:dispose", dispose);
  KSM_ENTER_BLT("clang:get-value", get_value);
  KSM_ENTER_BLT("printf", printf);
  KSM_ENTER_BLT("fprintf", fprintf);
  KSM_ENTER_BLT("sprintf", sprintf);
  KSM_ENTER_SPC("clang:variable-unassign", variable_unassign);
}

