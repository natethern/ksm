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
#include <string.h>
#include "ksm.h"

// flag in ksm_port
#define PORT_INPUT       1
#define PORT_OUTPUT      2

// predefined port kind
enum { STREAM_INPUT, STREAM_OUTPUT, STRING_INPUT,
       LAST_PORT_KIND };
static int next_port_kind = LAST_PORT_KIND;

int ksm_new_port_kind(void)
{
  return next_port_kind++;
}

#define MAKE_PORT(ptr,flag,kind) \
  ksm_itype(KSM_FLAG_PORT,(int)(ptr),flag,kind)
#define IS_PORT(x)            (ksm_flag(x)==KSM_FLAG_PORT)
#define PORT_PTR(port)        ((ksm_port *)ksm_ival(port,0))
#define PORT_FLAG(port)       ksm_ival(port,1)
#define PORT_KIND(port)       ksm_ival(port,2)

#define PORT_PTR_NO_LOCK(port)  ((ksm_port *)ksm_ival_no_lock(port,0))

static
ksm_t mark_port(ksm_t port)
{
  ksm_port *ptr = PORT_PTR_NO_LOCK(port);

  if( ptr->mark_aux && ptr->aux )
    (*ptr->mark_aux)(ptr->aux);
  return 0;
}

static
void dispose_port(ksm_t port)
{
  ksm_port *ptr = PORT_PTR_NO_LOCK(port);

  if( ptr->aux )
    (*ptr->dispose_aux)(ptr->aux);
  ksm_dispose(ptr);
}

ksm_t ksm_make_input_port(ksm_port *ptr, int kind)
{
  return MAKE_PORT(ptr, PORT_INPUT, kind);
}

ksm_t ksm_make_output_port(ksm_port *ptr, int kind)
{
  return MAKE_PORT(ptr, PORT_OUTPUT, kind);
}

int ksm_is_input_port(ksm_t x)
{ 
  return IS_PORT(x) && (PORT_FLAG(x) & PORT_INPUT);
}

int ksm_is_output_port(ksm_t x)
{ 
  return IS_PORT(x) && (PORT_FLAG(x) & PORT_OUTPUT);
}

int ksm_is_port(ksm_t x)
{
  return IS_PORT(x);
}

int ksm_port_kind(ksm_t port)
{
  return PORT_KIND(port);
}

void ksm_close_port(ksm_t port)
{
  ksm_port *ptr;

  ptr = PORT_PTR(port);
  if( ptr->aux ){
    (*ptr->dispose_aux)(ptr->aux);
    ptr->aux = 0;
  }
}

ksm_port *ksm_port_ptr(ksm_t port)
{ 
  ksm_port *ptr;

  ptr = PORT_PTR(port);
  if( ptr->aux == 0 )
    ksm_error("port is already closed: %k", port);
  return ptr;
}

int ksm_port_getc(ksm_t port)
{
  ksm_port *ptr;

  ptr = ksm_port_ptr(port);
  return (*ptr->getc)(ptr->aux);
}

int ksm_port_ungetc(int ch, ksm_t port)
{
  ksm_port *ptr;

  ptr = ksm_port_ptr(port);
  return (*ptr->ungetc)(ch, ptr->aux);
}

int ksm_port_eof_p(ksm_t port)
{
  ksm_port *ptr;

  ptr = ksm_port_ptr(port);
  return (*ptr->eof)(ptr->aux);
}

int ksm_port_char_ready_p(ksm_t port)
{
  ksm_port *ptr;

  ptr = ksm_port_ptr(port);
  return (*ptr->char_ready)(ptr->aux);
}

int ksm_port_putc(char ch, ksm_t port)
{
  ksm_port *ptr;

  ptr = ksm_port_ptr(port);
  return (*ptr->putc)(ch, ptr->aux);
}

int ksm_port_puts(char *s, ksm_t port)
{
  ksm_port *ptr;
  int n;

  ptr = ksm_port_ptr(port);
  n = strlen(s);
  while( n-- > 0 )
   if( (*ptr->putc)(*s++, ptr->aux) == EOF )
     return EOF;
  return 1;
}

int ksm_port_flush(ksm_t port)
{
  ksm_port *ptr;

  ptr = PORT_PTR(port);
  return (*ptr->flush)(ptr->aux);
}

/*** utility ********************************************************/

static int invalid_getc(void *aux) { return EOF; }
static int invalid_ungetc(char ch, void *aux) { return EOF; }
static int invalid_putc(char ch, void *aux) { return EOF; }
static int invalid_flush(void *aux) { return EOF; }
static int invalid_eof(void *aux) { return EOF; }
static int invalid_char_ready(void *aux) { return 0; }

/*** stream port ****************************************************/

static
int input_getc(void *aux)
{
  FILE *fp = aux;

  return getc(fp);
}

static
int input_ungetc(char ch, void *aux)
{
  FILE *fp = aux;

  return ungetc(ch, fp);
}

static int input_eof(void *aux)
{
  FILE *fp = aux;

  return feof(fp);
}

static int input_char_ready(void *aux)
{
  FILE *fp = aux;

  if( fp->_IO_read_ptr < fp->_IO_read_end )
    return 1;
  return ksm_char_ready_p(fileno(fp));
}

static void dispose_fp(void *aux)
{
  FILE *fp = aux;

  fclose(fp);
}

ksm_t ksm_make_input_file_port(FILE *fp)
{
  ksm_port *ptr;

  ptr = ksm_alloc(sizeof(ksm_port));
  ptr->getc = input_getc;
  ptr->ungetc = input_ungetc;
  ptr->putc = invalid_putc;
  ptr->flush = invalid_flush;
  ptr->eof = input_eof;
  ptr->char_ready = input_char_ready;
  ptr->mark_aux = 0;
  ptr->dispose_aux = dispose_fp;
  ptr->aux = fp;
  return MAKE_PORT(ptr, PORT_INPUT, STREAM_INPUT);
}

static int output_putc(char ch, void *aux)
{
  FILE *fp = aux;

  return putc(ch, fp);
}

static int output_flush(void *aux)
{
  FILE *fp = aux;

  return fflush(fp);
}

ksm_t ksm_make_output_file_port(FILE *fp)
{
  ksm_port *ptr;

  ptr = ksm_alloc(sizeof(ksm_port));
  ptr->getc = invalid_getc;
  ptr->ungetc = invalid_ungetc;
  ptr->putc = output_putc;
  ptr->flush = output_flush;
  ptr->eof = invalid_eof;
  ptr->char_ready = invalid_char_ready;
  ptr->mark_aux = 0;
  ptr->dispose_aux = dispose_fp;
  ptr->aux = fp;
  return MAKE_PORT(ptr, PORT_OUTPUT, STREAM_OUTPUT);
}

/*** string port ****************************************************/

typedef struct {
  char *start;
  char *end;
  char *ptr;
} string_info;

static 
int str_input_getc(void *aux)
{
  string_info *info = aux;

  if( info->ptr < info->end )
    return *info->ptr++;
  else
    return EOF;
}

static
int str_input_ungetc(char ch, void *aux)
{
  string_info *info = aux;

  if( info->ptr <= info->start )
    return EOF;
  *--info->ptr = ch;
  return ch;
}

static
int str_input_eof(void *aux)
{
  string_info *info = aux;

  return info->ptr >= info->end;
}

static
int str_input_char_ready(void *aux)
{
  string_info *info = aux;

  return info->ptr < info->end;
}

static
void dispose_string_info(void *aux)
{
  string_info *info = aux;

  ksm_dispose(info->start);
  ksm_dispose(info);
}

ksm_t ksm_make_string_input(char *str, int copy, int len)
     // if "copy" is true, "str" is copied
     // if "len" is negative, "len" is calculated by strlen(str)
{
  int n;
  string_info *info;
  ksm_port *p;

  info = ksm_alloc(sizeof(*info));
  n = (len < 0) ? strlen(str) : len;
  if( copy ){
    info->start = ksm_alloc(n+1);
    strcpy(info->start, str);
  }
  else
    info->start = str;
  info->end = info->start + n;
  info->ptr = info->start;
  p = ksm_alloc(sizeof(*p));
  p->getc = str_input_getc;
  p->ungetc = str_input_ungetc;
  p->putc = invalid_putc;
  p->flush = invalid_flush;
  p->eof = str_input_eof;
  p->char_ready = str_input_char_ready;
  p->mark_aux = 0;
  p->dispose_aux = dispose_string_info;
  p->aux = info;
  return ksm_make_input_port(p, STRING_INPUT);
}

/*** init ***********************************************************/

void ksm_init_port(void)
{
  ksm_install_marker(KSM_FLAG_PORT, mark_port);
  ksm_install_disposer(KSM_FLAG_PORT, dispose_port);
}
