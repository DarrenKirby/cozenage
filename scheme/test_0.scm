

(display (+ 34 45))
(newline)
(display "Hello, World!")
(newline)

(define (fact n)
  (if (= n 1) 1
      (* n (fact (- n 1)))))

#| ignore this
ignore this too |#

;;; This should also be ignored

(display "Factorial of 10 is: ")
(display (fact 10))
(newline)
