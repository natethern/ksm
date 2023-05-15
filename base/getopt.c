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

#include <getopt.h>
#include "ksm.h"

KSM_DEF_BLT(cmdline_args)
     // (cmdline-args [optstring optfun])
     // optfun: (lambda (ch arg) ...)  { arg = #f if no arg }
     // returns ARGS
{
  ksm_t optstring, optfun, val;
  ksm_t head, tail;
  char *str;
  int ch, i;

  ksm_list_begin(&head, &tail);
  ksm_list_extend(&head, &tail, ksm_make_string(ksm_argv[0]));
  if( KSM_MORE_ARGS() ){
    KSM_SET_ARGS2(optstring, optfun);
    str = ksm_string_value(optstring);
    while( 1 ){
      ch = getopt(ksm_argc, ksm_argv, str);
      if( ch == -1 )
	break;
      if( ch == '?' )
	ksm_error(0);
      val = optarg ? ksm_make_string(optarg) : ksm_false_object;
      ksm_funcall(optfun, ksm_list2(ksm_make_char(ch), val), KSM_ENV());
    }
    i = optind;
  }
  else{
    i = 1;
    goto L1;
  }
 L1:
  for(;i<ksm_argc;++i)
    ksm_list_extend(&head, &tail, ksm_make_string(ksm_argv[i]));
  return head;
}

void ksm_init_getopt(void)
{
  KSM_ENTER_BLT("cmdline-args", cmdline_args);
}
