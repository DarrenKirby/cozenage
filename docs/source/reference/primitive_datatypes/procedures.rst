Procedures
==========

In Cozenage, procedures are a fundamental, primitive data type, just like numbers, strings, or
booleans. This concept is central to the language's power. Because procedures are data, they are
considered **first-class citizens** which means they can be treated like any other value: stored in
variables, passed as arguments to other procedures, returned as results from procedure calls, and
even collected into lists or other data structures.

Cozenage provides a substantial library of built-in procedures for common tasks — arithmetic
(``+``, ``*``), list manipulation (``car``, ``cdr``, ``cons``), string operations, type predicates,
and more. But the real power of the language emerges when you define your own procedures and
compose them together.


Defining Procedures
-------------------

There are two equivalent ways to create a procedure in Cozenage: the explicit ``lambda`` form, and
the ``define`` shorthand. They produce identical results; the shorthand simply exists for
readability and convenience.


Using ``lambda``
~~~~~~~~~~~~~~~~

The ``lambda`` special form is the primitive mechanism for constructing a new procedure object.
Its syntax is:

.. code-block:: scheme

   (lambda (param ...) body ...)

The result is a procedure value that can be used immediately or bound to a name.

.. code-block:: scheme

   ;; Create a procedure and call it immediately
   ((lambda (x) (* x x)) 5)
   ;; => 25

   ;; Bind a lambda to a name using define
   (define square (lambda (x) (* x x)))
   (square 7)
   ;; => 49

   ;; A procedure with multiple parameters
   (define add (lambda (a b) (+ a b)))
   (add 3 4)
   ;; => 7


Using the ``define`` Shorthand
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``define`` form provides a more readable "sugared" syntax for creating and naming a procedure
in one step. The two forms below are exactly equivalent:

.. code-block:: scheme

   ;; Sugared form
   (define (square x) (* x x))

   ;; Equivalent desugared form
   (define square (lambda (x) (* x x)))

The sugared ``define`` form is idiomatic and will be used throughout most of this documentation,
but it is worth remembering that it is purely a notational convenience — ``lambda`` is always
what underlies it.


Multiple Expressions in a Body
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A procedure body may contain multiple expressions. All are evaluated in order, and the value of the
**last** expression is returned. Earlier expressions are evaluated only for their side effects (such
as printing output).

.. code-block:: scheme

   (define (describe-square x)
     (display "The square of ")
     (display x)
     (display " is ")
     (display (* x x))
     (newline)
     (* x x))   ; <-- this is the return value

   (describe-square 6)
   ;; Prints: The square of 6 is 36
   ;; => 36


Variadic Procedures
~~~~~~~~~~~~~~~~~~~~

A procedure can accept a variable number of arguments by using a "rest" parameter — a bare symbol
after a dot in the parameter list. All extra arguments are collected into a list and bound to that
symbol.

.. code-block:: scheme

   ;; Accept exactly one required argument, plus any number of extras
   (define (first-and-rest first . rest)
     (display "First: ")
     (display first)
     (display ", Rest: ")
     (display rest)
     (newline))

   (first-and-rest 1 2 3 4)
   ;; Prints: First: 1, Rest: (2 3 4)

   ;; Accept any number of arguments with no required ones
   (define (my-list . items) items)

   (my-list 'a 'b 'c)
   ;; => (a b c)


Procedures as Data
------------------

Because procedures are first-class values, they can be stored anywhere a value can be stored.


Storing Procedures in Variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You have already seen this with ``define``. But a procedure can be rebound, passed around, or
temporarily stored in a local variable just like a number or string.

.. code-block:: scheme

   ;; Store a built-in procedure in a local variable and call it
   (define (apply-twice f x)
     (let ((step f))   ; 'step' now holds the same procedure as 'f'
       (step (step x))))

   (apply-twice (lambda (n) (* n 2)) 3)
   ;; => 12


Storing Procedures in Data Structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Procedures can be placed in lists (or any other Cozenage data structure), enabling technique like
dispatch tables.

.. code-block:: scheme

   ;; A list of arithmetic procedure objects
   (define ops (list + - * /))

   ;; Apply each one to 10 and 2
   (map (lambda (op) (op 10 2)) ops)
   ;; => (12 8 20 5)

   ;; A simple dispatch table using an association list
   (define (make-calculator)
     (list
       (cons 'add (lambda (a b) (+ a b)))
       (cons 'sub (lambda (a b) (- a b)))
       (cons 'mul (lambda (a b) (* a b)))))

   (define calc (make-calculator))

   ;; Look up a procedure by name and call it
   (define (calculate op a b)
     ((cdr (assq op calc)) a b))

   (calculate 'add 10 5)   ; => 15
   (calculate 'mul  4 7)   ; => 28


Higher-Order Procedures
-----------------------

A **higher-order procedure** is one that accepts a procedure as an argument, returns a procedure as
its result, or both. This is one of Cozenage's most expressive features.


Passing Procedures as Arguments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The built-in procedures ``map`` and ``filter`` are classic examples: they accept a procedure and
apply it to elements of a list.

.. code-block:: scheme

   ;; map applies a procedure to every element of a list
   (map (lambda (x) (* x x)) '(1 2 3 4 5))
   ;; => (1 4 9 16 25)

   ;; filter keeps only elements for which the predicate returns true
   (filter odd? '(1 2 3 4 5 6))
   ;; => (1 3 5)

You can of course write your own higher-order procedures:

.. code-block:: scheme

   ;; Apply a procedure to a value and display the result
   (define (show-result f x)
     (display (f x))
     (newline))

   (show-result square 9)     ; Prints: 81
   (show-result odd?  7)      ; Prints: #t

   ;; Apply a binary procedure to all adjacent pairs in a list
   (define (pairwise-apply f lst)
     (if (or (null? lst) (null? (cdr lst)))
         '()
         (cons (f (car lst) (cadr lst))
               (pairwise-apply f (cdr lst)))))

   (pairwise-apply + '(1 2 3 4 5))
   ;; => (3 5 7 9)

   (pairwise-apply max '(3 1 4 1 5 9 2 6))
   ;; => (3 4 4 5 9 9 6)


Returning Procedures from Procedures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A procedure can construct and return a *new* procedure as its result. The returned procedure
**closes over** variables from the enclosing scope — this is a *closure*.

.. code-block:: scheme

   ;; Returns a procedure that adds 'n' to its argument
   (define (make-adder n)
     (lambda (x) (+ x n)))

   (define add5  (make-adder 5))
   (define add10 (make-adder 10))

   (add5 3)    ; => 8
   (add10 3)   ; => 13

   ;; Returns a procedure that multiplies by a fixed factor
   (define (make-multiplier factor)
     (lambda (x) (* x factor)))

   (define double (make-multiplier 2))
   (define triple (make-multiplier 3))

   (map double '(1 2 3 4 5))   ; => (2 4 6 8 10)
   (map triple '(1 2 3 4 5))   ; => (3 6 9 12 15)


Combining Higher-Order Techniques
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Higher-order procedures can be combined to build sophisticated abstractions from simple pieces.

.. code-block:: scheme

   ;; Compose two procedures: (compose f g) returns a procedure that
   ;; computes (f (g x))
   (define (compose f g)
     (lambda (x) (f (g x))))

   (define square-then-add1 (compose (make-adder 1) square))

   (square-then-add1 4)   ; => 17  (4^2 = 16, 16+1 = 17)
   (square-then-add1 5)   ; => 26

   ;; Repeatedly apply a procedure n times
   (define (repeat f n)
     (if (= n 0)
         (lambda (x) x)          ; identity procedure: return x unchanged
         (compose f (repeat f (- n 1)))))

   (define add3-five-times (repeat (make-adder 3) 5))

   (add3-five-times 0)    ; => 15
   (add3-five-times 10)   ; => 25

   ;; Build a pipeline of transformations
   (define (pipeline . procs)
     (lambda (x)
       (let loop ((val x) (ps procs))
         (if (null? ps)
             val
             (loop ((car ps) val) (cdr ps))))))

   (define process
     (pipeline
       (make-multiplier 2)      ; double
       (make-adder 10)          ; add 10
       (lambda (x) (* x x))))  ; square

   (process 3)    ; => ((3*2)+10)^2 = 16^2 = 256
   (process 5)    ; => ((5*2)+10)^2 = 20^2 = 400
