; test_advanced.scm
;
; Tests 'letrec' for mutual recursion and basic list helpers.
;
; --- EXPECTED OUTPUT ---
; Test 3.1 (List 'car'): 1
; Test 3.2 (List 'cadr'): 2
; Test 3.3 ('letrec' check 10): "even"
; Test 3.4 ('letrec' check 7): "odd"
; -------------------------


(define my-list (list 1 2 3 4 5))

(display "Test 3.1 (List 'car'): ")
(display (car my-list))
(newline)

(display "Test 3.2 (List 'cadr'): ")
(display (cadr my-list))
(newline)

; Define a function that uses 'letrec' for mutual recursion
; to determine if a number is even or odd.
(define (check-parity n)
  (letrec (
            (even? (lambda (x)
                     (if (= x 0)
                       #t
                       (odd? (- x 1)))))
            (odd? (lambda (x)
                    (if (= x 0)
                      #f
                      (even? (- x 1)))))
            )
    ; Body of the 'letrec'
    (if (even? n)
      "even"
      "odd")))

(display "Test 3.3 ('letrec' check 10): ")
(display (check-parity 10))
(newline)

(display "Test 3.4 ('letrec' check 7): ")
(display (check-parity 7))
(newline)
