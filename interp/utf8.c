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

int ksm_utf8_coding_bytes(char start_byte)
     // returns 1, 2, 3, 4, 5, or 6 if OK
     // returns -1 if invalid start_byte for UTF-8 encoding
{
  if( (start_byte & 0x80) == 0 )
    return 1;
  if( (start_byte & 0xE0) == 0xC0 )
    return 2;
  if( (start_byte & 0xF0) == 0xE0 )
    return 3;
  return -1;
}

int ksm_utf8_coded_bytes(char *buf)
{
  int ch = buf[0], ch1, ch2;

  if( (ch & 0x80) == 0 )
    return 1;
  if( (ch & 0xE0) == 0xC0 ){
    ch1 = buf[1];
    if( (ch1 & 0xC0) != 0x80 )
      return -1;
    return 2;
  }
  if( (ch & 0xF0) == 0xE0 ){
    ch1 = buf[1];
    ch2 = buf[2];
    if( (ch1 & 0xC0) != 0x80 || (ch2 & 0xC0) != 0x80 )
      return -1;
    return 3;
  }
  return -1;
}

int ksm_utf8_to_ucs4(char *bytes)
     // returns -1 if failure
{
  char ch = bytes[0], ch1, ch2;

  if( (ch & 0x80) == 0 )
    return ch;
  if( (ch & 0xE0) == 0xC0 ){
    ch1 = bytes[1];
    if( (ch1 & 0xC0) != 0x80 )
      return -1;
    return ((ch&0x1F)<<6)|(ch1&0x3F);
  }
  if( (ch & 0xF0) == 0xE0 ){
    ch1 = bytes[1];
    ch2 = bytes[2];
    if( (ch1 & 0xC0) != 0x80 || (ch2 & 0xC0) != 0x80 )
      return -1;
    return (ch&0x0F)<<12|((ch1&0x3F)<<6)|(ch2&0x3F);
  }
  return -1;
}

int ksm_utf8_required_bytes(int ucs4_code)
{
  if( ucs4_code >= 0 && ucs4_code <=0x7F )
    return 1;
  if( ucs4_code >= 0x80 && ucs4_code <= 0x7FF )
    return 2;
  if( ucs4_code >= 0x800 && ucs4_code <= 0xFFFF )
    return 3;
  return -1;
}

int ksm_utf8_from_ucs4(char *dst, int ucs4_code)
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
  if( ucs4_code >= 0x800 && ucs4_code <= 0xFFFF ){
    dst[0] = 0xE0 | ((ucs4_code>>12)&0x0F);
    dst[1] = 0x80 | ((ucs4_code>>6)&0x3F);
    dst[2] = 0x80 | (ucs4_code&0x3F);
    return 3;
  }
  ksm_error("can't handle character: %x", ucs4_code);
  return 0;
}

