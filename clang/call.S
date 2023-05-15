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

#if defined __linux__ && defined __powerpc__
	.align 2
	.section	".text"
	.global ksm_clang_call_glue
	.type ksm_clang_call_glue,@function

/***
# struct ret_storage {
#   double dval;
#   union {
#     long long llval;
#     int ival;
#   } u;
# };
#
# struct call_data {
#   void (*fun)(void);    // 0
#   int *gpr;             // 4
#   int ngpr;             // 8
#   double *fpr;          // 12
#   int nfpr;             // 16
#   int *spill;           // 20
#   int nspill;           // 24
#   struct ret_storage *store; // 28
#   int vararg;           // 32
# };
# 
# void call_glue(struct call_data *data);
#
# lwz 10,0(1)
# lwz 10,-4(10)  --> r10 contains original value of r3
***/

ksm_clang_call_glue:
	lwz 4,24(3)    # nspill
	andi. 10,4,1
	add 4,4,10
	mulli 4,4,-4
	addi 4,4,-16   # adjust for SPsave, LRsave, ..., r3save	
	stwux 1,1,4
	lwz 10,0(1)
	mflr 0
	stw 0,4(10)
	stw 3,-4(10)
	lwz 0,0(3)
	mtlr 0
	lwz 5,16(3)    # nfpr
	cmpwi 0,5,0
	beq L4
	lwz 4,12(3)    # fpr
	lfd 8,56(4)
	lfd 7,48(4)
	lfd 6,40(4)
	lfd 5,32(4)
	lfd 4,24(4)
	lfd 3,16(4)
	lfd 2,8(4)
	lfd 1,0(4)
L4:
	lwz 4,24(3)    # nspill
	cmpwi 0,4,0
	beq L2
	mtctr 4
	lwz 5,20(3)    # spill (src)
	addi 6,1,8     # (dst)
L3:
	lwz 7,0(5)
	stw 7,0(6)
	bdz L2
	addi 5,5,4
	addi 6,6,4
	b L3
L2:
	lwz 4,32(3)    # vararg
	cmpwi 0,4,0
	beq L1
	lwz 4,16(3)    # nfpr
	cmpwi 0,4,0
	beq L1
	creqv 6,6,6
	b L0
L1:
	crxor 6,6,6
L0:		
	lwz 3,4(3)     # gpr
	lwz 10,28(3)
	lwz 9,24(3)
	lwz 8,20(3)
	lwz 7,16(3)
	lwz 6,12(3)
	lwz 5,8(3)
	lwz 4,4(3)
	lwz 3,0(3)
	blrl
	lwz 10,0(1)
	lwz 9,-4(10)
	lwz 8,28(9)
	stfd 1,0(8)
	stw 3,8(8)
	stw 4,12(8)
	lwz 10,0(1)
	lwz 0,4(10)
	mtlr 0
	lwz 1,0(1)
	blr
.Lfe1:
	.size ksm_clang_call_glue,.Lfe1-ksm_call_glue
#elif defined __linux__ && defined __i386__ 
/***	
#  struct arg_unit {  // size: 16
#    int flag;   // [0] 0: integer, 1: double
#    int rsrv;   // [4] reserved
#    union {     // [8]
#      double dval;  
#      int    ival;
#    } u;
#  };

#  struct call_frame {
#    double dret;    // [0]
#    int    iret;    // [8]
#    void (*fun)();  // [12]
#    int    narg;    // [16]
#    struct arg_unit args[MAX_NARG];  // [20]
#  };
***/
.text
	.align 4
.globl ksm_clang_call_glue
	.type	 ksm_clang_call_glue,@function
ksm_clang_call_glue:
	pushl	%ebp
	movl	%esp, %ebp
	subl    $8, %esp	# 8(%ebp) has call_frame
	movl    8(%ebp), %edx
	movl    16(%edx), %eax
	movl    %eax, -4(%ebp)  # -4(%ebp) has narg
	addl    $20, %edx       # %edx has address of args[]
L2:		
	movl    -4(%ebp), %eax  # eax:	narg
	cmpl    $0, %eax
	jle     L1
	subl    $1, %eax
	sall    $4, %eax           # %eax has offset in args[]
	movl    (%eax,%edx), %ecx  # %ecx has flag
	addl    $8, %eax           # %eax has offfset of u
	cmpl	$1, %ecx
	jne	L3
	pushl   4(%eax,%edx)
L3:
	pushl   (%eax,%edx)
	subl    $1, -4(%ebp)
	jmp     L2
L1:
	movl    8(%ebp), %edx
	movl    12(%edx), %eax
	call    *%eax
	movl    8(%ebp), %edx
	movl    %eax, 8(%edx)
	fstpl   (%edx)
	leave
	ret
.Lfe1:
	.size	 ksm_clang_call_glue,.Lfe1-ksm_call_glue
#endif  
	
