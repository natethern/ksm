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

#define DEBUG_SYNTAX

typedef struct {
  ksm_t literals;
  ksm_t def_env;
  ksm_t pat_env;
  ksm_t run_env;
} syntax_env;

/*** syntax *********************************************************/

#define MAKE_SYNTAX(lit,rule,env) \
  ksm_itype(KSM_FLAG_SYNTAX,(int)(lit),(int)(rule),(int)(env))
#define IS_SYNTAX(x)          (ksm_flag(x)==KSM_FLAG_SYNTAX)
#define SYNTAX_LITERALS(stx)  ((ksm_t)ksm_ival(stx,0))
#define SYNTAX_RULES(stx)     ((ksm_t)ksm_ival(stx,1))
#define SYNTAX_ENV(stx)       ((ksm_t)ksm_ival(stx,2))

#define SYNTAX_LITERALS_NO_LOCK(stx)  ((ksm_t)ksm_ival_no_lock(stx,0))
#define SYNTAX_RULES_NO_LOCK(stx)     ((ksm_t)ksm_ival_no_lock(stx,1))
#define SYNTAX_ENV_NO_LOCK(stx)       ((ksm_t)ksm_ival_no_lock(stx,2))

static
void check_stx_literals(ksm_t lit)
{
  if( ksm_length(lit) < 0 )
    ksm_error("invalid literals: %k", lit);
}

static
void check_stx_rules(ksm_t rules)
     // (((pat ...) (template ...)) ...)
{
  ksm_t r, p, t, save;

  save = rules;
  while( ksm_is_cons(rules) ){
    r = ksm_car(rules);
    if( ksm_length(r) != 2 )
      ksm_error("invalid rule: %k", ksm_car(rules));
    p = ksm_car(r);
    t = ksm_car(ksm_cdr(r));
    if( !(ksm_length(p) >= 1) )
      ksm_error("invalid pattern: %k", ksm_car(r));
    if( !(ksm_length(t) >= 0) )
      ksm_error("invalid template: %k", ksm_car(ksm_cdr(r)));
    rules = ksm_cdr(rules);
  }
  if( !ksm_is_nil(rules) )
    ksm_error("invalid rules: %k", save);
}

ksm_t ksm_make_syntax(ksm_t literals, ksm_t rules, ksm_t env)
{
  check_stx_literals(literals);
  check_stx_rules(rules);
  return MAKE_SYNTAX(literals, rules, env);
}

int ksm_is_syntax(ksm_t x)
{
  return IS_SYNTAX(x);
}

static
ksm_t mark_syntax(ksm_t stx)
{
  ksm_mark_object(SYNTAX_LITERALS_NO_LOCK(stx));
  ksm_mark_object(SYNTAX_RULES_NO_LOCK(stx));
  ksm_mark_object(SYNTAX_ENV_NO_LOCK(stx));
  return 0;
}

/*** link ***********************************************************/

#define MAKE_LINK(key,env) \
    ksm_itype(KSM_FLAG_STX_LINK,(int)(key),(int)(env),0)
#define IS_LINK(x)       (ksm_flag(x)==KSM_FLAG_STX_LINK)
#define LINK_KEY(link)   ((ksm_t)ksm_ival(link,0))
#define LINK_ENV(link)   ((ksm_t)ksm_ival(link,1))

#define LINK_KEY_NO_LOCK(link) ((ksm_t)ksm_ival_no_lock(link,0))
#define LINK_ENV_NO_LOCK(link) ((ksm_t)ksm_ival_no_lock(link,1))

static
ksm_t mark_link(ksm_t link)
{
  ksm_mark_object(LINK_KEY_NO_LOCK(link));
  ksm_mark_object(LINK_ENV_NO_LOCK(link));
  return 0;
}

static
ksm_pair eval_link(ksm_t link, ksm_t dummy_env)
{
  ksm_t key, env, x;

  key = LINK_KEY(link);
  env = LINK_ENV(link);
  x = ksm_lookup(key, env);
  if( !x )
    ksm_error("can't happen in eval_link");
  return ksm_make_eval_done(x);
}

static
int fprint_link(FILE *fp, ksm_t link, int alt)
{
  return fprintf(fp, alt?"%#k":"%k", LINK_KEY(link));
}

/*** gensym *********************************************************/

ksm_t  ksm_gensym(void)
{
  static unsigned seed = 0;
  static char str[10];
  
  
  if( ++seed == 0 )
    ksm_fatal("gensym overflow");
  sprintf(str, "GENSYM-%u", seed);
  return ksm_make_key(str);
}

/*** utilities ******************************************************/

static ksm_t key_dots;   // ...    /*** GLOBAL ***/

static
int has_tail(ksm_t cons, int check_last)
{
  ksm_t obj;

  obj = ksm_cdr(cons);
  if( ksm_is_cons(obj) && (ksm_car(obj) == key_dots) ){
    if( check_last && !ksm_is_nil(ksm_cdr(obj)) )
      ksm_error("invalid pattern: %k", cons);
    return 1;
  }
  else
    return 0;
}

static
int is_literal(ksm_t key, syntax_env *env)
{
  ksm_t ls;

  ls = env->literals;
  while( ksm_is_cons(ls) ){
    if( key == ksm_car(ls) )
      return 1;
    ls = ksm_cdr(ls);
  }
  return 0;
}

static
int is_external(ksm_t key, syntax_env *env)
{
  return (ksm_lookup(key, env->run_env) != 0) ||
     (ksm_lookup(key, env->def_env) != 0);
}

static
void extend_binding(ksm_t key, ksm_t val, syntax_env *senv)
{
  senv->pat_env = ksm_prepend_local(key, val, senv->pat_env);
}

static
ksm_t map(ksm_t (*f)(ksm_t), ksm_t list)
{
  ksm_t head, tail;

  ksm_list_begin(&head, &tail);
  while( ksm_is_cons(list) ){
    ksm_list_extend(&head, &tail, (*f)(ksm_car(list)));
    list = ksm_cdr(list);
  }
  return head;
}

#define mapcar(list)   map(ksm_car, list)
#define mapcdr(list)   map(ksm_cdr, list)

static
int has_tail_element(ksm_t list)
{
  ksm_t x, save = list;
  int p, n;

  p = n = 0;
  while( ksm_is_cons(list) ){
    x = ksm_car(list);
    if( ksm_is_nil(x) )
      ++n;
    else if( ksm_is_cons(x) )
      ++p;
    list = ksm_cdr(list);
  }
  if( p && n )
    ksm_error("can't rewrite (has_tail_element): %k", save);
  return p;
}

static
ksm_t slice(ksm_t template, ksm_t base)
{
  ksm_t head, tail, x;

  ksm_list_begin(&head, &tail);
  while( ksm_is_cons(template) ){
    x = ksm_car(base);
    if( ksm_is_cons(x) ){
      if( has_tail(template, 0) )
	ksm_list_append(&head, &tail, ksm_car(x));
      else
	ksm_list_extend(&head, &tail, ksm_car(x));
      ksm_set_car(base, ksm_cdr(x));
    }
    else
      ksm_list_extend(&head, &tail, x);
    template = ksm_cdr(template);
    base = ksm_cdr(base);
  }
  return head;
}

static
ksm_t expand_tailed(ksm_t template, syntax_env *senv)
{
  if( ksm_is_key(template) ){
    ksm_t x;

    x = ksm_lookup_local(template, senv->pat_env, 0);
    if( !x || ksm_length(x) < 0 )
      ksm_error("can't rewrite (expand_tailed): %k", template);
    return x;
  }
  else if( ksm_is_cons(template) ){
    ksm_t bh, bt, x, h, t, s = template;
    
    ksm_list_begin(&bh, &bt);
    while( ksm_is_cons(template) ){
      x = ksm_car(template);
      if( ksm_is_key(x) || ksm_is_cons(x) )
	x = expand_tailed(x, senv);
      ksm_list_extend(&bh, &bt, x);
      if( has_tail(template, 0) )
	template = ksm_cdr(template);
      template = ksm_cdr(template);
    }
    template = s;
    ksm_list_begin(&h, &t);
    while( has_tail_element(bh) )
      ksm_list_extend(&h, &t, slice(template, bh));
    return h;
  }
  else{
    ksm_error("can't handle template: %k", template);
    return ksm_unspec_object;
  }
}

/*** match **********************************************************/

static
void bind(ksm_t pat, ksm_t expr, syntax_env *senv, int tail)
{
  if( ksm_is_key(pat) ){
    if( is_literal(pat, senv) )
      return;
    else{
      extend_binding(pat, expr, senv);
      return;
    }
  }
  else if( ksm_is_cons(pat) ){
    do{
      if( has_tail(pat, 1) ){
	bind(ksm_car(pat), expr, senv, 1);
	return;
      }
      bind(ksm_car(pat), tail ? mapcar(expr) : ksm_car(expr), senv, 0);
      expr = tail ? mapcdr(expr) : ksm_cdr(expr);
      pat = ksm_cdr(pat);
    } while( ksm_is_cons(pat) );
  }
}

static
int probe_match(ksm_t pat, ksm_t expr, syntax_env *senv)
     // returns 1 if match
{
  if( ksm_is_key(pat) ){
    if( is_literal(pat, senv) )
      return ksm_is_key(expr) && !(is_external(expr, senv)) && 
	(pat == expr);
    else
      return 1;
  }
  else if( ksm_is_cons(pat) ){
    while( ksm_is_cons(pat) ){
      if( has_tail(pat, 1) ){
	pat = ksm_car(pat);
	while( ksm_is_cons(expr) ){
	  if( probe_match(pat, ksm_car(expr), senv) == 0 )
	    return 0;
	  expr = ksm_cdr(expr);
	}
	return ksm_is_nil(expr);
      }
      if( !(ksm_is_cons(expr) && 
	    probe_match(ksm_car(pat), ksm_car(expr), senv)) )
	return 0;
      pat = ksm_cdr(pat);
      expr = ksm_cdr(expr);
    }
    return probe_match(pat, expr, senv);
  }
  else
    return ksm_equal(pat, expr);
}

static
int match(ksm_t pat, ksm_t expr, syntax_env *senv)
     // returns 1 if match, 0 if no match
{
  if( probe_match(pat, expr, senv) ){
    bind(pat, expr, senv, 0);
    return 1;
  }
  else
    return 0;
}

/*** rewrite ********************************************************/

static
ksm_t rewrite_key(ksm_t key, syntax_env *senv)
{
  ksm_t x;

  x = ksm_lookup_local(key, senv->pat_env, 0);
  if( x )
    return x;
  x = ksm_lookup(key, senv->def_env);
  if( x )
    return MAKE_LINK(key, senv->def_env);
  x = ksm_gensym();
  senv->pat_env = ksm_prepend_local(key, x, senv->pat_env);
  return x;
}

static
ksm_t rewrite(ksm_t template, syntax_env *senv)
{
  if( ksm_is_key(template) )
    return rewrite_key(template, senv);
  else if( ksm_is_cons(template) ){
    ksm_t head, tail, x;

    ksm_list_begin(&head, &tail);
    while( ksm_is_cons(template) ){
      if( has_tail(template, 0) ){
	x = expand_tailed(ksm_car(template), senv);
	ksm_list_append(&head, &tail, x);
	template = ksm_cdr(template);
      }
      else{
	x = rewrite(ksm_car(template), senv);
	ksm_list_extend(&head, &tail, x);
      }
      template = ksm_cdr(template);
    }
    return head;
  }
  else
    return template;
}

/*** expansion ******************************************************/

static
ksm_t ksm_expand_macro(ksm_t args, ksm_t syntax, ksm_t env)
{
  ksm_t rules, rule, pat, tmp;
  syntax_env senv;

  senv.literals = SYNTAX_LITERALS(syntax);
  senv.def_env = SYNTAX_ENV(syntax);
  senv.pat_env = ksm_nil_object;
  senv.run_env = env;
  rules = SYNTAX_RULES(syntax);
  while( ksm_is_cons(rules) ){
    rule = ksm_car(rules);
    pat = ksm_car(rule);
    tmp = ksm_car(ksm_cdr(rule));
    if( match(ksm_cdr(pat), args, &senv) ){
      return rewrite(tmp, &senv);
    }
    senv.pat_env = ksm_nil_object;
    rules = ksm_cdr(rules);
  }
  ksm_error("can't expand macro: %k", args);
  return ksm_unspec_object;
}

/*** interface ******************************************************/

KSM_DEF_SPC(syntax_rules)
     // (syntax-rules literals rules)
{
  ksm_t literals;

  KSM_POP_ARG(literals);
  if( ksm_length(literals) < 0 )
    ksm_error("incompatible literals (syntax-rules): %k", literals);
  return MAKE_SYNTAX(literals, KSM_ARGS(), KSM_ENV());
}

KSM_DEF_SPC(define_syntax)
     // (define-syntax key syntax-rules)
{
  ksm_t key, s, stx;

  KSM_SET_ARGS2(key, s);
  KSM_CONFIRM_KEY(key);
  stx = ksm_eval(s, KSM_ENV());
  if( stx == ksm_except )
    return ksm_except;
  if( !IS_SYNTAX(stx) )
    ksm_error("incompatible second arg (define-syntax): %k", s);
  if( !ksm_is_global(KSM_ENV()) )
    ksm_error("DEFINE-SYNTAX is allowed only in toplevel");
  ksm_enter_global(key, stx, KSM_ENV(), 1);
  return ksm_unspec_object;
}

static
ksm_t let_syntax_extend(ksm_t vars, ksm_t env, ksm_t environ)
     // vars: ((var val) ...)
     // returns ksm_except if exception encountered
{
  ksm_t x, var, val;

  while( ksm_is_cons(vars) ){
    x = ksm_car(vars);
    vars = ksm_cdr(vars);
    if( ksm_length(x) != 2 )
      ksm_error("invalid let-syntax form: %k", vars);
    var = ksm_cvt_key(ksm_first(x));
    val = ksm_eval(ksm_second(x), env);
    if( val == ksm_except )
      return ksm_except;
    if( !IS_SYNTAX(val) )
      ksm_error("syntax-rules expected: %k", val);
    environ = ksm_prepend_local(var, val, environ);
  }
  if( !ksm_is_nil(vars) )
    ksm_error("invalid let-syntax form: %k", vars);
  return environ;
}

/***
static
void let_syntax_extend(ksm_t x, ksm_t env, ksm_t *extend)
     // x: (key syntax-rules)
{
  ksm_t key, rules;

  if( ksm_length(x) != 2 )
    ksm_error("invalid LET-SYNTAX-FORM: %k", x);
  key = ksm_car(x);
  if( !ksm_is_key(key) )
    ksm_error("invalid LET-SYNTAX-FORM: %k", key);
  rules = ksm_eval(ksm_car(ksm_cdr(x)), env);
  if( !IS_SYNTAX(rules) )
    ksm_error("syntax-rules expected: %k", ksm_car(ksm_cdr(x)));
  *extend = ksm_prepend_local(key, rules, *extend);
}
***/

KSM_DEF_RSPC(let_syntax)
     // (let-syntax bindings body)
{
  ksm_t bindings, environ;

  KSM_POP_ARG(bindings);
  environ = let_syntax_extend(bindings, KSM_ENV(), KSM_ENV());
  if( environ == ksm_except )
    return ksm_make_eval_done(ksm_except);
  return ksm_eval_body(KSM_ARGS(), environ);
}

static
ksm_t letrec_syntax_extend1(ksm_t vars, ksm_t env)
     // vars: ((var expr) ...)
{
  ksm_t x, var;

  while( ksm_is_cons(vars) ){
    x = ksm_car(vars);
    vars = ksm_cdr(vars);
    if( ksm_length(x) != 2 )
      ksm_error("invalid letrec-syntax form: %k", vars);
    var = ksm_cvt_key(ksm_car(x));
    env = ksm_prepend_local(var, ksm_unspec_object, env);
  }
  if( !ksm_is_nil(vars) )
    ksm_error("invalid letrec-syntax form: %k", vars);
  return env;
}

static
ksm_t letrec_syntax_extend2(ksm_t vars, ksm_t env)
     // vars: ((var expr) ...)
{
  ksm_t x, var, val;

  while( ksm_is_cons(vars) ){
    x = ksm_car(vars);
    vars = ksm_cdr(vars);
    var = ksm_cvt_key(ksm_car(x));
    val = ksm_eval(ksm_second(x), env);
    if( val == ksm_except )
      return ksm_except;
    if( !IS_SYNTAX(val) )
      ksm_error("syntax-rules expected: %k", val);
    if( ksm_update_local(var, val, env, 0) != 0 )
      ksm_error("can't happen in letrec_syntax_extend2");
  }
  return env;
}

/***
static
void letrec_syntax_extend1(ksm_t x, ksm_t *extend)
     // x: (key syntax-rules)
{
  ksm_t key;

  if( ksm_length(x) != 2 )
    ksm_error("invalid LETREC-SYNTAX-FORM: %k", x);
  key = ksm_car(x);
  if( !ksm_is_key(key) )
    ksm_error("invalid LETREC-SYNTAX-FORM: %k", key);
  *extend = ksm_prepend_local(key, ksm_unspec_object, *extend);
}

static
void letrec_syntax_extend2(ksm_t x, ksm_t extend)
     // x: (key syntax-rules)
{
  ksm_t key, rules;

  key = ksm_car(x);
  rules = ksm_eval(ksm_car(ksm_cdr(x)), extend);
  if( !IS_SYNTAX(rules) )
    ksm_error("syntax-rules expected: %k", ksm_car(ksm_cdr(x)));
  if( ksm_update_local(key, rules, extend, 0) != 0 )
    ksm_error("can't happen in letrec_syntax_extend2");
}
***/

KSM_DEF_RSPC(letrec_syntax)
     // (letrec-syntax bindings body)
{
  ksm_t bindings, environ;

  KSM_POP_ARG(bindings);
  environ = letrec_syntax_extend1(bindings, KSM_ENV());
  environ = letrec_syntax_extend2(bindings, environ);
  if( environ == ksm_except )
    return ksm_make_eval_done(ksm_except);
  return ksm_eval_body(KSM_ARGS(), environ);
}

/*** evaluator ******************************************************/

ksm_pair eval_macro(ksm_t head, ksm_t args, ksm_t env)
{
  ksm_t expr;

  expr = ksm_expand_macro(args, head, env);
  return ksm_make_eval_pair(expr, env);
}

/*** init ***********************************************************/

void ksm_init_syntax(void)
{
  ksm_install_marker(KSM_FLAG_SYNTAX, mark_syntax);
  ksm_install_marker(KSM_FLAG_STX_LINK, mark_link);
  ksm_install_evaluator(KSM_FLAG_STX_LINK, eval_link);
  ksm_install_fprintf(KSM_FLAG_STX_LINK, fprint_link);
  key_dots = ksm_make_key("...");
  ksm_install_fun_info(KSM_FLAG_SYNTAX, eval_macro, 0);
  KSM_ENTER_SPC("syntax-rules", syntax_rules);
  KSM_ENTER_SPC("define-syntax", define_syntax);
  KSM_ENTER_RSPC("let-syntax", let_syntax);
  KSM_ENTER_RSPC("letrec-syntax", letrec_syntax);
}
