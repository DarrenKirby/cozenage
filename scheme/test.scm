;; test.scm
(define (assert label condition)
  (if (not condition)
    (begin (display "FAILED: ") (display label) (newline))
    (begin (display "PASSED: ") (display label) (newline))))

(assert "string-length in map" (equal? (map string-length '("a" "bb")) '(1 2)))
(assert "filter with logic" (equal? (filter (lambda (x) (<= (string-length x) 2)) '("a" "ccc")) '("a")))
