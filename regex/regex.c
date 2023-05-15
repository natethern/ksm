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
#include "regex.h"

static int regex_flag;

#define MAKE_REGEX(reg,str,match)  \
  ksm_itype(regex_flag,(int)(reg),(int)(str),(int)match)
#define IS_REGEX(x)   (ksm_flag(x)==regex_flag)
#define REGEX_REGEX(reg)  ((regex_t *)ksm_ival(reg,0))
#define REGEX_STR(reg)    ((ksm_t)ksm_ival(reg,1))
#define REGEX_MATCH(reg)  ((regmatch_t *)ksm_ival(reg,2))
#define SET_REGEX_STR(reg,str) ksm_set_ival(reg,1,(int)(str))

#define REGEX_STR_NO_LOCK(reg)    ((ksm_t)ksm_ival_no_lock(reg,1))

#define REGCOMP   utf8_regcomp
#define REGEXEC   utf8_regexec
#define REGFREE   utf8_regfree

static
ksm_t mark_regex(ksm_t reg)
{
  return REGEX_STR_NO_LOCK(reg);
}

static
void dispose_regex(ksm_t reg)
{
  regex_t *p = REGEX_REGEX(reg);
  REGFREE(p);
  ksm_dispose(p);
}

static
ksm_t ksm_make_regex(char *pat, int flag)
{
  regex_t *reg;
  regmatch_t *mp;

  reg = ksm_alloc(sizeof(regex_t));
  if( REGCOMP(reg, pat, flag) != 0 )
    ksm_error("can't compile regular expression \"%s\"", pat);
  mp = ksm_calloc(reg->re_nsub+1, sizeof(regmatch_t));
  return MAKE_REGEX(reg, 0, mp);
}

/***
static
ksm_t ksm_make_string_n(char *p, int n)
{
  char *q;

  q = ksm_alloc(n+1);
  memcpy(q, p, n);
  q[n] = '\0';
  return ksm_make_string_no_copy(q);
}
***/

KSM_DEF_BLT(regcomp)
     // (regex:regcomp pat [flag])
{
  ksm_t pat, flag;
  int f = 0;

  KSM_POP_ARG(pat);
  if( KSM_MORE_ARGS() ){
    KSM_SET_ARGS1(flag);
    f = ksm_int_value(flag);
  }
  return ksm_make_regex(ksm_string_value(pat), f);
}

KSM_DEF_BLT(regexec)
     // (regex:regexec reg str [flag])
{
  ksm_t reg, str, flag;
  regex_t *regp;
  char *s;
  int f, err;

  KSM_POP_ARG(reg);
  KSM_POP_ARG(str);
  if( KSM_MORE_ARGS() ){
    KSM_SET_ARGS1(flag);
    f = ksm_int_value(flag);
  }
  else
    f = 0;
  if( !IS_REGEX(reg) )
    ksm_error("regular expression expected: %k", reg);
  regp = REGEX_REGEX(reg);
  s = ksm_string_value(str);
  err = REGEXEC(regp, s, regp->re_nsub+1, REGEX_MATCH(reg), f);
  if( err == REG_NOMATCH )
    return ksm_false_object;
  if( err != 0 ){
    if( err == REG_ESPACE )
      ksm_error("out of memory (REGEX:REGEXEC)");
    ksm_error("REGEXEC:REGEXEC");
  }
  SET_REGEX_STR(reg, str);
  return ksm_true_object;
}

KSM_DEF_BLT(subexpr)
     // (regex:subexpr reg index)
{
  ksm_t reg, index;
  regex_t *regp;
  regmatch_t *mp;
  int i;
  char *p;

  KSM_SET_ARGS2(reg, index);
  if( !IS_REGEX(reg) )
    ksm_error("regular expression expected: %k", reg);
  regp = REGEX_REGEX(reg);
  i = ksm_int_value(index);
  if( !((i == 0) || (i > 0 && i <= regp->re_nsub)) )
    ksm_error("index out of range (REGEX:SUBEXPR): %d", i);
  mp = REGEX_MATCH(reg);
  p = ksm_string_value(REGEX_STR(reg));
  return ksm_make_string_n(p+mp[i].rm_so, mp[i].rm_eo-mp[i].rm_so);
}

KSM_DEF_BLT(split)
     // (regex:split regex string)
{
  ksm_t str, pat, head, tail;
  char *s, *p;
  regex_t *regp;
  regmatch_t *mp;

  KSM_SET_ARGS2(pat, str);
  s = ksm_string_value(str);
  if( !IS_REGEX(pat) )
    ksm_error("regex:regex expected: %k", pat);
  regp = REGEX_REGEX(pat);
  mp = ksm_calloc(1, sizeof(regmatch_t));
  ksm_list_begin(&head, &tail);
  while( REGEXEC(regp, s, 1, mp, 0) == 0 ){
    if( mp->rm_so == mp->rm_eo )
      ksm_error("REGEX:SPLIT: %s %s", s, p);
    ksm_list_extend(&head, &tail, ksm_make_string_n(s, mp->rm_so));
    s += mp->rm_eo;
  }
  if( *s )
    ksm_list_extend(&head, &tail, ksm_make_string(s));
  return head;
}

KSM_DEF_BLT(replace)
     // (regex:%replace regex string generator eflag count)
     // generator: (lambda (regex) ...) --> string
     // count: number of replacements, or -1 for replace-all
{
  ksm_t gen, str, eflag, cnt, rep;
  ksm_t head, tail, reg;
  char *sp;
  regex_t *regp;
  regmatch_t *mp;
  int ef, c, offset = 0, i;

  KSM_POP_ARG(reg);
  KSM_POP_ARG(str);
  KSM_POP_ARG(gen);
  KSM_POP_ARG(eflag);
  KSM_POP_ARG(cnt);
  KSM_SET_ARGS0();
  if( !IS_REGEX(reg) )
    ksm_error("regex:regex expected: %k", reg);
  SET_REGEX_STR(reg, str);
  regp = REGEX_REGEX(reg);
  mp = REGEX_MATCH(reg);
  sp = ksm_string_value(str);
  ksm_list_begin(&head, &tail);
  ef = ksm_int_value(eflag);
  c = ksm_int_value(cnt);
  while( 1 ){
    if( c == 0 || REGEXEC(regp, sp+offset, regp->re_nsub+1, mp, ef) != 0 ||
	mp[0].rm_eo <= 0 ){
      ksm_list_extend(&head, &tail, ksm_make_string(sp+offset));
      break;
    }
    ksm_list_extend(&head, &tail, 
		    ksm_make_string_n(sp+offset, mp[0].rm_so));
    for(i=0;i<=regp->re_nsub;++i){
      mp[i].rm_so += offset;
      mp[i].rm_eo += offset;
    }
    rep = ksm_funcall(gen, ksm_list1(reg), ksm_interact_env);
    ksm_list_extend(&head, &tail, rep);
    if( c > 0 )
      --c;
    offset = mp[0].rm_eo;
  }
  return ksm_string_append(head);
}

void ksm_init_regex_package(void)
{
  regex_flag = ksm_new_flag();
  ksm_install_marker(regex_flag, mark_regex);
  ksm_install_disposer(regex_flag, dispose_regex);
  KSM_ENTER_BLT("regex:%regcomp", regcomp);
  KSM_ENTER_BLT("regex:%regexec", regexec);
  KSM_ENTER_BLT("regex:subexpr", subexpr);
  KSM_ENTER_BLT("regex:split", split);
  KSM_ENTER_BLT("regex:%replace", replace);
  KSM_ENTER("regex:REG_ICASE", ksm_make_int(REG_ICASE));
  KSM_ENTER("regex:REG_NOSUB", ksm_make_int(REG_NOSUB));
  KSM_ENTER("regex:REG_NEWLINE", ksm_make_int(REG_NEWLINE));
  KSM_ENTER("regex:REG_NOSPEC", ksm_make_int(REG_NOSPEC));
  KSM_ENTER("regex:REG_NOTBOL", ksm_make_int(REG_NOTBOL));
  KSM_ENTER("regex:REG_NOTEOL", ksm_make_int(REG_NOTEOL));
}
