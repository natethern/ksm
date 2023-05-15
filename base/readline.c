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

static char *line;
static int size;
static int next;

static
void add_char(int ch)
{
  if( next >= size )
    line = ksm_realloc(line, size+BUFSIZ);
  line[next++] = ch;
}

static
char *readline(ksm_t port)
     // returns 0 if EOF
{
  int ch, peek;

  if( ksm_port_eof_p(port) )
    return 0;
  next = 0;
  ch = ksm_port_getc(port);
  if( ch == EOF )
    return 0;
  do{
    if( ch == '\\' ){
      peek = ksm_port_getc(port);
      switch(peek){
      case '\n': break;
      case EOF: add_char(ch); 
	goto out;
      default:
	add_char(ch);
	add_char(peek);
	break;
      }
    }
    else
      add_char(ch);
    ch = ksm_port_getc(port);
  } while( ch != EOF && ch != '\n' );
 out:
  add_char('\0');
  return line;
}

KSM_DEF_BLT(readline)
     // (readline [port])
{
  char *cp;
  ksm_t port;

  if( KSM_MORE_ARGS() ){
    KSM_SET_ARGS1(port);
    if( !ksm_is_input_port(port) )
      ksm_error("input port expected: %k", port);
  }
  else
    port = ksm_input();
  cp = readline(port);
  return cp ? ksm_make_string(cp) : ksm_eof_object;
}

void ksm_init_readline(void)
{
  KSM_ENTER_BLT("readline", readline);
}
