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

void ksm_free_strbuf(ksm_strbuf *buf)
{
  if( buf->start )
    ksm_dispose(buf->start);
}

void ksm_strbuf_reserve(ksm_strbuf *buf, int size)
{
  int n;

  if( buf->end - buf->start >= size )
    return;
  n = buf->next - buf->start;
  buf->start = ksm_realloc(buf->start, size);
  buf->end = buf->start + size;
  buf->next = buf->start + n;
}

void ksm_strbuf_put(char ch, ksm_strbuf *buf)
{
  int size;

  if( buf->next >= buf->end ){
    size = (buf->start == buf->end) ? 256 : (buf->end - buf->start)*2;
    ksm_strbuf_reserve(buf, size);
  }
  if( !(buf->next < buf->end) )
    ksm_fatal("can't happen (ksm_strbuf_put)");
  *buf->next++ = ch;
}

char *ksm_strbuf_content(ksm_strbuf *buf, int *size)
     // return the current content
{
  if( size )
    *size = buf->next - buf->start;
  if( buf->start )
    ksm_strbuf_put('\0', buf);
  return buf->start;
}

char *ksm_strbuf_extract(ksm_strbuf *buf, int *size)
     // returns a newly allocated string
{
  char *s;

  if( size )
    *size = buf->next - buf->start;
  s = buf->start;
  if( s ){
    ksm_strbuf_put('\0', buf);
    ksm_realloc(s, buf->next - buf->start);
    buf->start = buf->end = buf->next = 0;
  }
  return s;
}

void ksm_strbuf_rewind(ksm_strbuf *buf)
{
  buf->next = buf->start;
}
