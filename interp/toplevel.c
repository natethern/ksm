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
#include <unistd.h>
#include <ctype.h>
#include "ksm.h"

static
ksm_t next_port(ksm_t current_port)
     // returns NULL if EOF
{
  char *line;
  int size;

  if( current_port )
    ksm_close_port(current_port);
  line = ksm_readline("\n", ksm_stdin_port, &size);
  if( line )
    return ksm_make_bufport(ksm_make_string_input(line, 0, size));
  else
    return 0;
}

void ksm_toplevel(ksm_t global)
{
  ksm_t expr, val, *frame, port = 0;
  
  frame = ksm_registry_top();
  while(1){
    ksm_port_puts("> ", ksm_stdout_port);
    ksm_port_flush(ksm_stdout_port);

    port = next_port(port);
    if( port == 0 )
      break;
    ksm_registry_restore_with(frame, port);
    while(1){
      expr = ksm_read_toplevel(port);
      if( ksm_is_comment(expr) )
	continue;
      if( ksm_is_eof(expr) )
	break;
      val = ksm_eval(expr, global);
      if( val == ksm_except )
	ksm_error("can't find exception handler");
      ksm_display(val, ksm_stdout_port);
      ksm_port_putc('\n', ksm_stdout_port);
      ksm_registry_restore_with(frame, port);
    }
  }
}

