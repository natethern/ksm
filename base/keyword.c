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

#define MAKE_KEYWORD(key) \
  ksm_itype(KSM_FLAG_KEYWORD, (int)(key), 0, 0)
#define IS_KEYWORD(x)       (ksm_flag(x)==KSM_FLAG_KEYWORD)
#define KEYWORD_KEY(kw)     ((ksm_t)ksm_ival(kw,0))

#define KEYWORD_KEY_NO_LOCK(kw) ((ksm_t)ksm_ival_no_lock(kw,0))

static
ksm_t mark_keyword(ksm_t kw)
{
  ksm_mark_object(KEYWORD_KEY_NO_LOCK(kw));
  return 0;
}

int ksm_is_keyword(ksm_t x)
{
  return IS_KEYWORD(x);
}

ksm_t ksm_keyword_key(ksm_t kw)
{
  return KEYWORD_KEY(kw);
}

/*** read ***********************************************************/

static
ksm_t colon_reader(ksm_t port)
{
  ksm_t k;

  k = ksm_read_object(port);
  if( !ksm_is_key(k) )
    ksm_error("can't read keyword: %k", k);
  return MAKE_KEYWORD(k);
}

/*** print **********************************************************/

static
int fprintf_keyword(FILE *fp, ksm_t kw, int alt)
{
  return fprintf(fp, ":%s", ksm_fetch_key(KEYWORD_KEY(kw)));
}

/*** init ***********************************************************/

void ksm_init_keyword(void)
{
  ksm_install_marker(flag_keyword, mark_keyword);
  ksm_colon_reader = colon_reader;
  ksm_install_fprintf(flag_keyword, fprintf_keyword);
}
