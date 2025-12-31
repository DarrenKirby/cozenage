(define (integers-starting-from n)
    (stream n (integers-starting-from (+ n 1))))

(define integers (integers-starting-from 1))

(define (at n s)
  (if (= n 0)
    (head s)
    (at (- n 1) (tail s))))

(define (take n s)
  (if (or (= n 0) (null? s))
    '()
    (cons (head s) (take (- n 1) (tail s)))))

(define (s_map proc s)
  (if (null? s)
    '()
    (stream (proc (head s))
      (s_map proc (tail s)))))

(define (s_filter pred s)
  (cond ((null? s) '())
    ((pred (head s))
      (stream (head s) (s_filter pred (tail s))))
    (else (s_filter pred (tail s)))))

;; s_add adds two streams together
(define (s_add s1 s2)
  (stream (+ (head s1) (head s2))
    (s_add (tail s1) (tail s2))))

(define fibs
  (stream 0
    (stream 1
      (s_add fibs (tail fibs)))))
