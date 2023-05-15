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

#ifndef KSMEXT_H
#define KSMEXT_H

#include "ksm.h"
#include <signal.h>

// getopt.c
void ksm_init_getopt(void);

// hashtab.c
void ksm_init_hashtab(void);
ksm_t ksm_make_hashtab(int bucket_size);
int ksm_is_hashtab(ksm_t x);
int ksm_hashtab_enter(ksm_t key, ksm_t val, ksm_t tab, int create);
ksm_t ksm_hashtab_lookup(ksm_t key, ksm_t tab);

// lambdax.c
void ksm_init_lambdax(void);

// logic.c
void ksm_init_logic(void);

// queue.c
void ksm_init_queue(void);
ksm_t ksm_make_queue(void);
int ksm_is_queue(ksm_t x);
#define ksm_queue_is_empty(q)  ksm_is_nil(ksm_queue_content(q))
void ksm_enqueue(ksm_t queue, ksm_t item);
ksm_t ksm_dequeue(ksm_t queue);
ksm_t ksm_queue_content(ksm_t queue);

// readline.c
void ksm_init_readline(void);

// memstream.c
FILE *ksm_string_stream(char *s, char *opentype);

// sched.c
void ksm_init_sched_unit(void);

// signal.c
void ksm_init_signal_unit(void);
ksm_t ksm_make_sigset(void);
int ksm_is_sigset(ksm_t x);
sigset_t *ksm_sigset_ref(ksm_t x);

#endif
