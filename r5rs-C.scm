fun print(obj)
{
  display(obj);
  newline();
}

print("*************************");
print("4.1.2 Literal expressions");
print("*************************");
print(quote(a));
print(quote(#[a, b, c]));
print(quote(1+2));
print('a);
print('#[a, b, c]);
print('[]);
print('(1+2));
print('[quote, a]);
print(''a);
print('"abc");
print("abc");
print('145932);
print(145932);
print('#t);
print(#t);

; (print "*************************")
; (print "4.1.4 Procedures")
; (print "*************************")
; (print (lambda (x) (+ x x)))
; (print ((lambda (x) (+ x x)) 4))
; (define reverse-subtract
;   (lambda (x y) (- y x)))
; (print (reverse-subtract 7 10))
; (define add4
;   (let ((x 4))
;     (lambda (y) (+ x y))))
; (print (add4 6))
; (print ((lambda x x) 3 4 5 6))
; (print ((lambda (x y . z) z) 3 4 5 6))

