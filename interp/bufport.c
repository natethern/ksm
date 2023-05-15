#include <string.h>
#include "ksm.h"
#include "ksmext.h"

static int bufport_kind;

typedef struct bufport_aux_ {
  ksm_t src_port;
  char *buf_start;
  char *buf_end;
  char *cp;
  ksm_t filename;
  int lino;
} bufport_aux;

static void mark_bufport(void *aux)
{
  bufport_aux *p = aux;

  ksm_mark_object(p->src_port);
  ksm_mark_object(p->filename);
}

static void dispose_bufport(void *aux)
{
  bufport_aux *p = aux;

  if( p->buf_start )
    ksm_dispose(p->buf_start);
  ksm_dispose(p);
}

static void extend_head(bufport_aux *p, int n)
{
  int m, o;
  char *s;

  m = p->buf_end - p->buf_start;
  o = p->cp - p->buf_start;
  s = ksm_alloc(n+m);
  if( p->buf_start ){
    memcpy(s+n,p->buf_start,m);
    ksm_dispose(p->buf_start);
  }
  p->buf_start = s;
  p->buf_end = s+n+m;
  p->cp = s+n+o;
}

static int bufport_getc(void *aux)
{
  bufport_aux *p = aux;
  int ch;

  if( p->cp < p->buf_end )
    ch = *p->cp++;
  else
    ch = ksm_port_getc(p->src_port);
  if( (p->filename != ksm_unspec_object) && (ch == '\n') )
    p->lino++;
  return ch;
}

static int bufport_ungetc(char ch, void *aux)
{
  bufport_aux *p = aux;

  if( (p->filename != ksm_unspec_object) && (ch == '\n') )
    p->lino--;
  if( p->cp > p->buf_start ){
    *--p->cp = ch;
    return ch;
  }
  return ksm_port_ungetc(ch, p->src_port);
}

static int bufport_putc(char ch, void *aux)
{
  return EOF;
}

static int bufport_flush(void *aux)
{
  return EOF;
}

static int bufport_eof(void *aux)
{
  bufport_aux *p = aux;

  return (p->cp == p->buf_end) && ksm_port_eof_p(p->src_port);
}

static int bufport_char_ready(void *aux)
{
  bufport_aux *p = aux;

  return (p->cp < p->buf_end) || ksm_port_char_ready_p(p->src_port);
}

ksm_t ksm_make_bufport_with_filename(ksm_t src_port, ksm_t filename)
{
  ksm_port *port_impl;
  bufport_aux *p;

  port_impl = ksm_alloc(sizeof(ksm_port));
  port_impl->getc = bufport_getc;
  port_impl->ungetc = bufport_ungetc;
  port_impl->putc = bufport_putc;
  port_impl->flush = bufport_flush;
  port_impl->eof = bufport_eof;
  port_impl->char_ready = bufport_char_ready;
  port_impl->mark_aux = mark_bufport;
  port_impl->dispose_aux = dispose_bufport;
  port_impl->aux = p = ksm_alloc(sizeof(bufport_aux));
  p->src_port = src_port;
  p->buf_start = 0;
  p->buf_end = 0;
  p->cp = 0;
  p->filename = filename;
  p->lino = 1;
  return ksm_make_input_port(port_impl, bufport_kind);
}

ksm_t ksm_make_bufport(ksm_t src_port)
{
  return ksm_make_bufport_with_filename(src_port, ksm_unspec_object);
}

int ksm_is_bufport(ksm_t x)
{
  return ksm_is_port(x) && (ksm_port_kind(x) == bufport_kind);
}

void ksm_bufport_ungets(ksm_t bufport, char *s)
{
  int n, avail;
  ksm_port *pp;
  bufport_aux *p;

  pp = ksm_port_ptr(bufport);
  p = pp->aux;
  n = strlen(s);
  avail = p->buf_start - p->cp;
  if( avail < n )
    extend_head(p, n - avail);
  p->cp -= n;
  memcpy(p->cp, s, n);
}

int ksm_bufport_has_filename(ksm_t bufport)
{
  bufport_aux *p;

  p = ksm_port_ptr(bufport)->aux;
  return p->filename != ksm_unspec_object;
}

ksm_t ksm_bufport_filename(ksm_t bufport)
{
  bufport_aux *p;

  p = ksm_port_ptr(bufport)->aux;
  return p->filename;
}

int ksm_bufport_line_number(ksm_t bufport)
{
  bufport_aux *p;

  p = ksm_port_ptr(bufport)->aux;
  return p->lino;
}

void ksm_init_bufport(void)
{
  bufport_kind = ksm_new_port_kind();
}

