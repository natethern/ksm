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

static ksm_t key_quote;  /*** GLOBAL ***/

static
ksm_t read_quote(ksm_t port)
{
  ksm_t x;

  x = ksm_read_object(port);
  return ksm_cons(key_quote, ksm_cons(x, ksm_nil_object));
}

KSM_DEF_SPC(quote)
     // (quote arg)
{
  ksm_t arg;

  KSM_SET_ARGS1(arg);
  return arg;
}

void ksm_init_quote(void)
{
  key_quote = ksm_make_key("quote");
  ksm_quote_reader = read_quote;
  KSM_ENTER_SPC("quote", quote);
}

