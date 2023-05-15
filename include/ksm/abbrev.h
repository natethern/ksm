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

#ifndef ABBREV_H
#define ABBREV_H

#define cons(x,y)    ksm_cons(x,y)
#define is_cons(x)   ksm_is_cons(x)
#define car(x)       ksm_car(x)
#define cdr(x)       ksm_cdr(x)
#define set_car(x,v) ksm_set_car(x,v)
#define set_cdr(x,v) ksm_set_cdr(x,v)

#define caar(x)      ksm_car(ksm_car(x))
#define cadr(x)      ksm_car(ksm_cdr(x))
#define cdar(x)      ksm_cdr(ksm_car(x))
#define cddr(x)      ksm_cdr(ksm_cdr(x))
#define caaar(x)     ksm_car(ksm_car(ksm_car(x)))
#define caadr(x)     ksm_car(ksm_car(ksm_cdr(x)))
#define cadar(x)     ksm_car(ksm_cdr(ksm_car(x)))
#define caddr(x)     ksm_car(ksm_cdr(ksm_cdr(x)))
#define cdaar(x)     ksm_cdr(ksm_car(ksm_car(x)))
#define cdadr(x)     ksm_cdr(ksm_car(ksm_cdr(x)))
#define cddar(x)     ksm_cdr(ksm_cdr(ksm_car(x)))
#define cdddr(x)     ksm_cdr(ksm_cdr(ksm_cdr(x)))

#define length(x)    ksm_length(x)
#define first(x)     ksm_car(x)
#define second(x)    ksm_car(ksm_cdr(x))
#define third(x)     ksm_car(ksm_cdr(ksm_cdr(x)))
#define fourth(x)    ksm_cdr(ksm_cdr(ksm_cdr(ksm_cdr(x))))

#define nil          ksm_nil_object
#define is_nil(x)    ksm_is_nil(x)

#define unspec       ksm_unspec_object
#define is_unspec(x) ksm_is_unsepc(x)

#endif
