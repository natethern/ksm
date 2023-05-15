(define (print obj)
  (display obj)
  (newline))

(print "*************************")
(print "4.1.2 Literal expressions")
(print "*************************")
(print (quote a))
(print (quote #(a b c)))
(print (quote (+ 1 2)))
(print 'a)
(print '#(a b c))
(print '())
(print '(+ 1 2))
(print '(quote a))
(print ''a)
(print '"abc")
(print "abc")
(print '145932)
(print 145932)
(print '#t)
(print #t)

(print "*************************")
(print "4.1.4 Procedures")
(print "*************************")
(print (lambda (x) (+ x x)))
(print ((lambda (x) (+ x x)) 4))
(define reverse-subtract
  (lambda (x y) (- y x)))
(print (reverse-subtract 7 10))
(define add4
  (let ((x 4))
    (lambda (y) (+ x y))))
(print (add4 6))
(print ((lambda x x) 3 4 5 6))
(print ((lambda (x y . z) z) 3 4 5 6))

(print "*************************")
(print "4.1.5 Conditionals")
(print "*************************")
(print (if (> 3 2) 'yes 'no))
(print (if (> 2 3) 'yes 'no))
(print (if (> 3 2) (- 3 2) (+ 3 2)))

(print "*************************")
(print "4.1.5 Conditionals")
(print "*************************")
(define x 2)
(print (+ x 1))
(set! x 4)
(print (+ x 1))

(print "*************************")
(print "4.2.1 Conditionals")
(print "*************************")
(print (cond ((> 3 2) 'greater)
	     ((< 3 2) 'less)))
(print (cond ((> 3 3) 'greater)
	     ((< 3 3) 'less)
	     (else 'equal)))
(print (cond ((assv 'b '((a 1) (b 2))) => cadr)
	     (else #f)))
(print (case (* 2 3)
	 ((2 3 5 7) 'prime)
	 ((1 4 6 8 9) 'composite)))
(print (case (car '(c d))
	 ((a) 'a)
	 ((b) 'b)))
(print (case (car '(c d))
	 ((a e i o u) 'vowel)
	 ((w y) 'semivowel)
	 (else 'consonant)))
(print (and (= 2 2) (> 2 1)))
(print (and (= 2 2) (< 2 1)))
(print (and 1 2 'c '(f g)))
(print (and))
(print (or (= 2 2) (> 2 1)))
(print (or (= 2 2) (< 2 1)))
(print (or #f #f #f))
(print (or (memq 'b '(a b c)) (/ 3 0)))

(print "*************************")
(print "4.2.2 Binding constructs")
(print "*************************")
(print (let ((x 2) (y 3))
	 (* x y)))
(print (let ((x 2) (y 3))
	 (let ((x 7)
	       (z (+ x y)))
	   (* z x))))
(print (let ((x 2) (y 3))
	 (let* ((x 7)
		 (z (+ x y)))
	   (* z x))))
(print (letrec ((even?
		 (lambda (n)
		   (if (zero? n)
		       #t
		       (odd? (- n 1)))))
		(odd?
		 (lambda (n)
		   (if (zero? n)
		       #f
		       (even? (- n 1))))))
	 (even? 88)))

(print "*************************")
(print "4.2.3 Sequencing")
(print "*************************")
(define x 0)
(print (begin (set! x 5)
	      (+ x 1)))
(print (begin (display "4 plus 1 equals ")
	      (display (+ 4 1))))

(print "*************************")
(print "4.2.4 Iteration")
(print "*************************")
(print (do ((vec (make-vector 5))
	    (i 0 (+ i 1)))
	   ((= i 5) vec)
	 (vector-set! vec i i)))
(print (let ((x '(1 3 5 7 9)))
	 (do ((x x (cdr x))
	      (sum 0 (+ sum (car x))))
	     ((null? x) sum))))
(print
 (let loop ((numbers '(3 -2 1 6 -5))
	    (nonneg '())
	    (neg '()))
   (cond ((null? numbers) (list nonneg neg))
	 ((>= (car numbers) 0)
	  (loop (cdr numbers)
		(cons (car numbers) nonneg)
		neg))
	 ((< (car numbers) 0)
	  (loop (cdr numbers)
		nonneg
		(cons (car numbers) neg))))))

(print "*************************")
(print "4.2.6 Quasiquotation")
(print "*************************")
(print
 `(list ,(+ 1 2) 4))
(print
 (let ((name 'a)) `(list ,name ',name)))
(print
 `(a ,(+ 1 2) ,@(map abs '(4 -5 6)) b))
(print
 `(( foo ,(- 10 3)) ,@(cdr '(c)) . ,(car '(cons))))
(print
 `#(10 5 ,(sqrt 4) ,@(map sqrt '(16 9)) 8))
(print
 `(a `(b ,(+ 1 2) ,(foo ,(+ 1 3) d) e) f))
(print
 (let ((name1 'x)
       (name2 'y))
   `(a `(b ,,name1 ,',name2 d) e)))
(print
 (quasiquote (list (unquote (+ 1 2)) 4)))

(print "*************************")
(print "4.3.1 Binding constructs for syntactic keywords")
(print "*************************")
(print
 (let-syntax ((when (syntax-rules ()
				  ((when test stmt1 stmt2 ...)
				   (if test
				       (begin stmt1
					      stmt2 ...))))))
   (let ((if #t))
     (when if (set! if 'now))
	if)))

(print
 (let ((x 'outer))
   (let-syntax ((m (syntax-rules () ((m) x))))
     (let ((x 'inner))
       (m)))))

(print
 (letrec-syntax
  ((my-or (syntax-rules ()
			((my-or) #f)
			((my-or e) e)
			((my-or e1 e2 ...)
			 (let ((temp e1))
			   (if temp
			       temp
			       (my-or e2 ...)))))))
  (let ((x #f)
	(y 7)
	(temp 8)
	(let odd?)
	(if even?))
    (my-or x
	   (let temp)
	   (if y)
	   y))))

(print "*************************")
(print "4.3.2 Pattern language")
(print "*************************")
(define-syntax macro-cond
  (syntax-rules (else =>)
		((cond (else result1 result2 ...))
		 (begin result1 result2 ...))
		((cond (test => result))
		 (let ((temp test))
		   (if temp (result temp))))
		((cond (test => result) clause1 clause2 ...)
		 (let ((temp test))
		   (if temp (result temp)
		       (cond clause1 clause2 ...))))
		((cond (test)) test)
		((cond (test) clause1 clause2 ...)
		 (let ((temp test))
		   (if temp
		       temp
		       (cond clause1 clause2 ...))))
		((cond (test result1 result2 ...))
		 (if test
		     (begin result1 result2 ...)))
		((cond (test result1 result2 ...)
		       clause1 clause2 ...)
		 (if test
		     (begin result1 result2 ...)
		     (cond clause1 clause2 ...)))))
(print
 (let ((=> #f))
   (macro-cond (#t => 'ok))))

(print "*************************")
(print "5.2.1 Top level definitions")
(print "*************************")
(define add3
  (lambda (x) (+ x 3)))
(print (add3 3))
(define first car)
(print
 (first '(1 2)))

(print "*************************")
(print "5.2.2 Top level definitions")
(print "*************************")
(print
 (let ((x 5))
   (define foo (lambda (y) (bar x y)))
   (define bar (lambda (a b) (+ (* a b) a)))
   (foo (+ x 3))))
(print
 (let ((x 5))
   (letrec ((foo (lambda (y) (bar x y)))
	    (bar (lambda (a b) (+ (* a b) a))))
     (foo (+ x 3)))))

(print "*************************")
(print "6.1 Equivalence predicates")
(print "*************************")
(print
 (eqv? 'a 'a))
(print
 (eqv? 'a 'b))
(print 
 (eqv? 2 2))
(print
 (eqv? '() '()))
(print
 (eqv? 100000000 100000000))
(print
 (eqv? (cons 1 2) (cons 1 2)))
(print
 (eqv? (lambda () 1) 
       (lambda () 2)))
(print
 (eqv? #f 'nil))
(print
 (let ((p (lambda (x) x)))
   (eqv? p p)))
(print
 (eqv? "" ""))
(print
 (eqv? '#() '#()))
(print
 (eqv? (lambda (x) x)
       (lambda (x) x)))
(print
 (eqv? (lambda (x) x)
       (lambda (y) y)))
(define gen-counter
  (lambda ()
    (let ((n 0))
      (lambda () (set! n (+ n 1)) n))))
(print
 (let ((g (gen-counter)))
   (eqv? g g)))
(print
 (eqv? (gen-counter) (gen-counter)))
(define gen-loser
  (lambda ()
    (let ((n 0))
      (lambda () (set! n (+ n 1)) 27))))
(print
 (let ((g (gen-loser)))
   (eqv? g g)))
(print
 (eqv? (gen-loser) (gen-loser)))
(print
 (letrec ((f (lambda () (if (eqv? f g) 'both 'f)))
	  (g (lambda () (if (eqv? f g) 'both 'g))))
   (eqv? f g)))
(print
 (letrec ((f (lambda () (if (eqv? f g) 'f 'both)))
	  (g (lambda () (if (eqv? f g) 'g 'both))))
   (eqv? f g)))
(print
 (eqv? '(a) '(a)))
(print
 (eqv? "a" "a"))
(print
 (eqv? '(b) (cdr '(a b))))
(print
 (let ((x '(a)))
   (eqv? x x)))
(print
 (eq? 'a 'a))
(print
 (eq? '(a) '(a)))
(print
 (eq? (list 'a) (list 'a)))
(print
 (eq? "a" "a"))
(print
 (eq? "" ""))
(print
 (eq? '() '()))
(print
 (eq? 2 2))
(print
 (eq? #\A #\A))
(print
 (eq? car car))
(print
 (let ((n (+ 2 3)))
   (eq? n n)))
(print
 (let ((x '(a)))
   (eq? x x)))
(print
 (let ((x '#()))
   (eq? x x)))
(print
 (let ((p (lambda (x))))
   (eq? p p)))
(print
 (equal? 'a 'a))
(print
 (equal? '(a) '(a)))
(print
 (equal? '(a (b) c)
	 '(a (b) c)))
(print
 (equal? "abc" "abc"))
(print
 (equal? 2 2))
(print
 (equal? (make-vector 5 'a)
	 (make-vector 5 'a)))
(print
 (equal? (lambda (x) x)
	 (lambda (y) y)))

(print "*************************")
(print "6.2.5 Numerical operations")
(print "*************************")
(print (max 3 4))
(print (max 3.9 4))
(print (+ 3 4))
(print (+ 3))
(print (+))
(print (* 4))
(print (*))
(print (- 3 4))
(print (- 3 4 5))
(print (- 3))
(print (/ 3 4 5))
(print (/ 3))
(print (modulo 13 4))
(print (remainder 13 4))
(print (modulo -13 4))
(print (remainder -13 4))
(print (modulo 13 -4))
(print (remainder 13 -4))
(print (modulo -13 -4))
(print (remainder -13 -4))
(print (remainder -13 -4.0))
(print (gcd 32 -36))
(print (gcd))
(print (lcm 32 -36))
;(print (lcm 32.0 -36))
(print (lcm))
(print (floor -4.3))
(print (ceiling -4.3))
(print (truncate -4.3))
(print (round -4.3))
(print (floor 3.5))
(print (ceiling 3.5))
(print (truncate 3.5))
(print (round 3.5))
(print (round 7))

(print "*************************")
(print "6.2.6 Numerical input and output")
(print "*************************")
(print (string->number "100"))
(print (string->number "100" 16))
(print (string->number "1e2"))

(print "*************************")
(print "6.3.1 Booleans")
(print "*************************")
(print (not #t))
(print (not 3))
(print (not (list 3)))
(print (not #f))
(print (not '()))
(print (not (list)))
(print (not 'nil))
(print (boolean? #f))
(print (boolean? 0))
(print (boolean? '()))

(print "*************************")
(print "6.3.2 Pairs and lists")
(print "*************************")
(define x (list 'a 'b 'c))
(define y x)
(print y)
(print (list? y))
(set-cdr! x 4)
(print x)
(print (eqv? x y))
(print y)
(print (list? y))
(set-cdr! x x)
(print (list? x))
(print (pair? '(a . b)))
(print (pair? '(a b c)))
(print (pair? '()))
(print (pair? '#(a b)))
(print (cons 'a '()))
(print (cons '(a) '(b c d)))
(print (cons "a" '(b c)))
(print (cons 'a 3))
(print (cons '(a b) 'c))
(print (car '(a b c)))
(print (car '((a) b c d)))
(print (car '(1 . 2)))
(print (cdr '((a) b c d)))
(print (cdr '(1 . 2)))
(print (list? '(a b c)))
(print (list? '()))
(print (list? '(a . b)))
(print
 (let ((x (list 'a)))
   (set-cdr! x x)
   (list? x)))
(print (list 'a (+ 3 4) 'c))
(print (list))
(print (length '(a b c)))
(print (length '(a (b) (c d e))))
(print (length '()))
(print (append '(x) '(y)))
(print (append '(a) '(b c d)))
(print (append '(a (b)) '((c))))
(print (append '(a b) '(c . d)))
(print (append '() 'a))
(print (reverse '(a b c)))
(print (reverse '(a (b c) d (e (f)))))
(print (list-ref '(a b c d) 2))
(print (list-ref '(a b c d)
		 (inexact->exact (round 1.8))))
(print (memq 'a '(a b c)))
(print (memq 'b '(a b c)))
(print (memq 'a '(b c d)))
(print (memq (list 'a) '(b (a) c)))
(print (member (list 'a) '(b (a) c)))
(print (memq 101 '(100 101 102)))
(print (memv 101 '(100 101 102)))
(define e '((a 1) (b 2) (c 3)))
(print (assq 'a e))
(print (assq 'b e))
(print (assq 'd e))
(print (assq (list 'a) '(((a)) ((b)) ((c)))))
(print (assoc (list 'a) '(((a)) ((b)) ((c)))))
(print (assq 5 '((2 3) (5 7) (11 13))))
(print (assv 5 '((2 3) (5 7) (11 13))))

(print "*************************")
(print "6.3.3 Symbols")
(print "*************************")
(print (symbol? 'foo))
(print (symbol? (car '(a b))))
(print (symbol? "bar"))
(print (symbol? 'nil))
(print (symbol? '()))
(print (symbol? #f))
(print (symbol->string 'flying-fish))
(print (symbol->string 'Martin))
(print (symbol->string
	(string->symbol "Malvina")))
(print (eq? 'mISSISSIppi 'mississippi))
(print (string->symbol "mISSISSIppi"))
(print (eq? 'bitBlt (string->symbol "bitBlt")))
(print (eq? 'JollyWog
	    (string->symbol
	     (symbol->string 'JollyWog))))
(print (string=? "K. Harper, M.D."
		 (symbol->string
		  (string->symbol "K. Harper, M.D."))))

(print "*************************")
(print "6.3.6 Vectors")
(print "*************************")
(print (vector 'a 'b 'c))
(print (vector-ref '#(1 1 2 3 5 8 13 21) 5))
(print (vector-ref '#(1 1 2 3 5 8 13 21)
		   (let ((i (round (* 2 (acos -1)))))
		     (if (inexact? i)
			 (inexact->exact i)
			 i))))
(print
 (let ((vec (vector 0 '(2 2 2 2) "Anna")))
   (vector-set! vec 1 '("Sue" "Sue"))
   vec))
(print
 (vector->list '#(dah dah didah)))
(print
 (list->vector '(dididit dah)))

(print "*************************")
(print "6.4 Control features")
(print "*************************")
(print (procedure? car))
(print (procedure? 'car))
(print (procedure? (lambda (x) (* x x))))
(print (procedure? '(lambda (x) (* x x))))
(print (call-with-current-continuation procedure?))

(print "*************************")
(print "6.4 Control features")
(print "*************************")
(print (apply + (list 3 4)))
(define compose
  (lambda (f g)
    (lambda args
      (f (apply g args)))))
(print ((compose sqrt *) 12 75))
(print (map cadr '((a b) (d e) (g h))))
(print (map (lambda (n) (expt n n))
	    '(1 2 3 4 5)))
(print (map + '(1 2 3) '(4 5 6)))
(print (let ((count 0))
	 (map (lambda (ignored)
		(set! count (+ count 1))
		count)
	      '(a b))))
(print (let ((v (make-vector 5)))
	 (for-each (lambda (i)
		     (vector-set! v i (* i i)))
		   '(0 1 2 3 4))
	 v))
(print (force (delay (+ 1 2))))
(print (let ((p (delay (+ 1 2))))
	 (list (force p) (force p))))
(define a-stream
  (letrec ((next
	    (lambda (n)
	      (cons n (delay (next (+ n 1)))))))
    (next 0)))
(define head car)
(define tail
  (lambda (stream) (force (cdr stream))))
(print (head (tail (tail a-stream))))
(define count 0)
(define p
  (delay (begin (set! count (+ count 1))
		(if (> count x)
		    count
		    (force p)))))
(define x 5)
(print (force p))
(print (begin (set! x 10)
	      (force p)))
(print 
 (call-with-current-continuation
  (lambda (exit)
    (for-each (lambda (x)
		(if (negative? x)
		    (exit x)))
	      '(54 0 37 -3 245 19))
    #t)))
(define list-length
  (lambda (obj)
    (call-with-current-continuation
     (lambda (return)
       (letrec ((r
		 (lambda (obj)
		   (cond ((null? obj) 0)
			 ((pair? obj)
			  (+ (r (cdr obj)) 1))
			 (else (return #f))))))
	 (r obj))))))
(print (list-length '(1 2 3 4)))
(print (list-length '(a b . c)))

(print
 (call-with-values (lambda () (values 4 5))
		   (lambda (a b) b)))
(print (call-with-values * -))

(print
 (let ((path '())
       (c #f))
   (let ((add (lambda (s)
		(set! path (cons s path)))))
     (dynamic-wind
      (lambda () (add 'connect))
      (lambda ()
	(add (call-with-current-continuation
	      (lambda (c0)
		(set! c c0)
		'talk1))))
      (lambda () (add 'disconnect)))
     (if (< (length path) 4)
	 (c 'talk2)
	 (reverse path)))))

(print "*************************")
(print "6.5 Eval")
(print "*************************")
(print
 (eval '(* 7 3) (scheme-report-environment 5)))
(print
 (let ((f (eval '(lambda (f x) (f x x))
		(null-environment 5))))
   (f + 10)))

		
		    
       


	 

