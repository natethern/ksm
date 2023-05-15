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

#include <assert.h>

#define UTF8_L1(ch)   (((ch)&0x80)==0)
#define UTF8_L2(ch)   (((ch)&0xE0)==0xC0)
#define UTF8_L3(ch)   (((ch)&0xF0)==0xE0)
#define UTF8_A(ch)    (((ch)&0xC0)==0x80)

#define UTF8_S1(s)    UTF8_L1(*(s))
#define UTF8_S2(s)    (UTF8_L2(*(s))&&UTF8_A(*(s+1)))
#define UTF8_S3(s)    (UTF8_L3(*(s))&&UTF8_A(*(s+1))&&UTF8_A(*(s+2)))

#define UTF8_C1(s)    (*(s))
#define UTF8_C2(s)    ((((*(s))&0x1F)<<6)|((*(s+1))&0x3F))
#define UTF8_C3(s)    (((*(s)&0x0F)<<12)|((*(s+1))<<6)|((*(s+2))&0x3F))

#ifdef UTF8_BYTES
static
int utf8_bytes(int ucs4_code)
{
  if( ucs4_code >= 0 && ucs4_code <=0x7F )
    return 1;
  if( ucs4_code >= 0x80 && ucs4_code <= 0x7FF )
    return 2;
  /***
  if( ucs4_code >= 0x800 && ucs4_code <= 0xFFFF )
    return 3;
  ***/
  return 3;
}
#endif

#ifdef UTF8_SAVE
static
int utf8_save(char *dst, int ucs4_code)
     // returns saved bytes
{
  if( ucs4_code >= 0 && ucs4_code <=0x7F ){
    dst[0] = ucs4_code;
    return 1;
  }
  if( ucs4_code >= 0x80 && ucs4_code <= 0x7FF ){
    dst[0] = 0xC0 | ((ucs4_code>>6)&0x1F);
    dst[1] = 0x80 | (ucs4_code&0x3F);
    return 2;
  }

  /***
  if( ucs4_code >= 0x800 && ucs4_code <= 0xFFFF ){
    dst[0] = 0xE0 | ((ucs4_code>>12)&0x0F);
    dst[1] = 0x80 | ((ucs4_code>>6)&0x3F);
    dst[2] = 0x80 | (ucs4_code&0x3F);
    return 3;
  }
  ***/
  dst[0] = 0xE0 | ((ucs4_code>>12)&0x0F);
  dst[1] = 0x80 | ((ucs4_code>>6)&0x3F);
  dst[2] = 0x80 | (ucs4_code&0x3F);
  return 3;
}
#endif

#ifdef UTF8_CHECK
static
int utf8_check(char *s)
     // returns 0 if OK
{
  while( *s ){
    if( UTF8_S1(s) )
      s += 1;
    else if( UTF8_S2(s) )
      s += 2;
    else if( UTF8_S3(s) )
      s += 3;
    else
      return -1;
  }
  return 0;
}
#endif

#ifdef UTF8_PEEK
static
int utf8_peek(char *s)
{
  if( UTF8_S1(s) )
    return UTF8_C1(s);
  if( UTF8_S2(s) )
    return UTF8_C2(s);
  assert(UTF8_S3(s));
  return UTF8_C3(s);
}
#endif

#ifdef UTF8_PEEK2
static
int utf8_peek2(char *s)
{
  if( UTF8_S1(s) )
    s += 1;
  else if( UTF8_S2(s) )
    s += 2;
  else{
    assert(UTF8_S3(s));
    s += 3;
  }
  return utf8_peek(s);
}
#endif

#ifdef UTF8_NEXT
static
void utf8_next(char **sp)
{
  char *s = *sp;

  if( UTF8_S1(s) )
    *sp += 1;
  else if( UTF8_S2(s) )
    *sp += 2;
  else{
    assert( UTF8_S3(s) );
    *sp += 3;
  }
}
#endif

#ifdef UTF8_NEXT2
static
void utf8_next2(char **sp)
{
  utf8_next(sp);
  utf8_next(sp);
}
#endif

#ifdef UTF8_NEXT_N
static
void utf8_next_n(char **sp, int n)
{
  while( n-- > 0 )
    utf8_next(sp);
}
#endif

#ifdef UTF8_GET_NEXT
static
int utf8_get_next(char **sp)
{
  int ch = utf8_peek(*sp);

  utf8_next(sp);
  return ch;
}
#endif

#ifdef UTF8_BACK
static
char *utf8_back(char *s)
{
  while( UTF8_A(*--s) )
    ;
  return s;
}
#endif

#ifdef UTF8_FWD
static
char *utf8_fwd(char *s)
     // returns 0 if invalid UTF-8 sequence
{
  if( UTF8_S1(s) )
    return s+1;
  if( UTF8_S2(s) )
    return s+2;
  if( UTF8_S3(s) )
    return s+3;
  return 0;
}
#endif

