# Generated automatically from Makefile.in by configure.
#  Makefile for KSM-Scheme

#  Copyright (C) 2000 - 2001 Hangil Chang

#  This file is part of KSM-Scheme, Kit for Scheme Manipulation Scheme.
#  
#  KSM-Scheme is free software; you can distribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2, or (at your option)
#  any later version.
 
#  KSM-Scheme is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even implied warranty of MERCHANTABILITY
#  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
#  License for more details.

#  You should have received a copy of the GNU General Public License
#  along with KSM-Scheme; see the file COPYING. If not, write to the
#  Free Software Foundation, 59 Temple Place, Suite 330, Boston,
#  MA 02111 USA.                                                   

object = base.o hashtab.o lambdax.o getopt.o logic.o \
         queue.o readline.o memstream.o signal.o sched.o

libname = libbase.so

ksm_ver = 0.3.2
prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
dst_base = $(prefix)/ksm/ksm-$(ksm_ver)

CFLAGS = -Wall -g -Wno-format -D_GNU_SOURCE -I../include -fPIC  -DHAVE_GMP_H=1 -DHAVE_LIBGMP=1 -DHAVE_PTHREAD_H=1 -DHAVE_LIBPTHREAD=1

$(libname): $(object)
	gcc -shared -o $@ $(object) 

clean:
	rm -f *~ *.o $(libname) core

install:
	install -D $(libname) $(dst_base)/base/$(libname)
	install *.scm $(dst_base)/base

