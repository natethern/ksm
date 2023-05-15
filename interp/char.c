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

#include <ctype.h>
#include "ksm.h"

#define MAKE_CHAR(ch)  ksm_itype(KSM_FLAG_CHAR,ch,0,0)
#define IS_CHAR(x)     (ksm_flag(x)==KSM_FLAG_CHAR)
#define CHAR_VALUE(x)  ksm_ival(x,0)

ksm_t ksm_make_char(int ch)
{
  return MAKE_CHAR(ch);
}

int ksm_is_char(ksm_t x)
{
  return IS_CHAR(x);
}

int ksm_fetch_char(ksm_t ch)
{
  return CHAR_VALUE(ch);
}

static
int char_charvalue(ksm_t ch, int *result)
{
  *result = CHAR_VALUE(ch);
  return 0;
}

typedef int (*charvalue_fun)(ksm_t x, int *result);
KSM_INSTALLER(charvalue,charvalue_fun)   /*** GLOBAL ***/
#define GET_CHARVALUE(x)  get_charvalue(ksm_flag(x))

int ksm_char_value_x(ksm_t x, int *result)
     // returns 0 if OK
{
  charvalue_fun f;

  f = GET_CHARVALUE(x);
  return f ? (*f)(x, result) : -1;
}

int ksm_char_value(ksm_t ch)
{
  int c;

  if( ksm_char_value_x(ch, &c) != 0 )
    ksm_error("can't convert to char: %k", ch);
  return c;
}

static
int match_string(ksm_t port, char *s)
     // returns 0 if OK
{
  int ch;

  while( *s ){
    ch = ksm_port_getc(port);
    if( tolower(ch) != *s++ )
      return -1;
  }
  return 0;
}

ksm_t ksm_read_char(ksm_t port)
{
  int ch, trail, n;
  char buf[3], *cp;

  ch = ksm_port_getc(port);
  if( ch == EOF )
    ksm_error("unexpected EOF while reading character");
  n = ksm_utf8_coding_bytes(ch);
  if( n > 1 ){
    cp = buf;
    *cp++ = ch;
    while( --n > 0 ){
      ch = ksm_port_getc(port);
      if( ch == EOF )
	ksm_error("unexpected EOF while reading character");
      *cp++ = ch;
    }
    ch = ksm_utf8_to_ucs4(buf);
  }
  trail = ksm_port_getc(port);
  if( ch == 'U' ){
    if( trail == '{' ){
      ch = 0;
      while(1){
	trail = ksm_port_getc(port);
	if( trail >= '0' && trail <= '9' ){
	  ch <<= 4;
	  ch += (trail - '0');
	}
	else if( trail >= 'a' && trail <= 'f' ){
	  ch <<= 4; 
	  ch += (trail + 10 - 'a');
	}
	else if( trail >= 'A' && trail <= 'F' ){
	  ch <<= 4; 
	  ch += (trail + 10 - 'A');
	}
	else
	  break;
      }
      if( trail != '}' ){
	if( trail == EOF )
	  ksm_error("unexpected EOF while reading Unicode");
	ksm_error("unexpected Unicode: 0X%X", trail);
      }
    }
    else if( !isalnum(trail) ){
      ksm_port_ungetc(trail, port);
      ch = 'U';
    }
    else
      ksm_error("invalid character: #\\U%c", trail);
    return ksm_make_char(ch);
  }
  if( !isalnum(trail) ){
    ksm_port_ungetc(trail, port);
    return ksm_make_char(ch);
  }
  if( (ch == 's' || ch == 'S') ){
    if( match_string(port, "pace") != 0 )
      ksm_error("can't read #\\space");
    return ksm_make_char(' ');
  }
  if( (ch == 'n' || ch == 'N') ){
    if( match_string(port, "ewline") != 0 )
      ksm_error("can't read #\\newline");
    return ksm_make_char('\n');
  }
  ksm_error("can't read character: #%c%c", ch, trail);
  return ksm_unspec_object;
}

static
ksm_t read_char(char lead, ksm_t port)
{
  return ksm_read_char(port);
}

int ksm_fprintf_char(FILE *fp, int ch, int alt)
{
  char buf[3];
  int i, n;

  if( alt ){
    if( ch == ' ' )
      return fprintf(fp, "#\\space");
    if( ch == '\n' )
      return fprintf(fp, "#\\newline");
    if( ch >= 0x21 && ch <= 0x7e )
      return fprintf(fp, "#\\%c", ch);
    return fprintf(fp, "#\\U{%04X}", ch);
  }
  else{
    n = ksm_utf8_from_ucs4(buf, ch);
    for(i=0;i<n;++i)
      if( putc(buf[i], fp) == EOF )
	ksm_error("write failure: %x", buf[i]);
    return n;
  }
}

static
int fprint_char(FILE *fp, ksm_t ch, int alt)
{
  return ksm_fprintf_char(fp, CHAR_VALUE(ch), alt);
}

void ksm_write_char_to_port(ksm_t x, ksm_t port, int alt)
{
  char buf[50];

  if( snprintf(buf, 50, alt?"%#k":"%k", x) >= 50 )
    ksm_error("character write error: %x", CHAR_VALUE(x));
  if( ksm_port_puts(buf, port) == -1 )
    ksm_error("character write error: %x", CHAR_VALUE(x));
}

KSM_DEF_BLT(char_p)
     // (char? obj)
{
  ksm_t obj;

  KSM_SET_ARGS1(obj);
  return KSM_BOOL2OBJ(ksm_is_char(obj));
}

#define KSM_DEF_CHARCMP(name, cmp) \
KSM_DEF_BLT(name) \
{ \
  ksm_t x, y; \
  int a, b; \
 \
  KSM_SET_ARGS2(x, y); \
  a = ksm_char_value(x); \
  b = ksm_char_value(y); \
  return KSM_BOOL2OBJ(a cmp b); \
}

KSM_DEF_CHARCMP(char_eq, ==)
KSM_DEF_CHARCMP(char_lt, <)
KSM_DEF_CHARCMP(char_gt, >)
KSM_DEF_CHARCMP(char_le, <=)
KSM_DEF_CHARCMP(char_ge, >=)

#define KSM_DEF_CHARCMP_CI(name, cmp) \
KSM_DEF_BLT(name) \
{ \
  ksm_t x, y; \
  int a, b; \
 \
  KSM_SET_ARGS2(x, y); \
  a = ksm_char_value(x); \
  b = ksm_char_value(y); \
  return KSM_BOOL2OBJ(ksm_unicode_tolower(a) cmp ksm_unicode_tolower(b)); \
}

KSM_DEF_CHARCMP_CI(char_ci_eq, ==)
KSM_DEF_CHARCMP_CI(char_ci_lt, <)
KSM_DEF_CHARCMP_CI(char_ci_gt, >)
KSM_DEF_CHARCMP_CI(char_ci_le, <=)
KSM_DEF_CHARCMP_CI(char_ci_ge, >=)

#define KSM_DEF_CHARPRED(name, f) \
KSM_DEF_BLT(name) \
{ \
  ksm_t x; \
  int ch; \
 \
  KSM_SET_ARGS1(x); \
  ch = ksm_char_value(x); \
  return KSM_BOOL2OBJ(f(ch)); \
}

KSM_DEF_CHARPRED(char_alphabetic_p, isalpha)
KSM_DEF_CHARPRED(char_numeric_p, isdigit)
KSM_DEF_CHARPRED(char_whitespace_p, isspace)

KSM_DEF_BLT(char_upper_case_p)
{
  ksm_t x;
  int ch;

  KSM_SET_ARGS1(x);
  ch = ksm_char_value(x);
  if( ch >= 0 && ch <= 0x7F )
    return KSM_BOOL2OBJ(isupper(ch));
  else
    return KSM_BOOL2OBJ(ksm_unicode_isupper(ch));
}

KSM_DEF_BLT(char_lower_case_p)
{
  ksm_t x;
  int ch;

  KSM_SET_ARGS1(x);
  ch = ksm_char_value(x);
  if( ch >= 0 && ch <= 0x7F )
    return KSM_BOOL2OBJ(islower(ch));
  else
    return KSM_BOOL2OBJ(ksm_unicode_islower(ch));
}

KSM_DEF_BLT(char_to_integer)
     // (char->integer char)
{
  ksm_t x;
  int ch;

  KSM_SET_ARGS1(x);
  ch = ksm_char_value(x);
  return ksm_make_int(ch);
}

KSM_DEF_BLT(integer_to_char)
     // (integer->char n)
{
  ksm_t x;
  int n;

  KSM_SET_ARGS1(x);
  n = ksm_int_value(x);
  return ksm_make_char(n&0xff);
}

KSM_DEF_BLT(char_upcase)
     // (char-upcase char)
{
  ksm_t x;
  int ch;

  KSM_SET_ARGS1(x);
  ch = ksm_char_value(x);
  if( ch <= 0x7F )
    return ksm_make_char(toupper(ch));
  else
    return ksm_make_char(ksm_unicode_toupper(ch));
}

KSM_DEF_BLT(char_downcase)
     // (char-downcase char)
{
  ksm_t x;
  int ch;

  KSM_SET_ARGS1(x);
  ch = ksm_char_value(x);
  if( ch <= 0x7F )
    return ksm_make_char(tolower(ch));
  else
    return ksm_make_char(ksm_unicode_tolower(ch));
}

void ksm_init_char(void)
{
  ksm_install_sharp_reader('\\', read_char);
  ksm_install_fprintf(KSM_FLAG_CHAR, fprint_char);
  ksm_install_charvalue(KSM_FLAG_CHAR, char_charvalue);
  KSM_ENTER_BLT("char?", char_p);
  KSM_ENTER_BLT("char=?", char_eq);
  KSM_ENTER_BLT("char<?", char_lt);
  KSM_ENTER_BLT("char>?", char_gt);
  KSM_ENTER_BLT("char<=?", char_le);
  KSM_ENTER_BLT("char>=?", char_ge);
  KSM_ENTER_BLT("char-ci=?", char_ci_eq);
  KSM_ENTER_BLT("char-ci<?", char_ci_lt);
  KSM_ENTER_BLT("char-ci>?", char_ci_gt);
  KSM_ENTER_BLT("char-ci<=?", char_ci_le);
  KSM_ENTER_BLT("char-ci>=?", char_ci_ge);
  KSM_ENTER_BLT("char-alphabetic?", char_alphabetic_p);
  KSM_ENTER_BLT("char-numeric?", char_numeric_p);
  KSM_ENTER_BLT("char-whitespace?", char_whitespace_p);
  KSM_ENTER_BLT("char-upper-case?", char_upper_case_p);
  KSM_ENTER_BLT("char-lower-case?", char_lower_case_p);
  KSM_ENTER_BLT("char->integer", char_to_integer);
  KSM_ENTER_BLT("integer->char", integer_to_char);
  KSM_ENTER_BLT("char-upcase", char_upcase);
  KSM_ENTER_BLT("char-downcase", char_downcase);
}
