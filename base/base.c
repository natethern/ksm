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
#include <string.h>
#include "ksm.h"
#include "ksmext.h"

static
ksm_t sharp_literal_reader(char lead, ksm_t port)
     // #<...>
{
  char *s;
  int len;

  s = ksm_stuff_lexeme(0, port);
  len = strlen(s);
  if( !(len > 1 && s[len-1] == '>') )
    ksm_error("can't handle literal: #<%s", s);
  s[len-1] = '\0';
  if( strcmp(s, "eof") == 0 )
    return ksm_eof_object;
  if( strcmp(s, "unspecified") == 0 )
    return ksm_unspec_object;
  ksm_error("can't handle literal: #<%s", s);
  return ksm_unspec_object;
}

static
ksm_t eat_comment(char lead, ksm_t port)
{
  return ksm_eat_comment(port);
}

KSM_DEF_BLT(interp_file)
     // (enable-interpreter-file)
{
  KSM_SET_ARGS0();
  ksm_install_sharp_reader('!', eat_comment);
  return ksm_unspec_object;
}

KSM_DEF_BLT(exit)
     // (exit [code])
{
  int c = 0;

  if( KSM_MORE_ARGS() ){
    ksm_t code;

    KSM_SET_ARGS1(code);
    c = ksm_int_value(code);
  }
  exit(c);
  return ksm_unspec_object;
}

KSM_DEF_BLT(current_error_port)
     // (current-error-port)
{
  KSM_SET_ARGS0();
  return ksm_stderr_port;
}

KSM_DEF_BLT(set_read_case_sensitivity)
     // (set-read-case-sensitivity bool)
{
  ksm_t bool;

  KSM_SET_ARGS1(bool);
  ksm_set_read_case_sensitivity(!ksm_is_false(bool));
  return ksm_unspec_object;
}

KSM_DEF_BLT(string_to_expr)
     // (string->expr string)
{
  ksm_t string, x, port;
  char *cp;
  FILE *fp;

  KSM_SET_ARGS1(string);
  cp = ksm_string_value(string);
  if( *cp == '\0' )
    return string;
  fp = ksm_string_stream(cp, "r");
  port = ksm_make_input_file_port(fp);
  x = ksm_read_object(port);
  ksm_close_port(port);
  return x;
}

KSM_DEF_SPC(prog1)
     // (prog1 expr ...)
{
  ksm_t expr, seq, *frame;

  KSM_POP_ARG(expr);
  expr = ksm_eval(expr, KSM_ENV());
  if( expr == ksm_except )
    return ksm_except;
  frame = ksm_registry_top();
  seq = KSM_ARGS();
  while( ksm_is_cons(seq) ){
    if( ksm_eval(ksm_car(seq), KSM_ENV()) == ksm_except )
      return ksm_except;
    ksm_registry_restore(frame);
    seq = ksm_cdr(seq);
  }
  return expr;
}

KSM_DEF_SPC(define_private)
     // (define-private key val)
     // (define-private (key para ...) body ...)
{
  ksm_t key, val;

  if( !ksm_is_global(KSM_ENV()) )
    ksm_error("DEFINE-PRIVATE out of toplevel: %k", KSM_ARGS());
  if( ksm_interp_define_args(KSM_ARGS(), KSM_ENV(), &key, &val) != 0 )
    ksm_error("invalid args (DEFINE-PRIVATE): %k", KSM_ARGS());
  if( val == ksm_except )
    return ksm_except;
  ksm_enter_global(key, val, ksm_get_ctx()->private_global, 1);
  return ksm_unspec_object;
}

KSM_DEF_SPC(set_private)
     // (set-private! key val)
{
  ksm_t key, val;

  KSM_SET_ARGS2(key, val);
  if( !ksm_is_key(key) )
    ksm_error("symbol expected (SET-PRIVATE!): %k", key);
  val = ksm_eval(val, KSM_ENV());
  if( val == ksm_except )
    ksm_error("can't find exception handler");
  if( ksm_enter_global(key, val, ksm_get_ctx()->private_global, 0) != 0 )
    ksm_error("no such binding (SET-PRIVATE!): %k", key);
  return ksm_unspec_object;
}

KSM_DEF_BLT(gc)
     // (gc)
{
  KSM_SET_ARGS0();
  ksm_gc();
  return ksm_unspec_object;
}

KSM_DEF_BLT(flush_output_port)
     // (flush-output-port [port])
{
  ksm_t port;

  KSM_POP_OPTARG(port, (ksm_get_ctx()->output));
  KSM_SET_ARGS0();
  if( !ksm_is_output_port(port) )
    ksm_error("output port expected: %k", port);
  ksm_port_flush(port);
  return ksm_unspec_object;
}

KSM_DEF_BLT(trace_on)
     // (trace-on)
{
  KSM_SET_ARGS0();
  ksm_get_ctx()->trace_on = 1;
  return ksm_unspec_object;
}

KSM_DEF_BLT(trace_off)
     // (trace-off)
{
  KSM_SET_ARGS0();
  ksm_get_ctx()->trace_on = 0;
  return ksm_unspec_object;
}

void ksm_init_base(void)
{
  ksm_init_hashtab();
  ksm_init_keyword();
  ksm_init_lambdax();
  ksm_init_getopt();
  ksm_init_logic();
  ksm_init_queue();
  ksm_init_readline();
  ksm_init_signal_unit();
  ksm_init_sched_unit();
  ksm_install_sharp_reader('<', sharp_literal_reader);
  KSM_ENTER_BLT("%enable-interpreter-file", interp_file);
  KSM_ENTER_BLT("exit", exit);
  KSM_ENTER_BLT("current-error-port", current_error_port);
  KSM_ENTER_BLT("set-read-case-sensitivity", set_read_case_sensitivity);
  KSM_ENTER_BLT("string->expr", string_to_expr);
  KSM_ENTER_SPC("prog1", prog1);
  KSM_ENTER_SPC("define-private", define_private);
  KSM_ENTER_SPC("set-private!", set_private);
  KSM_ENTER_BLT("gc", gc);
  KSM_ENTER_BLT("flush-output-port", flush_output_port);
  KSM_ENTER_BLT("trace-on", trace_on);
  KSM_ENTER_BLT("trace-off", trace_off);
}
