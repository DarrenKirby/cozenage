Base lazy Library
=================

Overview
--------

What is Lazy Evaluation?
~~~~~~~~~~~~~~~~~~~~~~~~

In Cozenage, and in most programming languages, expressions are evaluated *eagerly*: as soon
as you write ``(+ 1 2)``, the interpreter computes ``3`` immediately. This is the behaviour you
have come to expect. Lazy evaluation inverts this: an expression is wrapped up and set aside,
and its value is only computed at the moment it is actually needed. Until then, it exists as
an *unevaluated promise*.

The ``(base lazy)`` library provides the tools to work with lazy evaluation explicitly. The
centrepiece is a simple contract: ``delay`` wraps an expression in a promise, and ``force``
redeems that promise, triggering evaluation and returning the result.

.. code-block:: scheme

    (define p (delay (+ 1 2)))   ; Nothing is computed yet. p is a promise.
    (force p)                    ; Now it is computed. => 3
    (force p)                    ; Computed again? No — the result is cached. => 3

This last point is important: a promise is evaluated **at most once**. Once forced, the result
is memoised — stored inside the promise object — and every subsequent call to ``force`` returns
the cached value immediately. This means any side effects inside a ``delay``\ed expression also
fire at most once:

.. code-block:: scheme

    (define count 0)
    (define p (delay (begin (set! count (+ count 1)) count)))

    (force p)   ; => 1   (count is now 1)
    (force p)   ; => 1   (count is still 1 — the body did not run again)


Promises Are Not Only for Streams
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Before discussing streams, it is worth understanding that ``delay`` and ``force`` are
independently useful tools, not merely scaffolding for streams.

**Deferring expensive work you may not need:**

.. code-block:: scheme

    (define expensive-result
      (delay (some-very-slow-computation)))

    (if (simple-check-passes?)
        (force expensive-result)   ; Only pay the cost if we actually need it.
        "skipped")

**Sharing a computation across multiple call sites:**

Because forcing a promise memoises its result, you can pass a single promise to multiple
parts of your program and guarantee the underlying computation runs exactly once, regardless
of how many times it is forced:

.. code-block:: scheme

    (define shared (delay (fetch-large-dataset)))

    (process-a (force shared))   ; Runs the computation.
    (process-b (force shared))   ; Returns the cached result instantly.

**Controlling exactly when a side effect fires:**

.. code-block:: scheme

    (define log-entry
      (delay (begin (write-to-log "event occurred") 'logged)))

    ; ... do other things ...

    (force log-entry)   ; The log entry is written precisely here, once.

None of these examples involve streams. Lazy evaluation is the *mechanism*; streams are one
powerful *application* of that mechanism.


What is a Stream?
~~~~~~~~~~~~~~~~~

A stream is a data structure that represents a sequence of values, potentially infinite, where
each element beyond the first is computed only when it is needed.

Compare a list and a stream representing the natural numbers:

.. code-block:: scheme

    ; A list must be finite — you cannot hold all natural numbers in memory.
    (define first-five-nats (list 0 1 2 3 4))

    ; A stream can be infinite — the tail is a promise, not a value.
    (define (integers-from n)
      (stream n (integers-from (+ n 1))))

    (define nats (integers-from 0))   ; Represents 0, 1, 2, 3, ... forever.

The ``stream`` constructor builds a stream node with two parts: an eager *head* (evaluated
immediately), and a lazy *tail* (wrapped in a promise, not evaluated until forced). This is
the crucial difference from a list, where both ``car`` and ``cdr`` are fully realised values.

Accessing elements forces only as much of the stream as is needed:

.. code-block:: scheme

    (head nats)              ; => 0   (no promises forced)
    (head (tail nats))       ; => 1   (one promise forced)
    (at 100 nats)            ; => 100 (100 promises forced, then discarded)

The stream itself is never fully realised in memory. At any given moment, only the portion
you have accessed exists as computed values.


The Relationship Between Lazy Evaluation and Streams
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A useful way to think about this:

- ``delay`` and ``force`` are to streams what ``cons``, ``car``, and ``cdr`` are to lists —
  the primitive building blocks, each useful on their own, but also the foundation for
  something larger.
- The ``stream`` constructor is essentially just ``cons`` where the tail is automatically
  ``delay``\ed for you.
- ``head`` and ``tail`` are essentially ``car`` and ``cdr`` where ``tail`` automatically
  ``force``\s the promise.

Streams are lazy evaluation applied systematically to sequences. Once you understand
``delay`` and ``force``, streams follow naturally.


Special Forms vs Procedures: A Note on This Library
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Most Cozenage libraries export only *procedures* — functions you call like ``(sqrt 4)`` or
``(string-length "hello")``. The ``(base lazy)`` library is unique in that it also exports
*special forms* (syntax): ``delay``, ``delay-force``, and ``stream``.

The distinction matters. A procedure evaluates all its arguments before the function body
runs. A special form controls evaluation itself — it can choose *not* to evaluate an argument,
or to evaluate it later. This is precisely what ``delay`` must do: if ``(delay expr)`` were a
procedure, ``expr`` would be evaluated before ``delay`` ever had a chance to defer it.

.. code-block:: scheme

    ; If delay were a procedure, this would loop forever:
    (define ones (delay (stream 1 ones)))   ; delay must NOT evaluate its argument eagerly.

Because these special forms must intercept evaluation before it happens, they cannot be
implemented as a library loaded at runtime in the usual way. The type infrastructure
(``CELL_PROMISE``, ``CELL_STREAM``) and the special form dispatch are wired into the core
interpreter; ``(import (base lazy))`` activates them and adds the accompanying procedures to
the environment.


Working Safely with Infinite Streams
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Infinite streams require some care. Certain operations are *lazy* — they return a new stream
without traversing the input — and are safe to apply to infinite streams:

.. code-block:: scheme

    (collect (lambda (n) (* n n)) nats)   ; Safe — returns a new infinite stream.
    (select even? nats)                   ; Safe — returns a new infinite stream.
    (weave nats nats)                     ; Safe — returns a new infinite stream.

Other operations are *eager consumers* — they must traverse the stream to produce a result —
and will diverge (loop forever) if applied to an infinite stream without first bounding it
with ``take``:

.. code-block:: scheme

    (reduce + 0 nats)                     ; Diverges — never terminates.
    (reduce + 0 (take 10 nats))           ; Safe — sums the first 10 elements. => 45

As a rule: if an operation returns a stream, it is lazy and safe. If it returns a plain
value, it is eager and requires a finite input.


A Complete Example
~~~~~~~~~~~~~~~~~~

To illustrate how the pieces fit together, here is a small program that computes the first
ten squared even numbers using a pipeline of stream operations:

.. code-block:: scheme

    (import (base lazy))

    ; An infinite stream of natural numbers, built with iterate.
    (define nats (iterate (lambda (n) (+ n 1)) 0))

    ; Filter to evens, then square each one — both operations are lazy.
    (define squared-evens
      (collect (lambda (n) (* n n))
               (select even? nats)))

    ; Only now do we force evaluation, by taking the first 10.
    (take 10 squared-evens)
    ; => (0 4 16 36 64 100 144 196 256 324)

At no point is the full stream of naturals, evens, or squared evens realised in memory.
Each call to ``take`` drives exactly as much evaluation as is needed, and no more.

Special Forms
-------------

.. _sf:delay:

delay
~~~~~~~~~

.. describe:: (delay expression)

    Wraps *expression* in a *promise* object without evaluating it. The
    expression will be evaluated later, on demand, when the promise is passed
    to ``force``. This is the fundamental building block of lazy evaluation in
    Cozenage.

    The promise memoises its result: the first call to ``force`` evaluates
    *expression* and caches the value inside the promise object. Every
    subsequent call to ``force`` on the same promise returns the cached value
    immediately, without re-evaluating *expression*. As a consequence, any
    side effects inside *expression* occur at most once.

    ``delay`` is a special form, not a procedure. If it were a procedure, its
    argument would be evaluated eagerly before ``delay`` could defer it,
    defeating the purpose entirely.

    :param expression: The expression to defer. Not evaluated at call time.
    :type expression: any
    :return: A promise object encapsulating *expression* and the enclosing
             environment.
    :rtype: promise

    **Example:**

    .. code-block:: scheme

        --> (define p (delay (+ 1 2)))
        --> p
        #<promise object:unevaluated>
        --> (force p)
        3
        --> (force p)
        3

    Side effects fire exactly once, regardless of how many times the promise
    is forced:

    .. code-block:: scheme

        --> (define count 0)
        --> (define p (delay (begin (set! count (+ count 1)) count)))
        --> (force p)
        1
        --> (force p)
        1
        --> count
        1

    ``delay`` is commonly used to defer expensive computations until their
    result is actually needed:

    .. code-block:: scheme

        --> (define result (delay (some-expensive-computation)))
        --> (if (need-result?)
        ...     (force result)
        ...     "skipped")

    .. note::

        The effect of *expression* returning multiple values is unspecified.
        ``delay`` is intended for use with single-valued expressions.

    .. seealso::

        :ref:`force <proc:force>`, :ref:`delay-force <sf:delay-force>`,
        :ref:`make-promise <proc:make-promise>`


----

.. _sf:delay-force:

delay-force
~~~~~~~~~~~~~~~

.. describe:: (delay-force expression)

    Similar to ``delay``, but intended for use in *iterative lazy algorithms*
    — those that construct long chains of nested promises. Where ``delay``
    would cause each link in the chain to be forced recursively (consuming
    stack space proportional to the chain length), ``delay-force`` causes
    ``force`` to iterate rather than recurse, using a trampoline mechanism.
    This allows iterative lazy algorithms to run in constant stack space.

    ``(delay-force expression)`` is conceptually similar to
    ``(delay (force expression))``, with the important difference that forcing
    the result will in effect result in a tail call to ``(force expression)``,
    whereas forcing ``(delay (force expression))`` might not.

    The expression passed to ``delay-force`` must evaluate to a promise when
    forced. If it returns any other type, ``force`` will signal an error.

    ``delay-force`` is a special form for the same reason as ``delay``: its
    argument must not be evaluated at call time.

    :param expression: An expression that must evaluate to a promise.
                       Not evaluated at call time.
    :type expression: any
    :return: A promise object in the ``LAZY`` state, which will trampoline
             through further promises when forced.
    :rtype: promise

    **Example:**

    The classic illustration is a stream traversal that would otherwise build
    a chain of deferred ``force`` calls proportional to the depth of
    traversal. Using ``delay-force``, this runs in constant stack space
    regardless of ``n``:

    .. code-block:: scheme

        --> (define nats (iterate (lambda (n) (+ n 1)) 0))
        --> (define (stream-ref-df s n)
        ...   (if (= n 0)
        ...       (make-promise (head s))
        ...       (delay-force (stream-ref-df (tail s) (- n 1)))))
        --> (force (stream-ref-df nats 10000))
        10000

    Without ``delay-force``, a naïve recursive implementation would consume
    stack depth proportional to ``n``, eventually overflowing for large
    values. With ``delay-force``, ``force`` trampolines through the chain
    iteratively.

    .. note::

        ``delay-force`` is an advanced form. For straightforward lazy
        evaluation, ``delay`` is sufficient. Reach for ``delay-force`` only
        when you are building algorithms that produce deeply nested promise
        chains — typically when writing recursive stream operations.

    .. seealso::

        :ref:`delay <sf:delay>`, :ref:`force <proc:force>`


----

.. _sf:stream:

stream
~~~~~~~~~~

.. describe:: (stream head tail)

    Constructs a stream node — the fundamental unit of a stream. *head* is
    evaluated eagerly and becomes the first element of the stream. *tail* is
    wrapped in a promise without being evaluated, and will only be forced when
    the next element of the stream is requested.

    This eager/lazy split is what makes infinite streams possible: the head is
    a concrete value, but the tail is a deferred computation that may itself
    return another stream node, another deferred computation, and so on,
    without limit.

    ``stream`` is a special form because its second argument must not be
    evaluated at construction time. A recursive stream definition such as the
    one below would loop forever if ``stream`` were a procedure:

    .. code-block:: scheme

        (define (integers-from n)
          (stream n (integers-from (+ n 1))))   ; tail is NOT evaluated here.

    :param head: The first element of the stream. Evaluated immediately.
    :type head: any
    :param tail: An expression producing the remainder of the stream.
                 Wrapped in a promise; not evaluated until needed.
    :type tail: any
    :return: A stream node whose head is the value of *head* and whose tail
             is a promise encapsulating *tail*.
    :rtype: stream

    **Example:**

    .. code-block:: scheme

        --> (define (integers-from n)
        ...   (stream n (integers-from (+ n 1))))
        --> (define nats (integers-from 0))
        --> nats
        #[0 ... #<promise object:unevaluated>]
        --> (head nats)
        0
        --> (head (tail nats))
        1
        --> (head (tail (tail nats)))
        2

    Streams are more naturally constructed using ``iterate`` for simple
    sequences, but ``stream`` is the primitive from which all streams are
    ultimately built:

    .. code-block:: scheme

        --> (define nats (iterate (lambda (n) (+ n 1)) 0))
        --> (take 5 nats)
        (0 1 2 3 4)

    A finite stream can be terminated by returning ``'()`` as the tail
    expression:

    .. code-block:: scheme

        --> (define (list->stream lst)
        ...   (if (null? lst)
        ...       '()
        ...       (stream (car lst) (list->stream (cdr lst)))))
        --> (define s (list->stream '(1 2 3)))
        --> (take 3 s)
        (1 2 3)

    .. seealso::

        :ref:`head <proc:head>`, :ref:`tail <proc:tail>`,
        :ref:`iterate <proc:iterate>`, :ref:`list->stream <proc:list->stream>`

Procedures
----------

.. _proc:force:

force
~~~~~~~~~

.. function:: (force promise)

    Forces the value of *promise*. If *promise* has not yet been evaluated,
    its expression is evaluated now, the result is cached inside the promise,
    and that result is returned. If *promise* has already been forced, the
    cached value is returned immediately without re-evaluation.

    If *promise* is not a promise, it is returned unchanged. This allows
    ``force`` to be called defensively on values that may or may not be
    promises.

    Promises created with ``delay-force`` are handled transparently via a
    trampoline: ``force`` iterates through chains of lazy promises rather than
    recursing, so deeply iterative lazy algorithms run in constant stack space.

    Re-entrant forcing — forcing a promise whose evaluation has already begun
    — is detected and signalled as an error.

    :param promise: A promise, or any other value.
    :type promise: promise
    :return: The value of the promise, or *promise* itself if it is not a
             promise.
    :rtype: any

    **Example:**

    .. code-block:: scheme

        --> (force (delay (+ 1 2)))
        3

    Forcing a non-promise returns the value unchanged:

    .. code-block:: scheme

        --> (force 42)
        42
        --> (force "hello")
        "hello"

    Memoisation — the body is evaluated exactly once:

    .. code-block:: scheme

        --> (define count 0)
        --> (define p (delay (begin (set! count (+ count 1)) count)))
        --> (force p)
        1
        --> (force p)
        1
        --> count
        1

    Forcing a ``delay-force`` promise trampolines through the chain
    iteratively, regardless of depth:

    .. code-block:: scheme

        --> (define nats (iterate (lambda (n) (+ n 1)) 0))
        --> (define (stream-ref-df s n)
        ...   (if (= n 0)
        ...       (make-promise (head s))
        ...       (delay-force (stream-ref-df (tail s) (- n 1)))))
        --> (force (stream-ref-df nats 10000))
        10000

    .. seealso::

        :ref:`delay <sf:delay>`, :ref:`delay-force <sf:delay-force>`,
        :ref:`make-promise <proc:make-promise>`


----

.. _proc:make-promise:

make-promise
~~~~~~~~~~~~~~~~

.. function:: (make-promise obj)

    Returns a promise that, when forced, will return *obj*. Unlike ``delay``,
    which defers evaluation of its argument, ``make-promise`` is a procedure
    and evaluates *obj* immediately — it wraps an already-computed value in a
    promise rather than deferring a computation.

    If *obj* is already a promise, it is returned unchanged. This makes
    ``make-promise`` idempotent: wrapping a promise in another promise has no
    effect.

    ``make-promise`` is useful when an interface requires a promise but you
    already have a concrete value — for example, as a base case in a recursive
    ``delay-force`` chain.

    :param obj: The value to wrap, or a promise to return unchanged.
    :type obj: any
    :return: A forced promise whose value is *obj*, or *obj* itself if it is
             already a promise.
    :rtype: promise

    **Example:**

    .. code-block:: scheme

        --> (define p (make-promise 42))
        --> (promise? p)
        #true
        --> (force p)
        42

    Passing an existing promise returns it unchanged:

    .. code-block:: scheme

        --> (define p (delay (+ 1 2)))
        --> (eq? p (make-promise p))
        #true

    Typical use as a base case in a ``delay-force`` chain:

    .. code-block:: scheme

        --> (define (stream-ref-df s n)
        ...   (if (= n 0)
        ...       (make-promise (head s))
        ...       (delay-force (stream-ref-df (tail s) (- n 1)))))
        --> (force (stream-ref-df nats 100))
        100

    .. seealso::

        :ref:`delay <sf:delay>`, :ref:`force <proc:force>`,
        :ref:`promise? <proc:promise?>`


----

.. _proc:promise?:

promise?
~~~~~~~~~~~~

.. function:: (promise? obj)

    Returns ``#true`` if *obj* is a promise, ``#false`` otherwise. A promise
    is any object created by ``delay``, ``delay-force``, or ``make-promise``.

    ``promise?`` returns ``#true`` regardless of whether the promise has been
    forced or not — both unevaluated and memoised promises satisfy the
    predicate.

    :param obj: The object to test.
    :type obj: any
    :return: ``#true`` if *obj* is a promise, ``#false`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

        --> (promise? (delay 42))
        #true
        --> (promise? (make-promise 42))
        #true
        --> (promise? 42)
        #false
        --> (promise? "hello")
        #false

    A forced promise is still a promise:

    .. code-block:: scheme

        --> (define p (delay (+ 1 2)))
        --> (force p)
        3
        --> (promise? p)
        #true

    .. seealso::

        :ref:`delay <sf:delay>`, :ref:`force <proc:force>`,
        :ref:`stream? <proc:stream?>`


----

.. _proc:stream?:

stream?
~~~~~~~~~~~

.. function:: (stream? obj)

    Returns ``#true`` if *obj* is a stream, ``#false`` otherwise. A stream is
    any object constructed by the ``stream`` special form, or returned by a
    stream-producing procedure such as ``iterate``, ``collect``, ``select``,
    or ``weave``.

    Note that the empty stream — represented by ``'()`` — is *not* considered
    a stream by this predicate. Use ``stream-null?`` to test for the empty
    stream.

    :param obj: The object to test.
    :type obj: any
    :return: ``#true`` if *obj* is a stream node, ``#false`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

        --> (define nats (iterate (lambda (n) (+ n 1)) 0))
        --> (stream? nats)
        #true
        --> (stream? 42)
        #false
        --> (stream? '())
        #false
        --> (stream? (delay 42))
        #false

    .. seealso::

        :ref:`stream-null? <proc:stream-null?>`, :ref:`promise? <proc:promise?>`,
        :ref:`stream <sf:stream>`


----

.. _proc:stream-null?:

stream-null?
~~~~~~~~~~~~~~~~

.. function:: (stream-null? obj)

    Returns ``#true`` if *obj* is the empty stream, ``#false`` otherwise. The
    empty stream is represented by ``'()``. This is the natural terminator for
    finite streams, and is what stream-producing procedures such as
    ``list->stream`` yield when their input is exhausted.

    Use ``stream-null?`` rather than ``null?`` when writing code that operates
    on streams, to make the intent clear to the reader.

    :param obj: The object to test.
    :type obj: any
    :return: ``#true`` if *obj* is the empty stream, ``#false`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

        --> (stream-null? '())
        #true
        --> (stream-null? (list->stream '()))
        #true
        --> (stream-null? (list->stream '(1 2 3)))
        #false
        --> (stream-null? 42)
        #false

    Typical use when recursing over a finite stream:

    .. code-block:: scheme

        --> (define (stream-length s)
        ...   (if (stream-null? s)
        ...       0
        ...       (+ 1 (stream-length (tail s)))))
        --> (stream-length (list->stream '(1 2 3 4 5)))
        5

    .. seealso::

        :ref:`stream? <proc:stream?>`, :ref:`list->stream <proc:list->stream>`

Stream Constructors
-------------------

.. _proc:iterate:

iterate
~~~~~~~~~~~

.. function:: (iterate proc seed)

    Returns an infinite stream whose first element is *seed*, and each
    subsequent element is the result of applying *proc* to the previous one.
    The stream produced is ``(seed (proc seed) (proc (proc seed)) ...)``.

    ``iterate`` is the general-purpose stream constructor. Most infinite
    streams can be expressed naturally as an iteration over a seed value,
    making explicit recursive ``stream`` definitions unnecessary in the
    common case.

    :param proc: A procedure of one argument, used to compute each successive
                 element from the previous one.
    :type proc: procedure
    :param seed: The first element of the stream, and the initial input to
                 *proc*.
    :type seed: any
    :return: An infinite stream starting at *seed*.
    :rtype: stream

    **Example:**

    .. code-block:: scheme

        --> (define nats (iterate (lambda (n) (+ n 1)) 0))
        --> (take 5 nats)
        (0 1 2 3 4)

    Powers of two:

    .. code-block:: scheme

        --> (define powers-of-2 (iterate (lambda (n) (* n 2)) 1))
        --> (take 8 powers-of-2)
        (1 2 4 8 16 32 64 128)

    ``iterate`` composes naturally with ``collect`` and ``select``:

    .. code-block:: scheme

        --> (define odds (select odd? (iterate (lambda (n) (+ n 1)) 0)))
        --> (take 5 odds)
        (1 3 5 7 9)

    .. seealso::

        :ref:`stream <sf:stream>`, :ref:`list->stream <proc:list->stream>`,
        :ref:`collect <proc:collect>`, :ref:`select <proc:select>`


----

.. _proc:list->stream:

list->stream
~~~~~~~~~~~~~~~~

.. function:: (list->stream list)

    Converts a proper list into a finite stream. The resulting stream contains
    the same elements in the same order as *list*, terminated by the empty
    stream ``'()``.

    This is the primary bridge between the list and stream worlds, allowing
    finite data held in lists to be processed by stream operations. The
    conversion is lazy: only as much of *list* as is actually consumed by
    subsequent stream operations will be traversed.

    If *list* is empty, the empty stream ``'()`` is returned immediately.

    :param list: A proper list to convert.
    :type list: list
    :return: A finite stream containing the elements of *list*, or ``'()``
             if *list* is empty.
    :rtype: stream

    **Example:**

    .. code-block:: scheme

        --> (define s (list->stream '(1 2 3 4 5)))
        --> (head s)
        1
        --> (take 3 s)
        (1 2 3)
        --> (stream? s)
        #true

    Stream operations can be applied directly to the result:

    .. code-block:: scheme

        --> (take 5 (collect (lambda (n) (* n n))
        ...                  (list->stream '(1 2 3 4 5))))
        (1 4 9 16 25)

        --> (take 3 (select odd? (list->stream '(1 2 3 4 5))))
        (1 3 5)

    .. seealso::

        :ref:`iterate <proc:iterate>`, :ref:`stream <sf:stream>`,
        :ref:`stream-null? <proc:stream-null?>`


----

Stream Accessors
----------------

.. _proc:head:

head
~~~~~~~~

.. function:: (head stream)

    Returns the first element of *stream*. The head is always a fully
    evaluated value — it is computed eagerly when the stream node is
    constructed.

    ``head`` is the stream analogue of ``car``.

    :param stream: A stream node.
    :type stream: stream
    :return: The first element of *stream*.
    :rtype: any

    **Example:**

    .. code-block:: scheme

        --> (define nats (iterate (lambda (n) (+ n 1)) 0))
        --> (head nats)
        0
        --> (head (tail nats))
        1
        --> (head (tail (tail nats)))
        2

    .. seealso::

        :ref:`tail <proc:tail>`, :ref:`at <proc:at>`


----

.. _proc:tail:

tail
~~~~~~~~

.. function:: (tail stream)

    Returns the remainder of *stream* after its first element. The tail is
    stored as a promise and is forced automatically by ``tail`` — the caller
    receives the next stream node (or ``'()`` if the stream is exhausted)
    without needing to call ``force`` explicitly.

    If *stream* is the empty stream ``'()``, the empty stream is returned.

    ``tail`` is the stream analogue of ``cdr``, with the addition of
    automatic forcing.

    :param stream: A stream node, or the empty stream.
    :type stream: stream
    :return: The next stream node, or ``'()`` if the stream is exhausted.
    :rtype: stream

    **Example:**

    .. code-block:: scheme

        --> (define nats (iterate (lambda (n) (+ n 1)) 0))
        --> (tail nats)
        #[1 ... #<promise object:native>]
        --> (head (tail nats))
        1

    Chaining ``tail`` calls traverses the stream element by element:

    .. code-block:: scheme

        --> (head (tail (tail (tail nats))))
        3

    Calling ``tail`` on the empty stream returns the empty stream:

    .. code-block:: scheme

        --> (tail '())
        ()

    .. seealso::

        :ref:`head <proc:head>`, :ref:`drop <proc:drop>`


----

.. _proc:at:

at
~~~~~~

.. function:: (at n stream)

    Returns the element at zero-based index *n* in *stream*, forcing as many
    tail promises as necessary to reach that position. Elements before index
    *n* are traversed but not returned.

    If the stream is exhausted before index *n* is reached, an error is
    signalled.

    :param n: A non-negative integer index.
    :type n: integer
    :param stream: A stream to index into.
    :type stream: stream
    :return: The element at position *n*.
    :rtype: any

    **Example:**

    .. code-block:: scheme

        --> (define nats (iterate (lambda (n) (+ n 1)) 0))
        --> (at 0 nats)
        0
        --> (at 5 nats)
        5
        --> (at 10000 nats)
        10000

    Works on any stream, including transformed ones:

    .. code-block:: scheme

        --> (define squares (collect (lambda (n) (* n n)) nats))
        --> (at 5 squares)
        25

    .. seealso::

        :ref:`head <proc:head>`, :ref:`take <proc:take>`,
        :ref:`drop <proc:drop>`


----

Stream Sequence Operations
--------------------------

.. _proc:take:

take
~~~~~~~~

.. function:: (take n stream)

    Returns a list containing the first *n* elements of *stream*, forcing
    exactly *n* tail promises. If *stream* contains fewer than *n* elements,
    all available elements are returned.

    The result is a plain list, not a stream. ``take`` is the primary way to
    extract a finite, fully evaluated sequence from a potentially infinite
    stream.

    :param n: The number of elements to take.
    :type n: integer
    :param stream: A stream to take elements from.
    :type stream: stream
    :return: A list of the first *n* elements of *stream*.
    :rtype: list

    **Example:**

    .. code-block:: scheme

        --> (define nats (iterate (lambda (n) (+ n 1)) 0))
        --> (take 5 nats)
        (0 1 2 3 4)
        --> (take 1 nats)
        (0)
        --> (take 0 nats)
        ()

    ``take`` is commonly used as the final step in a stream pipeline to
    materialise results:

    .. code-block:: scheme

        --> (take 5 (select even? nats))
        (0 2 4 6 8)

        --> (take 5 (collect (lambda (n) (* n n)) nats))
        (0 1 4 9 16)

    .. note::

        ``take`` is an eager consumer — it forces *n* elements immediately.
        For stream operations that remain lazy, see ``collect``, ``select``,
        and ``drop``.

    .. seealso::

        :ref:`drop <proc:drop>`, :ref:`at <proc:at>`,
        :ref:`reduce <proc:reduce>`


----

.. _proc:drop:

drop
~~~~~~~~

.. function:: (drop n stream)

    Returns the stream that remains after discarding the first *n* elements
    of *stream*. Unlike ``take``, the result is a stream, not a list —
    further lazy operations can be applied to it.

    If *stream* contains fewer than *n* elements, the empty stream is
    returned.

    :param n: The number of elements to discard.
    :type n: integer
    :param stream: A stream to drop elements from.
    :type stream: stream
    :return: The stream beginning at element *n*.
    :rtype: stream

    **Example:**

    .. code-block:: scheme

        --> (define nats (iterate (lambda (n) (+ n 1)) 0))
        --> (head (drop 5 nats))
        5
        --> (take 3 (drop 10 nats))
        (10 11 12)

    ``drop`` and ``take`` can be combined to extract an arbitrary slice of a
    stream:

    .. code-block:: scheme

        --> (take 5 (drop 100 nats))
        (100 101 102 103 104)

    .. seealso::

        :ref:`take <proc:take>`, :ref:`tail <proc:tail>`


----

Stream Transformers
-------------------

.. _proc:collect:

collect
~~~~~~~~~~~

.. function:: (collect proc stream)

    Returns a new stream formed by applying *proc* to each element of
    *stream*. The head is transformed eagerly; each subsequent element is
    transformed lazily as the stream is consumed.

    ``collect`` is the stream analogue of ``map``. It is safe to apply to
    infinite streams — no elements beyond the head are forced until the
    resulting stream is consumed.

    :param proc: A procedure of one argument to apply to each element.
    :type proc: procedure
    :param stream: The input stream.
    :type stream: stream
    :return: A new stream of transformed elements.
    :rtype: stream

    **Example:**

    .. code-block:: scheme

        --> (define nats (iterate (lambda (n) (+ n 1)) 0))
        --> (take 5 (collect (lambda (n) (* n 2)) nats))
        (0 2 4 6 8)

        --> (take 5 (collect (lambda (n) (* n n)) nats))
        (0 1 4 9 16)

    ``collect`` calls can be chained:

    .. code-block:: scheme

        --> (define doubled (collect (lambda (n) (* n 2)) nats))
        --> (take 5 (collect (lambda (n) (+ n 1)) doubled))
        (1 3 5 7 9)

    ``collect`` and ``select`` compose naturally:

    .. code-block:: scheme

        --> (take 5 (collect (lambda (n) (* n n))
        ...                  (select even? nats)))
        (0 4 16 36 64)

    .. seealso::

        :ref:`select <proc:select>`, :ref:`weave <proc:weave>`,
        :ref:`reduce <proc:reduce>`


----

.. _proc:select:

select
~~~~~~~~~~

.. function:: (select pred stream)

    Returns a new stream containing only those elements of *stream* for which
    *pred* returns a true value. Elements are tested lazily — only as many
    elements as are needed to satisfy the next request are ever forced.

    ``select`` is the stream analogue of ``filter``. It is safe to apply to
    infinite streams, provided the resulting stream is consumed with a
    bounding operation such as ``take`` or ``at``.

    :param pred: A predicate procedure of one argument.
    :type pred: procedure
    :param stream: The input stream.
    :type stream: stream
    :return: A new stream containing only elements that satisfy *pred*.
    :rtype: stream

    **Example:**

    .. code-block:: scheme

        --> (define nats (iterate (lambda (n) (+ n 1)) 0))
        --> (take 5 (select even? nats))
        (0 2 4 6 8)
        --> (take 5 (select odd? nats))
        (1 3 5 7 9)
        --> (at 3 (select even? nats))
        6

    ``select`` and ``collect`` compose freely:

    .. code-block:: scheme

        --> (take 5 (select even?
        ...                 (collect (lambda (n) (* n n)) nats)))
        (0 4 16 36 64)

    .. seealso::

        :ref:`collect <proc:collect>`, :ref:`weave <proc:weave>`,
        :ref:`reduce <proc:reduce>`


----

.. _proc:weave:

weave
~~~~~~~~~

.. function:: (weave stream1 stream2)

    Returns a new stream of pairs, combining *stream1* and *stream2*
    element-wise. The head of each pair is taken from *stream1*; the tail
    from *stream2*. The resulting stream is as long as the shorter of the two
    inputs — if either stream is exhausted, the result terminates.

    ``weave`` is the stream analogue of ``zip``. It is safe to apply to
    infinite streams.

    :param stream1: The first input stream.
    :type stream1: stream
    :param stream2: The second input stream.
    :type stream2: stream
    :return: A new stream of pairs ``(a . b)`` where *a* is from *stream1*
             and *b* is from *stream2*.
    :rtype: stream

    **Example:**

    .. code-block:: scheme

        --> (define nats   (iterate (lambda (n) (+ n 1)) 0))
        --> (define powers (iterate (lambda (n) (* n 2)) 1))
        --> (take 4 (weave nats powers))
        ((0 . 1) (1 . 2) (2 . 4) (3 . 8))

    When one stream is finite, the result terminates at the shorter stream:

    .. code-block:: scheme

        --> (take 5 (weave (list->stream '(a b c)) nats))
        ((a . 0) (b . 1) (c . 2))

    Pairs produced by ``weave`` can be processed with ``collect``:

    .. code-block:: scheme

        --> (take 5 (collect (lambda (p) (+ (car p) (cdr p)))
        ...                  (weave nats powers)))
        (1 3 6 11 20)

    .. seealso::

        :ref:`collect <proc:collect>`, :ref:`select <proc:select>`


----

Stream Reduction
----------------

.. _proc:reduce:

reduce
~~~~~~~~~~

.. function:: (reduce proc init stream)

    Folds *proc* over *stream*, accumulating a result. Starting with *init*
    as the initial accumulator, ``reduce`` applies ``(proc acc element)`` for
    each element in turn, where *acc* is the current accumulator and *element*
    is the current stream element. The final accumulator value is returned.

    ``reduce`` accepts both streams and plain lists as its third argument,
    making it polymorphic over both sequence types.

    .. warning::

        ``reduce`` is an eager consumer — it must traverse the entire input
        to produce a result. Applying it to an infinite stream will cause the
        interpreter to loop forever. Always bound infinite streams with
        ``take`` before reducing:

        .. code-block:: scheme

            (reduce + 0 (take 10 nats))   ; Safe.
            (reduce + 0 nats)             ; Diverges.

    :param proc: A procedure of two arguments: the current accumulator and
                 the current element.
    :type proc: procedure
    :param init: The initial accumulator value.
    :type init: any
    :param stream: A finite stream or proper list to fold over.
    :type stream: stream or list
    :return: The final accumulated value.
    :rtype: any

    **Example:**

    .. code-block:: scheme

        --> (reduce + 0 (list->stream '(1 2 3 4 5)))
        15
        --> (reduce * 1 (list->stream '(1 2 3 4 5)))
        120

    Reducing over a bounded slice of an infinite stream:

    .. code-block:: scheme

        --> (define nats (iterate (lambda (n) (+ n 1)) 0))
        --> (reduce + 0 (take 10 nats))
        45

    ``reduce`` also accepts plain lists directly:

    .. code-block:: scheme

        --> (reduce max 0 (list->stream '(3 1 4 1 5 9 2 6)))
        9
        --> (reduce string-append "" (list->stream '("a" "b" "c" "d")))
        "abcd"

    .. seealso::

        :ref:`take <proc:take>`, :ref:`collect <proc:collect>`,
        :ref:`select <proc:select>`

