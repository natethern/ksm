(use 'thread)

(define th 
  (thread:create
   (lambda ()
     (thread:with-cleanup
      (lambda () (display "cleanup") (newline))
      (lambda () (let loop () (thread:testcancel) (loop)))))))

