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

void ksm_init(void)
{
  ksm_init_object();
  ksm_init_special();
  ksm_init_print();
  ksm_init_key();  // this should be before ksm_init_permanent()
  ksm_init_permanent();
  ksm_init_cons();
  ksm_init_env();
  ksm_init_blt();
  ksm_init_quote();
  ksm_init_lambda();
  ksm_init_define();
  ksm_init_port();
  ksm_init_jumps();
  ksm_init_cont();
  ksm_init_equal();
  ksm_init_syntax();
  ksm_init_delay();
  ksm_init_sharp();
  ksm_init_vector();
  ksm_init_qq();
  ksm_init_numio();   // this should be before ksm_init_number()
  ksm_init_number();
  ksm_init_string();
  ksm_init_char();
  ksm_init_callwv();
  ksm_init_dywin();
  ksm_init_load();
  ksm_init_charset();
  ksm_blt_cond();
  ksm_blt_cons();
  ksm_blt_math();
  ksm_blt_pair();
  ksm_blt_io();
  ksm_blt_misc();
  ksm_start_interact_env();
  ksm_init_except();
  ksm_extension();
  ksm_init_keyword();
  ksm_init_bufport();
}
