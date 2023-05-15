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

#include <stdio.h>
#include <stdlib.h>

char *GrandPtr = "???";

void glue(void)
{
  printf("Glue! %s\n", GrandPtr);
}

#define M(a,b)     (((0xFFFFFFFFU<<(a))>>(31+(a)-(b)))<<(31-(b)))
#define F(a,b,x)   (((x)<<(31-(b)))&M(a,b))
#define C(x)       F(0,5,x)
#define rS(x)       F(6,10,x)
#define rD(x)       F(6,10,x)
#define rA(x)       F(11,15,x)
#define rB(x)       F(16,20,x)
#define SPR(spr0_4,spr5_9) (F(11,15,spr0_4)|F(16,20,spr5_9))

#define h(a)   ((((int)(a))>>16)&(0xFFFF))
#define ha(a)  ((((int)(a))&(0x8000))?(-h((int)(a))):h((int)(a)))
#define l(a)   (((int)(a))&0xFFFF)

#define addi(D,A,IMM)  (C(14)|rD(D)|rA(A)|F(16,31,IMM))
#define addis(D,A,IMM) (C(15)|rD(D)|rA(A)|F(16,31,IMM))
#define bcctrx(BO,BI,LK) (C(19)|rD(BO)|rA(BI)|F(21,30,528)|LK)
#define bclr(BO,BI,LK) (C(19)|rD(BO)|rA(BI)|F(21,30,16)|LK)
#define lwz(D,d,A)     (C(32)|rD(D)|rA(A)|F(16,31,d))
#define mfspr(D,nh,nl) (C(31)|rD(D)|SPR(nh,nl)|F(21,30,339))
#define mtspr(D,nh,nl) (C(31)|rD(D)|SPR(nh,nl)|F(21,30,467))
#define ori(A,S,UIMM)  (C(24)|rS(S)|rA(A)|F(16,31,UIMM))
#define stw(S,d,A)     (C(36)|rS(S)|rA(A)|F(16,31,d))
#define stwu(S,d,A)    (C(37)|rS(S)|rA(A)|F(16,31,d))

#define blr            bclr(20,0,0)
#define blrl           bclr(20,0,1)
#define bcctr(BO,BI)   bcctrx(BO,BI,0)
#define bcctrl(BO,BI)  bcctrx(BO,BI,1)
#define mtctr(D)       mtspr(D,9,0)
#define mflr(D)        mfspr(D,8,0)
#define mtlr(D)        mtspr(D,8,0)
#define mr(D,S)        ori(D,S,0)

void *PrivatePtr = "Private";

int ops[100] = 
{
  addis(11,0,0),    // fun
  addi(11,11,0),
  mtctr(11),
  addis(11,0,0),    // aux
  addi(11,11,0),
  lwz(0,0,11),
  addis(11,0,0),    // global
  addi(11,11,0),
  stw(0,0,11),
  bcctr(20,0)
};

typedef void (*void_f)();


/* jit_flush_code: from lightning-0.95 by Paolo Bonzini and Ian Piumarta */

static void
jit_flush_code(void* start, void* end)
{
  register char *dest = start;

  for (; dest <= (char *)end; dest += 4) {
    __asm__ __volatile__ ("dcbst 0,%0; sync; icbi 0,%0; isync"::"r"(dest));
  }
}

int main()
{
  void_f vf;
  int *p;

  p = malloc(sizeof(int)*10);
  memcpy(p, ops, sizeof(int)*10);
  p[0] |= ha(&glue);
  p[1] |= l(&glue);
  p[3] |= ha(&PrivatePtr);
  p[4] |= l(&PrivatePtr);
  p[6] |= ha(&GrandPtr);
  p[7] |= l(&GrandPtr);
  jit_flush_code(p,p+10);
  vf = (void_f)p;
  (*vf)();
}
