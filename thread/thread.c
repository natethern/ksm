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
#ifdef PTHREAD
#include "ksmext.h"
#include <semaphore.h>
#include <errno.h>

static int thread_flag;
static int mutex_flag;
static int cond_flag;
static int sem_flag;

#define MAKE_THREAD(id,ctx)  \
            ksm_itype(thread_flag, (int)id, (int)ctx, 0)
#define IS_THREAD(x)     (ksm_flag(x)==thread_flag)
#define THREAD_ID(x)     ((pthread_t)ksm_ival(x, 0))
#define THREAD_CTX(x)    ((ksm_ctx *)ksm_ival(x, 1))
#define CONFIRM_THREAD(x) \
            do{ if( !IS_THREAD(x) ) ksm_error("thread expected: %k", x); } \
            while(0)

#define MAKE_MUTEX(p)    ksm_itype(mutex_flag, (int)p, 0, 0)
#define IS_MUTEX(x)      (ksm_flag(x)==mutex_flag)
#define MUTEX_REF(x)     ((pthread_mutex_t *)ksm_ival(x, 0))
#define CONFIRM_MUTEX(x) \
            do{ if( !IS_MUTEX(x) ) ksm_error("mutex expected: %k", x); } \
            while(0)

#define MAKE_COND(p)     ksm_itype(cond_flag, (int)p, 0, 0)
#define IS_COND(x)       (ksm_flag(x)==cond_flag)
#define COND_REF(x)      ((pthread_cond_t *)ksm_ival(x, 0))
#define CONFIRM_COND(x) \
            do{ if( !IS_COND(x) ) \
                  ksm_error("condition variable expected: %k", x); } \
            while(0)

#define MAKE_SEM(p)      ksm_itype(sem_flag, (int)p, 0, 0)
#define IS_SEM(x)        (ksm_flag(x)==sem_flag)
#define SEM_REF(x)       ((sem_t *)ksm_ival(x, 0))
#define CONFIRM_SEM(x) \
            do{ if( !IS_SEM(x) ) \
                  ksm_error("semition variable expected: %k", x); } \
            while(0)

static
void *thread_start(void *arg)
{
  ksm_ctx *ctx = arg;
  ksm_t x;

  pthread_setspecific(ksm_ctx_key, arg);
  ksm_start_ctx(ctx, (int *)&arg);
  x = ksm_funcall(ctx->fun, ksm_nil_object, ksm_nil_object);
  ksm_finish_ctx(ctx, x);
  return 0;
}

KSM_DEF_BLT(create)
     // (thread:create fun &key (detachstate 'joinable)
     //                         (schedpolicy 'other)
     //                         (schedparam 0)
     //                         (inheritsched 'explicit)
     //                         (scope 'system))
     // :detachstate  -- 'joinable | 'detached
     // :schedpolicy  -- 'other | 'rr | 'fifo
     // :schedparam   -- integer
     // :inheritsched -- 'explicit | 'inherit
     // :scope        -- 'system | 'process
{
  ksm_t fun, x;
  ksm_ctx *ctx;
  pthread_t thread_id;
  pthread_attr_t attr;
  struct sched_param schedparam;
  int err, detach = 0;

  KSM_POP_ARG(fun);
  KSM_SET_ARGS0();
  if( pthread_attr_init(&attr) != 0 )
    ksm_error("failed to init thread attribute");
  KSM_POP_KEYARG(x, ksm_make_key("detachstate"), 0);
  if( x == ksm_make_key("detached") ){
    detach = 1;
    if( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0 ){
      pthread_attr_destroy(&attr);
      ksm_error("failed to set detachstate");
    }
  }
  KSM_POP_KEYARG(x, ksm_make_key("schedpolicy"), 0);
  if( x == ksm_make_key("rr") ){
    if( pthread_attr_setschedpolicy(&attr, SCHED_RR) != 0 ){
      pthread_attr_destroy(&attr);
      ksm_error("failed to set schedpolicy");
    }
  }
  else if( x == ksm_make_key("fifo") ){
    if( pthread_attr_setschedpolicy(&attr, SCHED_FIFO) != 0 ){
      pthread_attr_destroy(&attr);
      ksm_error("failed to set schedpolicy");
    }
  }
  KSM_POP_KEYARG(x, ksm_make_key("schedparam"), 0);
  if( x ){
    if( !ksm_is_int(x) ){
      pthread_attr_destroy(&attr);
      ksm_error("integer expected after :schedparam : %k", x);
    }
    schedparam.sched_priority = ksm_fetch_int(x);
    if( pthread_attr_setschedparam(&attr, &schedparam) != 0 ){
      pthread_attr_destroy(&attr);
      ksm_error("failed to set schedparam");
    }
  }
  KSM_POP_KEYARG(x, ksm_make_key("inheritsched"), 0);
  if( x == ksm_make_key("explicit") ){
    if( pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0 ){
      pthread_attr_destroy(&attr);
      ksm_error("failed to set inheritsched");
    }
  }
  else if( x == ksm_make_key("inherit") ){
    if( pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED) != 0 ){
      pthread_attr_destroy(&attr);
      ksm_error("failed to set inheritsched");
    }
  }
  KSM_POP_KEYARG(x, ksm_make_key("scope"), 0);
  if( x == ksm_make_key("system") ){
    if( pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM) != 0 ){
      pthread_attr_destroy(&attr);
      ksm_error("failed to set scope");
    }
  }
  else if( x == ksm_make_key("process") ){
    if( pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS) != 0 ){
      pthread_attr_destroy(&attr);
      ksm_error("failed to set scope");
    }
  }
    
  ctx = ksm_new_ctx(fun);
  ctx->detached = detach;
  err = pthread_create(&thread_id, &attr, thread_start, ctx);
  if( err != 0 ){
    pthread_attr_destroy(&attr);
    ksm_error("failed to create thread");
  }
  return MAKE_THREAD(thread_id, ctx);
}

KSM_DEF_BLT(thread_p)
     // (thread? obj)
{
  ksm_t x;

  KSM_POP_ARG(x);
  KSM_SET_ARGS0();
  return KSM_BOOL2OBJ(IS_THREAD(x));
}

KSM_DEF_BLT(exit)
     // (thread:exit retval)
{
  ksm_t ret;
  ksm_ctx *ctx;

  KSM_POP_ARG(ret);
  KSM_SET_ARGS0();
  ctx = ksm_get_ctx();
  ksm_exit_ctx(ctx, ret);
  pthread_exit(0);
  return ksm_unspec_object;
}

KSM_DEF_BLT(cancel)
     // (thread:cancel thread)
{
  ksm_t thread;
  ksm_ctx *ctx;
  int err;

  KSM_POP_ARG(thread);
  KSM_SET_ARGS0();
  CONFIRM_THREAD(thread);
  ctx = THREAD_CTX(thread);
  ksm_cancel_ctx(ctx);
  if( (err = pthread_cancel(THREAD_ID(thread))) ){
    ksm_error("failed to cancel thread");
  }
  return ksm_unspec_object;
}

KSM_DEF_BLT(join)
     // (thread:join thread)
{
  ksm_t thread;

  KSM_POP_ARG(thread);
  KSM_SET_ARGS0();
  CONFIRM_THREAD(thread);
  return ksm_join_ctx(THREAD_CTX(thread), THREAD_ID(thread));
}

KSM_DEF_BLT(setcancelstate)
     // (thread:setcancelstate state)
     // state: 'enable | 'disable
     // returns current state ('enable or 'disable)
{
  ksm_t state;
  int istate, ostate, err;

  KSM_POP_ARG(state);
  KSM_SET_ARGS0();
  if( state == ksm_make_key("enable") )
    istate = PTHREAD_CANCEL_ENABLE;
  else if( state == ksm_make_key("disable") )
    istate = PTHREAD_CANCEL_DISABLE;
  else
    ksm_error("invalid arg (SETCANCELSTATE): %k", state);
  err = pthread_setcancelstate(istate, &ostate);
  if( err )
    ksm_error("failed to setcancelstate: %d", err);
  if( ostate == PTHREAD_CANCEL_ENABLE )
    return ksm_make_key("enable");
  else if( ostate == PTHREAD_CANCEL_DISABLE )
    return ksm_make_key("disable");
  else{
    ksm_error("unknown cancel state: %d", ostate);
    return ksm_unspec_object;
  }
}

KSM_DEF_BLT(setcanceltype)
     // (thread:setcanceltype type)
     // type: 'deferred | 'asynchronous
     // returns current type ('deferred or 'asynchronous)
{
  ksm_t type;
  int itype, otype, err;

  KSM_POP_ARG(type);
  KSM_SET_ARGS0();
  if( type == ksm_make_key("deferred") )
    itype = PTHREAD_CANCEL_DEFERRED;
  else if( type == ksm_make_key("asynchronous") )
    itype = PTHREAD_CANCEL_ASYNCHRONOUS;
  else
    ksm_error("invalid arg (SETCANCELTYPE): %k", type);
  err = pthread_setcanceltype(itype, &otype);
  if( err )
    ksm_error("failed to setcanceltype: %d", err);
  if( otype == PTHREAD_CANCEL_DEFERRED )
    return ksm_make_key("deferred");
  else if( otype == PTHREAD_CANCEL_ASYNCHRONOUS )
    return ksm_make_key("asynchronous");
  else{
    ksm_error("unknown cancel type: %d", otype);
    return ksm_unspec_object;
  }
}

KSM_DEF_BLT(testcancel)
     // (thread:testcancel)
{
  KSM_SET_ARGS0();
  pthread_testcancel();
  return ksm_unspec_object;
}

static
void cleanup(void *arg)
{
  ksm_t fun = arg;

  ksm_funcall(fun, ksm_nil_object, ksm_nil_object);
}

KSM_DEF_BLT(with_cleanup)
     // (thread:with-cleanup cleanup-fun fun)
     // cleanup-fun: (lambda () ...)
     // fun:         (lambda () ...)
{
  ksm_t c, f, x;

  KSM_POP_ARG(c);
  KSM_POP_ARG(f);
  KSM_SET_ARGS0();
  pthread_cleanup_push(cleanup, c);
  x = ksm_funcall(f, ksm_nil_object, ksm_nil_object);
  pthread_cleanup_pop(0);
  return x;
}

static
void dispose_mutex(ksm_t x)
{
  pthread_mutex_t *p = MUTEX_REF(x);

  pthread_mutex_destroy(p);
  ksm_dispose(p);
}

KSM_DEF_BLT(make_mutex)
     // (thread:make-mutex)
{
  pthread_mutex_t *p;

  KSM_SET_ARGS0();
  p = ksm_alloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(p, 0);
  return MAKE_MUTEX(p);
}

KSM_DEF_BLT(mutex_lock)
     // (thread:mutex-lock mutex)
{
  ksm_t mutex;
  pthread_mutex_t *p;
  int err;

  KSM_POP_ARG(mutex);
  KSM_SET_ARGS0();
  CONFIRM_MUTEX(mutex);
  p = MUTEX_REF(mutex);
  if( (err = pthread_mutex_lock(p)) )
    ksm_error("failed to lock mutex: %d", err);
  return ksm_unspec_object;
}

KSM_DEF_BLT(mutex_trylock)
     // (thread:mutex-trylock mutex)
     // returns #t if successfully locked, #f otherwise
{
  ksm_t mutex;
  pthread_mutex_t *p;
  int err;

  KSM_POP_ARG(mutex);
  KSM_SET_ARGS0();
  CONFIRM_MUTEX(mutex);
  p = MUTEX_REF(mutex);
  err = pthread_mutex_trylock(p);
  return KSM_BOOL2OBJ((err==0));
}

KSM_DEF_BLT(mutex_unlock)
     // (thread:mutex-unlock mutex)
{
  ksm_t mutex;
  pthread_mutex_t *p;
  int err;

  KSM_POP_ARG(mutex);
  KSM_SET_ARGS0();
  CONFIRM_MUTEX(mutex);
  p = MUTEX_REF(mutex);
  if( (err = pthread_mutex_unlock(p)) )
    ksm_error("failed to unlock mutex: %d", err);
  return ksm_unspec_object;
}

static
void dispose_cond(ksm_t x)
{
  pthread_cond_t *p;

  p = COND_REF(x);
  pthread_cond_destroy(p);
  ksm_dispose(p);
}

KSM_DEF_BLT(make_cond)
     // (thread:make-cond)
{
  pthread_cond_t *p;

  KSM_SET_ARGS0();
  p = ksm_alloc(sizeof(*p));
  pthread_cond_init(p, 0);
  return MAKE_COND(p);
}

KSM_DEF_BLT(cond_signal)
     // (thread:cond-signal cond)
{
  ksm_t cond;
  pthread_cond_t *p;

  KSM_POP_ARG(cond);
  KSM_SET_ARGS0();
  CONFIRM_COND(cond);
  p = COND_REF(cond);
  pthread_cond_signal(p);
  return ksm_unspec_object;
}

KSM_DEF_BLT(cond_broadcast)
     // (thread:cond-broadcast cond)
{
  ksm_t cond;
  pthread_cond_t *p;

  KSM_POP_ARG(cond);
  KSM_SET_ARGS0();
  CONFIRM_COND(cond);
  p = COND_REF(cond);
  pthread_cond_broadcast(p);
  return ksm_unspec_object;
}

KSM_DEF_BLT(cond_wait)
     // (thread:cond-wait cond mutex)
{
  ksm_t cond, mutex;

  KSM_POP_ARG(cond);
  KSM_POP_ARG(mutex);
  CONFIRM_COND(cond);
  CONFIRM_MUTEX(mutex);
  pthread_cond_wait(COND_REF(cond), MUTEX_REF(mutex));
  return ksm_unspec_object;
}

KSM_DEF_BLT(cond_timedwait)
     // (thread:cond-timedwait cond mutex sec [nanosec])
{
  ksm_t cond, mutex, sec, nanosec;
  struct timespec timespec;
  int err;

  KSM_POP_ARG(cond);
  KSM_POP_ARG(mutex);
  KSM_POP_ARG(sec);
  KSM_POP_OPTARG(nanosec, ksm_make_int(0));
  CONFIRM_COND(cond);
  CONFIRM_MUTEX(mutex);
  timespec.tv_sec  = ksm_get_si(sec);
  timespec.tv_nsec = ksm_get_si(nanosec);
  err = pthread_cond_timedwait(COND_REF(cond), MUTEX_REF(mutex), 
			       &timespec);
  if( err == ETIMEDOUT || err == EINTR )
    return ksm_false_object;
  if( err )
    ksm_error("pthread_cond_timedwait failed");
  return ksm_true_object;
}

static
void dispose_sem(ksm_t x)
{
  sem_t *p;

  p = SEM_REF(x);
  sem_destroy(p);
  ksm_dispose(p);
}

KSM_DEF_BLT(make_sem)
     // (thread:make-sem value &key (shared #f))
{
  ksm_t val, shared;
  sem_t *p;
  int err;

  KSM_POP_ARG(val);
  KSM_SET_ARGS0();
  KSM_POP_KEYARG(shared, ksm_make_key("shared"), ksm_false_object);
  p = ksm_alloc(sizeof(*p));
  err = sem_init(p, shared == ksm_false_object ? 0 : 1, 
		 ksm_get_si(val));
  if( err ){
    ksm_dispose(p);
    ksm_error("failed to init semaphore: %d", err);
  }
  return MAKE_SEM(p);
}

KSM_DEF_BLT(sem_wait)
     // (thread:sem-wait semaphore)
{
  ksm_t sem;

  KSM_POP_ARG(sem);
  KSM_SET_ARGS0();
  CONFIRM_SEM(sem);
  sem_wait(SEM_REF(sem));
  return ksm_unspec_object;
}

KSM_DEF_BLT(sem_trywait)
     // (thread:sem-trywait semaphore)
{
  ksm_t sem;
  int ret;

  KSM_POP_ARG(sem);
  KSM_SET_ARGS0();
  CONFIRM_SEM(sem);
  ret = sem_trywait(SEM_REF(sem));
  return KSM_BOOL2OBJ((ret==0));
}

KSM_DEF_BLT(sem_post)
     // (thread:sem-post sem)
{
  ksm_t sem;
  int err;

  KSM_POP_ARG(sem);
  KSM_SET_ARGS0();
  CONFIRM_SEM(sem);
  if( (err = sem_post(SEM_REF(sem))) ){
    ksm_error("failed to post semaphore: %d", err);
  }
  return ksm_unspec_object;
}

KSM_DEF_BLT(sem_getvalue)
     // (thread:sem-getvalue sem)
{
  ksm_t sem;
  int val;

  KSM_POP_ARG(sem);
  KSM_SET_ARGS0();
  CONFIRM_SEM(sem);
  sem_getvalue(SEM_REF(sem), &val);
  return ksm_make_int(val);
}

KSM_DEF_BLT(getspecific)
     // (thread:getspecific key)
{
  ksm_t key;

  KSM_POP_ARG(key);
  KSM_SET_ARGS0();
  return ksm_get_specific(key);
}

KSM_DEF_BLT(setspecific)
     // (thread:setspecific key value)
{
  ksm_t key, value;

  KSM_POP_ARG(key);
  KSM_POP_ARG(value);
  KSM_SET_ARGS0();
  ksm_set_specific(key, value);
  return ksm_unspec_object;
}

KSM_DEF_BLT(sigmask)
     // (thread:sigmask how newmask)
{
  ksm_t how, newmask, oldmask;
  int meth, err;
  sigset_t *np, *op;

  KSM_POP_ARG(how);
  KSM_POP_ARG(newmask);
  KSM_SET_ARGS0();
  meth = ksm_get_si(how);
  if( !ksm_is_sigset(newmask) )
    ksm_error("sigset expected: %k", newmask);
  np = ksm_sigset_ref(newmask);
  oldmask = ksm_make_sigset();
  op = ksm_sigset_ref(oldmask);
  if( (err = pthread_sigmask(meth, np, op)) )
    ksm_error("failed to sigmask: %d", err);
  return oldmask;
}

KSM_DEF_BLT(kill)
     // (thread:kill thread signo)
{
  ksm_t arg_thread, arg_signo;
  int signo, err;

  KSM_POP_ARG(arg_thread);
  KSM_POP_ARG(arg_signo);
  KSM_SET_ARGS0();
  CONFIRM_THREAD(arg_thread);
  signo = ksm_get_si(arg_signo);
  if( (err = pthread_kill(THREAD_ID(arg_thread), signo)) )
    ksm_error("failed to pthread_kill: %d", err);
  return ksm_unspec_object;
}

KSM_DEF_BLT(sigwait)
     // (thread:sigwait sigset)
{
  ksm_t arg_sigset;
  int signo;

  KSM_POP_ARG(arg_sigset);
  KSM_SET_ARGS0();
  if( !ksm_is_sigset(arg_sigset) )
    ksm_error("sigset expected: %k", arg_sigset);
  sigwait(ksm_sigset_ref(arg_sigset), &signo);
  return ksm_make_int(signo);
}

KSM_DEF_BLT(self)
     // (thread:self)
{
  KSM_SET_ARGS0();
  return MAKE_THREAD(pthread_self(), ksm_get_ctx());
}

KSM_DEF_BLT(equal)
     // (thread:equal? thread1 thread2)
{
  ksm_t arg1, arg2;

  KSM_POP_ARG(arg1);
  KSM_POP_ARG(arg2);
  CONFIRM_THREAD(arg1);
  CONFIRM_THREAD(arg2);
  return KSM_BOOL2OBJ(pthread_equal(THREAD_ID(arg1), THREAD_ID(arg2)));
}

KSM_DEF_BLT(detach)
     // (thread:detach thread)
{
  ksm_t arg_thread;
  ksm_ctx *ctx;
  int err;

  KSM_POP_ARG(arg_thread);
  KSM_SET_ARGS0();
  CONFIRM_THREAD(arg_thread);
  ctx = THREAD_CTX(arg_thread);
  ctx->detached = 1;
  if( (err = pthread_detach(THREAD_ID(arg_thread))) )
    ksm_error("failed to pthread_detach: %d", err);
  return ksm_unspec_object;
}

#define ATFORK_PREPARE  ksm_make_key("@@atfork_prepare")
#define ATFORK_PARENT   ksm_make_key("@@atfork_parent")
#define ATFORK_CHILD    ksm_make_key("@@atfork_child")

static
int install_prepare(ksm_t f)
{
  ksm_t x;
  int first = 0;

  x = ksm_lookup_global(ATFORK_PREPARE, ksm_interact_env, 0);
  if( x == 0 ){
    x = ksm_nil_object;
    first = 1;
  }
  x = ksm_cons(f, x);
  ksm_enter_global(ATFORK_PREPARE, x, ksm_interact_env, 1);
  return first;
}

static
void install_parent(ksm_t f)
{
  ksm_t head, tail, x;

  x = ksm_lookup_global(ATFORK_PARENT, ksm_interact_env, 0);
  ksm_list_begin(&head, &tail);
  if( x )
    ksm_list_append(&head, &tail, x);
  ksm_list_extend(&head, &tail, f);
  ksm_enter_global(ATFORK_PARENT, head, ksm_interact_env, 1);
}

static
void install_child(ksm_t f)
{
  ksm_t head, tail, x;

  x = ksm_lookup_global(ATFORK_CHILD, ksm_interact_env, 0);
  ksm_list_begin(&head, &tail);
  if( x )
    ksm_list_append(&head, &tail, x);
  ksm_list_extend(&head, &tail, f);
  ksm_enter_global(ATFORK_CHILD, head, ksm_interact_env, 1);
}

static
void atfork_prepare(void)
{
  ksm_t list, proc;

  list = ksm_lookup_global(ATFORK_PREPARE, ksm_interact_env, 0);
  if( !list )
    return;
  while( ksm_is_cons(list) ){
    proc = ksm_car(list);
    list = ksm_cdr(list);
    ksm_funcall(proc, ksm_nil_object, ksm_nil_object);
  }
}

static
void atfork_parent(void)
{
  ksm_t list, proc;

  list = ksm_lookup_global(ATFORK_PARENT, ksm_interact_env, 0);
  if( !list )
    return;
  while( ksm_is_cons(list) ){
    proc = ksm_car(list);
    list = ksm_cdr(list);
    ksm_funcall(proc, ksm_nil_object, ksm_nil_object);
  }
}

static
void atfork_child(void)
{
  ksm_t list, proc;

  list = ksm_lookup_global(ATFORK_CHILD, ksm_interact_env, 0);
  if( !list )
    return;
  while( ksm_is_cons(list) ){
    proc = ksm_car(list);
    list = ksm_cdr(list);
    ksm_funcall(proc, ksm_nil_object, ksm_nil_object);
  }
}

KSM_DEF_BLT(atfork)
     // (thread:atfork prepare parent child)
{
  ksm_t arg_prepare, arg_parent, arg_child;
  int err, first;

  KSM_POP_ARG(arg_prepare);
  KSM_POP_ARG(arg_parent);
  KSM_POP_ARG(arg_child);
  KSM_SET_ARGS0();
  first = install_prepare(arg_prepare);
  install_parent(arg_parent);
  install_child(arg_child);
  if( first && 
      (err = pthread_atfork(atfork_prepare, atfork_parent, atfork_child)) )
    ksm_error("failed to pthread_atfork: %d", err);
  return ksm_unspec_object;
}

KSM_DEF_BLT(setschedparam)
     // (thread:setschedparam thread policy priority)
     // policy: @SCHED_RR | @SCHED_FIFO | @SCHED_OTHER
     // priority: non-negative integer
{
  ksm_t arg_thread, arg_policy, arg_priority;
  int policy, err;
  struct sched_param sched_param;

  KSM_POP_ARG(arg_thread);
  KSM_POP_ARG(arg_policy);
  KSM_POP_ARG(arg_priority);
  KSM_SET_ARGS0();
  CONFIRM_THREAD(arg_thread);
  policy = ksm_get_si(arg_policy);
  sched_param.sched_priority = ksm_get_si(arg_priority);
  if( (err = pthread_setschedparam(THREAD_ID(arg_thread),
				   policy, &sched_param)) ){
    ksm_error("pthread_setschedparam failed: %d", err);
  }
  return ksm_unspec_object;
}

KSM_DEF_BLT(getschedparam)
     // (thread:getschedparam thread)
     // ==> (policy . priority)
{
  ksm_t arg_thread;
  int policy, err;
  struct sched_param sched_param;

  KSM_POP_ARG(arg_thread);
  KSM_SET_ARGS0();
  CONFIRM_THREAD(arg_thread);
  if( (err = pthread_getschedparam(THREAD_ID(arg_thread),
				    &policy, &sched_param)) )
    ksm_error("pthread_getschedparam failed: %d", err);
  return ksm_cons(ksm_make_int(policy), 
		  ksm_make_int(sched_param.sched_priority));
}

void ksm_init_thread_package(void)
{
  if( sizeof(int) < sizeof(pthread_t) )
    ksm_fatal("thread package broken");
  thread_flag = ksm_new_flag();
  mutex_flag  = ksm_new_flag();
  ksm_install_disposer(mutex_flag, dispose_mutex);
  cond_flag  = ksm_new_flag();
  ksm_install_disposer(cond_flag, dispose_cond);
  sem_flag  = ksm_new_flag();
  ksm_install_disposer(sem_flag, dispose_sem);
  KSM_ENTER_BLT("thread?", thread_p);
  KSM_ENTER_BLT("thread:create", create);
  KSM_ENTER_BLT("thread:exit", exit);
  KSM_ENTER_BLT("thread:cancel", cancel);
  KSM_ENTER_BLT("thread:join", join);
  KSM_ENTER_BLT("thread:setcancelstate", setcancelstate);
  KSM_ENTER_BLT("thread:setcanceltype", setcanceltype);
  KSM_ENTER_BLT("thread:testcancel", testcancel);
  KSM_ENTER_BLT("thread:with-cleanup", with_cleanup);
  KSM_ENTER_BLT("thread:self", self);
  KSM_ENTER_BLT("thread:make-mutex", make_mutex);
  KSM_ENTER_BLT("thread:mutex-lock", mutex_lock);
  KSM_ENTER_BLT("thread:mutex-trylock", mutex_trylock);
  KSM_ENTER_BLT("thread:mutex-unlock", mutex_unlock);
  KSM_ENTER_BLT("thread:make-cond", make_cond);
  KSM_ENTER_BLT("thread:cond-signal", cond_signal);
  KSM_ENTER_BLT("thread:cond-broadcast", cond_broadcast);
  KSM_ENTER_BLT("thread:cond-wait", cond_wait);
  KSM_ENTER_BLT("thread:cond-timedwait", cond_timedwait);
  KSM_ENTER_BLT("thread:make-sem", make_sem);
  KSM_ENTER_BLT("thread:sem-wait", sem_wait);
  KSM_ENTER_BLT("thread:sem-trywait", sem_trywait);
  KSM_ENTER_BLT("thread:sem-post", sem_post);
  KSM_ENTER_BLT("thread:sem-getvalue", sem_getvalue);
  KSM_ENTER("thread:SEM-VALUE-MAX", ksm_make_int(SEM_VALUE_MAX));
  KSM_ENTER_BLT("thread:getspecific", getspecific);
  KSM_ENTER_BLT("thread:setspecific", setspecific);
  KSM_ENTER_BLT("thread:sigmask", sigmask);
  KSM_ENTER_BLT("thread:kill", kill);
  KSM_ENTER_BLT("thread:sigwait", sigwait);
  KSM_ENTER_BLT("thread:equal?", equal);
  KSM_ENTER_BLT("thread:detach", detach);
  KSM_ENTER_BLT("thread:atfork", atfork);
  KSM_ENTER_BLT("thread:setschedparam", setschedparam);
  KSM_ENTER_BLT("thread:getschedparam", getschedparam);
}
#else
void ksm_init_thread_package(void)
{

}
#endif    /* PTHREAD */
