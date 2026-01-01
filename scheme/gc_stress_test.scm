;;; Various stress tests to make sure that there are no
;;; references being held around that libgc is not collecting.
;;;
;;; To use this, load the file:
;;; --> (load "scheme/gc_stress_test.scm")
;;;
;;; Run gc-report to get baseline figures:
;;; --> (gc-report)
;;;
;;; This will print two reports, one prior to, and one after
;;; a forced collection:
;;;
;;; --> (gc-report)
;;; Before collection:
;;; ===== GC Report ==========================
;;; Heap size:           262144 bytes (0.25 MB)
;;; Used bytes:          90112 bytes (0.09 MB)
;;; Free bytes:          172032 bytes (0.16 MB)
;;; Unmapped bytes:      0 bytes (0.00 MB)
;;; Bytes since last GC: 74384 bytes
;;; Total bytes allocd:  74384 bytes (lifetime)
;;; GC collections:      1
;;; ==========================================
;;; After collection:
;;; ===== GC Report ==========================
;;; Heap size:           262144 bytes (0.25 MB)
;;; Used bytes:          90112 bytes (0.09 MB)
;;; Free bytes:          172032 bytes (0.16 MB)
;;; Unmapped bytes:      0 bytes (0.00 MB)
;;; Bytes since last GC: 0 bytes
;;; Total bytes allocd:  74384 bytes (lifetime)
;;; GC collections:      2
;;; ==========================================
;;;
;;; Now cycle between running a stress test, and
;;; printing the report multiple times:
;;;
;;; --> (closure-stress 50000)
;;; --> (gc-report)
;;; ...
;;; --> (closure-stress 50000)
;;; --> (gc-report)
;;; ...
;;; --> (closure-stress 50000)
;;; --> (gc-report)
;;; ...
;;;
;;; Watch for the 'Used bytes' figure to return to, or near, the baseline
;;; after each run/collection cycle. A monotonically increasing used bytes
;;; figure after each run of the stress test is a sign of reference leaks.
;;; Do note, hoewver, that '(gc-stress-retain n)' is intended to grow, and
;;; test that necesary references ARE retained.

(define (gc-stress n)
  (let loop ((i 0))
    (if (= i n)
      'done
      (begin
        (string-append "temporary-string-" (number->string i))
        (loop (+ i 1))))))

(define (gc-stress-chunked n chunk-size)
  (define (one-chunk start)
    (let loop ((i start))
      (if (= i (+ start chunk-size))
        'done
        (begin
          (string-append "temp-" (number->string i))
          (loop (+ i 1))))))

  (let outer ((i 0))
    (if (>= i n)
      'done
      (begin
        (one-chunk i)
        (outer (+ i chunk-size))))))

(define retained '())

(define (gc-stress-retain n)
  (let loop ((i 0))
    (if (= i n)
      'done
      (begin
        (set! retained
          (cons (string-append "keep-" (number->string i))
            retained))
        (loop (+ i 1))))))

(define (big-string n)
  (make-string n #\x))

(define (gc-stress-big n size)
  (let loop ((i 0))
    (if (= i n)
      'done
      (begin
        (big-string size)
        (loop (+ i 1))))))

(define (closure-stress n)
  (let loop ((i 0))
    (if (= i n)
      'done
      (begin
        ((lambda (x) (lambda () x)) i)
        (loop (+ i 1))))))
