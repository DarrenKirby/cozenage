; test_functions.scm
;
; Tests function definitions ('lambda'), 'let', and 'let*'.
;
; --- EXPECTED OUTPUT ---
; Test 2.1 (Simple 'lambda'): 25
; Test 2.2 ('let' scope): 78.53975
; Test 2.3 ('let*' sequential): 35
; -------------------------


; Define a function
(define (add-five n)
  (+ n 5))

(display "Test 2.1 (Simple 'lambda'): ")
; Call the function defined in the previous expression
(display (add-five 20))
(newline)

; Define a function that uses 'let' for local scope
(define (area-of-circle r)
  (let ((pi 3.14159))
    (* pi r r)))

(display "Test 2.2 ('let' scope): ")
; Call the function
(display (area-of-circle 5))
(newline)

(display "Test 2.3 ('let*' sequential): ")
; Test 'let*' for sequential variable definitions
(display
  (let* ((a 10)
          (b (* a 2))) ; 'b' depends on 'a'
    (+ a b 5)))
(newline)
