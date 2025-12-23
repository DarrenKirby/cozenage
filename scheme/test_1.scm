; test_simple.scm
;
; --- EXPECTED OUTPUT ---
; Test 1.1 (Simple Math): 30
; Test 1.2 (Using 'define'): 50
; Test 1.3 (Strings): hello world
; -------------------------


(display "Test 1.1 (Simple Math): ")
(display (+ 10 20))
(newline)

; Define a variable 'x'
(define x 5)

(display "Test 1.2 (Using 'define'): ")
; Use 'x' in a subsequent expression
(display (* x 10))
(newline)

(display "Test 1.3 (Strings): ")
(display (string-append "hello" " " "world"))
(newline)
