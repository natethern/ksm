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
#include <ctype.h>
#include "ksm.h"

int ksm_is_ws(int ch)
{
  return ch == ' ' || ch == '\t' || ch == '\n';
}

int ksm_is_delim(int ch)
{
  return ch == '(' || ch == ')' || ch == ';';
}

//static ksm_strbuf buf;
#define READ_BUF  (&ksm_get_ctx()->read_buf);

char *ksm_stuff_lexeme(int ch, ksm_t port)
     // if ch is '\0', ch is not stuffed
{
  ksm_strbuf *bufp = READ_BUF;

  ksm_strbuf_rewind(bufp);
  if( ch != '\0' )
    ksm_strbuf_put(ch, bufp);
  while(1){
    ch = ksm_port_getc(port);
    if( ch == EOF || ksm_is_ws(ch) )
      break;
    if( ksm_is_delim(ch) ){
      ksm_port_ungetc(ch, port);
      break;
    }
    ksm_strbuf_put(ch, bufp);
  }
  return ksm_strbuf_content(bufp, 0);
}

#define stuff_lexeme ksm_stuff_lexeme

static
void cvt_lower(char *s)
{
  int ch;

  while( (ch = *s) )
    *s++ = tolower(ch);
}

#define DUMMY_READER(name) \
static \
ksm_t dummy_##name##_reader(ksm_t port) \
{ \
  ksm_fatal(#name " reader not installed"); \
  return ksm_unspec_object; \
}

ksm_t ksm_eat_comment(ksm_t port)
{
  int ch;

  do{
    ch = ksm_port_getc(port);
  } while( !(ch == EOF || ch == '\n') );
  return ksm_comment_object;
}

DUMMY_READER(list)
DUMMY_READER(quote)
DUMMY_READER(backquote)
DUMMY_READER(comma)
DUMMY_READER(string)
DUMMY_READER(sharp)
DUMMY_READER(colon)
     //DUMMY_READER(lbracket)
     //DUMMY_READER(rbracket)
     //DUMMY_READER(lbrace)
     //DUMMY_READER(rbrace)
     //DUMMY_READER(bar)

ksm_t (*ksm_list_reader)(ksm_t port) = dummy_list_reader;  /*** GLOBAL ***/
ksm_t (*ksm_quote_reader)(ksm_t port) = dummy_quote_reader; /*** GLOBAL ***/
ksm_t (*ksm_backquote_reader)(ksm_t port) /*** GLOBAL ***/
                                      = dummy_backquote_reader;
ksm_t (*ksm_comma_reader)(ksm_t port) = dummy_comma_reader; /*** GLOBAL ***/
ksm_t (*ksm_string_reader)(ksm_t port) = dummy_string_reader; /*** GLOBAL ***/
ksm_t (*ksm_sharp_reader)(ksm_t port) = dummy_sharp_reader; /*** GLOBAL ***/
ksm_t (*ksm_colon_reader)(ksm_t port) = dummy_colon_reader; /*** GLOBAL ***/
ksm_t (*ksm_semicolon_reader)(ksm_t port) = ksm_eat_comment; /*** GLOBAL ***/
//ksm_t (*ksm_lbracket_reader)(ksm_t port)  = dummy_lbracket_reader;
//ksm_t (*ksm_rbracket_reader)(ksm_t port)  = dummy_rbracket_reader;
//ksm_t (*ksm_lbrace_reader)(ksm_t port)    = dummy_lbrace_reader;
//ksm_t (*ksm_rbrace_reader)(ksm_t port)    = dummy_rbrace_reader;
//ksm_t (*ksm_bar_reader)(ksm_t port)       = dummy_bar_reader;

ksm_t ksm_string_to_exact(char *s, int radix)
     // returns 0 if failure
{
  ksm_t x;

  if( (x = ksm_integer_maker(s, radix)) )
    return x;
#ifdef GMP
  if( (x = ksm_mpz_maker(s, radix)) )
    return x;
  if( (x = ksm_mpq_maker(s, radix)) )
    return x;
#endif
  return 0;
}

ksm_t ksm_string_to_inexact(char *s)
     // returns 0 if failure
{
  ksm_t x;

  if( (x = ksm_double_maker(s)) )
    return x;
  if( (x = ksm_complex_maker(s)) )
    return x;
  return 0;
}

#define read_number    ksm_string_to_number

ksm_t ksm_string_to_number(char *s, int radix)
{
  ksm_t x;

  if( (x = ksm_string_to_exact(s, radix)) )
    return x;
  if( radix == 10 )
    return ksm_string_to_inexact(s);
  return 0;
}

typedef ksm_t (*obj_maker)(char *);

#define MAX_MAKER 10
static obj_maker makers[MAX_MAKER]; /*** GLOBAL ***/
static int nmakers;                 /*** GLOBAL ***/

void ksm_install_object_maker(ksm_t (*f)(char *))
{
  if( nmakers >= MAX_MAKER )
    ksm_fatal("too many object makers");
  makers[nmakers++] = f;
}

static
ksm_t make_object(char *str)
{
  int i;
  ksm_t x;

  for(i=0;i<nmakers;++i){
    if( (x = (*makers[i])(str)) )
      return x;
  }
  return ksm_make_key(str);
}

static int case_sensitive = 1;   /*** GLOBAL ***/

void ksm_set_read_case_sensitivity(int case_sensitive_p)
{
  case_sensitive = case_sensitive_p;
}

ksm_t ksm_read_object(ksm_t port)
{
  int ch;
  ksm_t obj;
  char *lexeme;

 again:
  if( ksm_port_eof_p(port) )
    return ksm_eof_object;
  ch = ksm_port_getc(port);
  while( ksm_is_ws(ch) )
    ch = ksm_port_getc(port);
  if( ch == EOF )
    return ksm_eof_object;
  switch(ch){
  case '(': obj = (*ksm_list_reader)(port); break;
  case ')': return ksm_rpar_object;
  case '\'': obj = (*ksm_quote_reader)(port); break;
  case '`': obj = (*ksm_backquote_reader)(port); break;
  case ',': obj = (*ksm_comma_reader)(port); break;
  case '"': obj = (*ksm_string_reader)(port); break;
  case '#': obj = (*ksm_sharp_reader)(port); break;
  case ';': obj = (*ksm_semicolon_reader)(port); break;
  case ':': obj = (*ksm_colon_reader)(port); break;
    //case '[': obj = (*ksm_lbracket_reader)(port); break;
    //case ']': obj = (*ksm_rbracket_reader)(port); break;
    //case '{': obj = (*ksm_lbrace_reader)(port); break;
    //case '}': obj = (*ksm_rbrace_reader)(port); break;
    //case '|': obj = (*ksm_bar_reader)(port); break;
  default:
    lexeme = stuff_lexeme(ch, port);
    if( (ch >= '0' && ch <= '9') || ch == '+' || ch == '-' || ch == '.' ){
      if( (obj = read_number(lexeme, 10)) )
	return obj;
    }
    if( !case_sensitive )
      cvt_lower(lexeme);
    obj = make_object(lexeme);
    break;
  }
  if( ksm_is_comment(obj) )
    goto again;
  return obj;
}

ksm_t (*ksm_read_toplevel_fun)(ksm_t port) = ksm_read_object;

ksm_t ksm_read_toplevel(ksm_t port)
{
  return (*ksm_read_toplevel_fun)(port);
}
