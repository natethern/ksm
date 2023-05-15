%{
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

#include "ksm.h"
#define YYSTYPE ksm_t
#include "parse.tab.h"
extern ksm_t ksm_clike_get_port(void);
extern int ksm_clike_get_token_save(void);
extern void ksm_clike_set_token_save(int);
extern ksm_t ksm_clike_get_lval_save(void);
extern void ksm_clike_set_lval_save(ksm_t lval);
extern int ksm_clike_yyempty;
extern ksm_t ksm_clike_port;
#define PORT ksm_clike_port
#define YY_DECL int ksm_clike_lex()
#define YY_INPUT(buf,result,max_size) \
  { \
    int c = ksm_port_getc(PORT); \
    result = (c == EOF) ? YY_NULL : (buf[0]=c, 1); \
  }
static int yywrap();
ksm_t ksm_clike_lval;
extern ksm_t ksm_clike_simple_string(void);
extern ksm_t ksm_clike_complex_string(void);
%}
%%
fun          return FUN;
var          return VAR;
if           return IF;
else         return ELSE;
while        return WHILE;
do           return DO;
for          return FOR;
return       return RETURN;
break        return BREAK;
cond         return COND;
case         return CASE;
extern       return EXTERN;
void         return VOID;
char         return CHAR;
short        return SHORT;
int          return INT;
long         return LONG;
float        return FLOAT;
double       return DOUBLE;
signed       return SIGNED;
unsigned     return UNSIGNED;
struct       return STRUCT;
union        return UNION;
typedef      return TYPEDEF;
sizeof       return SIZEOF;
as           return AS;
"->"         return ARROW;
"..."        return DOTS;
"<="         return LE;
">="         return GE;
"=="         return EQ;
"!="         return NE;
"&&"         return AND;
"||"         return OR;
"++"         return INC;
"--"         return DEC;
"<<"         return LSHIFT;
">>"         return RSHIFT;
"*="         return MUL_ASSIGN;
"/="         return DIV_ASSIGN;
"%="         return REM_ASSIGN;
"+="         return ADD_ASSIGN;
"-="         return SUB_ASSIGN;
"<<="        return LSHIFT_ASSIGN;
">>="        return RSHIFT_ASSIGN;
"&="         return AND_ASSIGN;
"^="         return XOR_ASSIGN;
"|="         return OR_ASSIGN;
"#t"         ksm_clike_lval = ksm_true_object; return TRUE_OBJ;
"#f"         ksm_clike_lval = ksm_false_object; return FALSE_OBJ;
[a-zA-Z_][a-zA-Z_0-9:]*[a-zA-Z_0-9?]*   return IDENT;
[1-9][0-9]*   return DECIMAL_NUMBER;
0             return DECIMAL_NUMBER;
0[xX][0-9a-fA-F]+  return HEX_NUMBER;
0[0-7]+  return OCTAL_NUMBER;
0[bB][01]+  return BINARY_NUMBER;
[1-9][0-9]*(\.[0-9]*)?([Ee][-+]?[0-9]+)?  return FLOAT_NUMBER;
\.[0-9]+([Ee][-+]?[0-9]+)?  return FLOAT_NUMBER;
[1-9][0-9]*(\.[0-9]*)?([Ee][-+]?[0-9]+)?i  return IMAGINARY_NUMBER;
\.[0-9]+([Ee][-+]?[0-9]+)?i  return IMAGINARY_NUMBER;
\"[^"\\]*\"         return SIMPLE_STRING;
\"([^"\\]|\\.)*\"   return COMPLEX_STRING;
#\\[Ss][Pp][Aa][Cc][Ee]    return CHAR_SPACE;
#\\[Nn][Ee][Ww][Ll][Ii][Nn][Ee]  return CHAR_NEWLINE;
#\\U\{[0-9A-Fa-f]+\}  return CHAR_UNICODE;
#\\.   return CHAR;
[ \n\t]                   /* ignore space */
.                         return ksm_clike_text[0];
%%
static int yywrap()
{
  return 1;
}

void ksm_clike_dummy()
{
  yyunput(0,0);
}

char ksm_clike_hold_char()
{
  return yy_hold_char;
}

YY_BUFFER_STATE ksm_clike_current_buffer(void)
{
  return yy_current_buffer;
}

ksm_t ksm_clike_simple_string(void)
{
  return ksm_make_string_n(yytext+1, yyleng-2);
}

static int sbuf_size;
static char *sbuf;

ksm_t ksm_clike_complex_string(void)
{
  int n, ch, nn;
  char *s, *d;

  n = yyleng - 2;
  if( sbuf_size < n ){
    if( sbuf )
      ksm_dispose(sbuf);
    sbuf_size = n;
    sbuf = ksm_alloc(sbuf_size);
  }
  s = yytext+1;
  d = sbuf;
  while( n-- > 0 ){
    ch = *s++;
    if( ch == '"' )
      break;
    if( ch == '\\' ){
      if( n <= 0 )
	ksm_error("can't happen in ksm_clike_complex_string");
      --n;
      ch = *s++;
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

	  if( n <= 0 )
	    ksm_error("unexpected EOF while reading unicode");
	  ch = *s++;
	  --n;
	  if( ch != '{' )
	    ksm_error("can't read unicode: \\U%c\n", ch);
	  ch = 0;
	  while( 1 ){
	    if( n <= 0 )
	      ksm_error("unexpected EOF while reading unicode");
	    trail = *s++;
	    --n;
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
	  if( trail != '}' )
	    ksm_error("unexpected character while reading unicode: 0X%X", 
		      trail);
	  nn = ksm_utf8_from_ucs4(&buf[0], ch);
	  for(i=0;i<nn;++i)
	    *d++ = buf[i];
	  continue;
	}
      default: 
	*d++ = '\\';
	break;
      }
      *d++ = ch;
    }
    else
      *d++ = ch;
  }
  return ksm_make_string_n(sbuf, d-sbuf);
}

ksm_t ksm_clike_lex_char(void)
{
  return ksm_make_char(yytext[2]);
}

ksm_t ksm_clike_lex_unicode(void)
{
  int ch = 0, c;
  char *p, *q;
  
  p = yytext + 4;
  q = yytext + yyleng - 1;
  while( p < q ){
    c = *p++;
    ch *= 16;
    if( c >= '0' && c <= '9' )
      ch += c - '0';
    else if( c >= 'A' && c <= 'F' )
      ch += c - 'A' + 10;
    else if( c >= 'a' && c <= 'f' )
      ch += c - 'a' + 10;
    else
      ksm_error("can't happen in ksm_clike_lex_unicode");
  }
  return ksm_make_char(ch);
}

ksm_t ksm_clike_lex_ident(void)
{
  return ksm_make_key(yytext);
}

ksm_t ksm_clike_decimal_number(void)
{
  return ksm_string_to_exact(yytext, 10);
}

ksm_t ksm_clike_hex_number(void)
{
  return ksm_string_to_exact(yytext+2, 16);
}

ksm_t ksm_clike_octal_number(void)
{
  return ksm_string_to_exact(yytext+1, 8);
}

ksm_t ksm_clike_binary_number(void)
{
  return ksm_string_to_exact(yytext+2, 2);
}

ksm_t ksm_clike_float_number(void)
{
  return ksm_string_to_inexact(yytext);
}

ksm_t ksm_clike_imaginary_number(void)
{
  return ksm_string_to_inexact(yytext);
}
