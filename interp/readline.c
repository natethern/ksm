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

//static ksm_strbuf buf;
#define READLINE_BUF  (&ksm_get_ctx()->readline_buf)

#define finish(size)   ksm_strbuf_extract(READLINE_BUF, size)

char *ksm_readline(char *sep, ksm_t port, int *size)
     // returns an newly allocated string
     // allocated space can be freed by ksm_dispose
     // return 0 if EOF
{
  int ch;
  char *cand = 0;
  ksm_strbuf *bufp = READLINE_BUF;

  if( ksm_port_eof_p(port) )
    return 0;
  ksm_strbuf_rewind(bufp);
  while( 1 ){
    if( cand && *cand == '\0' )
      return finish(size);
    ch = ksm_port_getc(port);
    if( ch == EOF )
      return finish(size);
    ksm_strbuf_put(ch, bufp);
    if( cand ){
      if( *cand == ch )
	++cand;
      else
	cand = 0;
    }
    else{
      if( ch == *sep )
	cand = sep+1;
    }
  }
}


