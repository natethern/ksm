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


(define (filter pred list)
  (let ((queue (make-queue)))
    (let loop ((remain list))
      (cond ((null? remain) (queue-content queue))
	    (else (if (pred (car remain))
		      (enqueue! queue (car remain)))
		  (loop (cdr remain)))))))

(define (interpose list obj)
  (let ((queue (make-queue))
	(count 0))
    (let loop ((remain list))
      (cond 
       ((null? remain) (queue-content queue))
       ((null? (cdr remain)) (enqueue! queue (car remain))
	                     (queue-content queue))
       (else (enqueue! queue (car remain))
	     (enqueue! queue 
		      (if (procedure? obj) (obj count) obj))
	     (set! count (+ count 1))
	     (loop (cdr remain)))))))

(define (remove-if pred list)
  (let ((queue (make-queue)))
    (let loop ((remain list))
      (cond
       ((null? remain) (queue-content queue))
       ((pred (car remain)) (loop (cdr remain)))
       (else (enqueue! queue (car remain)) (loop (cdr remain)))))))

(define (first x)   (list-ref x 0))
(define (second x)  (list-ref x 1))
(define (third x)   (list-ref x 2))
(define (fourth x)  (list-ref x 3))
(define (fifth x)   (list-ref x 4))
(define (sixth x)   (list-ref x 5))
(define (seventh x) (list-ref x 6))
(define (eighth x)  (list-ref x 7))
(define (ninth x)   (list-ref x 8))
(define (tenth x)   (list-ref x 9))

(define (enumerate from to)
  (let ((queue (make-queue)))
    (do ((i from (+ i 1)))
	((> i to) (queue-content queue))
      (enqueue! queue i))))




