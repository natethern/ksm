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
#include <printf.h>
#include <string.h>
#include "ksm.h"

typedef int (*pfun_t)(FILE *fp, ksm_t x, int alt);
KSM_INSTALLER(fprintf, pfun_t)   /*** GLOBAL ***/
#define GET_PFUN(obj)  get_fprintf(ksm_flag(obj))

int ksm_PA_OBJECT = PA_LAST;     /*** GLOBAL ***/

int ksm_default_fprintf(FILE *fp, ksm_t x, int alt)
{
  return fprintf(fp, "#<%d:%p>", ksm_flag(x), x);
}

int ksm_fprintf(FILE *fp, ksm_t x, int alt)
{
  pfun_t pfun;

  pfun = GET_PFUN(x);
  if( pfun )
    return (*pfun)(fp, x, alt);
  else
    return ksm_default_fprintf(fp, x, alt);
}

static
int print_k(FILE *fp, const struct printf_info *info,
	    const void *const *args)
{
  int len;
  ksm_t x;
  char buffer[256], *p;

  x = *((ksm_t *)args[0]);
  if( info->width == 0 )
    return ksm_fprintf(fp, x, info->alt);
  else {
    if( info->width < 256 )
      p = buffer; 
    else
      p = ksm_alloc(info->width+1);
    len = snprintf(p, info->width, info->alt?"%#k":"%k", x);
    if( len >= info->width )
      len = ksm_fprintf(fp, x, info->alt);
    else
      len = fprintf(fp, "%*s",
		    (info->left ? -info->width : info->width), p);
    if( p != buffer )
      ksm_dispose(p);
    return len;
  }
}

static
int print_k_arginfo(const struct printf_info *info, size_t n,
			   int *argtypes)
{
  if( n > 0 )
    argtypes[0] = ksm_PA_OBJECT|PA_FLAG_PTR;  //PA_POINTER;
  return 1;
}

void ksm_init_print(void)
{
  register_printf_function('k', print_k, print_k_arginfo);
}
