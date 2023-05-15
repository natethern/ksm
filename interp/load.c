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

#define PRIVATE_GLOBAL  (ksm_get_ctx()->private_global)

void ksm_load(ksm_t port)
{
  ksm_t expr, private_save;
  ksm_t *frame;

  if( !ksm_is_bufport(port) )
    port = ksm_make_bufport(port);
  
  private_save = PRIVATE_GLOBAL;
  PRIVATE_GLOBAL = ksm_make_global(100, ksm_interact_env);
  frame = ksm_registry_top();
  while( 1 ){
    expr = ksm_read_toplevel(port);
    if( ksm_is_comment(expr) )
      continue;
    if( ksm_is_eof(expr) )
      break;
    ksm_eval(expr, PRIVATE_GLOBAL);
    ksm_registry_restore(frame);
  }
  PRIVATE_GLOBAL = private_save;
}

ksm_t ksm_init_private_global(void)
{
  if( PRIVATE_GLOBAL )
    ksm_error("private_global already used");
  PRIVATE_GLOBAL = ksm_make_global(100, ksm_interact_env);
  return PRIVATE_GLOBAL;
}

#define KSM_LOAD_FILE_NAME  (ksm_get_ctx()->load_file_name)

KSM_DEF_BLT(load)
     // (load filename [encoding])
{
  ksm_t fname, enc;
  char *fn, *save, *encode;
  ksm_t *frame, port;

  frame = ksm_registry_top();
  KSM_POP_ARG(fname);
  if( KSM_NO_MORE_ARGS() )
    encode = KSM_DEFAULT_ENCODING;
  else{
    KSM_SET_ARGS1(enc);
    if( ksm_is_false(enc) )
      encode = KSM_NO_ENCODING;
    if( ksm_is_true(enc) )
      encode = KSM_DEFAULT_ENCODING;
    encode = ksm_string_value(enc);
  }
  fn = ksm_string_value(fname);
  port = ksm_open_input_file(fn, encode);
  port = ksm_make_bufport_with_filename(port, fname);
  save = KSM_LOAD_FILE_NAME;
  KSM_LOAD_FILE_NAME = fn;
  ksm_load(port);
  ksm_close_port(port);
  KSM_LOAD_FILE_NAME = save;
  return ksm_unspec_object;
}

void ksm_init_load(void)
{
  KSM_ENTER_BLT("load", load);
}
