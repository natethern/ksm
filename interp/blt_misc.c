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

KSM_DEF_BLT(not)
     // (not obj)
{
  ksm_t obj;

  KSM_SET_ARGS1(obj);
  return KSM_BOOL2OBJ(ksm_is_false(obj));
}

KSM_DEF_BLT(boolean_p)
     // (boolean? obj)
{
  ksm_t obj;

  KSM_SET_ARGS1(obj);
  return KSM_BOOL2OBJ(ksm_is_true(obj)||ksm_is_false(obj));
}

KSM_DEF_BLT(symbol_p)
     // (symbol? obj)
{
  ksm_t obj;

  KSM_SET_ARGS1(obj);
  return KSM_BOOL2OBJ(ksm_is_key(obj));
}

KSM_DEF_BLT(symbol_to_string)
     // (symbol->string symbol)
{
  ksm_t key;

  KSM_SET_ARGS1(key);
  KSM_CONFIRM_KEY(key);
  return ksm_make_string(ksm_key_value(key));
}

KSM_DEF_BLT(string_to_symbol)
     // (string->symbol string)
{
  ksm_t string;
  char *p;

  KSM_SET_ARGS1(string);
  p = ksm_string_value(string);
  return ksm_make_key(p);
}

KSM_DEF_BLT(procedure_p)
     // (procedure? x)
{
  ksm_t x;

  KSM_SET_ARGS1(x);
  return KSM_BOOL2OBJ(ksm_procedure_p(x));
}

KSM_DEF_BLT(apply)
     // (apply proc arg ... args)
{
  ksm_t proc, funargs, head, tail, x;

  KSM_POP_ARG(proc);
  funargs = KSM_ARGS();
  ksm_list_begin(&head, &tail);
  while( ksm_is_cons(funargs) ){
    if( ksm_is_nil(ksm_cdr(funargs)) ){
      x = ksm_car(funargs);
      if( ksm_length(x) < 0 )
	ksm_error("incompatible last arg to APPLY: %k", x);
      if( ksm_is_cons(tail) )
	ksm_set_cdr(tail, x);
      else
	head = x;
      break;
    }
    ksm_list_extend(&head, &tail, ksm_car(funargs));
    funargs = ksm_cdr(funargs);
  }
  return ksm_funcall(proc, head, KSM_ENV());
}

static
ksm_t ksm_mapslice(ksm_t args)
     // returns 0 if no more args
{
  ksm_t head, tail, x;
  int ending = 0;

  if( !ksm_is_cons(args) )
    return 0;
  if( ksm_is_nil(ksm_car(args)) )
    ending = 1;
  if( !ending )
    ksm_list_begin(&head, &tail);
  while( ksm_is_cons(args) ){
    x = ksm_car(args);
    if( ksm_is_nil(x) && ending ){
      args = ksm_cdr(args);
      continue;
    }
    if( !((ksm_is_cons(x) && !ending) || (ksm_is_nil(x) && ending)) )
      ksm_error("incompatible args to MAP/FOR-EACH: %k", args);
    ksm_list_extend(&head, &tail, ksm_car(x));
    ksm_set_car(args, ksm_cdr(x));
    args = ksm_cdr(args);
  }
  return ending ? 0 : head;
}

KSM_DEF_BLT(map)
     // (map proc list ...)
{
  ksm_t proc, mapargs, x;
  ksm_t head, tail;

  KSM_POP_ARG(proc);
  mapargs = KSM_ARGS();
  ksm_list_begin(&head, &tail);
  while( (x = ksm_mapslice(mapargs)) ){
    x = ksm_funcall(proc, x, KSM_ENV());
    ksm_list_extend(&head, &tail, x);
  }
  return head;
}

KSM_DEF_BLT(for_each)
     // (for-each proc list ...)
{
  ksm_t proc, mapargs, x;
  
  KSM_POP_ARG(proc);
  mapargs = KSM_ARGS();
  while( (x = ksm_mapslice(mapargs)) ){
    ksm_funcall(proc, x, KSM_ENV());
  }
  return ksm_unspec_object;
}

KSM_DEF_BLT(eval)
     // (eval expr [env])
{
  ksm_t x, y;

  KSM_POP_ARG(x);
  if( KSM_NO_MORE_ARGS() )
    y = KSM_ENV();
  else{
    KSM_SET_ARGS1(y);
    if( !(ksm_is_local(y) || ksm_is_global(y)) )
      ksm_error("environment expected: %k", y);
  }
  return ksm_eval(x, y);
}

/***
static
char *report_keys[] = {
  "quote", "lambda", "if", "set!", "cond", "case", "and", "or", "let",
  "let*", "letrec", "begin", "do", "delay", "quasiquote", "let-syntax",
  "letrec-syntax", "syntax-rules", "define", "define-syntax", 
  "eqv?", "eq?", "equal?", "number?", "complex?", "real?", "rational?",
  "integer?", "exact?", "inexact?", "=", "<", ">", "<=", ">=", 
  "zero?", "positive?", "negative?", "odd?", "even?", "max", "min",
  "+", "*", "-", "/", "abs", "quotient", "remainder", "modulo",
  "gcd", "lcm", "numerator", "denominator", "floor", "ceiling", "truncate",
  "round", "rationalize", "exp", "log", "sin", "cos", "tan", "asin",
  "acos", "atan", "sqrt", "expt", "make-rectangular", "make-polar",
  "real-part", "imag-part", "magnitude", "angle", "exact->inexact",
  "inexact->exact", "number->string", "string->number", "not",
  "boolean?", "pair?", "cons", "car", "cdr", "set-car!", "set-cdr!",
  "caar", "cadr", "cdar", "cddr", "caaar", "caadr", "cadar",
  "caddr", "cdaar", "cdadr", "cddar", "cdddr",
  "caaaar", "caaadr", "caadar", "caaddr", "cadaar",
  "cadadr", "caddar", "cadddr", "cdaaar",
  "cdaadr", "cdadar", "cdaddr", "cddaar", "cddadr", "cdddar", "cddddr",
  "null?", "list?", "list", "length", "append", "reverse", "list-tail",
  "list-ref", "memq", "memv", "member", "assq", "assv", "assoc",
  "symbol?", "symbol->string", "string->symbol", "char?", "char=?",
  "char<?", "char>?", "char<=?", "char>=?", "char-ci=?", "char-ci<?",
  "char-ci>?", "char-ci<=?", "char-ci>=?", "char-alphabetic?",
  "char-numeric?", "char-whitespace?", "char-upper-case?",
  "char-lower-case?", "char->integer", "integer->char", "char-upcase",
  "char-downcase", "string?", "make-string", "string", "string-length",
  "string-ref", "string-set!", "string=?", "string-ci=?", "string<?", 
  "string>?", "string<=?", "string>=?", "string-ci<?", "string-ci>?",
  "string-ci<=?", "string-ci>=?", "substring", "string-append",
  "string->list", "list->string", "string-copy", "string-fill!",
  "vector?", "make-vector", "vector", "vector-length", "vector-ref",
  "vector-set!", "vector->list", "list->vector", "vector-fill!",
  "procedure?", "apply", "map", "for-each", "force", 
  "call-with-current-continuation", "values", "call-with-values",
  "dynamic-wind", "eval", "scheme-report-environment", "null-environment",
  "interaction-environment", "call-with-input-file", "call-with-output-file",
  "input-port?", "output-port?", "current-input-port", "current-output-port",
  "with-input-from-file", "with-output-to-file", "open-input-file",
  "open-output-file", "close-input-port", "close-output-port", 
  "read", "read-char", "peek-char", "eof-object?", "char-ready?",
  "write", "display", "newline", "write-char", "load", "transcript-on",
  "transcript-off",
  0 };
***/

static char *null_keys[] = {   /*** GLOBAL ***/
  "quote", "lambda", "if", "set!", "cond", "case", "and", "or", "let",
  "let*", "letrec", "begin", "do", "delay", "quasiquote", "let-syntax",
  "letrec-syntax", "syntax-rules", "define", "define-syntax", 
  0
};

static 
void stuff_env(char **p, ksm_t dst_global, ksm_t src_global)
{
  ksm_t key, x;

  while(*p){
    key = ksm_make_key(*p++);
    x = ksm_lookup_global(key, src_global, 0);
    if( !x )
      ksm_error("cannot make environment: %k", key);
    ksm_enter_global(key, x, dst_global, 1);
  }
}

KSM_DEF_BLT(scheme_report_environment)
     // (scheme-report-environment version)
{
  ksm_t version;
  int ver;

  KSM_SET_ARGS1(version);
  ver = ksm_int_value(version);
  if( !(ver == 5) )
    ksm_error("incompatible arg to SCHEME-REPORT-ENVIRONMENT: %d", ver);
  return ksm_report_env;
}

static ksm_t null_env;  /*** GLOBAL ***/

static
ksm_t ksm_get_null_env(void)
{
  if( !ksm_is_global(null_env) ){
    null_env = ksm_make_global(40, ksm_nil_object);
    stuff_env(null_keys, null_env, ksm_report_env);
  }
  return null_env;
}

KSM_DEF_BLT(null_environment)
     // (null-environment version)
{
  ksm_t version;
  int ver;

  KSM_SET_ARGS1(version);
  ver = ksm_int_value(version);
  if( !(ver == 5) )
    ksm_error("incompatible arg to NULL-ENVIRONMENT: %d", ver);
  return ksm_get_null_env();
}

KSM_DEF_BLT(interaction_environment)
     // (interaction-environment)
{
  KSM_SET_ARGS0();
  return ksm_interact_env;
}

KSM_DEF_BLT(transcript_on)
     // (transcript-on fname)
{
  //ksm_error("TRANSCRIPT-ON not supported");
  return ksm_unspec_object;
}

KSM_DEF_BLT(transcript_off)
     // (transcript-on fname)
{
  //ksm_error("TRANSCRIPT-OFF not supported");
  return ksm_unspec_object;
}

void ksm_blt_misc(void)
{
  null_env = ksm_nil_object;
  ksm_permanent(&null_env);
  KSM_ENTER_BLT("not", not);
  KSM_ENTER_BLT("boolean?", boolean_p);
  KSM_ENTER_BLT("symbol?", symbol_p);
  KSM_ENTER_BLT("symbol->string", symbol_to_string);
  KSM_ENTER_BLT("string->symbol", string_to_symbol);
  KSM_ENTER_BLT("procedure?", procedure_p);
  KSM_ENTER_BLT("apply", apply);
  KSM_ENTER_BLT("map", map);
  KSM_ENTER_BLT("for-each", for_each);
  KSM_ENTER_BLT("eval", eval);
  KSM_ENTER_BLT("scheme-report-environment", scheme_report_environment);
  KSM_ENTER_BLT("null-environment", null_environment);
  KSM_ENTER_BLT("interaction-environment", interaction_environment);
  KSM_ENTER_BLT("transcript-on", transcript_on);
  KSM_ENTER_BLT("transcript-off", transcript_off);
}
