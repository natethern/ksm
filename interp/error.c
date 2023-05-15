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
#include <stdlib.h>
#include <stdarg.h>
#include "ksm.h"

/*** input flush ****************************************************/

#include <termios.h>
#include <unistd.h>

void ksm_flush_stdin(void)
{
  tcflush(0, TCIFLUSH);
}

/*** error **********************************************************/

static
void default_error_prolog(void)
{ 
  exit(1); 
}

void (*ksm_error_prolog)(void) = default_error_prolog; /*** GLOBAL ***/

#define ERROR_LINE  (78*2)

void ksm_error(char *fmt, ...)
{
  va_list ap;
  char buf[ERROR_LINE], *lead = "ERROR: ";
  int n;

  if( fmt == 0 )
    goto L1;
  n = snprintf(buf, ERROR_LINE, "%s", lead);
  if( fmt ){
    va_start(ap, fmt);
    vsnprintf(buf+n, ERROR_LINE-n, fmt, ap);
    va_end(ap);
  }
  fprintf(stderr, "%s\n", buf);
 L1:
  (*ksm_error_prolog)();
}

/*** fatal **********************************************************/

static
void default_fatal_prolog(void){ exit(1); }

void (*ksm_fatal_prolog)(void) = default_fatal_prolog;  /*** GLOBAL ***/

void ksm_fatal(char *fmt, ...)
{
  va_list ap;
  char buf[ERROR_LINE], *lead = "FATAL: ";
  int n;

  n = snprintf(buf, ERROR_LINE, "%s", lead);
  if( fmt ){
    va_start(ap, fmt);
    vsnprintf(buf+n, ERROR_LINE-n, fmt, ap);
    va_end(ap);
  }
  fprintf(stderr, "%s\n", buf);
  (*ksm_fatal_prolog)();
}

