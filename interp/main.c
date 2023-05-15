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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int *ksm_stack_base;   /*** GLOBAL ***/

char *ksm_dir;         /*** GLOBAL ***/

static
void setup_dir(void)
{
  ksm_dir = getenv("KSM_DIR");
}

static char *ksm_init_file = "init.scm";  /*** GLOBAL ***/

static
void load(char *fname, int raise_error)
{
  FILE *fp;
  ksm_t port;

  fp = fopen(fname, "r");
  if( !fp ){
    if( raise_error ){
      perror(fname);
      exit(1);
    }
    return;
  }
  port = ksm_make_input_file_port(fp);
  port = ksm_make_bufport_with_filename(port, ksm_make_string(fname));
  ksm_load(port);
  ksm_close_port(port);
}

static
void load_init(void)
{
  char buf[256];

  if( ksm_dir ){
    if( snprintf(buf, 256, "%s/%s", ksm_dir, ksm_init_file) < 0 )
      ksm_fatal("too long KSMDIR: %s", ksm_dir);
    load(buf, 0);
  }
}

static
int ksm_next_flag = KSM_FLAG_LAST;   /*** GLOBAL ***/

int ksm_new_flag(void)
{
  if( ksm_next_flag >= KSM_FLAG_MAX )
    ksm_fatal("too many object types");
  return ksm_next_flag++;
}

/*** args ***********************************************************/

int ksm_argc;        /*** GLOBAL ***/
char **ksm_argv;     /*** GLOBAL ***/

static
void pull_arg(int i)
{
  int j;

  for(j=i+1;j<ksm_argc;++j)
    ksm_argv[j-1] = ksm_argv[j];
  --ksm_argc;
}

static char *i_opt;  /*** GLOBAL ***/
static int   h_opt;  /*** GLOBAL ***/

static
void help(void)
{
  printf("Usage: ksm [options] [file arg ...]\n");
  printf("Options:\n");
  printf("  -xl file      load file before starting the interpreter\n");
  printf("  -xc encoding  Set input/output encoding\n");
  printf("  -xh           Display this information\n");
}

#define NOARG_OPT(c, var) \
 { \
   if( *++p ){ \
     fprintf(stderr, "ksm: unexpected arg after -%c option (%s)\n", \
              c, p); \
     exit(1); \
   } \
   var = 1; \
 } 

#define ARG_OPT(var, errmsg) \
  { \
    if( *++p ) \
      var = p; \
    else{ \
      pull_arg(i); \
      if( !(i < ksm_argc && ksm_argv[i][0] != '-') ){ \
        fprintf(stderr, "%s\n", errmsg); \
        exit(1); \
      } \
      var = ksm_argv[i]; \
    } \
  }

static
void handle_opt(void)
{
  int i, c = 0;
  char *p;

  i = 1;
  while( i < ksm_argc ){
    p = ksm_argv[i];
    if( !( p[0] == '-' && p[1] == 'x' ) ){
      if( c++ >= 1 )
	return;
      ++p;
      ++i;
      continue;
    }
    ++p;
    switch(*++p){
    case 'l':
      ARG_OPT(i_opt, "ksm: file name required after -i option");
      break;
    case 'c':
      ARG_OPT(ksm_standard_encoding, 
	      "ksm: encoding name required after -c option");
      break;
    case 'h':
      NOARG_OPT('h', h_opt);
      break;
    default:
      return;
    }
    pull_arg(i);
  }
}

static
void init_arg(int argc, char **argv)
{
  ksm_argc = argc;
  ksm_argv = argv;
  handle_opt();
}

int main(int argc, char **argv)
{
  ksm_stack_base = &argc;
  init_arg(argc, argv);

  if( h_opt ){
    help();
    exit(0);
  }
    
  ksm_init();
  setup_dir();
  load_init();

  if( i_opt )
    load(i_opt, 1);

  if( ksm_argc > 1 ){
    pull_arg(0);
    load(ksm_argv[0], 1);
  }
  else
    ksm_toplevel(ksm_init_private_global());

  return 0;
}
