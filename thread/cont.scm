(use 'thread)

(thread:create
 (lambda ()
   (let ((c '())
	 (count 0))
     (dynamic-wind
	 (lambda () (display "prolog") (newline))
	 (lambda () 
	   (call/cc (lambda (cont)
		      (set! c cont)
		      (display "body") (newline))))
	 (lambda () (display "epilog") (newline)))
     (if (< count 5)
	 (begin
	   (set! count (+ count 1))
	   (c 1))))))


     