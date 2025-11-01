;;; Some helpful definitions whilst
;;; working through "The Little Schemer"

(define atom? (lambda (x)
     (and (not (pair? x)) (not (null? x)))))

(define (lat? l)
    (if (null? l) #t
        (if (atom? (car l))
            (lat? (cdr l)) #f)))

(define (member? a lat)
    (cond ((null? lat) #f)
      (else (or (eq? (car lat) a)
           (member? a (cdr lat))))))
