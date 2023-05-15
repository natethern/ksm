%{
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
#include "ksm/clang.h"
#define YYSTYPE ksm_t
#define YYPARSE_PARAM store_p
int ksm_clike_error(char *);
extern ksm_t ksm_clike_lval;
int ksm_clike_lex(void);
extern ksm_t ksm_clike_simple_string(void);
extern ksm_t ksm_clike_complex_string(void); 
extern ksm_t ksm_clike_lex_char(void);
extern ksm_t ksm_clike_lex_unicode(void);
extern ksm_t ksm_clike_lex_ident(void);
extern ksm_t ksm_clike_decimal_number(void);
extern ksm_t ksm_clike_octal_number(void);
extern ksm_t ksm_clike_hex_number(void);
extern ksm_t ksm_clike_binary_number(void);
extern ksm_t ksm_clike_float_number(void);
char *ksm_clike_error_string;
#define NIL ksm_nil_object
#define TRUE ksm_true_object
#define FALSE ksm_false_object
#define UNSPEC ksm_unspec_object
#define SYM(s)  ksm_make_key(s)
#define CONS(a,b) ksm_cons(a,b)
#define L1(a)     ksm_cons(a,ksm_nil_object)
#define L2(a,b)   ksm_list2(a,b)
#define L3(a,b,c) ksm_list3(a,b,c)
#define L4(a,b,c,d) ksm_list4(a,b,c,d)
#define L5(a,b,c,d,e) ksm_cons(a,ksm_list4(b,c,d,e))
#define LC3(a,b,c) ksm_cons(a,ksm_cons(b,c))
#define LC4(a,b,c,d) ksm_cons(a,ksm_cons(b,ksm_cons(c,d)))
#define Let    SYM("let")
#define Not    SYM("not")
#define Do     SYM("do")
#define Begin  SYM("begin")
#define YYERROR_VERBOSE 1
static ksm_t define_expr(ksm_t pair);
%}
%token IDENT
%token SIMPLE_STRING COMPLEX_STRING
%token CHAR CHAR_SPACE CHAR_NEWLINE CHAR_UNICODE
%token DECIMAL_NUMBER OCTAL_NUMBER HEX_NUMBER BINARY_NUMBER
%token FLOAT_NUMBER IMAGINARY_NUMBER
%token TRUE_OBJ FALSE_OBJ
%token LE GE EQ NE
%token AND OR
%token INC DEC
%token LSHIFT RSHIFT
%token MUL_ASSIGN DIV_ASSIGN REM_ASSIGN ADD_ASSIGN SUB_ASSIGN
%token LSHIFT_ASSIGN RSHIFT_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN
%token VAR FUN
%token IF WHILE DO FOR
%token RETURN BREAK
%token COND CASE
%token EXTERN
%token VOID CHAR SHORT INT LONG FLOAT DOUBLE SIGNED UNSIGNED
%token DOTS
%token STRUCT UNION TYPEDEF
%token ARROW
%token SIZEOF
%token AS

%nonassoc LOWER_THAN_ELSE 
%nonassoc ELSE
%%
prog: expr  
        { 
          *(ksm_t *)store_p = $1; 
        }
      | toplevel_stmt
          { 
            *(ksm_t *)store_p = $1; 
            YYACCEPT; 
          }
;

toplevel_stmt: expr ';' { $$ = $1; }
      | vardef { $$ = CONS(SYM("clike:define"),$1); }
      | fundef { $$ = $1; }
      | block  { $$ = $1; }
      | if_stmt { $$ = $1; }
      | while_stmt { $$ = $1; }
      | do_stmt { $$ = $1; }
      | for_stmt { $$ = $1; }
      | cond_stmt { $$ = $1; }
      | case_stmt { $$ = $1; }
      | declaration ';' { $$ = $1; }
;

stmt: expr ';' { $$ = $1; }
      | block { $$ = $1; }
      | if_stmt { $$ = $1; }
      | while_stmt { $$ = $1; }
      | do_stmt { $$ = $1; }
      | for_stmt { $$ = $1; }
      | return_stmt { $$ = $1; }
      | break_stmt { $$ = $1; }
      | cond_stmt { $$ = $1; }
      | case_stmt { $$ = $1; }
;

stmtlst: stmt { $$ = L1($1); }
      | stmt stmtlst { $$ = CONS($1, $2); }
;

vardef: VAR vardecltor_list ';' { $$ = $2; }
;

vardecltor: ident { $$ = CONS($1,UNSPEC); }
      | ident '=' assign_expr { $$ = CONS($1,$3); }
      | ident '[' const_expr ']'
          { $$ = CONS($1,L2(SYM("make-vector"),$3)); }
;

vardecltor_list: vardecltor { $$ = L1($1); }
      | vardecltor ',' vardecltor_list { $$ = CONS($1,$3); }
;

fundef: FUN ident '(' identlst ')' block
          { $$ = L3(SYM("define"),
                    CONS($2, $4),
                    L2(SYM("clike:with-return"),
                       L3(SYM("lambda"), NIL, $6))); } 
          /* (define (ident identlst) 
                     (clike:with-return 
                       (lambda () (let () ...)))) */
      | FUN ident '(' ')' block
          { $$ = LC3(SYM("define"),L1($2),$5); }
;

blockbody: stmtlst { $$ = $1; }
      | vardef blockbody 
          { $$ = ksm_append(ksm_map(define_expr,$1),$2); }
;

block: '{' blockbody '}' 
       { $$ = LC3(SYM("let"),ksm_nil_object,$2); }
;

identlst: ident { $$ = L1($1); }
      | ident ',' identlst { $$ = CONS($1,$3); }
;

if_stmt: IF '(' expr ')' stmt %prec LOWER_THAN_ELSE
           { $$ = L3(SYM("if"),L2(SYM("clike:true"),$3),$5); } 
      | IF '(' expr ')' stmt ELSE stmt 
          { $$ = L4(SYM("if"),L2(SYM("clike:true"),$3),$5,$7); }
;

while_stmt: WHILE '(' expr ')' stmt
              { $$ = L2(SYM("clike:with-break"),
	                L3(SYM("lambda"),
			   NIL,
			   L4(SYM("do"), NIL,
			      L1(L2(SYM("clike:false"), $3)),
			      $5))); }
	      /* (clike:with-break
                   (lambda () (do () ((clike:false expr)) stmt))) */

do_stmt: DO stmt WHILE '(' expr ')' ';'
           { ksm_t p = ksm_gensym();
             $$ = L2(SYM("clike:with-break"),
	             L3(SYM("lambda"), NIL,
		        L4(SYM("do"),
			   L1(L3(p,FALSE,L2(SYM("clike:false"),$5))),
			   L1(p), $2))); }
	   /* (clike:with-break 
	        (lambda () (do ((p #f (clike:false expr))) (p) stmt))) */

for_stmt: FOR '(' expr ';' expr ';' expr ')' stmt
	  { $$ = L2(SYM("clike:with-break"),
                 L4(SYM("lambda"),NIL,$3,
                 L5(SYM("do"),NIL,L1(L2(SYM("clike:false"),$5)),$9,$7))); }
    /* (clike:with-break
	 (lambda () expr1 (do () ((clike:false expr2)) stmt expr3))) */

return_stmt: RETURN expr ';' { $$ = L2(SYM("clike:return"), $2); }
;

break_stmt: BREAK ';' { $$ = L1(SYM("clike:break")); }
;

cond_stmt: COND '{' cond_clause_list '}'
        { $$ = CONS(SYM("cond"),$3); }
     /* (cond (pred stmt) ...) */

cond_clause: expr ':' stmt { $$ = L2(L2(SYM("clike:true"),$1),$3); }
     /* (prim_expr stmt) */
      | ELSE ':' stmt { $$ = L2(SYM("else"),$3); }
;

cond_clause_list: cond_clause { $$ = L1($1); }
      | cond_clause cond_clause_list { $$ = CONS($1,$2); }
;

case_stmt : CASE '(' expr ')' '{' case_clause_list '}'
      { $$ = LC3(SYM("case"),$3,$6); }
      /* (case expr . case_clause_list) */
;

case_clause: assign_expr_list ':' stmt 
      { $$ = L2($1,$3); }
      /* (assign_expr_list stmt) */
      | ELSE ':' stmt { $$ = L2(SYM("else"),$3); }
;

case_clause_list: case_clause { $$ = L1($1); }
      | case_clause case_clause_list { $$ = CONS($1,$2); }
;

declaration: EXTERN type_spec decltor
    { $$ = L3(SYM("define"),ksm_cdr($3),
       ksm_clang_resolve_csym(ksm_clang_modify_type($2,ksm_car($3)),
                              ksm_cdr($3))); }
    /* (define ident csym) */
      | EXTERN type_spec decltor AS ident
        /* (define ident csym) */
        { ksm_t type = ksm_clang_modify_type($2, ksm_car($3));
          $$ = L3(SYM("define"), $5, 
                  ksm_clang_resolve_csym(type, ksm_cdr($3))); }
      | struct_spec { $$ = $1; }
      | TYPEDEF type_spec decltor 
    /* (clang:typedef type ident ) */
        { ksm_t t = ksm_clang_modify_type($2,ksm_car($3));
          $$ = L3(SYM("clang:typedef"),t,ksm_cdr($3)); }
;

type_spec: VOID { $$ = ksm_clang_type_void; }
      | CHAR { $$ = ksm_clang_type_char; }
      | SIGNED CHAR { $$ = ksm_clang_type_schar; }
      | UNSIGNED CHAR { $$ = ksm_clang_type_uchar; }
      | SHORT { $$ = ksm_clang_type_short; }
      | SIGNED SHORT { $$ = ksm_clang_type_sshort; }
      | UNSIGNED SHORT { $$ = ksm_clang_type_ushort; }
      | INT { $$ = ksm_clang_type_int; }
      | SIGNED INT { $$ = ksm_clang_type_sint; }
      | UNSIGNED INT { $$ = ksm_clang_type_uint; }
      | LONG { $$ = ksm_clang_type_int; }
      | SIGNED LONG { $$ = ksm_clang_type_sint; }
      | UNSIGNED LONG { $$ = ksm_clang_type_uint; }
      | FLOAT { $$ = ksm_clang_type_float; }
      | DOUBLE { $$ = ksm_clang_type_double; }
      | struct_spec { $$ = $1; }
      | union_spec { $$ = $1; }
      | ident { $$ = ksm_clang_find_type($1); }
;

struct_spec: STRUCT '{' struct_decl_list '}' 
    { $$ = ksm_clang_def_struct(UNSPEC,$3,0); }
      | STRUCT ident '{' struct_decl_list '}' 
    { $$ = ksm_clang_def_struct($2,$4,0); }
      | STRUCT ident { $$ = ksm_clang_def_struct($2,NIL,0); }
;

union_spec: UNION '{' struct_decl_list '}' 
    { $$ = ksm_clang_def_struct(UNSPEC,$3,1); }
      | UNION ident '{' struct_decl_list '}' 
    { $$ = ksm_clang_def_struct($2,$4,1); }
      | UNION ident { $$ = ksm_clang_def_struct($2,NIL,1); }
;

struct_decl_list: struct_decl { $$ = L1($1); }
      | struct_decl struct_decl_list { $$ = CONS($1,$2); }
;

struct_decl: type_spec decltor ';'
    { ksm_t t = ksm_clang_modify_type($1,ksm_car($2));
      $$ = L2(t, ksm_cdr($2)); }
      | type_spec decltor ':' const_expr ';'
    { ksm_t t = ksm_clang_modify_type($1,ksm_car($2));
      $$ = L3(t, ksm_cdr($2), $4); }
;

decltor: direct_decltor { $$ = $1; }
    /* (modifiers . ident) */
      | pointer direct_decltor 
    { $$ = CONS(ksm_append($1,ksm_car($2)),ksm_cdr($2)); }
;

direct_decltor: ident { $$ = CONS(NIL,$1); }
      | '(' decltor ')' { $$ = $2; }
      | direct_decltor '[' const_expr ']' 
    { $$ = CONS(CONS(L2(SYM("[]"),$3),ksm_car($1)),ksm_cdr($1)); }
      | direct_decltor '[' ']' 
    { $$ = CONS(CONS(L2(SYM("[]"),ksm_make_int(0)),ksm_car($1)),
                ksm_cdr($1)); }
      | direct_decltor '(' para_type_list ')'
    { $$ = CONS(CONS($3,ksm_car($1)),ksm_cdr($1)); }
;

pointer: '*' { $$ = L1(SYM("*")); }
      | '*' pointer { $$ = CONS(SYM("*"),$2); }
;

para_type_list: parameter_list { $$ = $1; }
      | parameter_list ',' DOTS 
    { $$ = ksm_append($1,L1(ksm_clang_type_dots)); }
;

parameter_list: para_decl { $$ = L1($1); }
      | parameter_list ',' para_decl { $$ = ksm_append($1,L1($3)); }
;

para_decl: type_spec decltor 
    { $$ = ksm_clang_modify_type($1,ksm_car($2)); }
      | type_spec abstract_decltor
    { $$ = ksm_clang_modify_type($1,$2); }
      | type_spec { $$ = $1; }
;

abstract_decltor: pointer { $$ = $1; }
      | direct_abstract_decltor { $$ = $1; }
      | pointer direct_abstract_decltor 
    { $$ = ksm_append($1,$2); }
;

direct_abstract_decltor: '(' abstract_decltor ')' { $$ = $1; }
      | '[' ']' { $$ = L1(L2(SYM("[]"),ksm_make_int(0))); }
      | '[' const_expr ']' { $$ = L1(L2(SYM("[]"),$2)); }     
      | direct_abstract_decltor '[' ']'
    { $$ = CONS(L1(L2(SYM("[]"),ksm_make_int(0))),$1); }
      | direct_abstract_decltor '[' const_expr ']'
    { $$ = CONS(L1(L2(SYM("[]"),$3)),$1); }
      | direct_abstract_decltor '(' para_type_list ')' 
	{ $$ = CONS($3,$1); } 
;

expr: assign_expr { $$ = $1; }
      | expr ',' assign_expr { $$ = L3(Begin,$1,$3); }
;

assign_expr: logical_or_expr { $$ = $1; }
      | unary_expr '=' assign_expr
          { $$ = L3(SYM("set-value!"),$1,$3); }
      | unary_expr MUL_ASSIGN assign_expr
          { $$ = L3(SYM("clike:mul-assign"),$1,$3); }
      | unary_expr DIV_ASSIGN assign_expr
          { $$ = L3(SYM("clike:div-assign"),$1,$3); }
      | unary_expr REM_ASSIGN assign_expr
          { $$ = L3(SYM("clike:rem-assign"),$1,$3); }
      | unary_expr ADD_ASSIGN assign_expr
          { $$ = L3(SYM("clike:add-assign"),$1,$3); }
      | unary_expr SUB_ASSIGN assign_expr
          { $$ = L3(SYM("clike:sub-assign"),$1,$3); }
      | unary_expr LSHIFT_ASSIGN assign_expr
          { $$ = L3(SYM("clike:lshift-assign"),$1,$3); }
      | unary_expr RSHIFT_ASSIGN assign_expr
          { $$ = L3(SYM("clike:rshift-assign"),$1,$3); }
      | unary_expr AND_ASSIGN assign_expr
          { $$ = L3(SYM("clike:and-assign"),$1,$3); }
      | unary_expr XOR_ASSIGN assign_expr
          { $$ = L3(SYM("clike:xor-assign"),$1,$3); }
      | unary_expr OR_ASSIGN assign_expr
          { $$ = L3(SYM("clike:or-assign"),$1,$3); }
;

const_expr: logical_or_expr { $$ = $1; }
;

logical_or_expr: logical_and_expr { $$ = $1; }
      | logical_or_expr OR logical_and_expr
          { $$ = L3(SYM("or"),$1,$3); }
;

logical_and_expr: or_expr { $$ = $1; }
      | logical_and_expr AND or_expr { $$ = L3(SYM("and"),$1,$3); }
;

or_expr: xor_expr { $$ = $1; }
      | or_expr '|' xor_expr 
        { $$ = L3(SYM("logical-or"),$1,$3); }
;

xor_expr: and_expr { $$ = $1; }
      | xor_expr '^' and_expr
          { $$ = L3(SYM("logical-xor"),$1,$3); }
;

and_expr: equality_expr { $$ = $1; }
      | and_expr '&' equality_expr 
          { $$ = L3(SYM("logical-and"),$1,$3); }
;

equality_expr: relation_expr { $$ = $1; }
      | equality_expr EQ relation_expr 
          { $$ = L3(SYM("equal?"),$1,$3); }
      | equality_expr NE relation_expr
          { $$ = L2(SYM("not"),L3(SYM("equal?"),$1,$3)); }
;

relation_expr: shift_expr { $$ = $1; }
      | relation_expr '<' shift_expr { $$ = L3(SYM("<"),$1,$3); }
      | relation_expr '>' shift_expr { $$ = L3(SYM(">"),$1,$3); }
      | relation_expr LE shift_expr { $$ = L3(SYM("<="),$1,$3); }
      | relation_expr GE shift_expr { $$ = L3(SYM(">="),$1,$3); }
;

shift_expr: additive_expr { $$ = $1; }
      | shift_expr LSHIFT additive_expr 
          { $$ = L3(SYM("logical-lshift"),$1,$3); }
      | shift_expr RSHIFT additive_expr 
          { $$ = L3(SYM("logical-rshift"),$1,$3); }
;

additive_expr: multiplic_expr { $$ = $1; }
      | additive_expr '+' multiplic_expr { $$ = L3(SYM("+"),$1,$3); }
      | additive_expr '-' multiplic_expr { $$ = L3(SYM("-"),$1,$3); }
;

multiplic_expr: cast_expr { $$ = $1; }
      | multiplic_expr '*' cast_expr { $$ = L3(SYM("*"),$1,$3); }
      | multiplic_expr '/' cast_expr { $$ = L3(SYM("/"),$1,$3); }
      | multiplic_expr '%' cast_expr { $$ = L3(SYM("modulo"),$1,$3); }
;

cast_expr: unary_expr { $$ = $1; }
;

unary_expr: postfix_expr { $$ = $1; }
      | '-' cast_expr { $$ = L2(SYM("-"),$2); }
      | '+' cast_expr { $$ = L2(SYM("+"),$2); }
      | '~' cast_expr { $$ = L2(SYM("logical-complement"),$2); }
      | '!' cast_expr { $$ = L2(SYM("not"),$2); }
      | INC unary_expr { $$ = L2(SYM("clike:prefix-inc"),$2); }
      | DEC unary_expr { $$ = L2(SYM("clike:prefix-dec"),$2); }
      | '*' cast_expr { $$ = L2(SYM("clang:*"),$2); }
      | '&' cast_expr { $$ = L2(SYM("clang:&"),$2); }
;

postfix_expr: prim_expr { $$ = $1; }
      | postfix_expr '(' assign_expr_list ')'
          { $$ = CONS($1,$3); }
      | postfix_expr '(' ')' { $$ = L1($1); }
      | postfix_expr '[' expr ']' 
          { $$ = L3(SYM("clike:aref"),$1,$3); }
      | postfix_expr INC { $$ = L2(SYM("clike:postfix-inc"), $1); }
      | postfix_expr DEC { $$ = L2(SYM("clike:postfix-dec"), $1); }
      | postfix_expr '.' ident  /* (clang:. csym field) */
          { $$ = L3(SYM("clang:."),$1,L2(SYM("quote"),$3)); }
      | postfix_expr ARROW ident /* (clang:-> csym field) */
          { $$ = L3(SYM("clang:->"),$1,L2(SYM("quote"),$3)); }
;

prim_expr: literal { $$ = $1; }
      | ident { $$ = $1; }
      | '(' expr ')' { $$ = $2; }
      | FUN '(' identlst ')' '{' blockbody '}'
        { $$ = L3(SYM("lambda"),$3,
                  L2(SYM("clike:with-return"),
                     LC3(SYM("lambda"),NIL,$6))); }
    /* (lambda (identlst) 
         (clike:with-return (lambda () . blockbody))) */
      | FUN '(' ')' '{' blockbody '}'
        { $$ = L3(SYM("lambda"),NIL,
                  L2(SYM("clike:with-return"),
                     LC3(SYM("lambda"),NIL,$5))); }
    /* (lambda ()
         (clike:with-return (lambda () . blockbody))) */
      | '[' assign_expr_list ']' { $$ = $2; }
      | '[' ']' { $$ = NIL; }
      | '#' '[' assign_expr_list ']'
         { $$ = ksm_list_to_vector($3); }
      | '\'' prim_expr { $$ = L2(SYM("quote"),$2); }
      | SIZEOF '(' type_spec ')' 
          { $$ = L2(SYM("clike:sizeof"), $3); }
      | SIZEOF '(' type_spec abstract_decltor ')'
          { $$ = L2(SYM("clike:sizeof"), ksm_clang_modify_type($3,$4)); }
;

ident: IDENT { $$ = ksm_clike_lex_ident(); }
;

literal: SIMPLE_STRING { $$ = ksm_clike_simple_string(); }
      | COMPLEX_STRING { $$ = ksm_clike_complex_string(); }
      | CHAR { $$ = ksm_clike_lex_char(); }
      | CHAR_SPACE { $$ = ksm_make_char(' '); }
      | CHAR_NEWLINE { $$ = ksm_make_char('\n'); }
      | CHAR_UNICODE { $$ = ksm_clike_lex_unicode(); }
      | DECIMAL_NUMBER { $$ = ksm_clike_decimal_number(); }     
      | OCTAL_NUMBER { $$ = ksm_clike_octal_number(); }     
      | HEX_NUMBER { $$ = ksm_clike_hex_number(); }     
      | BINARY_NUMBER { $$ = ksm_clike_binary_number(); }     
      | FLOAT_NUMBER { $$ = ksm_clike_float_number(); }     
      | TRUE_OBJ { $$ = $1; }
      | FALSE_OBJ { $$ = $1; }
;

assign_expr_list: assign_expr { $$ = L1($1); }
      | assign_expr ',' assign_expr_list { $$ = CONS($1,$3); }

%%
int ksm_clike_error(char *s)
{
  ksm_clike_error_string = s;
  return 0;
}

int ksm_clike_yyempty = YYEMPTY;
int ksm_clike_yyeof = YYEOF;

static
ksm_t define_expr(ksm_t pair)
{
  if( !ksm_is_cons(pair) )
    ksm_error("can't happen (define_expr in parse.y)");
  return L3(SYM("define"), ksm_car(pair), ksm_cdr(pair));
}
