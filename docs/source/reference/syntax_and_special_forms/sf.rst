Special Forms Documentation
---------------------------

.. _sf:define:

.. describe:: (define variable expr)
              (define (name formals ...) body)

    Evaluates *expr* and binds the result to *variable* in the global
    environment, or defines a named procedure. ``define`` is the primary
    mechanism for introducing new bindings and is the top-level form from
    which all programs are constructed.

    It is an error to use a syntactic keyword as the target of a ``define``;
    attempts to rebind forms such as ``lambda``, ``if``, or ``let`` will raise
    an error. Built-in procedures, however, may be rebound.

    At the top level, ``define`` creates a binding in the global environment.
    Inside a procedure body or other local context, ``define`` may also appear
    at the head of a sequence of expressions — these *internal defines* are
    automatically transformed into an equivalent ``letrec`` expression, creating
    local bindings scoped to the enclosing body. The result is identical to
    writing ``letrec`` explicitly:

    .. code-block:: scheme

      (define (f x)
        (define a 1)
        (define b 2)
        (+ x a b))

      ; Is equivalent to:
      (define (f x)
        (letrec ((a 1)
                 (b 2))
          (+ x a b)))

    To create local bindings outside of an implicit body context, use ``let``,
    ``letrec``, or ``let*`` explicitly.

    **Variable binding**

    The basic form binds *variable* to the result of evaluating *expr*:

    .. code-block:: scheme

      (define variable expr)

    *expr* is evaluated before the binding is created. The bound name is
    returned.

    .. code-block:: scheme

      --> (define x 42)
      x
      --> x
      42
      --> (define greeting (string-append "hello" " " "world"))
      greeting
      --> greeting
      "hello world"
      --> (define double (lambda (n) (* n 2)))
      --> (double 5)
      10

    **Procedure shorthand**

    As a convenience, procedures may be defined without writing ``lambda``
    explicitly. The first element of the head list is the procedure name; the
    remaining elements are its formal parameters:

    .. code-block:: scheme

      (define (name formal1 formal2 ...) body)

    This is exactly equivalent to:

    .. code-block:: scheme

      (define name (lambda (formal1 formal2 ...) body))

    The procedure object is returned.

    .. code-block:: scheme

      --> (define (square x) (* x x))
      --> (square 7)
      49
      --> (define (add a b) (+ a b))
      --> (add 3 4)
      7

    **Variadic procedures**

    A procedure that accepts any number of arguments may be defined by using a
    bare symbol as the formal parameter list rather than a parenthesised list.
    All arguments are bound to that symbol as a list:

    .. code-block:: scheme

      --> (define (sum . args) (foldl + 0 args))
      --> (sum 1 2 3 4 5)
      15

    A procedure that accepts a fixed number of required arguments followed by
    zero or more additional arguments uses a dotted-tail formal list. The
    required arguments are bound normally; all remaining arguments are
    collected into a list and bound to the final symbol:

    .. code-block:: scheme

      --> (define (define (log label . values)
      ...   (display label)
      ...   (for-each (lambda (v) (display " ") (display v)) values))
      --> (log "result:" 1 2 3)
      result: 1 2 3

    **Multi-expression body**

    The procedure body may contain multiple expressions. They are evaluated in
    order and the value of the last expression is returned:

    .. code-block:: scheme

      --> (define (describe x)
      ...   (display "The value is: ")
      ...   (display x)
      ...   (newline)
      ...   x)
      --> (describe 99)
      The value is: 99
      99

    **Redefining bindings**

    An existing binding may be shadowed by defining it again. The new value
    replaces the old one globally:

    .. code-block:: scheme

      --> (define x 1)
      --> x
      1
      --> (define x 2)
      --> x
      2

    To mutate an existing binding in place, use ``set!`` instead.

.. _sf:lambda:

.. describe:: (lambda formals body)

    Evaluates to a procedure. ``lambda`` is the fundamental procedure
    construction form from which all other procedure-defining forms are
    ultimately derived. The ``define`` shorthand, named ``let``, and several
    other forms all expand to ``lambda`` internally.

    When a ``lambda`` expression is evaluated, the current environment is
    captured as part of the resulting procedure object. This captured
    environment is called the *closing environment*, and the procedure is said
    to *close over* any variables it references from that environment. When the
    procedure is later called, a new *invocation environment* is created by
    extending the closing environment with bindings of the formal parameters to
    the supplied arguments. The body is then evaluated in this invocation
    environment.

    The body may contain multiple expressions, which are evaluated in order.
    The value of the last expression is returned as the result of the
    procedure call. Leading ``define`` forms in the body are treated as
    internal definitions and are transformed into an equivalent ``letrec``
    expression scoped to the body.

    **Standard formals**

    The most common form takes a parenthesised list of zero or more symbols as
    its formal parameter list. Each symbol is bound to the corresponding
    argument when the procedure is called. It is an error to call the procedure
    with the wrong number of arguments.

    .. code-block:: scheme

      (lambda (formal1 formal2 ...) body)

    .. code-block:: scheme

      --> (define square (lambda (x) (* x x)))
      --> (square 7)
      49
      --> (define add (lambda (a b) (+ a b)))
      --> (add 3 4)
      7
      --> ((lambda (x y) (* x y)) 6 7)
      42

    A zero-argument lambda is written with an empty formal list:

    .. code-block:: scheme

      --> (define greet (lambda () "hello"))
      --> (greet)
      "hello"

    **Variadic formals**

    A bare symbol (not wrapped in a list) as the formal parameter accepts any
    number of arguments, which are bound collectively as a list:

    .. code-block:: scheme

      (lambda args body)

    .. code-block:: scheme

      --> (define my-list (lambda args args))
      --> (my-list 1 2 3)
      (1 2 3)
      --> (my-list)
      ()

    **Dotted-tail formals**

    A dotted-tail formal list accepts one or more required positional arguments
    followed by zero or more additional arguments. The required arguments are
    bound individually; all remaining arguments are collected into a list and
    bound to the final symbol after the dot:

    .. code-block:: scheme

      (lambda (formal1 formal2 . rest) body)

    .. code-block:: scheme

      --> (define f (lambda (a b . rest) (list a b rest)))
      --> (f 1 2 3 4 5)
      (1 2 (3 4 5))
      --> (f 1 2)
      (1 2 ())

    **Closures**

    Because a ``lambda`` captures its enclosing environment, it can reference
    variables from that environment even after the enclosing expression has
    returned. This is the basis of *closures*, one of the most powerful
    features of Scheme:

    .. code-block:: scheme

      --> (define (make-adder n)
      ...   (lambda (x) (+ x n)))
      --> (define add5 (make-adder 5))
      --> (add5 10)
      15
      --> (define add10 (make-adder 10))
      --> (add10 10)
      20

    Each call to ``make-adder`` produces a distinct procedure that closes over
    its own binding of ``n``.

    **Multi-expression body**

    The body may contain multiple expressions. They are evaluated in order and
    the value of the last is returned:

    .. code-block:: scheme

      --> (define (describe x)
      ...   (display "value: ")
      ...   (display x)
      ...   (newline)
      ...   x)
      --> (describe 42)
      value: 42
      42

    :param formals: A symbol, a list of symbols, or a dotted-tail list of
                    symbols specifying the formal parameters.
    :param body: One or more expressions forming the procedure body.
    :return: A procedure object.
    :rtype: procedure

.. _sf:quote:

.. describe:: (quote datum)
              'datum

    Returns *datum* as a literal value without evaluating it. ``quote`` is the
    mechanism for introducing literal constants into Scheme code: without it,
    any symbol or list appearing in an expression would be interpreted as a
    variable reference or procedure call respectively.

    The shorthand notation ``'datum`` is exactly equivalent to
    ``(quote datum)`` and is transformed into the full form by the parser
    before evaluation. The two forms are entirely interchangeable.

    *datum* may be any external representation of a Scheme object: a symbol,
    a number, a boolean, a character, a string, a list, a vector, or any other
    literal value.

    :param datum: Any Scheme object to return as a literal.
    :return: *datum*, unevaluated.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (quote foo)
      foo
      --> 'foo
      foo
      --> '(1 2 3)
      (1 2 3)
      --> (quote (1 2 3))
      (1 2 3)
      --> '(a b c)
      (a b c)
      --> '#(1 2 3)
      #(1 2 3)
      --> '()
      ()

    Without ``quote``, a symbol would be looked up as a variable and a list
    would be treated as a procedure call:

    .. code-block:: scheme

      --> (define x 42)
      --> x
      42
      --> 'x
      x
      --> (+ 1 2)
      3
      --> '(+ 1 2)
      (+ 1 2)

    Note that numbers, booleans, strings, and characters are
    *self-evaluating* — they evaluate to themselves without needing to be
    quoted. ``quote`` is most commonly needed for symbols and lists:

    .. code-block:: scheme

      --> 42
      42
      --> "hello"
      "hello"
      --> #t
      #t
      --> #\a
      #\a

.. _sf:if:

.. describe:: (if test consequent)
              (if test consequent alternate)

    Evaluates *test* and, depending on the result, evaluates and returns
    either *consequent* or *alternate*. ``if`` is the fundamental conditional
    form from which all other conditional constructs (``cond``, ``when``,
    ``unless``, ``case``) are ultimately derived.

    *test* is evaluated first. If the result is any value other than ``#f``,
    the test is considered true and *consequent* is evaluated and its value
    returned. If the result is ``#f``, *alternate* is evaluated and its value
    returned. If no *alternate* is provided and the test is false, an
    unspecified value is returned.

    Note that only ``#f`` is considered false — every other value, including
    ``0``, the empty list ``()``, and the empty string ``""``, is considered
    true. This is a fundamental property of Scheme's boolean model.

    Both *consequent* and *alternate* are evaluated as tail calls, meaning
    ``if`` participates correctly in tail-call optimisation. A conditional in
    tail position does not consume additional stack depth.

    :param test: The condition to evaluate.
    :type test: any
    :param consequent: The expression to evaluate if *test* is true.
    :param alternate: The expression to evaluate if *test* is false. If
                      omitted and *test* is false, an unspecified value is
                      returned.
    :return: The value of *consequent* or *alternate*, depending on *test*.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (if #t "yes" "no")
      "yes"
      --> (if #f "yes" "no")
      "no"
      --> (if (> 3 2) "greater" "not greater")
      "greater"
      --> (if 0 "true" "false")
      "true"
      --> (if '() "true" "false")
      "true"
      --> (if #f "unreachable")

    ``if`` is most commonly used to branch on the result of a predicate:

    .. code-block:: scheme

      --> (define (abs x)
      ...   (if (< x 0) (- x) x))
      --> (abs -5)
      5
      --> (abs 5)
      5

    Nested ``if`` expressions can express multi-way conditionals, though
    ``cond`` is usually clearer for more than two branches:

    .. code-block:: scheme

      --> (define (classify n)
      ...   (if (< n 0)
      ...       "negative"
      ...       (if (= n 0)
      ...           "zero"
      ...           "positive")))
      --> (classify -1)
      "negative"
      --> (classify 0)
      "zero"
      --> (classify 1)
      "positive"


.. _sf:cond:

.. describe:: (cond clause ...)

    Evaluates a sequence of test clauses in order, executing the body of the
    first clause whose test returns a true value. ``cond`` is the idiomatic
    Scheme form for multi-way conditionals and is cleaner than deeply nested
    ``if`` expressions when more than two branches are needed.

    ``cond`` is a derived form, transformed at parse time into a nest of
    ``let`` and ``if`` expressions.

    Each *clause* takes one of the following forms:

    .. code-block:: scheme

      (test expr ...)        ; standard clause
      (test => proc)         ; arrow clause
      (test)                 ; test-only clause
      (else expr ...)        ; else clause

    Clauses are evaluated in order from top to bottom. The first clause whose
    *test* returns a true value is selected and its body evaluated. Remaining
    clauses are not evaluated. If no clause matches and no ``else`` clause is
    present, an unspecified value is returned.

    **Standard clause**

    If *test* evaluates to a true value, the body expressions are evaluated
    in order and the value of the last is returned:

    .. code-block:: scheme

      --> (cond ((= 1 2) "no")
      ...       ((= 1 1) "yes"))
      "yes"
      --> (define (classify n)
      ...   (cond ((< n 0) "negative")
      ...         ((= n 0) "zero")
      ...         (else    "positive")))
      --> (classify -3)
      "negative"
      --> (classify 0)
      "zero"
      --> (classify 7)
      "positive"

    **Test-only clause**

    If a clause contains only a test expression and no body, the value of
    the test itself is returned when it is true:

    .. code-block:: scheme

      --> (cond (#f)
      ...       (42))
      42
      --> (cond ((assv 2 '((1 "one") (2 "two"))))
      ...       (else #f))
      (2 "two")

    The test-only form is useful when the test expression itself produces the
    desired result, as in the ``assv`` example above.

    **Arrow clause**

    The ``=>`` form evaluates *test*, and if it is true, calls *proc* with
    the value of *test* as its single argument. It is an error if *proc* does
    not accept exactly one argument. The test expression is evaluated exactly
    once:

    .. code-block:: scheme

      (test => proc)

    .. code-block:: scheme

      --> (cond ((assv 2 '((1 "one") (2 "two"))) => cdr)
      ...       (else "not found"))
      ("two")
      --> (cond ((+ 1 1) => (lambda (x) (* x x))))
      4

    The arrow form is equivalent to:

    .. code-block:: scheme

      (let ((tmp test))
        (if tmp (proc tmp) ...))

    **Else clause**

    An ``else`` clause may appear as the last clause and matches when no
    preceding test succeeded. It is an error for ``else`` to appear in any
    position other than last:

    .. code-block:: scheme

      --> (cond (#f "no")
      ...       (else "fallback"))
      "fallback"

    ``else`` may not be used as a variable name, as it is a reserved
    syntactic keyword.

    :param clause: One or more clauses, each beginning with a test expression
                   or ``else``. The last clause may be an ``else`` clause.
    :return: The value of the last expression in the first matching clause,
             the value of the test in a test-only clause, the result of
             calling *proc* on the test value in an arrow clause, or
             unspecified if no clause matches.
    :rtype: any

.. _sf:import:

.. describe:: (import import-set ...)

    Loads one or more named libraries into the current environment, making
    their exported bindings available for use. Each *import-set* is a
    two-element list of the form ``(library-group name)``, where
    *library-group* identifies the collection the library belongs to and
    *name* identifies the specific library within that collection.

    .. note::

        User-defined and third-party library loading is not yet supported.
        Currently, ``import`` only loads libraries from the built-in
        Cozenage standard library collection, referenced using the ``base``
        group identifier. Support for user and third-party libraries,
        including the R7RS import modifiers ``only``, ``except``, ``prefix``,
        and ``rename``, is planned for a future release.

    **Cozenage standard libraries**

    All built-in Cozenage libraries are referenced using ``base`` as the
    library group. The following libraries are currently available:

    - ``bits`` — bitwise operations on integers
    - ``cxr`` — extended ``car``/``cdr`` compositions (e.g. ``caddr``,
      ``cadddr``)
    - ``file`` — procedures for working with the local filesystem
    - ``lazy`` — lazy evaluation and streams
    - ``math`` — mathematical procedures, covering the R7RS ``inexact`` and
      ``complex`` libraries
    - ``random`` — random number generation and collection shuffling
    - ``system`` — OS and hardware interfacing procedures
    - ``time`` — date and time procedures

    Multiple libraries may be imported in a single ``import`` expression by
    providing multiple *import-sets*:

    .. code-block:: scheme

      (import (base bits) (base math))

    :param import-set: One or more two-element lists of the form
                       ``(library-group name)``.
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

      --> (import (base math))
      #t
      --> (import (base bits))
      #t
      --> (import (base bits) (base random))
      #t
      --> (import (my-libs utils))
      Error: import: user-defined libraries not yet supported

.. _sf:let:

.. describe:: (let bindings body)
              (let name bindings body)

    Evaluates a sequence of expressions in a new local environment extended
    with a set of variable bindings. ``let`` is the primary form for
    introducing local variables, and together with ``lambda`` forms the
    foundation of lexical scoping in Scheme.

    **Standard** ``let``

    Each binding in *bindings* is a two-element list ``(variable init)``.
    All *init* expressions are evaluated first, in the current environment
    and in an unspecified order. A new child environment is then created in
    which each *variable* is bound to the result of its corresponding *init*.
    The body expressions are evaluated in order in this new environment, and
    the value of the last expression is returned.

    Because all *init* expressions are evaluated before any binding takes
    place, a binding's *init* cannot refer to other variables being bound in
    the same ``let`` — for that, use ``let*`` or ``letrec``.

    .. code-block:: scheme

      (let ((variable1 init1)
            (variable2 init2)
            ...)
        body ...)

    .. code-block:: scheme

      --> (let ((x 1) (y 2)) (+ x y))
      3
      --> (let ((x 10))
      ...   (let ((x 20) (y x))
      ...     y))
      10
      --> (let ((a 3) (b 4))
      ...   (define (hyp a b) (sqrt (+ (* a a) (* b b))))
      ...   (hyp a b))
      5.0

    The last example illustrates that an internal ``define`` is permitted at
    the head of a ``let`` body and are scoped locally to that body.

    It is an error to provide duplicate variable names within the same
    ``let`` binding list.

    **Named** ``let``

    When the first argument after ``let`` is a symbol rather than a binding
    list, the form is a *named let*. This variant binds the symbol to a
    procedure whose parameters are the bound variables and whose body is the
    ``let`` body, then immediately calls it with the initial values. The named
    procedure is visible within the body, allowing it to be called recursively
    to implement iteration or recursion without defining a separate procedure.

    .. code-block:: scheme

      (let name ((variable1 init1)
                 (variable2 init2)
                 ...)
        body ...)

    Named ``let`` is transformed at parse time into an equivalent ``letrec``
    expression:

    .. code-block:: scheme

      ; This named let:
      (let loop ((i 0) (acc 0))
        (if (> i 10)
            acc
            (loop (+ i 1) (+ acc i))))

      ; Is equivalent to:
      (letrec ((loop (lambda (i acc)
                       (if (> i 10)
                           acc
                           (loop (+ i 1) (+ acc i))))))
        (loop 0 0))

    Named ``let`` is the idiomatic Scheme way to express loops. Because the
    recursive call to *name* is in tail position, it is optimised into an
    iteration and does not consume additional stack depth:

    .. code-block:: scheme

      --> (let loop ((i 0) (acc 0))
      ...   (if (> i 10)
      ...       acc
      ...       (loop (+ i 1) (+ acc i))))
      55
      --> (let fact ((n 5) (acc 1))
      ...   (if (= n 0)
      ...       acc
      ...       (fact (- n 1) (* acc n))))
      120
      --> (let build ((n 5) (result '()))
      ...   (if (= n 0)
      ...       result
      ...       (build (- n 1) (cons n result))))
      (1 2 3 4 5)

    :param bindings: A list of ``(variable init)`` pairs, or in the named
                     form, a symbol followed by a list of ``(variable init)``
                     pairs.
    :param body: One or more expressions to evaluate in the extended
                 environment. The value of the last expression is returned.
    :return: The value of the last body expression.
    :rtype: any

.. _sf:let*:

.. describe:: (let* bindings body)

    Like ``let``, but evaluates and binds each variable sequentially from
    left to right, so that each *init* expression is evaluated in an
    environment where all preceding bindings are already visible. This makes
    ``let*`` useful when later bindings depend on earlier ones.

    Each binding in *bindings* is a two-element list ``(variable init)``.
    Unlike ``let``, variable names need not be distinct — a later binding
    may shadow an earlier one within the same ``let*``.

    ``let*`` is a derived form, transformed at parse time into a nest of
    sequential ``let`` expressions, one per binding:

    .. code-block:: scheme

      ; This let*:
      (let* ((x 1)
             (y (+ x 1))
             (z (+ y 1)))
        (list x y z))

      ; Is equivalent to:
      (let ((x 1))
        (let ((y (+ x 1)))
          (let ((z (+ y 1)))
            (list x y z))))

    .. code-block:: scheme

      (let* ((variable1 init1)
             (variable2 init2)
             ...)
        body ...)

    .. code-block:: scheme

      --> (let* ((x 1) (y (+ x 1))) (list x y))
      (1 2)
      --> (let* ((x 2) (y (* x 3)) (z (+ x y))) z)
      8
      --> (let* ((x 10) (x (+ x 1)) (x (* x 2))) x)
      22
      --> (let* () 42)
      42

    The last two examples illustrate two notable properties: a variable may
    be rebound within the same ``let*`` (each occurrence shadows the
    previous), and an empty binding list is valid and simply evaluates the
    body in the current environment.

    Contrast with ``let``, where all *init* expressions are evaluated before
    any binding takes place:

    .. code-block:: scheme

      --> (let ((x 1))
      ...   (let ((x 2) (y x))
      ...     y))
      1        ; y is bound to the outer x = 1

      --> (let ((x 1))
      ...   (let* ((x 2) (y x))
      ...     y))
      2        ; y is bound to the inner x = 2

    :param bindings: A list of ``(variable init)`` pairs, evaluated and
                     bound left to right. Duplicate names are permitted.
    :param body: One or more expressions to evaluate in the extended
                 environment. The value of the last expression is returned.
    :return: The value of the last body expression.
    :rtype: any

.. _sf:letrec:

.. describe:: (letrec bindings body)

    Like ``let``, but all bindings are visible to all *init* expressions,
    making it possible to define mutually recursive procedures. ``letrec`` is
    the standard form for expressing groups of procedures that call each other.

    Each binding in *bindings* is a two-element list ``(variable init)``. The
    variables are first bound to unspecified placeholder values in a new local
    environment. All *init* expressions are then evaluated in that environment
    — where all variables are already in scope — and each variable is updated
    to the result of its corresponding *init*. The body expressions are then
    evaluated in order in the resulting environment, and the value of the last
    expression is returned.

    The critical restriction on ``letrec`` is that no *init* expression may
    refer to the *value* of any variable being bound in the same ``letrec``
    at the time of evaluation. In practice this restriction is almost never a
    concern, since the most common use of ``letrec`` is to bind ``lambda``
    expressions — a lambda captures its environment but does not evaluate the
    body until it is called, so the restriction is satisfied automatically.
    Attempting to use the value of a binding during initialisation (for
    example by calling one of the procedures being defined from within an
    *init* expression) is an error.

    It is an error for a variable to appear more than once in the binding
    list.

    .. code-block:: scheme

      (letrec ((variable1 init1)
               (variable2 init2)
               ...)
        body ...)

    .. code-block:: scheme

      --> (letrec ((even? (lambda (n)
      ...                   (if (= n 0) #t (odd? (- n 1)))))
      ...          (odd?  (lambda (n)
      ...                   (if (= n 0) #f (even? (- n 1))))))
      ...   (even? 10))
      #t

      --> (letrec ((fact (lambda (n acc)
      ...                  (if (= n 0)
      ...                      acc
      ...                      (fact (- n 1) (* n acc))))))
      ...   (fact 10 1))
      3628800

    The key distinction from ``let`` and ``let*`` is mutual visibility: in
    the ``even?``/``odd?`` example above, each procedure can refer to the
    other because both names are in scope when the lambdas are evaluated.
    This would not be possible with ``let`` or ``let*``.

    An internal ``define`` at the head of the body is permitted and they are
    scoped locally to the ``letrec`` body.

    **Relationship to** ``letrec*``

    ``letrec`` evaluates all *init* expressions before performing any
    assignments, and the order of evaluation is unspecified. ``letrec*``
    evaluates and assigns each binding sequentially from left to right,
    meaning later *init* expressions may safely refer to the values of
    earlier bindings. Use ``letrec`` when defining mutually recursive
    procedures where initialisation order does not matter; use ``letrec*``
    when bindings must be initialised in a specific order.

    :param bindings: A list of ``(variable init)`` pairs. All variables are
                     in scope during evaluation of all *init* expressions.
                     Duplicate variable names are not permitted.
    :param body: One or more expressions to evaluate in the extended
                 environment. The value of the last expression is returned.
    :return: The value of the last body expression.
    :rtype: any

.. _sf:letrec*:

.. describe:: (letrec* bindings body)

    Like ``letrec``, but evaluates and assigns each binding sequentially from
    left to right, so that each *init* expression is evaluated in an
    environment where all preceding bindings have already been assigned their
    values. This makes ``letrec*`` useful when mutually recursive procedures
    need to be defined and their initialisers have dependencies on one another.

    It is an error for a variable to appear more than once in the binding
    list.

    ``letrec*`` is a derived form, transformed at parse time by peeling each
    binding into a nested ``letrec``, one per binding:

    .. code-block:: scheme

      ; This letrec*:
      (letrec* ((x 1)
                (y (+ x 1))
                (z (+ x y)))
        (list x y z))

      ; Is equivalent to:
      (letrec ((x 1))
        (letrec ((y (+ x 1)))
          (letrec ((z (+ x y)))
            (list x y z))))

    An empty binding list is valid and is equivalent to ``(letrec () body...)``:

    .. code-block:: scheme

      (letrec* ((variable1 init1)
                (variable2 init2)
                ...)
        body ...)

    .. code-block:: scheme

      --> (letrec* ((x 1) (y (+ x 1)) (z (+ x y)))
      ...   (list x y z))
      (1 2 3)

      --> (letrec* ((even? (lambda (n)
      ...                    (if (= n 0) #t (odd? (- n 1)))))
      ...           (odd?  (lambda (n)
      ...                    (if (= n 0) #f (even? (- n 1))))))
      ...   (odd? 7))
      #t

    The mutual recursion example works because even though bindings are
    evaluated left to right, each *init* is a ``lambda`` — the body of the
    lambda is not evaluated until the procedure is called, by which point all
    bindings are fully initialised.

    **Relationship to** ``letrec``

    ``letrec`` evaluates all *init* expressions before performing any
    assignments and in an unspecified order, whereas ``letrec*`` evaluates
    and assigns each binding in strict left-to-right order. For bindings that
    are all ``lambda`` expressions with no inter-dependencies in the *init*
    expressions themselves, the two forms are interchangeable. Use ``letrec*``
    when later *init* expressions depend on the values of earlier bindings.

    :param bindings: A list of ``(variable init)`` pairs, evaluated and
                     assigned left to right. All variables are in scope
                     during evaluation of all *init* expressions. Duplicate
                     variable names are not permitted.
    :param body: One or more expressions to evaluate in the extended
                 environment. The value of the last expression is returned.
    :return: The value of the last body expression.
    :rtype: any

.. _sf:set!:

.. describe:: (set! variable expr)

    Evaluates *expr* and stores the result in the location to which *variable*
    is already bound. Unlike ``define``, which always creates a new binding,
    ``set!`` mutates an existing one. It is an error if *variable* is not
    already bound either in some enclosing local scope or in the global
    environment.

    ``set!`` searches for the binding by walking up the local environment
    chain from the innermost scope outward, then checking the global
    environment if no local binding is found. The assignment is made in
    whichever frame the binding is found — so a ``set!`` inside a nested
    scope will correctly mutate the binding in the enclosing scope where it
    was originally established, rather than creating a new local binding.

    It is an error to use a syntactic keyword as the target of ``set!``.
    Returns an unspecified value.

    :param variable: The name of an already-bound variable to mutate.
    :type variable: symbol
    :param expr: The expression whose value will be stored.
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

      --> (define x 1)
      --> x
      1
      --> (set! x 42)
      --> x
      42

    ``set!`` mutates the binding in whichever scope it was originally
    established:

    .. code-block:: scheme

      --> (define counter 0)
      --> (define (increment!) (set! counter (+ counter 1)))
      --> (increment!)
      --> counter
      1
      --> (increment!)
      --> counter
      2

    ``set!`` inside a closure mutates the closed-over binding, making it
    possible to implement stateful objects:

    .. code-block:: scheme

      --> (define (make-counter)
      ...   (let ((n 0))
      ...     (lambda ()
      ...       (set! n (+ n 1))
      ...       n)))
      --> (define c (make-counter))
      --> (c)
      1
      --> (c)
      2
      --> (define c2 (make-counter))
      --> (c2)
      1
      --> (c)
      3

    Each call to ``make-counter`` produces a closure over a distinct binding
    of ``n``, so ``c`` and ``c2`` maintain independent counts.

    Attempting to ``set!`` an unbound variable raises an error:

    .. code-block:: scheme

      --> (set! undefined-var 42)
      Error: set!: Unbound symbol: 'undefined-var'

.. _sf:begin:

.. describe:: (begin expr ...)

    Evaluates *expr* expressions sequentially from left to right and returns
    the value of the last expression. ``begin`` is used to group multiple
    expressions into a single form wherever only one expression is expected,
    and to sequence side effects such as assignments, display calls, or
    input/output operations.

    All but the last expression are evaluated purely for their side effects;
    their return values are discarded. The last expression is evaluated as a
    tail call, so ``begin`` participates correctly in tail-call optimisation
    when used in tail position.

    :param expr: One or more expressions to evaluate in order.
    :return: The value of the last expression.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (begin 1 2 3)
      3
      --> (begin
      ...   (display "hello ")
      ...   (display "world")
      ...   (newline))
      hello world
      --> (define x 0)
      --> (begin
      ...   (set! x (+ x 1))
      ...   (set! x (+ x 1))
      ...   x)
      2

    ``begin`` is commonly used in conditional branches that need to perform
    multiple operations:

    .. code-block:: scheme

      --> (define (log-and-return msg val)
      ...   (begin
      ...     (display msg)
      ...     (newline)
      ...     val))
      --> (log-and-return "computing..." 42)
      computing...
      42

    Note that in a ``lambda`` or ``let`` body, multiple expressions are
    already evaluated sequentially without needing ``begin`` explicitly —
    the body itself acts as an implicit ``begin``:

    .. code-block:: scheme

      --> (define (f x)
      ...   (display x)
      ...   (newline)
      ...   (* x x))

      ; Is equivalent to:
      --> (define (f x)
      ...   (begin
      ...     (display x)
      ...     (newline)
      ...     (* x x)))

    .. note::

        R7RS specifies a second *splicing* form of ``begin`` for use at the
        top level, where ``begin`` acts as a transparent wrapper that splices
        its contents — including ``define`` forms — into the surrounding
        context as if they had been written there directly. This
        implementation supports only the expression form documented here.


.. _sf:and:

.. describe:: (and test ...)

    Evaluates *test* expressions from left to right, returning ``#f``
    immediately if any expression evaluates to ``#f``, without evaluating
    any remaining expressions. If all expressions evaluate to true values,
    the value of the last expression is returned. If no expressions are
    provided, returns ``#t``.

    This short-circuit behaviour makes ``and`` useful both as a logical
    connective and as a conditional guard — the expressions to the right of
    any given test are only evaluated if all preceding tests passed.

    The last expression is evaluated as a tail call, so ``and`` participates
    correctly in tail-call optimisation when used in tail position.

    Note that ``and`` returns the value of the last expression rather than
    simply ``#t`` when all tests pass. This means ``and`` can be used to
    return a meaningful value when all conditions are satisfied, not just a
    boolean.

    :param test: Zero or more expressions to evaluate in order.
    :return: ``#f`` if any expression evaluates to ``#f``; otherwise the
             value of the last expression. Returns ``#t`` if no expressions
             are provided.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (and #t #t)
      #t
      --> (and #t #f)
      #f
      --> (and 1 2 3)
      3
      --> (and 1 #f 3)
      #f
      --> (and)
      #t

    Short-circuit evaluation means later expressions are not evaluated if an
    earlier one returns ``#f``:

    .. code-block:: scheme

      --> (define x 0)
      --> (and #f (set! x 99))
      #f
      --> x
      0

    ``and`` returns the value of the last expression, not merely ``#t``:

    .. code-block:: scheme

      --> (and (> 3 2) (* 6 7))
      42
      --> (and (string? "hello") (string-length "hello"))
      5

    ``and`` is commonly used to chain guard conditions before a computation:

    .. code-block:: scheme

      --> (define (safe-divide a b)
      ...   (and (number? b)
      ...        (not (zero? b))
      ...        (/ a b)))
      --> (safe-divide 10 2)
      5
      --> (safe-divide 10 0)
      #f

.. _sf:or:

.. describe:: (or test ...)

    Evaluates *test* expressions from left to right, returning the value of
    the first expression that evaluates to a true value, without evaluating
    any remaining expressions. If all expressions evaluate to ``#f``, returns
    ``#f``. If no expressions are provided, returns ``#f``.

    This short-circuit behaviour makes ``or`` useful both as a logical
    connective and as a conditional default — expressions to the right of
    any given test are only evaluated if all preceding tests returned ``#f``.

    Like ``and``, ``or`` returns the actual value of the first true expression
    rather than simply ``#t``. This makes it useful for expressing a sequence
    of fallback values, returning the first one that is not ``#f``.

    ``or`` is a derived form, transformed at parse time into a nest of ``let``
    and ``if`` expressions. Each test expression is bound to a temporary
    variable to ensure it is evaluated exactly once before being tested and
    potentially returned:

    .. code-block:: scheme

      ; This or:
      (or e1 e2 e3)

      ; Is equivalent to:
      (let ((tmp e1))
        (if tmp
            tmp
            (let ((tmp e2))
              (if tmp
                  tmp
                  e3))))

    :param test: Zero or more expressions to evaluate in order.
    :return: The value of the first expression that evaluates to a true value,
             or ``#f`` if all expressions evaluate to ``#f``, or if no
             expressions are provided.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (or #f #f)
      #f
      --> (or #f 42)
      42
      --> (or 1 2 3)
      1
      --> (or #f #f 3)
      3
      --> (or)
      #f

    Short-circuit evaluation means later expressions are not evaluated once
    a true value is found:

    .. code-block:: scheme

      --> (define x 0)
      --> (or 42 (set! x 99))
      42
      --> x
      0

    ``or`` returns the value of the first true expression, not merely ``#t``:

    .. code-block:: scheme

      --> (or #f (+ 1 2))
      3
      --> (or #f #f (* 6 7))
      42

    A common idiom is to use ``or`` to supply a default value when a
    computation might return ``#f``:

    .. code-block:: scheme

      --> (define (lookup key alist)
      ...   (let ((result (assv key alist)))
      ...     (or result "not found")))
      --> (lookup 2 '((1 . "one") (2 . "two")))
      (2 . "two")
      --> (lookup 99 '((1 . "one") (2 . "two")))
      "not found"

.. _sf:case:

.. describe:: (case key clause ...)

    Evaluates *key* and compares its value against lists of literal data
    values in each clause, executing the body of the first matching clause.
    ``case`` is the idiomatic form for dispatch on a fixed set of known
    values, analogous to a switch statement in other languages.

    ``case`` is a derived form, transformed at parse time into a ``let``
    wrapping a ``cond`` expression. The *key* expression is evaluated exactly
    once and its value bound to a temporary variable, which is then tested
    against each datum list using ``memv``.

    Each *clause* takes one of the following forms:

    .. code-block:: scheme

      ((datum ...) expr ...)      ; standard clause
      ((datum ...) => proc)       ; arrow clause
      (else expr ...)             ; else clause
      (else => proc)              ; else arrow clause

    The *datum* values are literal constants — they are not evaluated. Datum
    comparison uses ``eqv?``, making ``case`` suitable for dispatching on
    numbers, characters, symbols, and booleans. It is an error for the same
    datum to appear in more than one clause.

    **Standard clause**

    If the value of *key* is ``eqv?`` to any datum in the clause's datum
    list, the body expressions are evaluated in order and the value of the
    last is returned. Remaining clauses are not evaluated:

    .. code-block:: scheme

      --> (case (* 2 3)
      ...   ((2 3 5 7) "prime")
      ...   ((1 4 6 8 9) "composite"))
      "composite"

      --> (define (day-type day)
      ...   (case day
      ...     ((monday tuesday wednesday thursday friday) "weekday")
      ...     ((saturday sunday) "weekend")
      ...     (else "unknown")))
      --> (day-type 'monday)
      "weekday"
      --> (day-type 'saturday)
      "weekend"
      --> (day-type 'holiday)
      "unknown"

    **Arrow clause**

    The ``=>`` form evaluates *proc* and calls it with the value of *key* as
    its single argument. It is an error if *proc* does not accept exactly one
    argument:

    .. code-block:: scheme

      --> (case (* 2 3)
      ...   ((6) => (lambda (x) (* x x)))
      ...   (else 0))
      36

    The ``=>`` form is also valid in an ``else`` clause:

    .. code-block:: scheme

      --> (case 99
      ...   ((1 2 3) "small")
      ...   (else => (lambda (x) (string-append "got: " (number->string x)))))
      "got: 99"

    **Else clause**

    An ``else`` clause may appear as the last clause and matches when no
    preceding clause matched. If no clause matches and no ``else`` is
    present, an unspecified value is returned:

    .. code-block:: scheme

      --> (case 42
      ...   ((1) "one")
      ...   ((2) "two")
      ...   (else "other"))
      "other"

    **Equivalence to** ``cond``

    The transform makes the relationship between ``case`` and ``cond``
    explicit. A ``case`` expression of the form:

    .. code-block:: scheme

      (case key
        ((d1 d2) expr1)
        ((d3)    expr2)
        (else    expr3))

    Is equivalent to:

    .. code-block:: scheme

      (let ((tmp key))
        (cond ((memv tmp '(d1 d2)) expr1)
              ((memv tmp '(d3))    expr2)
              (else                expr3)))

    :param key: An expression whose value is compared against the datums in
                each clause.
    :param clause: One or more clauses. Each clause begins with either a
                   list of literal datum values or ``else``. The last clause
                   may be an ``else`` clause.
    :return: The value of the last expression in the matching clause, or the
             result of calling *proc* on *key* in the arrow form, or
             unspecified if no clause matches.
    :rtype: any

.. _sf:do:

.. describe:: (do ((variable init step) ...) (test expr ...) command ...)

    A general-purpose iteration construct that binds a set of variables,
    updates them on each iteration according to *step* expressions, and
    terminates when a *test* condition becomes true. ``do`` is the idiomatic
    Scheme form for imperative-style loops with explicit state variables.

    ``do`` is a derived form, transformed at parse time into an equivalent
    named ``let`` expression.

    **Syntax**

    .. code-block:: scheme

      (do ((variable1 init1 step1)
           (variable2 init2 step2)
           ...)
          (test expr ...)
        command ...)

    Each variable clause has three parts:

    - *variable* — the loop variable name.
    - *init* — an expression evaluated once before the loop begins to
      provide the variable's initial value.
    - *step* — an expression evaluated at the end of each iteration to
      produce the variable's value for the next iteration. If *step* is
      omitted, the variable retains its current value unchanged on the next
      iteration, as if ``(variable init variable)`` had been written.

    The test clause ``(test expr ...)`` specifies the termination condition.
    *test* is evaluated at the beginning of each iteration. When it returns
    a true value the loop terminates: the *expr* expressions are evaluated
    in order and the value of the last is returned as the result of the
    entire ``do`` expression. If no *expr* expressions are present, the
    result is unspecified.

    The *command* expressions, if any, appear after the test clause. They
    are evaluated in order for their side effects on each iteration where
    *test* is false, before the step expressions are evaluated. Their return
    values are discarded.

    It is an error for a variable to appear more than once in the variable
    clause list.

    **Evaluation order**

    1. All *init* expressions are evaluated (in unspecified order) and the
       variables are bound to the results.
    2. *test* is evaluated. If true, the result *expr* expressions are
       evaluated and the last value returned.
    3. If *test* is false, the *command* expressions are evaluated in order
       for effect.
    4. All *step* expressions are evaluated (in unspecified order) and the
       variables are rebound to the results.
    5. Go to step 2.

    **Example**

    Summing integers from 1 to 10:

    .. code-block:: scheme

      --> (do ((i 1 (+ i 1))
      ...      (sum 0 (+ sum i)))
      ...     ((> i 10) sum))
      55

    Building a list in reverse:

    .. code-block:: scheme

      --> (do ((i 5 (- i 1))
      ...      (result '() (cons i result)))
      ...     ((= i 0) result))
      (1 2 3 4 5)

    Using *command* expressions for side effects:

    .. code-block:: scheme

      --> (do ((i 0 (+ i 1)))
      ...     ((= i 5))
      ...   (display i)
      ...   (display " "))
      0 1 2 3 4

    A variable with no *step* retains its value each iteration:

    .. code-block:: scheme

      --> (do ((i 0 (+ i 1))
      ...      (limit 5))
      ...     ((= i limit) "done"))
      "done"

    **Equivalence to named** ``let``

    The transform makes the relationship to named ``let`` explicit. A ``do``
    expression of the form:

    .. code-block:: scheme

      (do ((v1 i1 s1)
           (v2 i2 s2))
          (test expr)
        command)

    Is equivalent to:

    .. code-block:: scheme

      (let loop ((v1 i1)
                 (v2 i2))
        (if test
            expr
            (begin
              command
              (loop s1 s2))))

    :param variable: One or more loop variable clauses, each of the form
                     ``(variable init)`` or ``(variable init step)``.
    :param test: A termination clause of the form ``(test expr ...)``. When
                 *test* is true the loop exits and the *expr* values are
                 returned.
    :param command: Zero or more expressions evaluated for side effects on
                    each iteration where *test* is false.
    :return: The value of the last *expr* in the test clause, or unspecified
             if no *expr* is present.
    :rtype: any

.. _sf:when:

.. describe:: (when test expr ...)

    Evaluates *test* and, if the result is true, evaluates the body
    expressions in order and returns the value of the last one. If *test*
    evaluates to ``#f``, no body expressions are evaluated and an unspecified
    value is returned.

    ``when`` is a derived form, transformed at parse time into an equivalent
    ``if`` expression with an unspecified else branch:

    .. code-block:: scheme

      ; This when:
      (when test
        expr1
        expr2)

      ; Is equivalent to:
      (if test
          (begin expr1 expr2)
          unspecified)

    ``when`` is the idiomatic form for one-sided conditionals where the false
    branch is not needed and the true branch has multiple expressions or side
    effects. It is cleaner than ``if`` in these cases since it avoids an
    explicit ``begin`` and makes the intent clear.

    .. note::

        R7RS specifies that the result of ``when`` is always unspecified.
        This implementation deviates by returning the value of the last body
        expression when the test is true, which is more useful in practice
        and consistent with how most Scheme implementations behave.

    :param test: The condition to evaluate.
    :type test: any
    :param expr: One or more expressions to evaluate if *test* is true.
    :return: The value of the last body expression if *test* is true,
             otherwise unspecified.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (when #t (display "yes") (newline) 42)
      yes
      42
      --> (when #f (display "never"))
      --> (define x 0)
      --> (when (> 5 3)
      ...   (set! x 99)
      ...   x)
      99


.. _sf:unless:

.. describe:: (unless test expr ...)

    Evaluates *test* and, if the result is ``#f``, evaluates the body
    expressions in order and returns the value of the last one. If *test*
    evaluates to a true value, no body expressions are evaluated and an
    unspecified value is returned. ``unless`` is the logical complement of
    ``when``.

    ``unless`` is a derived form, transformed at parse time into an equivalent
    ``if`` expression with the branches swapped:

    .. code-block:: scheme

      ; This unless:
      (unless test
        expr1
        expr2)

      ; Is equivalent to:
      (if test
          unspecified
          (begin expr1 expr2))

    .. note::

        R7RS specifies that the result of ``unless`` is always unspecified.
        This implementation deviates by returning the value of the last body
        expression when the test is false, consistent with the behaviour of
        ``when``.

    :param test: The condition to evaluate.
    :type test: any
    :param expr: One or more expressions to evaluate if *test* is ``#f``.
    :return: The value of the last body expression if *test* is false,
             otherwise unspecified.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (unless #f (display "yes") (newline) 42)
      yes
      42
      --> (unless #t (display "never"))
      --> (define x 0)
      --> (unless (> 2 5)
      ...   (set! x 99)
      ...   x)
      99
      --> (define (check-positive n)
      ...   (unless (positive? n)
      ...     (error "expected positive number")))

.. _sf:else:

.. describe:: else

    A reserved syntactic keyword used as the final clause in ``cond`` and
    ``case`` expressions to provide a default branch that matches when no
    preceding clause has been selected. ``else`` is not a procedure or value
    and cannot be used outside of these contexts.

    Internally, ``else`` is treated as a true value, so a ``cond`` or
    ``case`` expression with an ``else`` clause will always have a matching
    branch. It is an error to use ``else`` in any position other than the
    final clause of a ``cond`` or ``case`` expression, and it is an error to
    attempt to rebind ``else`` as a variable name.

    .. code-block:: scheme

      --> (cond (#f "no") (else "yes"))
      "yes"
      --> (case 99 ((1) "one") (else "other"))
      "other"
      --> (define else #f)
      Error: define: syntax keyword 'else' cannot be used as a variable

    See :ref:`sf:cond` and :ref:`sf:case` for full usage examples.

.. _sf:quasiquote:

.. describe:: (quasiquote datum)
              `datum

    Produces a structure similar to ``quote`` but with selective evaluation.
    Within a quasiquoted expression, most data is treated as literal, but
    forms wrapped in ``unquote`` (``,'``) are evaluated and their values
    inserted, and forms wrapped in ``unquote-splicing`` (``,@``) are
    evaluated and their list contents spliced in place. This makes
    ``quasiquote`` the primary tool for constructing code templates, and it
    is used pervasively in macro definitions.

    The shorthand `` `datum `` is exactly equivalent to
    ``(quasiquote datum)`` and is transformed into the full form by the
    parser before evaluation.

    ``quasiquote`` is a derived form, transformed at parse time into an
    equivalent expression built from ``quote``, ``list``, and ``append``
    calls. Atoms and unmodified subforms are quoted; ``unquote`` forms are
    inserted directly; ``unquote-splicing`` forms are appended. Vectors
    within a quasiquoted expression are handled by transforming the expansion
    and wrapping it in ``list->vector``.

    :param datum: A template expression, possibly containing ``unquote``
                  and ``unquote-splicing`` forms.
    :return: The constructed data structure.
    :rtype: any

    **Basic usage**

    Without any unquoting, ``quasiquote`` behaves identically to ``quote``:

    .. code-block:: scheme

      --> `(1 2 3)
      (1 2 3)
      --> `(a b c)
      (a b c)

    **With** ``unquote``

    Individual elements preceded by ``,`` are evaluated and their values
    inserted into the surrounding structure:

    .. code-block:: scheme

      --> (define x 42)
      --> `(the answer is ,x)
      (the answer is 42)
      --> (define a 1)
      --> (define b 2)
      --> `(,a ,b ,(+ a b))
      (1 2 3)
      --> `(list has ,(length '(a b c)) elements)
      (list has 3 elements)

    **With** ``unquote-splicing``

    Elements preceded by ``,@`` are evaluated and must produce a list; the
    list's contents are spliced directly into the surrounding structure rather
    than inserted as a nested list:

    .. code-block:: scheme

      --> (define nums '(1 2 3))
      --> `(a ,@nums b)
      (a 1 2 3 b)
      --> `(,@(list 1 2) ,@(list 3 4))
      (1 2 3 4)

    Contrast with plain ``unquote``, which inserts the list as a single
    nested element:

    .. code-block:: scheme

      --> `(a ,nums b)
      (a (1 2 3) b)
      --> `(a ,@nums b)
      (a 1 2 3 b)

    **Nested quasiquotes**

    Quasiquotes may be nested. The depth of nesting determines which
    ``unquote`` forms are active — an ``unquote`` at depth 1 is evaluated,
    but an ``unquote`` at depth 2 (inside a nested quasiquote) is treated as
    literal until the outer quasiquote is expanded:

    .. code-block:: scheme

      --> `(a `(b ,(+ 1 2) ,,x) c)
      (a (quasiquote (b (unquote (+ 1 2)) (unquote 42))) c)

    **Vectors**

    Quasiquoted vectors are supported; unquoting within a vector template
    works as expected:

    .. code-block:: scheme

      --> (define n 99)
      --> `#(1 ,n 3)
      #(1 99 3)


.. _sf:unquote:

.. describe:: (unquote expr)
              ,expr

    Within a ``quasiquote`` expression, causes *expr* to be evaluated and
    its value inserted into the surrounding quasiquoted structure. Outside
    of a ``quasiquote``, ``unquote`` is a syntax error.

    The shorthand ``,expr`` is exactly equivalent to ``(unquote expr)`` and
    is transformed into the full form by the parser.

    :param expr: An expression to evaluate and insert.
    :return: Not applicable — ``unquote`` is only meaningful within
             ``quasiquote``.

    .. code-block:: scheme

      --> (define x 7)
      --> `(the value is ,x)
      (the value is 7)
      --> `(the value is ,(* x x))
      (the value is 49)
      --> ,x
      Error: unquote: must be contained within a 'quasiquote' expression


.. _sf:unquote-splicing:

.. describe:: (unquote-splicing expr)
              ,@expr

    Within a ``quasiquote`` expression, causes *expr* to be evaluated — it
    must produce a proper list — and its elements spliced directly into the
    surrounding quasiquoted structure at that position. Unlike ``unquote``,
    which inserts a value as a single element, ``unquote-splicing`` flattens
    the list into the enclosing structure. Outside of a ``quasiquote``,
    ``unquote-splicing`` is a syntax error.

    The shorthand ``,@expr`` is exactly equivalent to
    ``(unquote-splicing expr)`` and is transformed into the full form by
    the parser.

    :param expr: An expression that evaluates to a proper list whose elements
                 will be spliced into the surrounding structure.
    :return: Not applicable — ``unquote-splicing`` is only meaningful within
             ``quasiquote``.

    .. code-block:: scheme

      --> (define xs '(1 2 3))
      --> `(a ,@xs b)
      (a 1 2 3 b)
      --> `(,@(map (lambda (x) (* x x)) '(1 2 3 4)))
      (1 4 9 16)
      --> `(,@'() a b)
      (a b)
      --> ,@xs
      Error: unquote-splice: must be contained within a 'quasiquote' expression

.. _sf:defmacro:

.. describe:: (defmacro name formals body)

    Defines *name* as a macro. When the macro is subsequently called, its
    argument forms are passed *unevaluated* to *body*, which is evaluated at
    macro-expansion time to produce a new syntactic form. That form is then
    substituted into the program in place of the macro call and evaluated
    normally. This two-phase process — expansion followed by evaluation —
    is what distinguishes macros from procedures.

    *formals* follows the same conventions as ``lambda``: a parenthesised
    list of symbols for fixed arguments, a bare symbol to collect all
    arguments as a list, or a dotted-tail list for fixed arguments with a
    variadic tail.

    ``defmacro`` creates a global binding for *name*, analogous to a
    top-level ``define``. It is an error if *name* is not a symbol or if
    any element of *formals* is not a symbol.

    .. note::

        ``defmacro`` implements *non-hygienic* macros in the classic Lisp
        tradition. This means that identifiers introduced by a macro
        expansion may accidentally capture bindings at the call site, or
        be captured by them. Hygienic macros (``syntax-rules``,
        ``define-syntax``), which prevent such capture automatically, are
        not currently supported.

    **Basic usage**

    ``quasiquote``, ``unquote``, and ``unquote-splicing`` are the natural
    tools for constructing the expansion, though any expression that produces
    a valid syntactic form may be used:

    .. code-block:: scheme

      --> (defmacro my-and (a b)
      ...   `(if ,a ,b #f))
      --> (my-and #t 42)
      42
      --> (my-and #f 42)
      #f

      --> (defmacro swap! (a b)
      ...   `(let ((tmp ,a))
      ...      (set! ,a ,b)
      ...      (set! ,b tmp)))
      --> (define x 1)
      --> (define y 2)
      --> (swap! x y)
      --> (list x y)
      (2 1)

    **Variadic macros**

    A bare symbol as *formals* collects all argument forms as a list:

    .. code-block:: scheme

      --> (defmacro my-begin forms
      ...   `((lambda () ,@forms)))
      --> (my-begin
      ...   (display "one ")
      ...   (display "two ")
      ...   (display "three"))
      one two three

    **Using the expansion**

    It can be instructive to see what a macro expands to before it is
    evaluated. Since the body is evaluated with the argument forms bound as
    data, you can inspect the expansion by quoting the call:

    .. code-block:: scheme

      --> (defmacro nested-test (x)
      ...   `(list ,x (list ,x)))
      --> (nested-test 5)
      (5 (5))

      --> (defmacro kond (predicate consequent alternative)
      ...   `(if ,predicate ,consequent ,alternative))
      --> (kond (= 1 1) 'yes 'no)
      yes

    **Hygiene caveat**

    Because ``defmacro`` is non-hygienic, a macro that introduces a binding
    may inadvertently capture a variable of the same name at the call site:

    .. code-block:: scheme

      --> (defmacro bad-swap! (a b)
      ...   `(let ((temp ,a))    ; 'temp' introduced by the macro
      ...      (set! ,a ,b)
      ...      (set! ,b temp)))
      --> (define temp 99)
      --> (define x 1)
      --> (define y 2)
      --> (bad-swap! x temp)    ; 'temp' at call site clashes with macro's 'temp'
      --> (list x temp)
      (2 1)                     ; 'temp' was captured -- result may surprise

    The ``swap!`` example above avoids this by using a sufficiently unlikely
    binding name. The standard solution in non-hygienic macro systems is to
    use ``gensym``-style unique names for any bindings introduced by the
    macro, or to rely on quasiquote to construct the expansion carefully.

    :param name: The symbol to bind as the macro name.
    :type name: symbol
    :param formals: A symbol, a list of symbols, or a dotted-tail list of
                    symbols specifying the macro's argument names.
    :param body: An expression evaluated at macro-expansion time with the
                 argument forms bound. Must produce a valid syntactic form.
    :return: The macro procedure object.
    :rtype: procedure

.. _sf:with-gc-stats:

.. describe:: (with-gc-stats expr)

    Evaluates *expr* and prints a summary of garbage collector heap usage
    before and after the evaluation. Useful for inspecting the memory
    behaviour of an expression — whether it allocates heavily, triggers
    collection, or leaves the heap largely unchanged.

    ``with-gc-stats`` is implemented as a special form rather than a
    procedure so that *expr* is not evaluated before the initial heap
    measurement is taken, ensuring the before/after figures reflect only
    the work done by *expr* itself.

    The heap size is measured by forcing a full GC collection before each
    measurement. Growth may be negative if the evaluation of *expr* allowed
    previously live objects to become collectible.

    The return value is the result of evaluating *expr*, so
    ``with-gc-stats`` can be wrapped around any expression without
    disrupting the surrounding code.

    .. note::

        This is a debugging utility and is not part of R7RS. Output is
        printed directly to standard output as a side effect.

    :param expr: Any expression to evaluate and measure.
    :return: The result of evaluating *expr*.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (with-gc-stats (make-list 100000 0))

      --- GC Monitor ---
      Heap Before: 2097152 bytes
      Heap After:  4194304 bytes
      Growth:      2097152 bytes
      ------------------
      (0 0 0 ...)

      --> (with-gc-stats (+ 1 2))

      --- GC Monitor ---
      Heap Before: 2097152 bytes
      Heap After:  2097152 bytes
      Growth:      0 bytes
      ------------------
      3

