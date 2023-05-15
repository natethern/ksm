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
#include <signal.h>

static int sigset_flag;

#define MAKE_SIGSET(p)    ksm_itype(sigset_flag, (int)p, 0, 0)
#define IS_SIGSET(x)      (ksm_flag(x)==sigset_flag)
#define SIGSET_REF(x)     ((sigset_t *)ksm_ival(x, 0))
#define CONFIRM_SIGSET(x) \
          do{ if( !IS_SIGSET(x) ) ksm_error("sigset expected: %k", x); \
          } while(0)

static
void dispose_sigset(ksm_t x)
{
  ksm_dispose(SIGSET_REF(x));
}

ksm_t ksm_make_sigset(void)
{
  sigset_t *p;

  p = ksm_alloc(sizeof(*p));
  return MAKE_SIGSET(p);
}

int ksm_is_sigset(ksm_t x)
{
  return IS_SIGSET(x);
}

sigset_t *ksm_sigset_ref(ksm_t x)
{
  return SIGSET_REF(x);
}

KSM_DEF_BLT(make_sigset)
     // (make-sigset)
{
  sigset_t *p;

  p = ksm_alloc(sizeof(*p));
  sigemptyset(p);
  return MAKE_SIGSET(p);
}

KSM_DEF_BLT(sigset_p)
     // (sigset? x)
{
  ksm_t x;

  KSM_POP_ARG(x);
  KSM_SET_ARGS0();
  return KSM_BOOL2OBJ(IS_SIGSET(x));
}

KSM_DEF_BLT(sigemptyset)
     // (sigemptyset sigset)
{
  ksm_t sig;

  KSM_POP_ARG(sig);
  KSM_SET_ARGS0();
  CONFIRM_SIGSET(sig);
  sigemptyset(SIGSET_REF(sig));
  return ksm_unspec_object;
}

KSM_DEF_BLT(sigfillset)
     // (sigfillset sigset)
{
  ksm_t sig;

  KSM_POP_ARG(sig);
  KSM_SET_ARGS0();
  CONFIRM_SIGSET(sig);
  sigfillset(SIGSET_REF(sig));
  return ksm_unspec_object;
}

KSM_DEF_BLT(sigaddset)
     // (sigaddset sigset signum)
{
  ksm_t sig, num;
  int err;

  KSM_POP_ARG(sig);
  KSM_POP_ARG(num);
  KSM_SET_ARGS0();
  CONFIRM_SIGSET(sig);
  if( (err = sigaddset(SIGSET_REF(sig), ksm_get_si(num))) )
    ksm_error("failed to sigaddset: %d", err);
  return ksm_unspec_object;
}

KSM_DEF_BLT(sigdelset)
     // (sigaddset sigset signum)
{
  ksm_t sig, num;
  int err;

  KSM_POP_ARG(sig);
  KSM_POP_ARG(num);
  KSM_SET_ARGS0();
  CONFIRM_SIGSET(sig);
  if( (err = sigdelset(SIGSET_REF(sig), ksm_get_si(num))) )
    ksm_error("failed to sigdelset: %d", err);
  return ksm_unspec_object;
}

KSM_DEF_BLT(sigismember)
     // (sigismember sig signum)
{
  ksm_t sig, num;
  int res;

  KSM_POP_ARG(sig);
  KSM_POP_ARG(num);
  KSM_SET_ARGS0();
  CONFIRM_SIGSET(sig);
  res = sigismember(SIGSET_REF(sig), ksm_get_si(num));
  if( res == -1 )
    ksm_error("failed to sigdelset: %d", res);
  return KSM_BOOL2OBJ(res);
}

#define E     KSM_ENTER
#define I     ksm_make_int

void ksm_init_signal_unit(void)
{
  sigset_flag = ksm_new_flag();
  ksm_install_disposer(sigset_flag, dispose_sigset);
  KSM_ENTER_BLT("make-sigset", make_sigset);
  KSM_ENTER_BLT("sigset?", sigset_p);
  KSM_ENTER_BLT("sigemptyset", sigemptyset);
  KSM_ENTER_BLT("sigfillset", sigfillset);
  KSM_ENTER_BLT("sigaddset", sigaddset);
  KSM_ENTER_BLT("sigdelset", sigdelset);
  KSM_ENTER_BLT("sigismember?", sigismember);

  E("@SIGHUP",    I(1));  /* Hangup (POSIX).  */
  E("@SIGINT",    I(2));  /* Interrupt (ANSI).  */
  E("@SIGQUIT",   I(3));  /* Quit (POSIX).  */
  E("@SIGILL",    I(4));  /* Illegal instruction (ANSI).  */
  E("@SIGTRAP",   I(5));  /* Trace trap (POSIX).  */
  E("@SIGABRT",   I(6));  /* Abort (ANSI).  */
  E("@SIGIOT",    I(6));  /* IOT trap (4.2 BSD).  */
  E("@SIGBUS",    I(7));  /* BUS error (4.2 BSD).  */
  E("@SIGFPE",    I(8));  /* Floating-point exception (ANSI). */
  E("@SIGKILL",   I(9));  /* Kill, unblockable (POSIX).  */
  E("@SIGUSR1",   I(10)); /* User-defined signal 1 (POSIX).  */
  E("@SIGSEGV",   I(11)); /* Segmentation violation (ANSI).  */
  E("@SIGUSR2",   I(12)); /* User-defined signal 2 (POSIX).  */
  E("@SIGPIPE",   I(13)); /* Broken pipe (POSIX).  */
  E("@SIGALRM",   I(14)); /* Alarm clock (POSIX).  */
  E("@SIGTERM",   I(15)); /* Termination (ANSI).  */
  E("@SIGSTKFLT", I(16)); /* Stack fault.  */
  E("@SIGCLD",    I(17)); /* Same as SIGCHLD (System V).  */
  E("@SIGCHLD",   I(17)); /* Child status has changed (POSIX). */
  E("@SIGCONT",   I(18)); /* Continue (POSIX).  */
  E("@SIGSTOP",   I(19)); /* Stop, unblockable (POSIX).  */
  E("@SIGTSTP",   I(20)); /* Keyboard stop (POSIX).  */
  E("@SIGTTIN",   I(21)); /* Background read from tty (POSIX).  */
  E("@SIGTTOU",   I(22)); /* Background write to tty (POSIX).  */
  E("@SIGURG",    I(23)); /* Urgent condition on socket (4.2 BSD).  */
  E("@SIGXCPU",   I(24)); /* CPU limit exceeded (4.2 BSD).  */
  E("@SIGXFSZ",   I(25)); /* File size limit exceeded (4.2 BSD).  */
  E("@SIGVTALRM", I(26)); /* Virtual alarm clock (4.2 BSD).  */
  E("@SIGPROF",   I(27)); /* Profiling alarm clock (4.2 BSD).  */
  E("@SIGWINCH",  I(28)); /* Window size change (4.3 BSD, Sun).  */
  E("@SIGPOLL",   I(29)); /* Pollable event occurred (System V).  */
  E("@SIGIO",     I(29)); /* I/O now possible (4.2 BSD).  */
  E("@SIGPWR",    I(30)); /* Power failure restart (System V).  */
  E("@SIGUNUSED", I(31));

  E("@SIG_BLOCK",   I(SIG_BLOCK));
  E("@SIG_UNBLOCK", I(SIG_UNBLOCK));
  E("@SIG_SETMASK", I(SIG_SETMASK));
}
