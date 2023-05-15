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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define LINE_SIZE    1024
static char dir[LINE_SIZE];
static char *ksmprog = KSMPROG;
extern char **environ;

static
char *dir_stop(char *s)
{
  char *p;

  if( (p = strrchr(s, '/')) )
    while( --p > s )
      if( *p == '/' )
	return p;
  return 0;
}

int main(int argc, char **argv)
{
  char *s;

  if( (s = getenv("KSM_PROG")) )
    ksmprog = s;
  if( !(s = dir_stop(ksmprog)) ){
    fprintf(stderr, "can't find KSM home directory from pathname: %s\n",
	    ksmprog);
    exit(1);
  }
  
  if( snprintf(dir, LINE_SIZE, "%s=%.*s", "KSM_DIR", 
	       s - ksmprog, ksmprog) < 0 ){
    fprintf(stderr, "pathname too long: %s\n", ksmprog);
    exit(1);
  }
  if( !(putenv(dir) == 0) ){
    fprintf(stderr, "can't putenv: %s\n", dir);
    exit(1);
  }
  argv[0] = s+1;
  execve(ksmprog, argv, environ);
  perror(ksmprog);
  exit(1);
}
