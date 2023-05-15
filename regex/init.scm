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

(define *regex:dir* (path-directory-part (load-file-name)))

(load-shared (path-append *regex:dir* "libregex.so") 
	     "ksm_init_regex_package")

(define (regex:regcomp pattern &key (ignore-case #f) (newline #f))
  (let ((flag 0))
    (if ignore-case (set! flag (logical-or flag regex:REG_ICASE)))
    (if newline     (set! flag (logical-or flag regex:REG_NEWLINE)))
    (regex:%regcomp pattern flag)))

(define (regex:regexec regex string &key (not-bol #f) (not-eol #f))
  (let ((flag 0))
    (if not-bol (set! flag (logical-or flag regex:REG_NOTBOL)))
    (if not-eol (set! flag (logical-or flag regex:REG_NOTEOL)))
    (regex:%regexec regex string flag)))

(define (regex:replace regex string generator
		      &key (not-bol #f) (not-eol #f) (count -1))
  (let ((flag 0))
    (if not-bol (set! flag (logical-or flag regex:REG_NOTBOL)))
    (if not-eol (set! flag (logical-or flag regex:REG_NOTEOL)))
    (regex:%replace regex string generator flag count)))





