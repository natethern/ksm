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

#ifndef KSM_H
#define KSM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#if HAVE_GMP_H && HAVE_LIBGMP
#define GMP
#endif

#if HAVE_PTHREAD_H && HAVE_LIBPTHREAD
#define PTHREAD
#endif

#ifdef GMP
#include <gmp.h>
#endif

#ifdef PTHREAD
#include <pthread.h>
#endif

typedef union ksm_object *ksm_t;

typedef struct {
  ksm_t expr;
  ksm_t env;
} ksm_pair;

enum { KSM_NUMREP_INT, 
#ifdef GMP
       KSM_NUMREP_MPZ,
       KSM_NUMREP_MPQ,
#endif
       KSM_NUMREP_DOUBLE,
       KSM_NUMREP_COMPLEX,
       KSM_NUMREP_LAST
 };

typedef struct {
  union {
    int ival;
    double dval;
#ifdef GMP
    mpz_ptr zval;
    mpq_ptr qval;
#endif
    void *pval;
  } u;
  int kind;
} ksm_numrep;

typedef struct {
  int exact_p;
  ksm_t (*object)(ksm_numrep *self);
  void (*clone)(ksm_numrep *self);
  void (*promote)(ksm_numrep *arg);
  void (*exact_to_inexact)(ksm_numrep *self);
  void (*inexact_to_exact)(ksm_numrep *self);
  int (*get_si)(ksm_numrep *self);
  double (*get_d)(ksm_numrep *self);
  int (*cmp)(ksm_numrep *self, ksm_numrep *arg);
  int (*sgn)(ksm_numrep *self);
  int (*add)(ksm_numrep *self, ksm_numrep *arg); // 0 done; -1 again
  int (*mul)(ksm_numrep *self, ksm_numrep *arg);
  int (*sub)(ksm_numrep *self, ksm_numrep *arg);
  int (*div)(ksm_numrep *self, ksm_numrep *arg);
  int (*neg)(ksm_numrep *self);
  int (*quotient)(ksm_numrep *self, ksm_numrep *arg);
  int (*remainder)(ksm_numrep *self, ksm_numrep *arg);
  int (*modulo)(ksm_numrep *self, ksm_numrep *arg);
  int (*gcd)(ksm_numrep *self, ksm_numrep *arg);
  int (*lcm)(ksm_numrep *self, ksm_numrep *arg);
  int (*expt)(ksm_numrep *self, ksm_numrep *arg);
  int (*and)(ksm_numrep *self, ksm_numrep *arg);
  int (*or)(ksm_numrep *self, ksm_numrep *arg);
  int (*xor)(ksm_numrep *self, ksm_numrep *arg);
  void (*complement)(ksm_numrep *self);
  int (*lshift)(ksm_numrep *self, int n);
  int (*rshift)(ksm_numrep *self, int n);
  // inexact only
  void (*floor)(ksm_numrep *self);
  void (*ceiling)(ksm_numrep *self);
  void (*truncate)(ksm_numrep *self);
  void (*round)(ksm_numrep *self);
  void (*exp)(ksm_numrep *self);
  void (*log)(ksm_numrep *self);
  void (*sin)(ksm_numrep *self);
  void (*cos)(ksm_numrep *self);
  void (*tan)(ksm_numrep *self);
  void (*asin)(ksm_numrep *self);
  void (*acos)(ksm_numrep *self);
  void (*atan)(ksm_numrep *self);
  void (*atan2)(ksm_numrep *self, ksm_numrep *arg);
  void (*sqrt)(ksm_numrep *self);
} ksm_numrep_op;

typedef struct {
  int (*getc)(void *aux); 
  int (*ungetc)(char ch, void *aux);    // returns EOF if error
  int (*putc)(char ch, void *aux); // returns EOF if error
  int (*flush)(void *aux); // returns EOF if error
  int (*eof)(void *aux);
  int (*char_ready)(void *aux);
  void (*mark_aux)(void *aux);
  void (*dispose_aux)(void *aux);
  void *aux;
} ksm_port;

typedef struct {
  char *start;
  char *end;
  char *next;
} ksm_strbuf;

enum { KSM_FLAG_FREE, KSM_FLAG_IMM, 
       KSM_FLAG_KEY, KSM_FLAG_CONS, KSM_FLAG_LOCAL, KSM_FLAG_GLOBAL,
       KSM_FLAG_BLT, KSM_FLAG_SPC, KSM_FLAG_RSPC,
       KSM_FLAG_LAMBDA, KSM_FLAG_PORT, 
       KSM_FLAG_DYWIN, KSM_FLAG_CONT,
       KSM_FLAG_SYNTAX, KSM_FLAG_STX_LINK,
       KSM_FLAG_DELAY, KSM_FLAG_VECTOR,
       KSM_FLAG_INT, KSM_FLAG_DOUBLE, KSM_FLAG_STRING, KSM_FLAG_CHAR,
       KSM_FLAG_CALLWV, KSM_FLAG_KEYWORD, KSM_FLAG_EXCEPT,
#ifdef GMP
       KSM_FLAG_MPZ, KSM_FLAG_MPQ,
#endif
       KSM_FLAG_COMPLEX, 
       KSM_FLAG_LAST };

#define KSM_FLAG_MAX   100

struct ksm_registry;

enum { KSM_THREAD_EMPTY, KSM_THREAD_CREATED, KSM_THREAD_RUNNING,
       KSM_THREAD_FINISHED, KSM_THREAD_EXITED, KSM_THREAD_CANCELED,
       KSM_THREAD_JOINED
};

typedef struct ksm_ctx_ {
  struct ksm_registry *registry;
  int state;
  int detached;
  ksm_t fun;        // thread function
  ksm_t result;     // thread result
  ksm_t input;
  ksm_t output;
  ksm_t jumps;
  ksm_t private_global;
  ksm_t specific;
  int trace_on;
  int *stack_base;
  int serial;    // serial number of the thread
  char *load_file_name;
  char *load_shared_file_name;
  ksm_strbuf read_buf;
  ksm_strbuf readline_buf;
  ksm_strbuf readstr_buf;
  struct ksm_ctx_ *next_free;
} ksm_ctx;

// object.c
void ksm_init_object(void);
ksm_t ksm_itype(int flag, int a, int b, int c);
ksm_t ksm_itype_no_lock(int flag, int a, int b, int c);
ksm_t ksm_dtype(int flag, double d);
int ksm_flag(ksm_t p);
int ksm_flag_no_lock(ksm_t p);
int ksm_ival(ksm_t p, int i);
int ksm_ival_no_lock(ksm_t p, int i);
int ksm_ival_reg(ksm_t p, int i);
void ksm_set_ival(ksm_t p, int i, int val);
void ksm_set_ival_no_lock(ksm_t p, int i, int val);
double ksm_dval(ksm_t p);
void ksm_set_address_of_ival2_in_ival1(ksm_t x);
void *ksm_alloc(unsigned size);
void *ksm_calloc(size_t nmemb, size_t size);
void *ksm_realloc(void *ptr, size_t size);
void ksm_dispose(void *p);
ksm_t *ksm_registry_top(void);
void ksm_registry_restore(ksm_t *p);
ksm_t ksm_registry_restore_with(ksm_t *p, ksm_t x);
void ksm_registry_restore_with2(ksm_t *p, ksm_t x, ksm_t y);
void ksm_registry_add(ksm_t obj);
void ksm_registry_add_no_lock(ksm_t obj);
unsigned ksm_registry_size(void);
void ksm_registry_save_image(ksm_t *buf); 
     // size of buf should be at least ksm_registry_size()
void ksm_registry_restore_image(ksm_t *buf, unsigned size);
void ksm_install_marker(int flag, ksm_t (*marker)(ksm_t));
void ksm_install_disposer(int flag, void (*disposer)(ksm_t));
void ksm_mark_object(ksm_t x);
void ksm_gc(void);
void ksm_add_gc_prolog(void (*fun)(void));
extern int ksm_gc_shower;
#ifdef PTHREAD
void ksm_lock(void);
void ksm_unlock(void);
ksm_ctx *ksm_get_ctx(void);
ksm_ctx *ksm_new_ctx(ksm_t fun);
void ksm_start_ctx(ksm_ctx *ctx, int *stack_base);
void ksm_finish_ctx(ksm_ctx *ctx, ksm_t result);
void ksm_exit_ctx(ksm_ctx *ctx, ksm_t result);
void ksm_cancel_ctx(ksm_ctx *ctx);
ksm_t ksm_join_ctx(ksm_ctx *ctx, pthread_t thread_id);
ksm_t ksm_get_specific(ksm_t key);
void ksm_set_specific(ksm_t key, ksm_t value);
extern pthread_key_t ksm_ctx_key;
#else    /* NON-PTHREAD */
extern ksm_ctx ksm_main_ctx[1];
#define ksm_get_ctx()  (&ksm_main_ctx[0])
#define ksm_lock()
#define ksm_unlock()
#endif   /* PTHREAD */

// mpz.c mpq.c
#ifdef GMP
extern ksm_numrep_op ksm_mpz_op;
extern ksm_numrep_op ksm_mpq_op;
#endif

// blt.c
void ksm_init_blt(void);
ksm_t ksm_make_blt(ksm_t (*f)(ksm_t args, ksm_t env));
ksm_t ksm_make_spc(ksm_t (*f)(ksm_t args, ksm_t env));
ksm_t ksm_make_rspc(ksm_pair (*f)(ksm_t args, ksm_t env));

//blt_io.c
void ksm_blt_io(void);
ksm_t ksm_input(void);
ksm_t ksm_output(void);
ksm_t ksm_stdin_port;
ksm_t ksm_stdout_port;
ksm_t ksm_stderr_port;
void ksm_write_x(ksm_t obj, ksm_t port, int alt);
#define ksm_write(obj, port) ksm_write_x(obj, port, 1)
#define ksm_display(obj, port) ksm_write_x(obj, port, 0)
extern char *ksm_standard_encoding;
#define KSM_NO_ENCODING                   0
#define KSM_DEFAULT_ENCODING  ((char *)(-1))
ksm_t ksm_open_input_fp(FILE *fp, char *encoding);
ksm_t ksm_open_output_fp(FILE *fp, char *encoding);
ksm_t ksm_open_input_file(char *file, char *encoding);
ksm_t ksm_open_output_file(char *file, char *encoding);

// blt_xxx.c
void ksm_blt_cond(void);
void ksm_blt_cons(void);
void ksm_blt_math(void);
void ksm_blt_pair(void);
void ksm_blt_misc(void);

// bufport.c
void ksm_init_bufport(void);
ksm_t ksm_make_bufport(ksm_t src_port);
ksm_t ksm_make_bufport_with_filename(ksm_t src_port, ksm_t filename);
int ksm_is_bufport(ksm_t x);
void ksm_bufport_ungets(ksm_t bufport, char *s);
int ksm_bufport_has_filename(ksm_t bufport);
ksm_t ksm_bufport_filename(ksm_t bufport);
int ksm_bufport_line_number(ksm_t bufport);

// callwv.c
void ksm_init_callwv(void);
int ksm_is_callwv(ksm_t x);

// char.c
void ksm_init_char(void);
ksm_t ksm_make_char(int ch);
int ksm_is_char(ksm_t x);
int ksm_fetch_char(ksm_t ch);
int ksm_char_value(ksm_t ch);
int ksm_char_value_x(ksm_t x, int *result);
int ksm_fprintf_char(FILE *fp, int ch, int alt);
void ksm_install_charvalue(int flag, int (*f)(ksm_t x, int *result));
void ksm_write_char_to_port(ksm_t x, ksm_t port, int alt);
ksm_t ksm_read_char(ksm_t port);

// charset.c
void ksm_init_charset(void);
ksm_t ksm_make_input_charset_decoding_port(char *code, ksm_t src_port);
ksm_t ksm_make_output_charset_encoding_port(char *code, ksm_t src_port);

// complex.c
extern ksm_numrep_op ksm_complex_op;

// cons.c
void ksm_init_cons(void);
ksm_t ksm_cons(ksm_t car, ksm_t cdr);
ksm_t ksm_car(ksm_t cons);
ksm_t ksm_cdr(ksm_t cons);
int ksm_is_cons(ksm_t x);
void ksm_set_car(ksm_t cons, ksm_t car);
void ksm_set_cdr(ksm_t cons, ksm_t cdr);
void ksm_install_fun_info(int flag, 
  ksm_pair (*fun)(ksm_t head, ksm_t args, ksm_t env), int eval_args);
ksm_t ksm_funcall(ksm_t fun, ksm_t args, ksm_t env);
ksm_t ksm_call_setter(ksm_t fun, ksm_t args, ksm_t val, ksm_t env);
int ksm_procedure_p(ksm_t x);
int ksm_length(ksm_t x);  // returns -1 if improper list
void ksm_list_begin(ksm_t *head, ksm_t *tail);
void ksm_list_extend(ksm_t *head, ksm_t *tail, ksm_t item);
void ksm_list_append(ksm_t *head, ksm_t *tail, ksm_t list);
ksm_t ksm_map_x(ksm_t (*f)(), ksm_t list, void *aux1, void *aux2);
#define ksm_map(f,l)       ksm_map_x(f,l,0,0)
#define ksm_map1(f,l,a)    ksm_map_x(f,l,a,0)
#define ksm_map2(f,l,a,b)  ksm_map_x(f,l,a,b)
void ksm_foreach_x(void (*f)(), ksm_t list, void *a1, void *a2, void *a3);
#define ksm_foreach(f,l)        ksm_foreach_x(f,l,0,0,0)
#define ksm_foreach1(f,l,a)     ksm_foreach_x(f,l,a,0,0)
#define ksm_foreach2(f,l,a,b)   ksm_foreach_x(f,l,a,b,0)
#define ksm_foreach3(f,l,a,b,c) ksm_foreach_x(f,l,a,b,c)
ksm_t ksm_safe_car(ksm_t x);
ksm_t ksm_safe_cdr(ksm_t x);
ksm_t ksm_last_cdr(ksm_t list);
ksm_t ksm_list_ref(ksm_t list, int i);
ksm_t ksm_list1(ksm_t a);
ksm_t ksm_list2(ksm_t a, ksm_t b);
ksm_t ksm_list3(ksm_t a, ksm_t b, ksm_t c);
ksm_t ksm_list4(ksm_t a, ksm_t b, ksm_t c, ksm_t d);
void ksm_extend_list(ksm_t cons, ksm_t ele);
ksm_t ksm_append(ksm_t list1, ksm_t list2);
#define ksm_caar(x)  (ksm_car(ksm_car(x)))
#define ksm_cadr(x)  (ksm_car(ksm_cdr(x)))
#define ksm_cdar(x)  (ksm_cdr(ksm_car(x)))
#define ksm_cddr(x)  (ksm_cdr(ksm_cdr(x)))

// cont.c
void ksm_init_cont(void);
int ksm_is_cont(ksm_t x);
//void ksm_install_cont_op(void *(*fun_create)(void), void (*fun_mark)(void *),
//	 void (*fun_free)(void *), void (*fun_restore)(void *));
//ksm_t ksm_cont_ret(ksm_t cont);
//void ksm_cont_set_ret(ksm_t cont, ksm_t val);
//ksm_t ksm_cont_jumps(ksm_t cont);
//jmp_buf *ksm_cont_regs(ksm_t cont);
//void ksm_cont_prep_stack_image(ksm_t cont);
//void ksm_cont_restore(ksm_t cont);
//extern int *ksm_cont_p;
//extern int *ksm_cont_q;
//extern unsigned ksm_cont_n;

// cready.c
int ksm_char_ready_p(int fd);

// define.c
void ksm_init_define(void);
int ksm_interp_define_args(ksm_t args, ksm_t env, ksm_t *key, ksm_t *val);
     // returns 0 if it is a define form, -1 if not
int ksm_interp_define(ksm_t form, ksm_t env, ksm_t *key, ksm_t *val);
     // returns 0 if it is a define form, -1 if not
void ksm_install_updater(int flag, void (*f)(ksm_t local, ksm_t val));
void ksm_enter_setter(ksm_t key, ksm_t setter);
void ksm_update_binding(ksm_t local, ksm_t val);

// delay.c
void ksm_init_delay(void);

// double.c
extern ksm_numrep_op ksm_double_op;

// dywin.c
void ksm_init_dywin(void);
ksm_t ksm_make_dywin(ksm_t before, ksm_t thunk, ksm_t after);
int ksm_is_dywin(ksm_t x);
ksm_t ksm_dywin_before(ksm_t dywin);
ksm_t ksm_dywin_thunk(ksm_t dywin);
ksm_t ksm_dywin_after(ksm_t dywin);

// env.c
void ksm_init_env(void);
ksm_t ksm_prepend_local(ksm_t key, ksm_t val, ksm_t local);
int ksm_is_local(ksm_t x);
ksm_t ksm_local_key(ksm_t local);
ksm_t ksm_local_value(ksm_t local);
ksm_t ksm_local_next(ksm_t local);
void ksm_set_local_value(ksm_t local, ksm_t value);
void ksm_set_local_value_no_lock(ksm_t local, ksm_t value);
int ksm_update_local(ksm_t key, ksm_t val, ksm_t local, ksm_t *next);
    // returns 0 if success, otherwise returns -1
ksm_t ksm_lookup_local(ksm_t key, ksm_t local, ksm_t *next);
     // returns 0 if not found
ksm_t ksm_search_local(ksm_t key, ksm_t local, ksm_t *next);
ksm_t ksm_make_global(size_t size, ksm_t next_global);
int ksm_enter_global(ksm_t key, ksm_t val, ksm_t global, int create);
// returns 0 if success
int ksm_is_global(ksm_t x);
ksm_t ksm_next_global(ksm_t global);
ksm_t ksm_lookup_global(ksm_t key, ksm_t global, ksm_t *next);
ksm_t ksm_lookup(ksm_t key, ksm_t env);  // returns 0 if not found
int ksm_update(ksm_t key, ksm_t val, ksm_t env); // returns 0 if success
void ksm_start_interact_env(void);
extern ksm_t ksm_report_env;
extern ksm_t ksm_interact_env;

// env.c (dict_impl)
typedef struct ksm_dict_impl_ {
  int size;
  ksm_t *tab;
  int entries;
} dict_impl;

typedef struct ksm_dict_impl_ ksm_dict_impl;
ksm_dict_impl *ksm_make_dict_impl(int size);
void ksm_dispose_dict_impl(ksm_dict_impl *p);
void ksm_mark_dict_impl(ksm_dict_impl *p);
ksm_t ksm_search_dict_impl(ksm_t key, dict_impl *p);
     // returns a local, or 0 if no such entry and !create
     // should be called with ksm_lock()-ed
void ksm_enter_dict_impl(ksm_t key, ksm_t val, dict_impl *p);
     // should be called with ksm_lock()-ed

// equal.c
void ksm_init_equal(void);
void ksm_install_equal(int flag, int (*f)(ksm_t, ksm_t));
int ksm_equal(ksm_t a, ksm_t b);
int ksm_eqv(ksm_t a, ksm_t b);
int ksm_memq(ksm_t a, ksm_t list);
int ksm_memv(ksm_t a, ksm_t list);
int ksm_member(ksm_t a, ksm_t list);
ksm_t ksm_assq(ksm_t key, ksm_t alist);
ksm_t ksm_assv(ksm_t key, ksm_t alist);
ksm_t ksm_assoc(ksm_t key, ksm_t alist);

// error.c
void ksm_fatal(char *fmt, ...);
extern void (*ksm_fatal_prolog)(void);
void ksm_error(char *fmt, ...);
extern void (*ksm_error_prolog)(void);
void ksm_flush_stdin(void);

// eval.c
ksm_t ksm_eval(ksm_t expr, ksm_t env);
void ksm_install_evaluator(int flag, ksm_pair (*f)(ksm_t expr, ksm_t env));
ksm_pair ksm_make_eval_pair(ksm_t expr, ksm_t env);
ksm_pair ksm_make_eval_done(ksm_t expr);
int ksm_is_eval_done(ksm_pair pair);
ksm_t ksm_eval_args(ksm_t args, ksm_t env);
ksm_pair ksm_eval_seq(ksm_t seq, ksm_t env);
ksm_t ksm_eval_pair(ksm_pair pair);

// except.c
void ksm_init_except(void);
ksm_t ksm_with_handler(ksm_t (*handler)(ksm_t ex, ksm_t args, void *aux),
		       ksm_t fun, void *aux);
ksm_t ksm_raise_except(ksm_t kind, ksm_t args);
extern ksm_t ksm_except;

// ext.c
void ksm_extension(void);
extern char *ksm_load_shared_file_name;

// jumps.c
void ksm_init_jumps(void);
ksm_t ksm_search_jumps(int (*pred)(ksm_t)); // returns #nil if not found
void ksm_sweep_jumps(void);
ksm_t ksm_rewind_jumps(ksm_t target); // returns #nil if not found
void ksm_push_jump(ksm_t jump);
void ksm_pop_jump(void);
void ksm_reset_jumps(void);
ksm_t ksm_fetch_jumps(void);
void ksm_set_jumps(ksm_t jump_list);

// init.c
void ksm_init(void);

// int.c
extern ksm_numrep_op ksm_int_op;

// key.c
void ksm_init_key(void);
ksm_t ksm_make_key(char *s);
int ksm_is_key(ksm_t x);
char *ksm_fetch_key(ksm_t key);
char *ksm_key_value(ksm_t x);
unsigned ksm_key_hash(ksm_t key);
unsigned ksm_key_hash_no_lock(ksm_t key);
unsigned ksm_hash(char *s);
int ksm_cvt_key_x(ksm_t x, ksm_t *result); // returns 0 if OK
ksm_t ksm_cvt_key(ksm_t x);
int ksm_fprintf_key(FILE *fp, ksm_t key, int alt);

// keyword.c
void ksm_init_keyword(void);
int ksm_is_keyword(ksm_t x);
ksm_t ksm_keyword_key(ksm_t kw);
ksm_t ksm_keyword_arg(ksm_t args, ksm_t key, ksm_t if_not_found);

// lambda.c
void ksm_init_lambda(void);
ksm_t ksm_make_lambda(ksm_t vars, ksm_t body, ksm_t env);
ksm_pair ksm_eval_body(ksm_t seq, ksm_t env);
extern ksm_t (*ksm_lambda_extender)(ksm_t vars, ksm_t args, ksm_t env);

// load.c
void ksm_init_load(void);
void ksm_load(ksm_t port);
ksm_t ksm_init_private_global(void);
  // returns private_global
//extern char *ksm_load_file_name;

// main.c
int *ksm_stack_base;
char *ksm_dir;
int ksm_new_flag(void);
extern int ksm_argc;
extern char **ksm_argv;

// number.c
void ksm_init_number(void);
ksm_t ksm_make_int(int i);
int ksm_is_int(ksm_t x);
int ksm_fetch_int(ksm_t x);
int ksm_int_value(ksm_t x);
int ksm_int_value_x(ksm_t x, int *result);
void ksm_install_intvalue(int flag, int (*f)(ksm_t, int *result));
int ksm_fprintf_int(FILE *fp, int i, int alt);
ksm_t ksm_integer_maker(char *s, int radix);
ksm_t ksm_make_double(double d);
int ksm_is_double(ksm_t x);
double ksm_fetch_double(ksm_t x);
double ksm_double_value(ksm_t x);
int ksm_double_value_x(ksm_t x, double *result);
void ksm_install_doublevalue(int flag, int (*f)(ksm_t x, double *result));
int ksm_fprintf_double(FILE *fp, double d, int alt);
ksm_t ksm_make_complex(double real, double imag);
ksm_t ksm_complex_maker(char *s);
ksm_t ksm_double_maker(char *s);
int ksm_get_si(ksm_t x);
double ksm_get_d(ksm_t x);
int ksm_exact_p(ksm_t x);
int ksm_inexact_p(ksm_t x);
ksm_t ksm_exact_to_inexact(ksm_t x);
ksm_t ksm_inexact_to_exact(ksm_t x);

#ifdef GMP
ksm_t ksm_make_mpz(mpz_ptr zp);
ksm_t ksm_mpz_maker(char *s, int radix);
ksm_t ksm_make_mpq(mpq_ptr qp);
ksm_t ksm_mpq_maker(char *s, int radix);
#endif // GMP

void ksm_install_numcvter(int flag, int (*cvter)(ksm_numrep *, ksm_t));
  // (*cvter)() returns 0 if success
int ksm_is_number(ksm_t x);
int ksm_number_eqv(ksm_t x, ksm_t y);
int ksm_set_numrep(ksm_numrep *p, ksm_t x);
  // returns 0 if success, non-zero if failure
void ksm_numrep_set_int(ksm_numrep *p, int i);
void ksm_numrep_set_longlong(ksm_numrep *p, long long ll);
void ksm_numrep_set_double(ksm_numrep *p, double d);
#define ksm_set_numrep_0(p)  ksm_numrep_set_int(p,0)
#define ksm_set_numrep_1(p)  ksm_numrep_set_int(p,1)
void ksm_numrep_promote(ksm_numrep *p, int numrep_kind);
void ksm_numrep_clone(ksm_numrep *p);
int ksm_numrep_exact_p(ksm_numrep *p);
int ksm_numrep_eq(ksm_numrep *p, ksm_numrep *q);
int ksm_numrep_lt(ksm_numrep *p, ksm_numrep *q);
int ksm_numrep_gt(ksm_numrep *p, ksm_numrep *q);
int ksm_numrep_le(ksm_numrep *p, ksm_numrep *q);
int ksm_numrep_ge(ksm_numrep *p, ksm_numrep *q);
void ksm_numrep_add(ksm_numrep *p, ksm_numrep *q);
void ksm_numrep_mul(ksm_numrep *p, ksm_numrep *q);
void ksm_numrep_sub(ksm_numrep *p, ksm_numrep *q);
void ksm_numrep_div(ksm_numrep *p, ksm_numrep *q);
void ksm_numrep_neg(ksm_numrep *p);
void ksm_numrep_quotient(ksm_numrep *p, ksm_numrep *q);
void ksm_numrep_remainder(ksm_numrep *p, ksm_numrep *q);
void ksm_numrep_modulo(ksm_numrep *p, ksm_numrep *q);
void ksm_numrep_gcd(ksm_numrep *p, ksm_numrep *q);
void ksm_numrep_lcm(ksm_numrep *p, ksm_numrep *q);
void ksm_numrep_and(ksm_numrep *p, ksm_numrep *q);
void ksm_numrep_or(ksm_numrep *p, ksm_numrep *q);
void ksm_numrep_xor(ksm_numrep *p, ksm_numrep *q);
void ksm_numrep_complement(ksm_numrep *p);
void ksm_numrep_lshift(ksm_numrep *p, int n);
void ksm_numrep_rshift(ksm_numrep *p, int n);
void ksm_numrep_floor(ksm_numrep *p);
void ksm_numrep_ceiling(ksm_numrep *p);
void ksm_numrep_truncate(ksm_numrep *p);
void ksm_numrep_round(ksm_numrep *p);
void ksm_numrep_exp(ksm_numrep *p);
void ksm_numrep_log(ksm_numrep *p);
void ksm_numrep_sin(ksm_numrep *p);
void ksm_numrep_cos(ksm_numrep *p);
void ksm_numrep_tan(ksm_numrep *p);
void ksm_numrep_asin(ksm_numrep *p);
void ksm_numrep_acos(ksm_numrep *p);
void ksm_numrep_atan(ksm_numrep *p);
void ksm_numrep_atan2(ksm_numrep *y, ksm_numrep *x);
void ksm_numrep_sqrt(ksm_numrep *p);
void ksm_numrep_expt(ksm_numrep *p, ksm_numrep *q);
void ksm_numrep_exact_to_inexact(ksm_numrep *p);
void ksm_numrep_inexact_to_exact(ksm_numrep *p);
int ksm_numrep_zero_p(ksm_numrep *p);
int ksm_numrep_positive_p(ksm_numrep *p);
int ksm_numrep_negative_p(ksm_numrep *p);
ksm_t ksm_numrep_to_object(ksm_numrep *p);

// numio.c
void ksm_init_numio(void);

// path.c
void ksm_init_path(void);
ksm_t ksm_path_append(ksm_t args);
ksm_t ksm_dir_part(char *path);

// perm.c
void ksm_init_permanent(void);
void ksm_permanent(ksm_t *p);

// port.c
void ksm_init_port(void);
int ksm_new_port_kind(void);
ksm_t ksm_make_input_port(ksm_port *ptr, int kind);
ksm_t ksm_make_output_port(ksm_port *ptr, int kind);
int ksm_is_port(ksm_t x);
int ksm_is_input_port(ksm_t x);
int ksm_is_output_port(ksm_t x);
void ksm_close_port(ksm_t port);
int ksm_port_kind(ksm_t port);
ksm_port *ksm_port_ptr(ksm_t port);
ksm_t ksm_make_input_file_port(FILE *fp);
ksm_t ksm_make_output_file_port(FILE *fp);
ksm_t ksm_make_string_input(char *str, int copy, int len);
  // if "copy" is true, "str" is copied
  // if "len" is negative, len is calculated by strlen(str)
int ksm_port_getc(ksm_t port);
int ksm_port_ungetc(int ch, ksm_t port);
int ksm_port_eof_p(ksm_t port);
int ksm_port_char_ready_p(ksm_t port);
int ksm_port_putc(char ch, ksm_t port);
int ksm_port_puts(char *s, ksm_t port);
int ksm_port_flush(ksm_t port);
  // returns -1 if error

// print.c
void ksm_init_print(void);
int ksm_fprintf(FILE *fp, ksm_t x, int alt);
int ksm_default_fprintf(FILE *fp, ksm_t x, int alt);
void ksm_install_fprintf(int flag, int (*f)(FILE *fp, ksm_t x, int alt));
extern int ksm_PA_OBJECT;

// quote.c
void ksm_init_quote(void);

// qq.c
void ksm_init_qq(void);

// read.c
ksm_t ksm_read_object(ksm_t port);
ksm_t (*ksm_read_toplevel_fun)(ksm_t port);
ksm_t ksm_read_toplevel(ksm_t port);
int ksm_is_ws(int ch);
int ksm_is_delim(int ch);
ksm_t ksm_eat_comment(ksm_t port);  // returns ksm_comment_object
ksm_t ksm_string_to_exact(char *s, int radix); // returns 0 if failure
ksm_t ksm_string_to_inexact(char *s);          // returns 0 if failure
ksm_t ksm_string_to_number(char *s, int radix);  // returns 0 if failure
char *ksm_stuff_lexeme(int ch, ksm_t port);  
  // if ch == '\0', ch is not stuffed
extern ksm_t (*ksm_list_reader)(ksm_t port);
extern ksm_t (*ksm_quote_reader)(ksm_t port);
extern ksm_t (*ksm_backquote_reader)(ksm_t port);
extern ksm_t (*ksm_comma_reader)(ksm_t port);
extern ksm_t (*ksm_string_reader)(ksm_t port);
extern ksm_t (*ksm_sharp_reader)(ksm_t port);
extern ksm_t (*ksm_colon_reader)(ksm_t port);
extern ksm_t (*ksm_semicolon_reader)(ksm_t port);
//extern ksm_t (*ksm_lbracket_reader)(ksm_t port);
//extern ksm_t (*ksm_rbracket_reader)(ksm_t port);
//extern ksm_t (*ksm_lbrace_reader)(ksm_t port);
//extern ksm_t (*ksm_rbrace_reader)(ksm_t port);
//extern ksm_t (*ksm_bar_reader)(ksm_t port);
void ksm_install_object_maker(ksm_t (*f)(char *));
  // first in, first invoked; (*f)() returns 0 if can't convert
void ksm_set_read_case_sensitivity(int case_sensitive_p);

// readline.c
char *ksm_readline(char *sep, ksm_t port, int *size);
  // returns an newly allocated string
  // allocated space can be freed by ksm_dispose
  // sep: delimiter string
  // if "size" is not NULL, string length is stored at *size

// sharp.c
void ksm_init_sharp(void);
void ksm_install_sharp_reader(int leading_char, 
			      ksm_t (*f)(char lead, ksm_t port));

// special.c
void ksm_init_special(void);
ksm_t ksm_make_imm(char *name);
extern ksm_t ksm_nil_object;
extern ksm_t ksm_eof_object;
extern ksm_t ksm_true_object;
extern ksm_t ksm_false_object;
extern ksm_t ksm_unspec_object;
extern ksm_t ksm_comment_object;
extern ksm_t ksm_rpar_object;
#define ksm_is_nil(x)      ((x)==ksm_nil_object)
#define ksm_is_eof(x)      ((x)==ksm_eof_object)
#define ksm_is_true(x)     ((x)==ksm_true_object)
#define ksm_is_false(x)    ((x)==ksm_false_object)
#define ksm_is_unspec(x)   ((x)==ksm_unspec_object)
#define ksm_is_comment(x)  ((x)==ksm_comment_object)
#define ksm_is_rpar(x)     ((x)==ksm_rpar_object)

// strbuf.c
void ksm_free_strbuf(ksm_strbuf *buf);
void ksm_strbuf_reserve(ksm_strbuf *buf, int size);
void ksm_strbuf_put(char ch, ksm_strbuf *buf);
char *ksm_strbuf_content(ksm_strbuf *buf, int *size);
  // if "size" is not NULL, string length is stored in *size
char *ksm_strbuf_extract(ksm_strbuf *buf, int *size);
  // if "size" is not NULL, string length is stored in *size
void ksm_strbuf_rewind(ksm_strbuf *buf);

// string.c
void ksm_init_string(void);
ksm_t ksm_make_string(char *s);
ksm_t ksm_make_string_no_copy(char *s);
ksm_t ksm_make_string_n(char *s, int n);
int ksm_is_string(ksm_t x);
ksm_t ksm_string_append(ksm_t list_of_strings);
char *ksm_string_value(ksm_t str);
int ksm_string_value_x(ksm_t x, char **result); // returns 0 if OK
ksm_t ksm_substring(char *s, int start, int end);
int ksm_fprintf_string(FILE *fp, char *str, int alt);
ksm_t ksm_read_string(ksm_t port, ksm_strbuf *bufp);

// syntax.c
void ksm_init_syntax(void);
ksm_t ksm_make_syntax(ksm_t literals, ksm_t rules, ksm_t env);
int ksm_is_syntax(ksm_t x);
ksm_t ksm_gensym(void);

// toplevel.c
void ksm_toplevel(ksm_t global_env);

// unicode.c
int ksm_unicode_isupper(int code);
int ksm_unicode_islower(int code);
int ksm_unicode_toupper(int code);
int ksm_unicode_tolower(int code);

// utf8.c
int ksm_utf8_coding_bytes(char start_byte);
     // returns 1, 2, or 3 if OK
     // returns -1 if invalid start_byte for UTF-8 encoding
int ksm_utf8_coded_bytes(char *buf);
     // returns 1, 2, or 3 if OK
     // returns -1 if invalid start_byte for UTF-8 encoding
int ksm_utf8_to_ucs4(char *bytes);
     // returns -1 if failure
int ksm_utf8_required_bytes(int ucs4_code);
     // returns 1, 2, or 3 if OK
     // returns -1 if invalid start_byte for UTF-8 encoding
int ksm_utf8_from_ucs4(char *dst, int ucs4_code);
     // returns number of bytes stored

// vector.c
void ksm_init_vector(void);
int ksm_is_vector(ksm_t x);
int ksm_vector_size(ksm_t vec);
ksm_t ksm_vector_ref(ksm_t vec, int index);
void ksm_vector_set(ksm_t vec, int index, ksm_t val);
ksm_t ksm_vector_to_list(ksm_t vec);
ksm_t ksm_list_to_vector(ksm_t list);

#define ksm_first(x)       ksm_car(x)
#define ksm_second(x)      ksm_car(ksm_cdr(x))
#define ksm_third(x)       ksm_car(ksm_cdr(ksm_cdr(x)))
#define ksm_fourth(x)      ksm_car(ksm_cdr(ksm_cdr(ksm_cdr(x))))
#define ksm_fifth(x)       ksm_car(ksm_cdr(ksm_cdr(ksm_cdr(ksm_cdr(x)))))

#define KSM_INSTALLER(name,ftypename) \
static ftypename name##s[KSM_FLAG_MAX];\
void ksm_install_##name(int flag, ftypename fun)\
{\
  if( !(flag >= 0 && flag < KSM_FLAG_MAX) )\
    ksm_fatal("too many object types");\
  name##s[flag] = fun;\
}\
static ftypename get_##name(int flag)\
{\
  return name##s[flag];\
}

#define KSM_INSTALLER_X(name,structname) \
static structname name##s[KSM_FLAG_MAX]; \
static void install_##name(int flag, structname *data) \
{ \
  if( !(flag >= 0 && flag < KSM_FLAG_MAX) ) \
    ksm_fatal("too many object types"); \
  memcpy(&name##s[flag], data, sizeof(structname)); \
} \
static structname *get_##name(int flag) \
{ \
  return &name##s[flag]; \
}

#define KSM_ENTER(str, val) \
  ksm_enter_global(ksm_make_key(str), val, ksm_interact_env, 1)

#define KSM_DEF_BLT(name) \
static ksm_t blt_##name(ksm_t args, ksm_t env)

#define KSM_ENTER_BLT(str, name) \
  ksm_enter_global(ksm_make_key(str), ksm_make_blt(blt_##name), \
                   ksm_interact_env, 1)

#define KSM_ENTER_BLT_SETTER(str, name) \
  ksm_enter_setter(ksm_make_key(str), ksm_make_blt(blt_##name))

#define KSM_DEF_SPC(name) \
static ksm_t spc_##name(ksm_t args, ksm_t env)

#define KSM_ENTER_SPC(str, name) \
  ksm_enter_global(ksm_make_key(str), ksm_make_spc(spc_##name), \
                   ksm_interact_env, 1)

#define KSM_ENTER_SPC_SETTER(str, name) \
  ksm_enter_setter(ksm_make_key(str), ksm_make_spc(spc_##name))

#define KSM_DEF_RSPC(name) \
static ksm_pair rspc_##name(ksm_t args, ksm_t env)

#define KSM_ENTER_RSPC(str,name) \
  ksm_enter_global(ksm_make_key(str), ksm_make_rspc(rspc_##name), \
                   ksm_interact_env, 1)

#define KSM_DEF_PRED(name, pred) \
KSM_DEF_BLT(name) \
{ \
  ksm_t x; \
 \
  KSM_SET_ARGS1(x); \
  return KSM_BOOL2OBJ(pred(x)); \
}

#define KSM_ENV()          env
#define KSM_ARGS()         args
#define KSM_NARG()         ksm_length(args)
#define KSM_NO_MORE_ARGS() \
            (ksm_is_nil(args) || ksm_is_keyword(ksm_car(args)))
#define KSM_MORE_ARGS()    (!KSM_NO_MORE_ARGS())
#define KSM_POP_ARG(var) \
            do{ if( KSM_NO_MORE_ARGS() ) \
                  ksm_error("too few args"); \
                if( !ksm_is_cons(args) ) \
                  ksm_error("can't happen in KSM_POP_ARG"); \
                var = ksm_car(args); \
                args = ksm_cdr(args); \
            } while(0)
#define KSM_POP_OPTARG(var, if_not_supplied) \
            do{ if( KSM_NO_MORE_ARGS() ) \
                  var = if_not_supplied; \
                else{ \
                  if( !ksm_is_cons(args) ) \
                    ksm_error("can't happen in KSM_POP_OPTARG"); \
                  var = ksm_car(args); \
                  args = ksm_cdr(args); \
                } \
            } while(0)
#define KSM_POP_KEYARG(var, key, if_not_supplied) \
            (var) = ksm_keyword_arg(args, key, if_not_supplied) 

#define KSM_SET_ARGS0() \
  do{ if(KSM_MORE_ARGS()) ksm_error("too many args: %k", args); } \
  while(0)
#define KSM_SET_ARGS1(arg1) KSM_POP_ARG(arg1); KSM_SET_ARGS0()
#define KSM_SET_ARGS2(arg1, arg2) \
                        KSM_POP_ARG(arg1); \
                        KSM_POP_ARG(arg2); \
                        KSM_SET_ARGS0()
#define KSM_SET_ARGS3(arg1, arg2, arg3) \
                        KSM_POP_ARG(arg1); \
                        KSM_POP_ARG(arg2); \
                        KSM_POP_ARG(arg3); \
                        KSM_SET_ARGS0()
#define KSM_SET_ARGS4(arg1, arg2, arg3, arg4) \
                        KSM_POP_ARG(arg1); \
                        KSM_POP_ARG(arg2); \
                        KSM_POP_ARG(arg3); \
                        KSM_POP_ARG(arg4); \
                        KSM_SET_ARGS0()
//#define KSM_FIRST_ARG()  ksm_car(KSM_ARGS())

#define KSM_BOOL2OBJ(bool) ((bool)?ksm_true_object:ksm_false_object)

#define KSM_CONFIRM_TYPE(pred, type_str, obj) \
  do{if(!pred(obj)) ksm_error("%s expected: %k", type_str, obj);} \
  while(0)

#define KSM_CONFIRM_INT(obj) \
                             KSM_CONFIRM_TYPE(ksm_is_int,"integer",obj)
#define KSM_CONFIRM_DOUBLE(obj) \
                             KSM_CONFIRM_TYPE(ksm_is_double,"real",obj)
#define KSM_CONFIRM_STRING(obj)  \
                             KSM_CONFIRM_TYPE(ksm_is_string,"string",obj)
#define KSM_CONFIRM_CHAR(obj)  \
                             KSM_CONFIRM_TYPE(ksm_is_char,"char",obj)
#define KSM_CONFIRM_CONS(obj) \
                             KSM_CONFIRM_TYPE(ksm_is_cons,"cons",obj)
#define KSM_CONFIRM_VEC(obj)  \
                             KSM_CONFIRM_TYPE(ksm_is_vec,"vector",obj)
#define KSM_CONFIRM_KEY(obj)  \
                             KSM_CONFIRM_TYPE(ksm_is_key,"symbol",obj)
#define KSM_CONFIRM_INPUT_PORT(obj)   \
                      KSM_CONFIRM_TYPE(ksm_is_input_port,"input port",obj)
#define KSM_CONFIRM_OUTPUT_PORT(obj)  \
                      KSM_CONFIRM_TYPE(ksm_is_output_port,"output port",obj)
#define KSM_CONFIRM_LIST(obj) \
  do{ if(!(ksm_is_nil(obj)||ksm_is_cons(obj))) \
        ksm_error("list expected: %k", obj); } \
  while(0) 
#define KSM_CONFIRM_DELAY(obj) \
                             KSM_CONFIRM_TYPE(ksm_is_delay,"delay",obj)

#if defined __linux__ && defined __powerpc__ && defined __GNUC__
#define KSM_LINUXPPC
#elif defined __linux__ && defined __i386__ && defined __GNUC__
#define KSM_I386
#endif

#endif
