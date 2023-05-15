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

#include <dlfcn.h>
#include "ksm.h"

#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT 0
#endif

//char *ksm_load_shared_file_name;
#define KSM_LOAD_SHARED_FILE_NAME (ksm_get_ctx()->load_shared_file_name)

KSM_DEF_BLT(load_shared)
     // (load-shared lib-name [init-fun])
{
  ksm_t x, y;
  char *s, *t;
  void *lib, (*init)(void);

  KSM_POP_ARG(x);
  s = ksm_string_value(x);
  if( KSM_NO_MORE_ARGS() )
    t = 0;
  else{
    KSM_POP_ARG(y);
    KSM_SET_ARGS0();
    t = ksm_string_value(y);
  }
  KSM_LOAD_SHARED_FILE_NAME = s;
  lib = dlopen(s, RTLD_NOW | RTLD_GLOBAL);
  if( !lib )
    ksm_error("can't open shared library: %s", dlerror());
  if( t ){
    init = dlsym(RTLD_DEFAULT, t);
    if( !init )
      ksm_error("can't find function: %s", t);
    (*init)();
  }
  return ksm_unspec_object;
}

#define KSM_LOAD_FILE_NAME  (ksm_get_ctx()->load_file_name)

KSM_DEF_BLT(load_file_name)
     // (load-file-name)
{
  if( !KSM_LOAD_FILE_NAME )
    ksm_error("no file loaded (LOAD-FILE-NAME)");
  return ksm_make_string(KSM_LOAD_FILE_NAME);
}

KSM_DEF_BLT(ksm_directory)
     // (ksm-directory)
{
  KSM_SET_ARGS0();
  return ksm_make_string(ksm_dir);
}

KSM_DEF_BLT(error)
     // (error str)
{
  char *s = 0;

  if( KSM_MORE_ARGS() &&
      ksm_string_value_x(ksm_car(KSM_ARGS()), &s) == 0 )
      ksm_error("%s", s);
  else
    ksm_error("");
  return ksm_unspec_object;
}

KSM_DEF_BLT(fatal)
     // (fatal str)
{
  char *s = 0;

  if( KSM_MORE_ARGS() &&
      ksm_string_value_x(ksm_car(KSM_ARGS()), &s) == 0 )
      ksm_fatal("%s", s);
  else
    ksm_fatal("");
  return ksm_unspec_object;
}

/***
KSM_DEF_BLT(standard_error_port)
     // (standard-error-port)
{
  return ksm_make_output_port(stderr);
}
***/

void ksm_extension(void)
{
  ksm_init_path();
  KSM_ENTER_BLT("load-shared", load_shared);
  KSM_ENTER_BLT("load-file-name", load_file_name);
  KSM_ENTER_BLT("ksm-directory", ksm_directory);
  KSM_ENTER_BLT("error", error);
  KSM_ENTER_BLT("fatal", fatal);
  //KSM_ENTER_BLT("standard-error-port", standard_error_port);
}
