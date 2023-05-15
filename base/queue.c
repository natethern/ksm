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

static int queue_flag;

#define MAKE_QUEUE() \
  ksm_itype(queue_flag,(int)ksm_nil_object,(int)ksm_nil_object,0)
#define IS_QUEUE(x)    (ksm_flag(x)==queue_flag)
#define QUEUE_HEAD(q)  ((ksm_t)ksm_ival(q,0))
#define QUEUE_TAIL(q)  ((ksm_t)ksm_ival(q,1))
#define SET_HEAD(q,x)  ksm_set_ival(q,0,(int)(x))
#define SET_TAIL(q,x)  ksm_set_ival(q,1,(int)(x))

#define QUEUE_HEAD_NO_LOCK(q)  ((ksm_t)ksm_ival_no_lock(q,0))

static
ksm_t mark_queue(ksm_t q)
{
  ksm_mark_object(QUEUE_HEAD_NO_LOCK(q));
  return 0;
}

ksm_t ksm_make_queue(void)
{
  return MAKE_QUEUE();
}

int ksm_is_queue(ksm_t x)
{
  return IS_QUEUE(x);
}

void ksm_enqueue(ksm_t queue, ksm_t item)
{
  ksm_t x;

  x = ksm_cons(item, ksm_nil_object);
  if( ksm_is_nil(QUEUE_TAIL(queue)) )
    SET_HEAD(queue, x);
  else
    ksm_set_cdr(QUEUE_TAIL(queue), x);
  SET_TAIL(queue, x);
}

ksm_t ksm_dequeue(ksm_t queue)
{
  ksm_t x, y;

  if( ksm_is_nil(QUEUE_HEAD(queue)) )
    ksm_error("can't dequeue from empty queue");
  x = QUEUE_HEAD(queue);
  y = ksm_cdr(x);
  SET_HEAD(queue, y);
  if( ksm_is_nil(y) )
    SET_TAIL(queue, y);
  return ksm_car(x);
}

ksm_t ksm_queue_content(ksm_t queue)
{
  return QUEUE_HEAD(queue);
}

KSM_DEF_BLT(make_queue)
     // (make-queue)
{
  KSM_SET_ARGS0();
  return MAKE_QUEUE();
}

KSM_DEF_BLT(queue_p)
     // (queue? x)
{
  ksm_t x;

  KSM_SET_ARGS1(x);
  return KSM_BOOL2OBJ(ksm_is_queue(x));
}

KSM_DEF_BLT(queue_empty_p)
     // (queue-empty? queue)
{
  ksm_t q;

  KSM_SET_ARGS1(q);
  if( !IS_QUEUE(q) )
    ksm_error("queue expected: %k", q);
  return KSM_BOOL2OBJ(ksm_is_nil(QUEUE_HEAD(q)));
}

KSM_DEF_BLT(enqueue)
     // (enqueue queue item)
{
  ksm_t q, x;

  KSM_SET_ARGS2(q, x);
  if( !IS_QUEUE(q) )
    ksm_error("queue expected: %k", q);
  ksm_enqueue(q, x);
  return ksm_unspec_object;
}

KSM_DEF_BLT(dequeue)
     // (dequeue queue)
{
  ksm_t q;

  KSM_SET_ARGS1(q);
  if( !IS_QUEUE(q) )
    ksm_error("queue expected (DEQUEUE!): %k", q);
  return ksm_dequeue(q);
}

KSM_DEF_BLT(queue_content)
     // (queue-content queue)
{
  ksm_t q;

  KSM_SET_ARGS1(q);
  if( !IS_QUEUE(q) )
    ksm_error("queue expected: %k", q);
  return ksm_queue_content(q);
}

KSM_DEF_BLT(queue_head)
     // (queue-head queue)
{
  ksm_t q;

  KSM_SET_ARGS1(q);
  if( !IS_QUEUE(q) )
    ksm_error("queue expected: %k", q);
  if( !ksm_is_cons(QUEUE_HEAD(q)) )
    ksm_error("can't find head of empty queue");
  return ksm_car(QUEUE_HEAD(q));
}

KSM_DEF_BLT(queue_tail)
     // (queue-tail queue)
{
  ksm_t q;

  KSM_SET_ARGS1(q);
  if( !IS_QUEUE(q) )
    ksm_error("queue expected: %k", q);
  if( !ksm_is_cons(QUEUE_TAIL(q)) )
    ksm_error("can't find head of empty queue");
  return ksm_car(QUEUE_TAIL(q));
}

void ksm_init_queue(void)
{
  queue_flag = ksm_new_flag();
  ksm_install_marker(queue_flag, mark_queue);
  KSM_ENTER_BLT("make-queue", make_queue);
  KSM_ENTER_BLT("queue?", queue_p);
  KSM_ENTER_BLT("queue-empty?", queue_empty_p);
  KSM_ENTER_BLT("enqueue!", enqueue);
  KSM_ENTER_BLT("dequeue!", dequeue);
  KSM_ENTER_BLT("queue-content", queue_content);
  KSM_ENTER_BLT("queue-head", queue_head);
  KSM_ENTER_BLT("queue-tail", queue_tail);
}
