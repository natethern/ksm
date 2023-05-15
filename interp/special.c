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

#define MAKE_IMM(str)   ksm_itype(KSM_FLAG_IMM, (int)str, 0, 0)
#define IMM_STR(x)      ((char *)ksm_ival(x,0))

static
int fprintf_imm(FILE *fp, ksm_t x, int alt)
{
  return fprintf(fp, "%s", IMM_STR(x));
}

ksm_t ksm_nil_object;     /*** GLOBAL ***/
ksm_t ksm_eof_object;     /*** GLOBAL ***/
ksm_t ksm_true_object;    /*** GLOBAL ***/
ksm_t ksm_false_object;   /*** GLOBAL ***/
ksm_t ksm_unspec_object;  /*** GLOBAL ***/
ksm_t ksm_comment_object; /*** GLOBAL ***/
ksm_t ksm_rpar_object;    /*** GLOBAL ***/

ksm_t ksm_make_imm(char *str)
{
  return MAKE_IMM(str);
}

void ksm_init_special(void)
{
  ksm_install_fprintf(KSM_FLAG_IMM, fprintf_imm);

  ksm_nil_object = MAKE_IMM("()");
  ksm_eof_object = MAKE_IMM("#<eof>");
  ksm_true_object = MAKE_IMM("#t");
  ksm_false_object = MAKE_IMM("#f");
  ksm_unspec_object = MAKE_IMM("#<unspecified>");
  ksm_comment_object = MAKE_IMM("#<comment>");
  ksm_rpar_object = MAKE_IMM("#<rpar>");
}


