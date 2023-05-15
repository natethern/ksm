(use 'thread)

(define (make-stage)
  (let ((v (make-vector 5)))
    (vector-set! v 0 '())   ;; next
    (vector-set! v 1 0)     ;; thread
    (vector-set! v 2 (thread:make-mutex))  ;; mutex
    (vector-set! v 3 (make-queue))         ;; input queue
    (vector-set! v 4 (thread:make-cond))   ;; avail
    v))

(define (stage-next stage)
  (vector-ref stage 0))

(define (stage-set-next stage next)
  (vector-set! stage 0 next))

(define (stage-thread stage)
  (vector-ref stage 1))

(define (stage-set-thread stage thread)
  (vector-set! stage 1 thread))

(define (stage-mutex stage)
  (vector-ref stage 2))

(define (stage-input-queue stage)
  (vector-ref stage 3))

(define (stage-cond-avail stage)
  (vector-ref stage 4))

(define (stage-put-input stage data)
  (thread:mutex-lock (stage-mutex stage))
  (enqueue! (stage-input-queue stage) data)
  (thread:cond-signal (stage-cond-avail stage))
  (thread:mutex-unlock (stage-mutex stage)))

(define (stage-get-input stage)
  (thread:mutex-lock (stage-mutex stage))
  (let loop ()
    (if (queue-empty? (stage-input-queue stage))
	(begin
	  (thread:cond-wait (stage-cond-avail stage)
			    (stage-mutex stage))
	  (loop))))
  (let ((data (dequeue! (stage-input-queue stage))))
    (thread:mutex-unlock (stage-mutex stage))
    data))

(define (make-stage-routine fun stage)
  (lambda ()
    (let loop ()
      (let ((data (fun (stage-get-input stage))))
	(if (not (null? (stage-next stage)))
	    (stage-put-input (stage-next stage) data)))
      (loop))))

(define (thread:pipeline . args)
  (let loop ((head '())
	     (tail '())
	     (args args))
    (if (null? args)
	head
	(let ((stage (make-stage)))
	  (if (null? head)
	      (begin
		(set! head stage)
		(set! tail stage))
	      (begin
		(stage-set-next tail stage)
		(set! tail stage)))
	  (stage-set-thread stage
		(thread:create (make-stage-routine (car args) stage)))
	  (loop head tail (cdr args))))))


(define p
  (thread:pipeline
   (lambda (x) (display x) (newline) (+ x 1))
   (lambda (y) (display y) (newline))))



  