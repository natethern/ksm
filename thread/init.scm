;;  Copyright (C) 2000 - 2001 Hangil Chang

;;  This file is part of KSM-Scheme, Kit for Scheme Manipulation Scheme.
   
;;  KSM-Scheme is free software; you can distribute it and/or modify it
;;  under the terms of the GNU General Public License as published by
;;  the Free Software Foundation; either version 2, or (at your option)
;;  any later version.
  
;;  KSM-Scheme is distributed in the hope that it will be useful, but
;;  WITHOUT ANY WARRANTY; without even implied warranty of MERCHANTABILITY
;;  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
;;  License for more details.

;;  You should have received a copy of the GNU General Public License
;;  along with KSM-Scheme; see the file COPYING. If not, write to the
;;  Free Software Foundation, 59 Temple Place, Suite 330, Boston,
;;  MA 02111 USA.                                                   

(define *thread:dir* (path-directory-part (load-file-name)))

(load-shared (path-append *thread:dir* "libthread.so") 
	     "ksm_init_thread_package")

