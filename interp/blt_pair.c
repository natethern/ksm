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

KSM_DEF_BLT(pair_p)
     // (pair? obj)
{
  ksm_t obj;

  KSM_SET_ARGS1(obj);
  return KSM_BOOL2OBJ(ksm_is_cons(obj));
}

KSM_DEF_BLT(cons)
     // (cons x y)
{
  ksm_t x, y;

  KSM_SET_ARGS2(x, y);
  return ksm_cons(x, y);
}

KSM_DEF_BLT(car)
     // (car x)
{
  ksm_t x;

  KSM_SET_ARGS1(x);
  KSM_CONFIRM_CONS(x);
  return ksm_car(x);
}

KSM_DEF_BLT(cdr)
     // (cdr x)
{
  ksm_t x;

  KSM_SET_ARGS1(x);
  KSM_CONFIRM_CONS(x);
  return ksm_cdr(x);
}

KSM_DEF_BLT(set_car)
     // (set-car! pair obj)
{
  ksm_t pair, obj;

  KSM_SET_ARGS2(pair, obj);
  KSM_CONFIRM_CONS(pair);
  ksm_set_car(pair, obj);
  return ksm_unspec_object;
}

KSM_DEF_BLT(set_cdr)
     // (set-cdr! pair obj)
{
  ksm_t pair, obj;

  KSM_SET_ARGS2(pair, obj);
  KSM_CONFIRM_CONS(pair);
  ksm_set_cdr(pair, obj);
  return ksm_unspec_object;
}

#define get_caar(obj) ksm_safe_car(ksm_safe_car(obj))
#define get_cadr(obj) ksm_safe_car(ksm_safe_cdr(obj))
#define get_cdar(obj) ksm_safe_cdr(ksm_safe_car(obj))
#define get_cddr(obj) ksm_safe_cdr(ksm_safe_cdr(obj))
#define get_caaar(obj) get_caar(ksm_safe_car(obj))
#define get_caadr(obj) get_caar(ksm_safe_cdr(obj))
#define get_cadar(obj) get_cadr(ksm_safe_car(obj))
#define get_caddr(obj) get_cadr(ksm_safe_cdr(obj))
#define get_cdaar(obj) get_cdar(ksm_safe_car(obj))
#define get_cdadr(obj) get_cdar(ksm_safe_cdr(obj))
#define get_cddar(obj) get_cddr(ksm_safe_car(obj))
#define get_cdddr(obj) get_cddr(ksm_safe_cdr(obj))
#define get_caaaar(obj) get_caaar(ksm_safe_car(obj))
#define get_caaadr(obj) get_caaar(ksm_safe_cdr(obj))
#define get_caadar(obj) get_caadr(ksm_safe_car(obj))
#define get_caaddr(obj) get_caadr(ksm_safe_cdr(obj))
#define get_cadaar(obj) get_cadar(ksm_safe_car(obj))
#define get_cadadr(obj) get_cadar(ksm_safe_cdr(obj))
#define get_caddar(obj) get_caddr(ksm_safe_car(obj))
#define get_cadddr(obj) get_caddr(ksm_safe_cdr(obj))
#define get_cdaaar(obj) get_cdaar(ksm_safe_car(obj))
#define get_cdaadr(obj) get_cdaar(ksm_safe_cdr(obj))
#define get_cdadar(obj) get_cdadr(ksm_safe_car(obj))
#define get_cdaddr(obj) get_cdadr(ksm_safe_cdr(obj))
#define get_cddaar(obj) get_cddar(ksm_safe_car(obj))
#define get_cddadr(obj) get_cddar(ksm_safe_cdr(obj))
#define get_cdddar(obj) get_cdddr(ksm_safe_car(obj))
#define get_cddddr(obj) get_cdddr(ksm_safe_cdr(obj))

#define DEF_CXR(s,f) \
KSM_DEF_BLT(s) \
{ \
  ksm_t x; \
  KSM_SET_ARGS1(x); \
  return f(x); \
}

DEF_CXR(caar, get_caar)
DEF_CXR(cadr, get_cadr)
DEF_CXR(cdar, get_cdar)
DEF_CXR(cddr, get_cddr)
DEF_CXR(caaar, get_caaar)
DEF_CXR(caadr, get_caadr)
DEF_CXR(cadar, get_cadar)
DEF_CXR(caddr, get_caddr)
DEF_CXR(cdaar, get_cdaar)
DEF_CXR(cdadr, get_cdadr)
DEF_CXR(cddar, get_cddar)
DEF_CXR(cdddr, get_cdddr)
DEF_CXR(caaaar, get_caaaar)
DEF_CXR(caaadr, get_caaadr)
DEF_CXR(caadar, get_caadar)
DEF_CXR(caaddr, get_caaddr)
DEF_CXR(cadaar, get_cadaar)
DEF_CXR(cadadr, get_cadadr)
DEF_CXR(caddar, get_caddar)
DEF_CXR(cadddr, get_cadddr)
DEF_CXR(cdaaar, get_cdaaar)
DEF_CXR(cdaadr, get_cdaadr)
DEF_CXR(cdadar, get_cdadar)
DEF_CXR(cdaddr, get_cdaddr)
DEF_CXR(cddaar, get_cddaar)
DEF_CXR(cddadr, get_cddadr)
DEF_CXR(cdddar, get_cdddar)
DEF_CXR(cddddr, get_cddddr)

KSM_DEF_BLT(null_p)
     // (null? obj)
{
  ksm_t obj;

  KSM_SET_ARGS1(obj);
  return KSM_BOOL2OBJ(ksm_is_nil(obj));
}

KSM_DEF_BLT(list_p)
     // (list? obj)
{
  ksm_t obj, x, head, tail;

  KSM_SET_ARGS1(obj);
  ksm_list_begin(&head, &tail);
  while( ksm_is_cons(obj) ){
    x = ksm_car(obj);
    obj = ksm_cdr(obj);
    if( ksm_memq(x, head) )
      return ksm_false_object;
    ksm_list_extend(&head, &tail, x);
  }
  return KSM_BOOL2OBJ(ksm_is_nil(obj));
}

KSM_DEF_BLT(list)
     // (list obj ...)
{
  return KSM_ARGS();
}

KSM_DEF_BLT(length)
     // (length list)
{
  ksm_t x;
  int n;

  KSM_SET_ARGS1(x);
  n = ksm_length(x);
  if( n < 0 )
    ksm_error("proper list expected: %k", x);
  return ksm_make_int(n);
}

KSM_DEF_BLT(append)
     // (append list ...)
{
  ksm_t head, tail, x;

  ksm_list_begin(&head, &tail);
  while( KSM_MORE_ARGS() ){
    KSM_POP_ARG(x);
    if( KSM_NO_MORE_ARGS() ){
      if( ksm_is_cons(tail) )
	ksm_set_cdr(tail, x);
      else
	head = x;
      break;
    }
    ksm_list_append(&head, &tail, x);
  }
  return head;
}

KSM_DEF_BLT(reverse)
     // (reverse list)
{
  ksm_t list, rev;

  KSM_SET_ARGS1(list);
  rev = ksm_nil_object;
  while( ksm_is_cons(list) ){
    rev = ksm_cons(ksm_car(list), rev);
    list = ksm_cdr(list);
  }
  if( !ksm_is_nil(list) )
    ksm_error("proper list expected: %k", list);
  return rev;
}

KSM_DEF_BLT(list_tail)
     // (list-tail list k)
{
  ksm_t list, k, save;
  int i;

  KSM_SET_ARGS2(list, k);
  save = list;
  i = ksm_int_value(k);
  while( i-- > 0 ){
    if( !ksm_is_cons(list) )
      ksm_error("list has fewer elements: %k", save);
    list = ksm_cdr(list);
  }
  return list;
}

KSM_DEF_BLT(list_ref)
     // (list-ref list k)
{
  ksm_t list, k, save;
  int i;

  KSM_SET_ARGS2(list, k);
  save = list;
  i = ksm_int_value(k);
  while( i-- > 0 ){
    if( !ksm_is_cons(list) )
      ksm_error("list has too few elements: %k", save);
    list = ksm_cdr(list);
  }
  if( !ksm_is_cons(list) )
    ksm_error("list has too few elements: %k", save);
  return ksm_car(list);
}

KSM_DEF_BLT(memq)
     // (memq obj list)
{
  ksm_t obj, list;

  KSM_SET_ARGS2(obj, list);
  while( ksm_is_cons(list) ){
    if( obj == ksm_car(list) )
      return list;
    list = ksm_cdr(list);
  }
  return ksm_false_object;
}

KSM_DEF_BLT(memv)
     // (memv obj list)
{
  ksm_t obj, list;

  KSM_SET_ARGS2(obj, list);
  while( ksm_is_cons(list) ){
    if( ksm_eqv(obj, ksm_car(list)) )
      return list;
    list = ksm_cdr(list);
  }
  return ksm_false_object;
}

KSM_DEF_BLT(member)
     // (member obj list)
{
  ksm_t obj, list;

  KSM_SET_ARGS2(obj, list);
  while( ksm_is_cons(list) ){
    if( ksm_equal(obj, ksm_car(list)) )
      return list;
    list = ksm_cdr(list);
  }
  return ksm_false_object;
}

KSM_DEF_BLT(assq)
     // (assq obj alist)
{
  ksm_t obj, alist;

  KSM_SET_ARGS2(obj, alist);
  return ksm_assq(obj, alist);
}

KSM_DEF_BLT(assv)
     // (assv obj alist)
{
  ksm_t obj, alist;

  KSM_SET_ARGS2(obj, alist);
  return ksm_assv(obj, alist);
}

KSM_DEF_BLT(assoc)
     // (assoc obj alist)
{
  ksm_t obj, alist;

  KSM_SET_ARGS2(obj, alist);
  return ksm_assoc(obj, alist);
}

void ksm_blt_pair(void)
{
  KSM_ENTER_BLT("pair?", pair_p);
  KSM_ENTER_BLT("cons", cons);
  KSM_ENTER_BLT("car", car);
  KSM_ENTER_BLT("cdr", cdr);
  KSM_ENTER_BLT("set-car!", set_car);
  KSM_ENTER_BLT("set-cdr!", set_cdr);
  KSM_ENTER_BLT("caar", caar);
  KSM_ENTER_BLT("cadr", cadr);
  KSM_ENTER_BLT("cdar", cdar);
  KSM_ENTER_BLT("cddr", cddr);
  KSM_ENTER_BLT("caaar", caaar);
  KSM_ENTER_BLT("caadr", caadr);
  KSM_ENTER_BLT("cadar", cadar);
  KSM_ENTER_BLT("caddr", caddr);
  KSM_ENTER_BLT("cdaar", cdaar);
  KSM_ENTER_BLT("cdadr", cdadr);
  KSM_ENTER_BLT("cddar", cddar);
  KSM_ENTER_BLT("cdddr", cdddr);
  KSM_ENTER_BLT("caaaar", caaaar);
  KSM_ENTER_BLT("caaadr", caaadr);
  KSM_ENTER_BLT("caadar", caadar);
  KSM_ENTER_BLT("caaddr", caaddr);
  KSM_ENTER_BLT("cadaar", cadaar);
  KSM_ENTER_BLT("cadadr", cadadr);
  KSM_ENTER_BLT("caddar", caddar);
  KSM_ENTER_BLT("cadddr", cadddr);
  KSM_ENTER_BLT("cdaaar", cdaaar);
  KSM_ENTER_BLT("cdaadr", cdaadr);
  KSM_ENTER_BLT("cdadar", cdadar);
  KSM_ENTER_BLT("cdaddr", cdaddr);
  KSM_ENTER_BLT("cddaar", cddaar);
  KSM_ENTER_BLT("cddadr", cddadr);
  KSM_ENTER_BLT("cdddar", cdddar);
  KSM_ENTER_BLT("cddddr", cddddr);
  KSM_ENTER_BLT("null?", null_p);
  KSM_ENTER_BLT("list?", list_p);
  KSM_ENTER_BLT("list", list);
  KSM_ENTER_BLT("length", length);
  KSM_ENTER_BLT("append", append);
  KSM_ENTER_BLT("reverse", reverse);
  KSM_ENTER_BLT("list-tail", list_tail);
  KSM_ENTER_BLT("list-ref", list_ref);
  KSM_ENTER_BLT("memq", memq);
  KSM_ENTER_BLT("memv", memv);
  KSM_ENTER_BLT("member", member);
  KSM_ENTER_BLT("assq", assq);
  KSM_ENTER_BLT("assv", assv);
  KSM_ENTER_BLT("assoc", assoc);
}
