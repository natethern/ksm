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

struct memstream {
  void *cp;
  int   id;
  int   sz;
};

static
_IO_ssize_t reader(struct _IO_FILE *cookie, void *buffer, 
		   _IO_ssize_t size)
{
  struct memstream *p = (struct memstream *)cookie;
  _IO_ssize_t n;

  if( p->id < p->sz ){
    if( (p->sz - p->id) >= size ){
      memcpy(buffer, p->cp+p->id, size);
      p->id += size;
      return size;
    }
    else{
      n = p->sz - p->id;
      memcpy(buffer, p->cp+p->id, n);
      p->id = p->sz;
      return n;
    }
  }
  else
    return 0;
}

static
_IO_ssize_t writer(struct _IO_FILE *cookie, const void *buffer, 
		   _IO_ssize_t size)
{
  return -1;
}

static
int seeker(struct _IO_FILE *cookie, _IO_off_t position, int whence)
{
  return -1;
}

static
int cleaner(struct _IO_FILE *cookie)
{
  struct memstream *p = (struct memstream *)cookie;

  ksm_dispose(p->cp);
  ksm_dispose(p);
  return 0;
}

static
_IO_cookie_io_functions_t cookie_funs = { 
  (void *)reader, 
  (void *)writer, 
  (void *)seeker, 
  (void *)cleaner };

FILE *ksm_string_stream(char *s, char *opentype)
{
  struct memstream *ms;

  ms = ksm_alloc(sizeof(*ms));
  ms->sz = strlen(s);
  ms->cp = ksm_alloc(ms->sz+1);
  strcpy(ms->cp, s);
  ms->id = 0;
  return fopencookie(ms, opentype, cookie_funs);
}

