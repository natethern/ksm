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

ksm_t ksm_path_append(ksm_t args)
{
  int len = 0, n = 0;
  char *s, *p, *q, *sep = "/";
  ksm_t save = args;

  while(ksm_is_cons(args)){
    s = ksm_string_value(ksm_car(args));
    len += strlen(s);
    ++n;
    args = ksm_cdr(args);
  }
  if( !ksm_is_nil(args) )
    ksm_error("proper list expected: %k", save);
  if( n > 0 )
    len += strlen(sep)*(n-1);
  p = q = ksm_alloc(len+1);
  args = save;
  while(ksm_is_cons(args)){
    s = ksm_string_value(ksm_car(args));
    strcpy(q, s);
    q += strlen(s);
    if( --n > 0 ){
      strcpy(q, sep);
      q += strlen(sep);
    }
    args = ksm_cdr(args);
  }
  *q = '\0';
  return ksm_make_string_no_copy(p);
}

ksm_t ksm_dir_part(char *path)
{
  char *p;

  if( *path == '/' ){
    p = strrchr(path, '/');
    if( p == path )
      return ksm_make_string("/.");
    return ksm_substring(path, 0, p-path);
  }
  return ksm_make_string(".");
}

KSM_DEF_BLT(path_append)
     // (path-append node node ...)
{
  return ksm_path_append(KSM_ARGS());
}

KSM_DEF_BLT(path_directory_part)
     // (path-directory-part path)
{
  ksm_t x;
  char *s;
  
  KSM_POP_ARG(x);
  KSM_SET_ARGS0();
  s = ksm_string_value(x);
  return ksm_dir_part(s);
}

void ksm_init_path(void)
{
  KSM_ENTER_BLT("path-append", path_append);
  KSM_ENTER_BLT("path-directory-part", path_directory_part);
}
