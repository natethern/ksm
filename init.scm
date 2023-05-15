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

(load (path-append (ksm-directory) "base/init.scm"))

(define (load-directory-name)
  (path-directory-part (load-file-name)))

(define *ksm:packages*
  '(clang regex clike thread))

(let ()
  (define (file-path directory)
    (path-append (ksm-directory) directory "init.scm"))
  (define (install symbol-directory)
    (use-install symbol-directory (file-path symbol-directory)))
  (for-each install *ksm:packages*))

(use 'clang)
(use 'clike)

(%enable-interpreter-file)

;; NUMBER->BOOLEAN (procedure) : converts number to boolean
;; (number->boolean number)
;; example:
;; > (number->boolean 1) --> #t
;; > (number->boolean 0) --> #f

(define (number->boolean num)
  (if (zero? num) #f #t))

(define (gc-shower-off)
  (use 'clang)
  (set! (clang:sym int ksm_gc_shower) 0))

(define (gc-shower-on)
  (use 'clang)
  (set! (clang:sym int ksm_gc_shower) 1))

