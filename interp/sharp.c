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

#include <stdio.h>
#include "ksm.h"

typedef ksm_t (*read_fun)(char lead, ksm_t port);

static read_fun readers[256];   /*** GLOBAL ***/

void ksm_install_sharp_reader(int leading_char, 
			      ksm_t (*f)(char lead, ksm_t port))
{
  if( !(leading_char >= 0 && leading_char < 256) )
    ksm_fatal("can't install sharp-reader: %d", leading_char);
  readers[leading_char] = f;
}

static
ksm_t sharp_reader(ksm_t port)
{
  read_fun f;
  int ch;

  ch = ksm_port_getc(port);
  if( ch == EOF )
    return ksm_make_key("#");
  f = readers[ch&0xff];
  if( !f )
    ksm_error("can't handle literal: #%c", ch);
  return (*f)(ch, port);
}

static
ksm_t tf_reader(char lead, ksm_t port)
{
  int ch;
  ksm_t x;

  x = (lead == 't') ? ksm_true_object : ksm_false_object;
  ch = ksm_port_getc(port);
  if( ch == EOF || ksm_is_ws(ch) )
    return x;
  if( !ksm_is_delim(ch) )
    ksm_error("can't handle literal (tf_reader): #%c%x", lead, ch);
  ksm_port_ungetc(ch, port);
  return x;
}

void ksm_init_sharp(void)
{
  ksm_sharp_reader = sharp_reader;
  ksm_install_sharp_reader('t', tf_reader);
  ksm_install_sharp_reader('f', tf_reader);
}
