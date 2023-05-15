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


(let ((base-dir (path-directory-part (load-file-name))))
  (load-shared (path-append base-dir "libbase.so") "ksm_init_base")
  (load (path-append base-dir "generic.scm")))

(define-private *base:packages* (make-hashtab 100))
(define-private *base:used-packages* (make-hashtab 100))

;;; USE-INSTALL
;;; (use-install symbol path)
;;; installs path of source file as the symbol. The source file can be later
;;; loaded by (use symbol).
;;; example:
;;; (use-install 'clang (path-append (ksm-directory) "clang/init.scm"))

(define (use-install symbol path)
  (hashtab-enter! *base:packages* symbol path))

;;; USE
;;; (use symbol)
;;; loads the source file that has been installed by the symbol name.
;;; If the source file has been loaded by a previous USE, the file
;;; is not loaded again.

(define (use symbol)
  (if (not (hashtab-includes? *base:packages* symbol))
      (begin
	(display symbol)
	(newline)
	(fatal "can't find package to USE")))
  (if (not (hashtab-includes? *base:used-packages* symbol))
      (begin
	(hashtab-enter! *base:used-packages* symbol #t)
	(load (hashtab-lookup *base:packages* symbol)))))

;;; LOG10
;;; (log10 x)
;;; returns log10(x)

(define log10
  (let ((loge10 (log 10)))
    (lambda (x)
      (/ (log x) loge10))))
