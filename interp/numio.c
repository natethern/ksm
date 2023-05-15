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

#include <stdlib.h>
#include <errno.h>
#include "ksm.h"

KSM_DEF_BLT(number_to_string)
     // (number->string z [radix])
{
  ksm_t z, r;
  ksm_numrep p;
  int rdx, i;
  char buf[34], *cp;
  div_t x;

  KSM_POP_ARG(z);
  if( ksm_set_numrep(&p, z) != 0 )
    ksm_error("number expected: %k", z);
  if( KSM_NO_MORE_ARGS() )
    rdx = 10;
  else{
    KSM_SET_ARGS1(r);
    rdx = ksm_int_value(r);
    if( !(rdx >= 2 && rdx <= 16) )
      ksm_error("invalid radix: %d", rdx);
  }
  if( rdx == 10 ){
    char *s;

    asprintf(&s, "%k", z);
    return ksm_make_string_no_copy(s);
  }
  switch(p.kind){
  case KSM_NUMREP_INT:
    i = abs(p.u.ival);
    if( i == 0 )
      return ksm_make_string("0");
    cp = &buf[34];
    *--cp = '\0';
    while( i > 0 ){
      x = div(i, rdx);
      if( x.rem < 10 )
	*--cp = x.rem + '0';
      else
	*--cp = 'A' + x.rem - 10;
      i = x.quot;
    }
    if( p.u.ival < 0 )
      *--cp = '-';
    return ksm_make_string(cp);
#ifdef GMP
  case KSM_NUMREP_MPZ:
    return ksm_make_string_no_copy(mpz_get_str(0, rdx, p.u.zval));
#endif
  default:
    ksm_error("can't happen (number->string): %d", p.kind);
    return ksm_unspec_object;
  }
}

KSM_DEF_BLT(string_to_number)
     // (string->number string [radix])
{
  ksm_t string, radix, x;
  int rdx;
  char *s;

  KSM_POP_ARG(string);
  s = ksm_string_value(string);
  if( KSM_NO_MORE_ARGS() )
    rdx = 10;
  else{
    KSM_SET_ARGS1(radix);
    rdx = ksm_int_value(radix);
    if( !(rdx >= 2 && rdx <= 16) )
      return ksm_false_object;
  }
  x = ksm_string_to_number(s, rdx);
  if( x == 0 )
    return ksm_false_object;
  return x;
}

static
int char_to_rdx(int ch)
{
  switch(ch){
  case 'b': return 2;
  case 'o': return 8;
  case 'x': return 16;
  default: return 10;
  }
}

#define BLIT_MAX   128

static
ksm_t sharp_reader(char lead, ksm_t port)
{
  char *p, *s;
  int rdx = 0;
  int exact = 0;  // 0: not specified, 1: #e, 2: #i, 
  ksm_t x;
  
  p = s = ksm_stuff_lexeme(lead, port);
  // 'e', 'i', 'b', 'o', 'd', or 'x'
  switch(*p){
  case 'b': case 'o': case 'd': case 'x': 
    rdx = char_to_rdx(*p);
    break;
  case 'e': exact = 1; break;
  case 'i': exact = 2; break;
  default: ksm_error("can't happen (sharp_reader in numio.c): #%s", s);
  }

  switch(*++p){
  case 'b': case 'o': case 'd': case 'x': 
    if( rdx != 0 ) 
      ksm_error("deuplicate radix specification: #%s", s);
    rdx = char_to_rdx(*p++);
    break;
  case 'e':
    if( exact != 0 )
      ksm_error("duplicate exact/inexact specification: #%s", s);
    exact = 1;
    ++p;
    break;
  case 'i':
    if( exact != 0 )
      ksm_error("duplicate exact/inexact specification: #%s", s);
    exact = 2;
    ++p;
    break;
  }
  if( rdx == 0 )
    rdx = 10;

  if( exact & 1 ){   // #e
    x = ksm_string_to_number(p, rdx);
    if( x == 0 )
      ksm_error("can't handle literal: %s", s);
    if( ksm_inexact_p(x) )
      x = ksm_inexact_to_exact(x);
  }
  else if( exact & 2 ){ // #i
    x = ksm_string_to_number(p, rdx);
    if( x == 0 )
      ksm_error("can't handle literal: %s", s);
    if( ksm_exact_p(x) )
      x = ksm_exact_to_inexact(x);
  }
  else{
    x = ksm_string_to_number(p, rdx);
    if( x == 0 )
      ksm_error("can't handle literal: %s", s);
  }
  return x;
}

void ksm_init_numio(void)
{
  KSM_ENTER_BLT("number->string", number_to_string);
  KSM_ENTER_BLT("string->number", string_to_number);
  ksm_install_sharp_reader('b', sharp_reader);
  ksm_install_sharp_reader('o', sharp_reader);
  ksm_install_sharp_reader('d', sharp_reader);
  ksm_install_sharp_reader('x', sharp_reader);
  ksm_install_sharp_reader('e', sharp_reader);
  ksm_install_sharp_reader('i', sharp_reader);
}
