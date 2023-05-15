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
#include "ksm.h"

char *ksm_standard_encoding;  /*** GLOBAL ***/

#define INPUT    (ksm_get_ctx()->input)
#define OUTPUT   (ksm_get_ctx()->output)

ksm_t ksm_stdin_port;      /*** GLOBAL ***/
ksm_t ksm_stdout_port;     /*** GLOBAL ***/
ksm_t ksm_stderr_port;     /*** GLOBAL ***/

#define DECODING_PORT(code, port) \
  ksm_make_input_charset_decoding_port(code, port)
#define ENCODING_PORT(code, port) \
  ksm_make_output_charset_encoding_port(code, port)

ksm_t ksm_open_input_fp(FILE *fp, char *encoding)
{
  ksm_t port;

  port = ksm_make_input_file_port(fp);
  if( encoding == KSM_NO_ENCODING )
    return port;
  else if( encoding == KSM_DEFAULT_ENCODING ){
    if( ksm_standard_encoding == 0 )
      return port;
    else
      return DECODING_PORT(ksm_standard_encoding, port);
  }
  else
    return DECODING_PORT(encoding, port);
}

ksm_t ksm_open_output_fp(FILE *fp, char *encoding)
{
  ksm_t port;

  port = ksm_make_output_file_port(fp);
  if( encoding == KSM_NO_ENCODING )
    return port;
  else if( encoding == KSM_DEFAULT_ENCODING ){
    if( ksm_standard_encoding == 0 )
      return port;
    else
      return ENCODING_PORT(ksm_standard_encoding, port);
  }
  else
    return ENCODING_PORT(encoding, port);
}

ksm_t ksm_open_input_file(char *file, char *encoding)
{
  FILE *fp;

  fp = fopen(file, "r");
  if( fp == 0 )
    ksm_error("can't open file: %s\n", file);
  return ksm_open_input_fp(fp, encoding);
}

ksm_t ksm_open_output_file(char *file, char *encoding)
{
  FILE *fp;

  fp = fopen(file, "w");
  if( fp == 0 )
    ksm_error("can't open file: %s\n", file);
  return ksm_open_output_fp(fp, encoding);
}

static
void parse_args(ksm_t args, char **string, char **encode, ksm_t *proc)
     // args: (string [encoding] proc)  if proc is not NULL
     // args: (string [encoding]) if proc is NULL
{
  ksm_t str, x, enc;

  KSM_POP_ARG(str);
  if( proc ){
    KSM_POP_ARG(x);
    if( ksm_is_string(x) ){
      enc = x;
      KSM_SET_ARGS1(x);
    }
    else{
      KSM_SET_ARGS0();
      enc = ksm_true_object;
    }
    *proc = x;
  }
  else{
    if( KSM_NO_MORE_ARGS() )
      enc = ksm_true_object;
    else
      KSM_SET_ARGS1(enc);
  }
  *string = ksm_string_value(str);
  if( ksm_is_true(enc) )
    *encode = KSM_DEFAULT_ENCODING;
  else if( ksm_is_nil(enc) )
    *encode = KSM_NO_ENCODING;
  else
    *encode = ksm_string_value(enc);
}

KSM_DEF_BLT(call_with_input_file)
     // (call-with-input-file string [encoding] proc)
{
  ksm_t proc, port, val;
  char *fn, *encode;

  parse_args(KSM_ARGS(), &fn, &encode, &proc);
  port = ksm_open_input_file(fn, encode);
  val = ksm_funcall(proc, ksm_cons(port, ksm_nil_object), KSM_ENV());
  ksm_close_port(port);
  return val;
}

KSM_DEF_BLT(call_with_output_file)
     // (call-with-output-file string [encoding] proc)
{
  ksm_t proc, port, val;
  char *fn, *encode;

  parse_args(KSM_ARGS(), &fn, &encode, &proc);
  port = ksm_open_output_file(fn, encode);
  val = ksm_funcall(proc, ksm_cons(port, ksm_nil_object), KSM_ENV());
  ksm_close_port(port);
  return val;
}

KSM_DEF_PRED(input_port_p, ksm_is_input_port)
KSM_DEF_PRED(output_port_p, ksm_is_output_port)

KSM_DEF_BLT(current_input_port)
     // (current-input-port)
{
  KSM_SET_ARGS0();
  return INPUT;
}

KSM_DEF_BLT(current_output_port)
     // (current-output-port)
{
  KSM_SET_ARGS0();
  return OUTPUT;
}

KSM_DEF_BLT(with_input_from_file)
     // (with-input-from-file string [encoding] thunk)
{
  ksm_t thunk, val, save, *frame;
  char *fn, *encode;

  parse_args(KSM_ARGS(), &fn, &encode, &thunk);
  frame = ksm_registry_top();
  save = INPUT;
  ksm_registry_add(save);
  INPUT = ksm_open_input_file(fn, encode);
  val = ksm_funcall(thunk, ksm_nil_object, KSM_ENV());
  ksm_close_port(INPUT);
  INPUT = save;
  ksm_registry_restore_with(frame, val);
  return val;
}

KSM_DEF_BLT(with_output_to_file)
     // (with-output-to-file string thunk)
{
  ksm_t thunk, val, save, *frame;
  char *fn, *encode;

  parse_args(KSM_ARGS(), &fn, &encode, &thunk);
  frame = ksm_registry_top();
  save = OUTPUT;
  ksm_registry_add(save);
  OUTPUT = ksm_open_output_file(fn, encode);
  val = ksm_funcall(thunk, ksm_nil_object, KSM_ENV());
  ksm_close_port(OUTPUT);
  OUTPUT = save;
  ksm_registry_restore_with(frame, val);
  return val;
}

KSM_DEF_BLT(open_input_file)
     // (open-input-file string [encoding])
{
  char *fn, *encode;
  
  parse_args(KSM_ARGS(), &fn, &encode, 0);
  return ksm_open_input_file(fn, encode);
}

KSM_DEF_BLT(open_output_file)
     // (open-output-file string [encoding])
{
  char *fn, *encode;

  parse_args(KSM_ARGS(), &fn, &encode, 0);
  return ksm_open_output_file(fn, encode);
}

KSM_DEF_BLT(close_input_port)
     // (close-input-port port)
{
  ksm_t port;

  KSM_POP_ARG(port);
  if( !ksm_is_input_port(port) )
    ksm_error("input port expected: %k", port);
  ksm_close_port(port);
  return ksm_unspec_object;
}

KSM_DEF_BLT(close_output_port)
     // (close-output-port port)
{
  ksm_t port;

  KSM_POP_ARG(port);
  if( !ksm_is_output_port(port) )
    ksm_error("output port expected: %k", port);
  ksm_close_port(port);
  return ksm_unspec_object;
}

static
ksm_t opt_port(ksm_t args, int output_p)
{
  ksm_t port;

  if( ksm_is_nil(args) )
    return output_p ? OUTPUT : INPUT;

  if( ksm_length(args) != 1 )
    ksm_error("too many args: %k", args);
  port = ksm_car(args);
  if( output_p ){
    if( !ksm_is_output_port(port) )
      ksm_error("output port expected: %k", port);
  }
  else{
    if( !ksm_is_input_port(port) )
      ksm_error("input port expected: %k", port);
  }
  return port;
}


KSM_DEF_BLT(read)
     // (read [port])
{
  ksm_t port;

  port = opt_port(KSM_ARGS(), 0);
  return ksm_read_object(port);
}

KSM_DEF_BLT(read_char)
     // (read-char [port])
{
  ksm_t port;
  int ch;

  port = opt_port(KSM_ARGS(), 0);
  ch = ksm_port_getc(port);
  return (ch == EOF) ? ksm_eof_object : ksm_make_char(ch);
}

KSM_DEF_BLT(peek_char)
     // (peek-char [port])
{
  ksm_t port;
  int ch;

  port = opt_port(KSM_ARGS(), 0);
  if( ksm_port_eof_p(port) || (ch = ksm_port_getc(port)) == EOF )
    return ksm_eof_object;
  if( ksm_port_ungetc(ch, port) == EOF )
    ksm_error("ungetc failed (PEEK-CHAR): %k", port);
  return ksm_make_char(ch);
}

KSM_DEF_PRED(eof_object_p, ksm_is_eof)

KSM_DEF_BLT(char_ready_p)
     // (char-ready? [port])
{
  ksm_t port;

  port = opt_port(KSM_ARGS(), 0);
  return KSM_BOOL2OBJ(ksm_port_char_ready_p(port));
}

void ksm_write_x(ksm_t obj, ksm_t port, int alt)
{
  char *s, *fmt, buf[200];
  int size = 200, n;

  if( ksm_is_char(obj) ){
    ksm_write_char_to_port(obj, port, alt);
    return;
  }

  fmt = alt ? "%#k" : "%k";
  s = buf;
  n = snprintf(s, size, fmt, obj);
  if( n >= 200 ){
    s = ksm_alloc(n+1);
    snprintf(s, n+1, fmt, obj);
  }
  if( ksm_port_puts(s, port) == EOF )
    ksm_error("write error: %k", obj);
  if( s != buf )
    ksm_dispose(s);
}

KSM_DEF_BLT(write)
     // (write obj [port])
{
  ksm_t obj, port;

  KSM_POP_ARG(obj);
  port = opt_port(KSM_ARGS(), 1);
  ksm_write_x(obj, port, 1);
  return ksm_unspec_object;
}

KSM_DEF_BLT(display)
     // (display obj [port])
{
  ksm_t obj, port;

  KSM_POP_ARG(obj);
  port = opt_port(KSM_ARGS(), 1);
  ksm_write_x(obj, port, 0);
  return ksm_unspec_object;
}

void ksm_newline(ksm_t port)
{
  if( ksm_port_putc('\n', port) == EOF )
    ksm_error("write error (NEWLINE)");
}

KSM_DEF_BLT(newline)
     // (newline [port])
{
  ksm_t port;

  port = opt_port(KSM_ARGS(), 1);
  ksm_newline(port);
  return ksm_unspec_object;
}

KSM_DEF_BLT(write_char)
     // (write-char char [port])
{
  ksm_t obj, port;

  KSM_POP_ARG(obj);
  port = opt_port(KSM_ARGS(), 1);
  if( ksm_port_putc(ksm_char_value(obj), port) == EOF )
    ksm_error("write error (WRITE-CHAR): %k", obj);
  return ksm_unspec_object;
}

ksm_t ksm_input(void)
{
  return INPUT;
}

ksm_t ksm_output(void)
{
  return OUTPUT;
}

#define STANDARD_INPUT_CODING(port) \
  ksm_make_input_charset_decoding_port(ksm_standard_encoding, port)
#define STANDARD_OUTPUT_CODING(port) \
  ksm_make_output_charset_encoding_port(ksm_standard_encoding, port)
#define STANDARD_CHARSET ksm_standard_encoding

static
void ksm_set_io(void)
{
  ksm_stdin_port = ksm_make_input_file_port(stdin);
  ksm_stdout_port = ksm_make_output_file_port(stdout);
  ksm_stderr_port = ksm_make_output_file_port(stderr);
  if( STANDARD_CHARSET ){
    ksm_stdin_port = STANDARD_INPUT_CODING(ksm_stdin_port);
    ksm_stdout_port = STANDARD_OUTPUT_CODING(ksm_stdout_port);
    ksm_stderr_port = STANDARD_OUTPUT_CODING(ksm_stderr_port);
  }

  INPUT = ksm_stdin_port;
  OUTPUT = ksm_stdout_port;
}

void ksm_blt_io(void)
{
  ksm_set_io();
  KSM_ENTER_BLT("call-with-input-file", call_with_input_file);
  KSM_ENTER_BLT("call-with-output-file", call_with_output_file);
  KSM_ENTER_BLT("input-port?", input_port_p);
  KSM_ENTER_BLT("output-port?", output_port_p);
  KSM_ENTER_BLT("current-input-port", current_input_port);
  KSM_ENTER_BLT("current-output-port", current_output_port);
  KSM_ENTER_BLT("with-input-from-file", with_input_from_file);
  KSM_ENTER_BLT("with-output-to-file", with_output_to_file);
  KSM_ENTER_BLT("open-input-file", open_input_file);
  KSM_ENTER_BLT("open-output-file", open_output_file);
  KSM_ENTER_BLT("close-input-port", close_input_port);
  KSM_ENTER_BLT("close-output-port", close_output_port);
  KSM_ENTER_BLT("read", read);
  KSM_ENTER_BLT("read-char", read_char);
  KSM_ENTER_BLT("peek-char", peek_char);
  KSM_ENTER_BLT("eof-object?", eof_object_p);
  KSM_ENTER_BLT("char-ready?", char_ready_p);
  KSM_ENTER_BLT("write", write);
  KSM_ENTER_BLT("display", display);
  KSM_ENTER_BLT("newline", newline);
  KSM_ENTER_BLT("write-char", write_char);
}


