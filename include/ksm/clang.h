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

#ifndef CLANG_H
#define CLANG_H

enum { KSM_CLANG_TYPE_INT,
       KSM_CLANG_TYPE_SINT,
       KSM_CLANG_TYPE_UINT,
       KSM_CLANG_TYPE_CHAR,
       KSM_CLANG_TYPE_SCHAR,
       KSM_CLANG_TYPE_UCHAR,
       KSM_CLANG_TYPE_SHORT,
       KSM_CLANG_TYPE_SSHORT,
       KSM_CLANG_TYPE_USHORT,
       KSM_CLANG_TYPE_FLOAT,
       KSM_CLANG_TYPE_DOUBLE,
       KSM_CLANG_TYPE_LONGLONG,
       KSM_CLANG_TYPE_SLONGLONG,
       KSM_CLANG_TYPE_ULONGLONG,
       KSM_CLANG_TYPE_VOID,
       KSM_CLANG_TYPE_DOTS,
       KSM_CLANG_TYPE_FUN, 
       KSM_CLANG_TYPE_POINTER, 
       KSM_CLANG_TYPE_ARRAY, 
       KSM_CLANG_TYPE_STRUCT, 
       KSM_CLANG_TYPE_UNION, 
       KSM_CLANG_TYPE_ENUM,
       KSM_CLANG_TYPE_LAST };

// clang.c
void ksm_init_clang(void);
int ksm_clang_is_csym(ksm_t x);
ksm_t ksm_clang_make_csym(ksm_t type, void *adr);
ksm_t ksm_clang_csym_type(ksm_t csym);
void *ksm_clang_csym_adr(ksm_t csym);
ksm_t ksm_clang_find_type(ksm_t name);
ksm_t ksm_clang_make_pointer(ksm_t pointer_type, void *pointer_value);
// e.g., int *ip; ksm_t ip_rep; 
// ip_rep = ksm_clang_make_pointer(
//            ksm_clang_type_pointer(ksm_clang_type_int), ip);
ksm_t ksm_clang_resolve_csym(ksm_t type, ksm_t ident);
ksm_t ksm_clang_modify_type(ksm_t base, ksm_t modifiers);
ksm_t ksm_clang_def_struct(ksm_t ident, ksm_t spec, int union_p);
ksm_t ksm_clang_aref(ksm_t csym, int index);
void ksm_clang_aset(ksm_t csym, int index, ksm_t val);

// type.c
void ksm_clang_init_type(void);
int ksm_clang_is_type(ksm_t x);
int ksm_clang_type_size(ksm_t t);
int ksm_clang_type_align(ksm_t t);
int ksm_clang_type_kind(ksm_t t);
extern ksm_t ksm_clang_type_int;
extern ksm_t ksm_clang_type_sint;
extern ksm_t ksm_clang_type_uint;
extern ksm_t ksm_clang_type_char;
extern ksm_t ksm_clang_type_schar;
extern ksm_t ksm_clang_type_uchar;
extern ksm_t ksm_clang_type_short;
extern ksm_t ksm_clang_type_sshort;
extern ksm_t ksm_clang_type_ushort;
extern ksm_t ksm_clang_type_float;
extern ksm_t ksm_clang_type_double;
extern ksm_t ksm_clang_type_longlong;
extern ksm_t ksm_clang_type_slonglong;
extern ksm_t ksm_clang_type_ulonglong;
extern ksm_t ksm_clang_type_void;
extern ksm_t ksm_clang_type_dots;
#define ksm_clang_type_is_int(t) ((t)==ksm_clang_type_int)
#define ksm_clang_type_is_sint(t) ((t)==ksm_clang_type_sint)
#define ksm_clang_type_is_uint(t) ((t)==ksm_clang_type_uint)
#define ksm_clang_type_is_char(t) ((t)==ksm_clang_type_char)
#define ksm_clang_type_is_schar(t) ((t)==ksm_clang_type_schar)
#define ksm_clang_type_is_uchar(t) ((t)==ksm_clang_type_uchar)
#define ksm_clang_type_is_short(t) ((t)==ksm_clang_type_short)
#define ksm_clang_type_is_sshort(t) ((t)==ksm_clang_type_sshort)
#define ksm_clang_type_is_ushort(t) ((t)==ksm_clang_type_ushort)
#define ksm_clang_type_is_float(t) ((t)==ksm_clang_type_float)
#define ksm_clang_type_is_double(t) ((t)==ksm_clang_type_double)
#define ksm_clang_type_is_longlong(t) ((t)==ksm_clang_type_longlong)
#define ksm_clang_type_is_slonglong(t) ((t)==ksm_clang_type_slonglong)
#define ksm_clang_type_is_ulonglong(t) ((t)==ksm_clang_type_ulonglong)
#define ksm_clang_type_is_void(t) ((t)==ksm_clang_type_void)
#define ksm_clang_type_is_dots(t) ((t)==ksm_clang_type_dots)
ksm_t ksm_clang_type_fun(ksm_t ret_type);
int ksm_clang_type_is_fun(ksm_t t);
void ksm_clang_type_fun_para(ksm_t t, ksm_t name, ksm_t type);
int ksm_clang_type_fun_npara(ksm_t t);
void ksm_clang_type_fun_ipara(ksm_t t, int i, ksm_t *name, ksm_t *type);
void ksm_clang_type_fun_set_ptype(ksm_t t, ksm_t name, ksm_t type);
void ksm_clang_type_fun_sweep(ksm_t t);
ksm_t ksm_clang_type_fun_ret(ksm_t t);
void ksm_clang_type_fun_set_dots(ksm_t t);
int ksm_clanng_type_fun_has_dots(ksm_t t);
void ksm_clang_type_fun_set_upar(ksm_t t);
int ksm_clanng_type_fun_has_upar(ksm_t t);
int ksm_clang_type_fun_eq(ksm_t a, ksm_t b);
ksm_t ksm_clang_type_pointer(ksm_t pointee_type);
int ksm_clang_type_is_pointer(ksm_t t);
ksm_t ksm_clang_type_pointer_pointee(ksm_t t);
int ksm_clang_type_pointer_eq(ksm_t p1, ksm_t p2);
ksm_t ksm_clang_type_array(ksm_t ele_type, int size);
int ksm_clang_type_is_array(ksm_t t);
int ksm_clang_type_array_size(ksm_t t);
ksm_t ksm_clang_type_array_ele(ksm_t array_type);
int ksm_clang_type_array_eq(ksm_t a, ksm_t b);
ksm_t ksm_clang_type_struct(ksm_t name);
ksm_t ksm_clang_type_union(ksm_t name);
int ksm_clang_type_is_struct(ksm_t t);
int ksm_clang_type_is_union(ksm_t t);
int ksm_clang_type_struct_is_incomplete(ksm_t t);
int ksm_clang_type_struct_add_field(ksm_t t, ksm_t name, ksm_t type, 
				    int cur_ofs);
  // returns next offset
void ksm_clang_type_struct_add_bit_field(ksm_t t, ksm_t name,
   ksm_t field_type, int offset, int bit_offset, int bit_length);
void ksm_clang_type_struct_end_pad(ksm_t t);
int ksm_clang_type_struct_nfield(ksm_t t);
void ksm_clang_type_struct_ifield(ksm_t t, int i, ksm_t *name, ksm_t *type,
				  int *offset);
int ksm_clang_type_struct_search(ksm_t t, ksm_t name, ksm_t *type,
   int *offset, int *left_shift, int *right_shift);
   // returns 0 if found, -1 if not found 
void ksm_clang_type_struct_set_fields(ksm_t struct_type_dst,
				      ksm_t struct_type_src);
int ksm_clang_type_struct_eq(ksm_t struct_type, ksm_t x);
ksm_t ksm_clang_type_enum(void);
int ksm_clang_type_is_enum(ksm_t x);
int ksm_clang_type_enum_add(ksm_t t, ksm_t key); // returns value
void ksm_clang_type_enum_addx(ksm_t t, ksm_t key, int value);
int ksm_clang_type_enum_includes(ksm_t t, ksm_t key);
int ksm_clang_type_equal(ksm_t ta, ksm_t tb);

// callframe.c
struct ksm_clang_callframe;
void ksm_clang_init_callframe(struct ksm_clang_callframe *p, void (*f)());
void ksm_clang_arg_int(struct ksm_clang_callframe *p, int i);
void ksm_clang_arg_double(struct ksm_clang_callframe *p, double d);
void ksm_clang_arg_float(struct ksm_clang_callframe *p, double val);
void ksm_clang_set_vararg(struct ksm_clang_callframe *p);
void ksm_clang_call(struct ksm_clang_callframe *p);

int ksm_clang_ret_int(struct ksm_clang_callframe *p);
double ksm_clang_ret_double(struct ksm_clang_callframe *p);

#endif
