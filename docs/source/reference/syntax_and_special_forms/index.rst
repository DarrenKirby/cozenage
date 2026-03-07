Syntax and Special Forms
========================

Special forms are the syntactic backbone of Cozenage. Where procedures are
values that can be passed around, stored in variables, and called at runtime,
special forms are fixed constructs that the evaluator recognises by shape and
handles directly — they are what most languages would call *syntax* or
*keywords*. You cannot redefine a special form, pass it as an argument, or
use it as a value. Attempting to rebind one raises an error.

The distinction matters because procedures always evaluate all their arguments
before the procedure body runs. Special forms are not bound by this rule: they
can choose which sub-expressions to evaluate, when to evaluate them, and how
many times. This selective evaluation is what makes special forms necessary.
``if``, for example, must evaluate its test first and then evaluate *either*
the consequent *or* the alternate — never both. If ``if`` were an ordinary
procedure, both branches would be evaluated before any choice was made, which
would be both semantically wrong and potentially disastrous for recursive
programs. The same logic applies to ``and``, ``or``, ``when``, ``cond``, and
every other conditional form: they exist as special forms precisely because
short-circuit evaluation cannot be achieved with procedures.

``quote`` is the starkest example of this principle. Its entire purpose is to
*suppress* evaluation — to return its argument as a literal datum rather than
treating it as an expression to be evaluated. No procedure could do this, since
a procedure would receive the already-evaluated argument.

``lambda``, ``define``, ``set!``, and ``defmacro`` are special because they
interact with the *environment* rather than just computing values.
``lambda`` captures the current environment as a closure. ``define`` introduces
new bindings into the global environment. ``set!`` locates an existing binding
anywhere in the enclosing scope chain and mutates it. These operations are
intrinsically tied to the evaluator's internal machinery and cannot be
expressed as ordinary calls.

``begin``, ``let``, ``letrec``, ``let*``, ``letrec*``, and ``do`` are special
because they establish *sequential* or *scoped* evaluation contexts — regions
of code with their own local bindings and their own evaluation order. They
are the tools from which all structured programs are built.


**Functional programming and why it matters here**

Cozenage is a Scheme, and Scheme is a functional language. Understanding what
that means in practice helps explain why the special forms are designed the
way they are.

In most imperative languages — C, Java, Python — a program is fundamentally a
sequence of instructions that modify shared state: variables are assigned,
arrays are updated, objects are mutated, and the program proceeds step by step
through these mutations. The meaning of a piece of code depends not just on
what it says but on what state the world is in when it runs.

Functional programming takes a different view. The primary unit of computation
is the *expression*, not the statement. Expressions have values; they do not
*do* things, they *produce* things. A function, in the mathematical sense, maps
inputs to outputs without modifying anything else. Code written in this style
is easier to reason about because the value of an expression depends only on
its inputs, not on hidden state elsewhere in the program.

Scheme does not enforce pure functional style — ``set!``, ``string-set!``,
and similar mutation procedures exist and are sometimes the right tool — but
the language is designed to make the functional style natural and efficient.
In particular:

- **Procedures are first-class values.** A procedure can be passed as an
  argument, returned from a function, and stored in a data structure. This is
  what makes higher-order procedures like ``map``, ``filter``, and ``foldl``
  possible, and it is why ``lambda`` is such a central form.

- **Recursion replaces loops.** Rather than mutating a counter variable in a
  loop, a functional program calls itself with updated arguments. Cozenage
  implements *tail-call optimisation*, which means a recursive call in tail
  position (the last thing a function does before returning) is automatically
  converted into a jump rather than a new stack frame. This makes recursion as
  efficient as iteration for well-structured programs, and it is why named
  ``let`` and ``do`` are idiomatic for loops rather than a ``while``
  construct.

- **Lexical scoping and closures.** Every function carries its defining
  environment with it. This makes it possible to write functions that return
  functions, creating private state without global variables:

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

  Each call to ``make-counter`` produces an independent counter whose state
  is entirely private. This pattern — combining ``lambda``, ``let``, and
  ``set!`` — is the Scheme idiom for encapsulation.


**Choosing the right form**

The special forms cover a small number of fundamental patterns. Knowing which
form to reach for in a given situation is the core skill of Scheme programming.

*Binding local variables:* Use ``let`` when the initial values are independent
of each other. Use ``let*`` when later bindings depend on earlier ones. Use
``letrec`` or ``letrec*`` when the bindings are mutually recursive procedures.
For the common case of a procedure that just needs a few local names, ``let``
is almost always right:

.. code-block:: scheme

  --> (let ((x (expensive-computation))
  ...       (y (other-computation)))
  ...   (+ x y))

Using ``let`` here ensures ``expensive-computation`` and ``other-computation``
are each called exactly once, and their results are given readable names.

*Defining procedures:* Use the ``define`` shorthand for top-level procedures.
Use ``lambda`` directly when you need an anonymous procedure — as an argument
to ``map``, ``filter``, or ``for-each``, or when returning a procedure from
another procedure. Use named ``let`` for self-contained recursive loops that
do not need a name outside the loop body:

.. code-block:: scheme

  --> (define (sum-list lst)
  ...   (let loop ((remaining lst) (acc 0))
  ...     (if (null? remaining)
  ...         acc
  ...         (loop (cdr remaining) (+ acc (car remaining))))))

*Conditionals:* Use ``if`` for simple two-way branches. Use ``cond`` for
multi-way dispatch where each branch has its own test. Use ``case`` for
dispatch on a fixed set of literal values. Use ``when`` or ``unless`` for
one-sided conditionals where the false branch is unimportant:

.. code-block:: scheme

  --> (when (file-exists? "config.scm")
  ...   (load "config.scm"))

  --> (cond ((< x 0)  "negative")
  ...       ((= x 0)  "zero")
  ...       (else     "positive"))

  --> (case (day-of-week)
  ...   ((saturday sunday) "weekend")
  ...   (else              "weekday"))

*Sequencing side effects:* Use ``begin`` to group multiple expressions where
only one is expected — for example, in the true branch of an ``if``. In a
procedure body or ``let`` body, multiple expressions are already sequenced
implicitly and ``begin`` is not needed.

*Iteration:* Use named ``let`` for general recursion and iteration. Use ``do``
for imperative-style counted loops with explicit step expressions. Both compile
to the same underlying mechanism and are equally efficient:

.. code-block:: scheme

  ; Named let style
  --> (let loop ((i 0))
  ...   (when (< i 5)
  ...     (display i)
  ...     (loop (+ i 1))))
  01234

  ; do style
  --> (do ((i 0 (+ i 1)))
  ...     ((= i 5))
  ...   (display i))
  01234

*Mutation:* Use ``set!`` sparingly and deliberately. In functional style,
the need for ``set!`` is a signal to consider whether the computation could
be restructured to pass updated values as arguments instead. That said,
``set!`` is sometimes the clearest and most efficient tool — particularly for
implementing stateful objects, caches, or counters.

*Constructing code as data:* Use ``quasiquote`` with ``unquote`` and
``unquote-splicing`` whenever you need to build a list or expression that is
mostly literal but with a few computed values inserted. This is the natural
syntax for ``defmacro`` bodies, but is equally useful for constructing
association lists, configuration structures, or any other data that has a
fixed shape with variable content:

.. code-block:: scheme

  --> (define (make-point x y) `(point ,x ,y))
  --> (make-point 3 4)
  (point 3 4)

  --> (define fields '(name age email))
  --> `(record ,@fields)
  (record name age email)

*Macros:* Use ``defmacro`` when you need a new syntactic form that cannot be
expressed as a procedure — most commonly when you need to control evaluation
order or introduce new binding forms. Prefer procedures over macros wherever
possible, since procedures are easier to reason about, test, and compose.
When you do write a macro, use ``quasiquote`` to construct the expansion and
take care to avoid accidental variable capture by choosing unlikely names for
any bindings the macro introduces.


**Derived forms**

Several special forms in this implementation are *derived* — they are
transformed into simpler primitives before evaluation rather than being
evaluated directly. The derived forms are ``when``, ``unless``, ``or``,
``cond``, ``case``, ``do``, ``let*``, ``letrec*``, named ``let``, and
``quasiquote``. Their semantics are defined entirely by their expansions; they
exist purely as a convenience for the programmer. This is worth knowing because
it means error messages from derived forms may sometimes refer to the primitive
form they expanded into (``if``, ``let``, ``letrec``, etc.) rather than the
original source form.

.. toctree::
   :maxdepth: 1
   :caption: Contents:

   sf
