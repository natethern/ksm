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

#include <string.h>
#include "ksm.h"

#define MAKE_STRING(s) \
  ksm_itype(KSM_FLAG_STRING,(int)(s),0,0)
#define IS_STRING(x)  (ksm_flag(x)==KSM_FLAG_STRING)
#define STRING_VALUE(x)  ((char *)ksm_ival(x,0))
#define SET_STRING_VALUE(x,s) ksm_set_ival(x,0,(int)(s))

#define STRING_VALUE_NO_LOCK(x)  ((char *)ksm_ival_no_lock(x,0))

static
void dispose_string(ksm_t str)
{
  ksm_dispose(STRING_VALUE_NO_LOCK(str));
}

ksm_t ksm_make_string(char *s)
{
  char *p;

  p = ksm_alloc(strlen(s)+1);
  strcpy(p, s);
  return MAKE_STRING(p);
}

ksm_t ksm_make_string_no_copy(char *s)
{
  return MAKE_STRING(s);
}

ksm_t ksm_make_string_n(char *s, int n)
{
  char *p;

  p = ksm_alloc(n+1);
  memcpy(p, s, n);
  p[n] = '\0';
  return MAKE_STRING(p);
}

int ksm_is_string(ksm_t x)
{
  return IS_STRING(x);
}

char *ksm_fetch_string(ksm_t s)
{
  return STRING_VALUE(s);
}

int ksm_string_value_x(ksm_t x, char **result)
     // returns 0 if OK
{
  if( IS_STRING(x) ){
    *result = STRING_VALUE(x);
    return 0;
  }
  if( ksm_is_key(x) ){
    *result = ksm_fetch_key(x);
    return 0;
  }
  return -1;
}

char *ksm_string_value(ksm_t x)
{
  char *s;

  if( IS_STRING(x) )
    return STRING_VALUE(x);
  if( ksm_string_value_x(x, &s) == 0 )
    return s;
  ksm_error("can't convert to string: %k", x);
  return 0;
}

ksm_t ksm_read_string(ksm_t port, ksm_strbuf *bufp)
     // first char in PORT is the one immediately following `"'
{
  int ch;
  
  ksm_strbuf_rewind(bufp);
  while( 1 ){
    ch = ksm_port_getc(port);
    if( ch == '"' ){
      ksm_strbuf_put('\0', bufp);
      break;
    }
    if( ch == '\\' ){
      ch = ksm_port_getc(port);
      if( ch == EOF )
	ksm_error("unexpected EOF");
      switch(ch){
      case '"': break;
      case 'n': ch = '\n'; break;
      case 'r': ch = '\r'; break;
      case 't': ch = '\t'; break;
      case '\\': ch = '\\'; break;
      case 'U':
	{
	  int trail, i, n;
	  char buf[3];

	  if( (ch = ksm_port_getc(port)) != '{' ){
	    if( ch == EOF )
	      ksm_error("unexpected EOF while reading unicode");
	    ksm_error("can't read unicode: \\U%c\n", ch);
	  }
	  ch = 0;
	  while( 1 ){
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
	    ksm_error("unexpected character while reading unicode: 0X%X", 
		      trail);
	  }
	  n = ksm_utf8_from_ucs4(&buf[0], ch);
	  for(i=0;i<n;++i)
	    ksm_strbuf_put(buf[i], bufp);
	  continue;
	}
      default: 
	ksm_strbuf_put('\\', bufp);
	break;
      }
      ksm_strbuf_put(ch, bufp);
    }
    else
      ksm_strbuf_put(ch, bufp);
  }
  return ksm_make_string_no_copy(ksm_strbuf_extract(bufp, 0));
}

#define READSTR_BUF (&ksm_get_ctx()->readstr_buf)

static 
ksm_t read_str(ksm_t port)
{
  return ksm_read_string(port, READSTR_BUF);
}

int ksm_fprintf_string(FILE *fp, char *str, int alt)
{
  int n, i, ch;

  if( !alt )
    return fprintf(fp, "%s", str);

  n = i = fprintf(fp, "%c", '"');
  if( i <= 0 )
    return i;
  while( (ch = *str++) ){
    switch(ch){
    case '"': i = fprintf(fp, "\\\""); break;
    case '\n' : i = fprintf(fp, "\\n"); break;
    case '\r' : i = fprintf(fp, "\\r"); break;
    case '\t' : i = fprintf(fp, "\\t"); break;
    case '\\': i = fprintf(fp, "\\\\"); break;
    default: 
      if( ch >= 0 && ch <= 0x7f )
	i = fprintf(fp, "%c", ch);
      else{
	int bytes = ksm_utf8_coded_bytes(--str);

	if( bytes >= 2 && bytes <= 3 ){
	  i = fprintf(fp, "\\U{%04X}", ksm_utf8_to_ucs4(str));
	  str += bytes;
	}
	else
	  ksm_fatal("can't happen (ksm_fprintf_string): %d", bytes);
      }
    }
    if( i <= 0 )
      return i;
    n += i;
  }
  i = fprintf(fp, "%c", '"');
  if( i <= 0 )
    return i;
  return n + i;
}

static
int fprint_string(FILE *fp, ksm_t str, int alt)
{
  return ksm_fprintf_string(fp, STRING_VALUE(str), alt);
}

static
int equal_string(ksm_t a, ksm_t b)
{
  if( !IS_STRING(b) )
    return 0;
  return strcmp(ksm_fetch_string(a), ksm_fetch_string(b)) == 0;
}

KSM_DEF_BLT(string_p)
{
  ksm_t x;

  KSM_SET_ARGS1(x);
  return KSM_BOOL2OBJ(ksm_is_string(x));
}

static
char *string_fill(int k, int ch)
{
  int n;
  char *p, *q, buf[3];

  n = ksm_utf8_required_bytes(ch);
  switch(n){
  case 1: p = ksm_alloc(k+1); memset(p, ch, k); p[k] = '\0'; break;
  case 2:
  case 3:
    p = q = ksm_alloc(k*n+1);
    ksm_utf8_from_ucs4(buf, ch);
    while( k-- > 0 ){
      memcpy(q, buf, n);
      q += n;
    }
    *q = '\0';
    break;
  default:
    p = 0;
    ksm_error("can't handle character: %x", ch);
  }
  return p;
}

static
ksm_t make_string_fill(int k, int ch)
{
  return MAKE_STRING(string_fill(k, ch));
}

KSM_DEF_BLT(make_string)
{
  int k, ch;
  ksm_t x;

  KSM_POP_ARG(x);
  k = ksm_int_value(x);
  if( !(k >= 0) )
    ksm_error("invalid first arg to MAKE-STRING: %k", k);
  if( KSM_MORE_ARGS() ){
    KSM_SET_ARGS1(x);
    ch = ksm_char_value(x);
  }
  else
    ch = '?';
  return make_string_fill(k, ch);
}

static
int required_byte_length(ksm_t list)
{
  int count = 0, ch, n;
  ksm_t x;

  while( ksm_is_cons(list) ){
    x = ksm_car(list);
    list = ksm_cdr(list);
    ch = ksm_char_value(x);
    n = ksm_utf8_required_bytes(ch);
    if( n == -1 )
      ksm_error("can't handle character: %d", ch);
    count += n;
  }
  if( !ksm_is_nil(list) )
    return -1;
  return count;
}

static
ksm_t list_to_string(ksm_t list)
{
  ksm_t obj;
  char *p, *q;
  int len, ch;

  len = required_byte_length(list);
  if( len < 0 )
    ksm_error("proper list expected: %k", list);
  p = q = ksm_alloc(len+1);
  p[len] = '\0';
  while( ksm_is_cons(list) ){
    obj = ksm_car(list);
    list = ksm_cdr(list);
    ch = ksm_char_value(obj);
    ksm_utf8_from_ucs4(q, ch);
    q += ksm_utf8_required_bytes(ch);
  }
  return MAKE_STRING(p);
}

KSM_DEF_BLT(string)
{
  return list_to_string(KSM_ARGS());
}

static
int utf8_string_length(char *p)
{
  int count = 0, n;

  while( *p ){
    n = ksm_utf8_coded_bytes(p);
    if( n == -1 )
      ksm_error("can't handle character: %x", *p);
    p += n;
    ++count;
  }
  return count;
}

KSM_DEF_BLT(string_length)
{
  ksm_t x;
  char *p;

  KSM_SET_ARGS1(x);
  p = ksm_string_value(x);
  return ksm_make_int(utf8_string_length(p));
}

static
char *utf8_string_ref(char *str, int index)
{
  int n;

  while( index-- > 0 ){
    n = ksm_utf8_coded_bytes(str);
    if( n == -1 )
      ksm_error("can't handle character: %x", *str);
    str += n;
  }
  return str;
}

KSM_DEF_BLT(string_ref)
     // (string-ref string k)
{
  ksm_t x, y;
  char *sp;
  int i;

  KSM_SET_ARGS2(x, y);
  sp = ksm_string_value(x);
  i = ksm_int_value(y);
  sp = utf8_string_ref(sp, i);
  return ksm_make_char(ksm_utf8_to_ucs4(sp));
}

static
void shift_string(char *p, int n)
{
  int len;

  len = strlen(p);
  memmove(p-n, p, len);
  *(p-n+len) = '\0';
}

KSM_DEF_BLT(string_set)
     // (string-set! string k char)
{
  ksm_t x, y, z;
  char *sp, *p;
  int i, n, m, ch, disp;

  KSM_SET_ARGS3(x, y, z);
  sp = ksm_string_value(x);
  i = ksm_int_value(y);
  ch = ksm_char_value(z);
  p = utf8_string_ref(sp, i);
  n = ksm_utf8_coded_bytes(p);
  m = ksm_utf8_required_bytes(ch);
  if( n == -1 )
    ksm_error("can't handle character: %x", *p);
  if( m == -1 )
    ksm_error("can't handle character: %x", ch);
  if( n == m )
    ksm_utf8_from_ucs4(p, ch);
  else{
    if( m > n ){
      disp = p - sp;
      sp = ksm_realloc(sp, strlen(sp)+m-n);
      SET_STRING_VALUE(x, sp);
      p = sp + disp;
    }
    shift_string(p+n, n-m);
    ksm_utf8_from_ucs4(p, ch);
  }
  return ksm_unspec_object;
}

#define KSM_DEF_STRCMP(name, fun, cmp) \
KSM_DEF_BLT(name) \
{ \
  ksm_t x, y; \
  char *p, *q; \
 \
  KSM_SET_ARGS2(x, y); \
  p = ksm_string_value(x); \
  q = ksm_string_value(y); \
  return KSM_BOOL2OBJ(fun(p,q) cmp 0); \
}

KSM_DEF_STRCMP(str_eq, strcmp, ==)
KSM_DEF_STRCMP(str_lt, strcmp, <)
KSM_DEF_STRCMP(str_gt, strcmp, >)
KSM_DEF_STRCMP(str_le, strcmp, <=)
KSM_DEF_STRCMP(str_ge, strcmp, >=)
KSM_DEF_STRCMP(str_ci_eq, strcasecmp, ==)
KSM_DEF_STRCMP(str_ci_lt, strcasecmp, <)
KSM_DEF_STRCMP(str_ci_gt, strcasecmp, >)
KSM_DEF_STRCMP(str_ci_le, strcasecmp, <=)
KSM_DEF_STRCMP(str_ci_ge, strcasecmp, >=)

ksm_t ksm_substring(char *str, int start, int end)
{
  char *p, *q, *r;
  int k;

  p = utf8_string_ref(str, start);
  q = utf8_string_ref(str, end);
  k = q - p;
  r = ksm_alloc(k+1);
  memcpy(r, p, k);
  r[k] = '\0';
  return MAKE_STRING(r);
}

KSM_DEF_BLT(substring)
     // (substring string start end)
{
  ksm_t arg1, arg2, arg3;
  int i, j;
  char *p;

  KSM_SET_ARGS3(arg1, arg2, arg3);
  p = ksm_string_value(arg1);
  i = ksm_int_value(arg2);
  j = ksm_int_value(arg3);
  return ksm_substring(p, i, j);
}

static
int str_total_length(ksm_t args)
{
  int n = 0;
  char *s;

  while( ksm_is_cons(args) ){
    s = ksm_string_value(ksm_car(args));
    n += strlen(s);
    args = ksm_cdr(args);
  }
  if( !ksm_is_nil(args) )
    return -1;
  return n;
}

ksm_t ksm_string_append(ksm_t list)
{
  int len;
  ksm_t str;
  char *p, *q, *s;

  len = str_total_length(list);
  if( len < 0 )
    ksm_error("can't append strings: %k", list);
  p = q = ksm_alloc(len+1);
  p[len] = '\0';
  while( ksm_is_cons(list) ){
    str = ksm_car(list);
    list = ksm_cdr(list);
    s = ksm_string_value(str);
    strcpy(p, s);
    p += strlen(s);
  }
  return MAKE_STRING(q);
}

KSM_DEF_BLT(string_append)
     // (string-append string ...)
{
  return ksm_string_append(KSM_ARGS());
}

KSM_DEF_BLT(string_to_list)
     // (string->list string)
{
  ksm_t str, head, tail;
  int n, ch;
  char *p;

  KSM_SET_ARGS1(str);
  p = ksm_string_value(str);
  ksm_list_begin(&head, &tail);
  while(*p){
    n = ksm_utf8_coded_bytes(p);
    ch = ksm_utf8_to_ucs4(p);
    p += n;
    ksm_list_extend(&head, &tail, ksm_make_char(ch));
  }
  return head;
}

KSM_DEF_BLT(list_to_string)
{
  ksm_t list;

  KSM_SET_ARGS1(list);
  return list_to_string(list);
}

KSM_DEF_BLT(string_copy)
     // (string-copy string)
{
  ksm_t str;
  char *src, *dst;

  KSM_SET_ARGS1(str);
  src = ksm_string_value(str);
  dst = ksm_alloc(strlen(src)+1);
  strcpy(dst, src);
  return MAKE_STRING(dst);
}

KSM_DEF_BLT(string_fill)
     // (string-fill! string char)
{
  ksm_t str, chr;
  char *p;
  int ch, len;

  KSM_SET_ARGS2(str, chr);
  p = ksm_string_value(str);
  ch = ksm_char_value(chr);
  len = utf8_string_length(p);
  SET_STRING_VALUE(str, string_fill(len, ch));
  ksm_dispose(p);
  return ksm_unspec_object;
}

void ksm_init_string(void)
{
  ksm_install_disposer(KSM_FLAG_STRING, dispose_string);
  ksm_string_reader = read_str;
  ksm_install_fprintf(KSM_FLAG_STRING, fprint_string);
  ksm_install_equal(KSM_FLAG_STRING, equal_string);
  KSM_ENTER_BLT("string?", string_p);
  KSM_ENTER_BLT("make-string", make_string);
  KSM_ENTER_BLT("string", string);
  KSM_ENTER_BLT("string-length", string_length);
  KSM_ENTER_BLT("string-ref", string_ref);
  KSM_ENTER_BLT("string-set!", string_set);
  KSM_ENTER_BLT("string=?", str_eq);
  KSM_ENTER_BLT("string<?", str_lt);
  KSM_ENTER_BLT("string>?", str_gt);
  KSM_ENTER_BLT("string<=?", str_le);
  KSM_ENTER_BLT("string>=?", str_ge);
  KSM_ENTER_BLT("string-ci=?", str_ci_eq);
  KSM_ENTER_BLT("string-ci<?", str_ci_lt);
  KSM_ENTER_BLT("string-ci>?", str_ci_gt);
  KSM_ENTER_BLT("string-ci<=?", str_ci_le);
  KSM_ENTER_BLT("string-ci>=?", str_ci_ge);
  KSM_ENTER_BLT("substring", substring);
  KSM_ENTER_BLT("string-append", string_append);
  KSM_ENTER_BLT("string->list", string_to_list);
  KSM_ENTER_BLT("list->string", list_to_string);
  KSM_ENTER_BLT("string-copy", string_copy);
  KSM_ENTER_BLT("string-fill!", string_fill);
}
