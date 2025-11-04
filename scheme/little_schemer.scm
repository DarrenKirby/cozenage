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

(define (rember a lat)
   (cond ((null? lat) '())
         ((eq? (car lat) a) (cdr lat))
          (else (cons (car lat) (rember a (cdr lat))))))

(define (firsts l)
  (cond ((null? l) '())
    (else (cons (car (car l)) (firsts (cdr l))))))

(define (insertr new old lat)
    (cond ((null? lat) '())
      (else (cond ((eq? (car lat) old) (cons old (cons new (cdr lat))))
           (else (cons (car lat) (insertr new old (cdr lat))))))))
