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

#define MAX_PERM  30
static ksm_t *perms[MAX_PERM];   /*** GLOBAL ***/
static int n_perms;              /*** GLOBAL ***/

void ksm_permanent(ksm_t *p)
{
  if( n_perms >= MAX_PERM )
    ksm_fatal("too many permanents (ksm_permanent)");
  perms[n_perms++] = p;
}

static
void mark_perm(void)
{
  int i;


  for(i=0;i<n_perms;++i)
    ksm_mark_object(*perms[i]);
}

void ksm_init_permanent(void)
{
  ksm_add_gc_prolog(mark_perm);
}
