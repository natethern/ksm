(use 'thread)
(use 'clang)

(define fork (clang:sym int (fork)))
(define getpid (clang:sym int (getpid)))

(thread:atfork (lambda () (printf "%d: prepare 1\n" (getpid)))
               (lambda () (printf "%d: parent  1\n" (getpid)))
	       (lambda () (printf "%d: child   1\n" (getpid))))

(thread:atfork (lambda () (printf "%d: prepare 2\n" (getpid)))
               (lambda () (printf "%d: parent  2\n" (getpid)))
	       (lambda () (printf "%d: child   2\n" (getpid))))

(define id (fork))

(printf "%d: Hello\n" id)
