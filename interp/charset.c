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

#include <iconv.h>
#include <stdio.h>
#include <errno.h>
#include "ksm.h"

typedef struct {
  ksm_t port;
  iconv_t cd;
  char buf[4];
  char *ptr;
  char *end;
} icvt_info;

static 
void mark_icvt(void *aux)
{
  icvt_info *info = aux;

  ksm_mark_object(info->port);
}

static
void dispose_icvt(void *aux)
{
  icvt_info *info = aux;

  ksm_close_port(info->port);
  iconv_close(info->cd);
  ksm_dispose(aux);
}

#define INPUT_BUFSIZE 100

static
int input_fill(icvt_info *info)
     // returns 0 if OK, -1 if end of file
{
  char ibuf[INPUT_BUFSIZE], *ip, *op;
  int nbuf = 0, ch, n, isize, osize;

  if( ksm_port_eof_p(info->port) )
    return EOF;
  ch = ksm_port_getc(info->port);
  if( ch == EOF )
    return EOF;
  ibuf[nbuf++] = ch;

  while( 1 ){
    ip = ibuf;
    isize = nbuf;
    op = info->buf;
    osize = 4;
    n = iconv(info->cd, (void *)&ip, &isize, &op, &osize);
    if( n != -1 ){
      info->ptr = info->buf;
      info->end = info->buf + 4 - osize;
      break;
    }
    else{
      if( errno != EINVAL ){
	perror("iconv");
	ksm_error("error while converting charset");
      }
      ch = ksm_port_getc(info->port);
      if( ch == EOF )
	ksm_error("unexpected EOF while converting charset");
      if( nbuf >= INPUT_BUFSIZE )
	ksm_error("buffer overflow while converting charset");
      ibuf[nbuf++] = ch;
    }
  }
  return 0;
}

static
int input_getc(void *aux)
{
  icvt_info *info = aux;

  if( info->ptr == info->end )
    if( input_fill(info) == EOF )
      return EOF;
  if( info->ptr == info->end )
    ksm_error("can't happen (input_getc)");
  return *info->ptr++;
}

static
int input_ungetc(char ch, void *aux)
{
  icvt_info *info = aux;

  if( info->ptr == info->buf )
    return -1;
  *--info->ptr = ch;
  return ch;
}

static int input_putc(char ch, void *aux){ return EOF; }
static int input_flush(void *aux){ return EOF; }

static
int input_eof(void *aux)
{
  icvt_info *info = aux;

  if( info->ptr != info->end )
    return 0;
  return ksm_port_eof_p(info->port);
}

static
int input_char_ready(void *aux)
{
  icvt_info *info = aux;

  if( info->ptr != info->end )
    return 1;
  return ksm_port_char_ready_p(info->port);
}

static int decoding_port_kind;

ksm_t ksm_make_input_charset_decoding_port(char *code, ksm_t src_port)
{
  ksm_port *ptr;
  icvt_info *info;

  ptr = ksm_alloc(sizeof(*ptr));
  ptr->getc = input_getc;
  ptr->ungetc = input_ungetc;
  ptr->putc = input_putc;
  ptr->flush = input_flush;
  ptr->eof = input_eof;
  ptr->char_ready = input_char_ready;
  ptr->mark_aux = mark_icvt;
  ptr->dispose_aux = dispose_icvt;
  info = ksm_alloc(sizeof(*info));
  info->port = src_port;
  info->cd = iconv_open("UTF-8", code);
  if( info->cd == (iconv_t)-1 ){
    ksm_dispose(ptr);
    ksm_error("can't decode charset: %s", code);
  }
  info->end = info->buf;
  info->ptr = info->buf;
  ptr->aux = info;
  return ksm_make_input_port(ptr, decoding_port_kind);
}

typedef struct {
  ksm_t port;
  iconv_t cd;
  char buf[3];
  int nbuf;
} ocvt_info;

static 
void mark_ocvt(void *aux)
{
  ocvt_info *info = aux;

  ksm_mark_object(info->port);
}

static
void dispose_ocvt(void *aux)
{
  ocvt_info *info = aux;

  ksm_close_port(info->port);
  iconv_close(info->cd);
  ksm_dispose(aux);
}

static int output_getc(void *aux){ return EOF; }
static int output_ungetc(char ch, void *aux){ return EOF; }

#define OUTPUT_BUFSIZE 100

static
int output_putc(char ch, void *aux)
{
  ocvt_info *info = aux;
  char *ip, *op, *cp, obuf[OUTPUT_BUFSIZE];
  int rc, isize, osize;

  if( info->nbuf >= 3 )
    ksm_error("buffer overflow (output_putc)");
  info->buf[info->nbuf++] = ch;
  ip = info->buf;
  isize = info->nbuf;
  op = obuf;
  osize = OUTPUT_BUFSIZE;
  rc = iconv(info->cd, (void *)&ip, &isize, &op, &osize);
  if( rc != -1 ){
    cp = obuf;
    while( cp < op )
      ksm_port_putc(*cp++, info->port);
    info->nbuf = 0;
  }
  else{
    if( errno != EINVAL ){
      perror("iconv");
      ksm_error("can't convert charset");
    }
  }
  return ch;
}

static
int output_flush(void *aux)
{
  ocvt_info *info = aux;

  return ksm_port_flush(info->port);
}

static int output_eof(void *aux){ return 1; }
static int output_char_ready(void *aux){ return 0; }

static int encoding_port_kind;

ksm_t ksm_make_output_charset_encoding_port(char *code, ksm_t src_port)
{
  ksm_port *ptr;
  ocvt_info *info;

  ptr = ksm_alloc(sizeof(*ptr));
  ptr->getc = output_getc;
  ptr->ungetc = output_ungetc;
  ptr->putc = output_putc;
  ptr->flush = output_flush;
  ptr->eof = output_eof;
  ptr->char_ready = output_char_ready;
  ptr->mark_aux = mark_ocvt;
  ptr->dispose_aux = dispose_ocvt;
  info = ksm_alloc(sizeof(*info));
  info->port = src_port;
  info->cd = iconv_open(code, "UTF-8");
  if( info->cd == (iconv_t)-1 ){
    ksm_dispose(ptr);
    ksm_error("can't decode charset: %s", code);
  }
  info->nbuf = 0;
  ptr->aux = info;
  return ksm_make_output_port(ptr, encoding_port_kind);
}

void ksm_init_charset(void)
{
  decoding_port_kind = ksm_new_port_kind();
  encoding_port_kind = ksm_new_port_kind();
}

