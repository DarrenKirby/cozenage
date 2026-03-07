Predicate Procedures
====================

Overview
--------

Predicate procedures are procedures that test an object for a specific property
and return a boolean value — either ``#t`` (true) or ``#f`` (false). By
convention, the names of predicate procedures end with a question mark ``?``.

**Type Predicates**

The most fundamental predicates are the *type predicates*, which test whether
an object belongs to a particular type. Every first-class type in this
implementation has a corresponding type predicate: ``number?``, ``boolean?``,
``null?``, ``pair?``, ``list?``, ``procedure?``, ``symbol?``, ``string?``,
``char?``, ``vector?``, ``bytevector?``, ``port?``, ``set?``, ``hash?``, and
``eof-object?``. These are the primary tool for runtime type dispatch and
defensive programming.

Note that ``list?`` is stricter than ``pair?``: a pair is any cons cell,
whereas a list is specifically a chain of pairs terminated by the empty list
``()``. An improper list such as ``(1 . 2)`` satisfies ``pair?`` but not
``list?``. The empty list ``()`` satisfies ``null?`` and ``list?`` but not
``pair?``.

**Numeric Tower Predicates**

The numeric type predicates — ``complex?``, ``real?``, ``rational?``,
``integer?``, ``exact-integer?``, and ``bigint?`` — reflect the structure of
the numeric tower. The tower is hierarchical: every integer is rational, every
rational is real, and every real is complex. Consequently these predicates are
not mutually exclusive: ``(rational? 42)`` and ``(integer? 42)`` are both
``#t``. The key distinctions are:

- ``complex?`` is equivalent to ``number?`` — all numbers are complex.
- ``real?`` returns ``#t`` for any number with a zero imaginary part.
- ``rational?`` returns ``#t`` for exact numbers and finite inexact reals.
- ``integer?`` follows the tower definition: ``(integer? 3.0)`` is ``#t``
  since ``3.0`` has no fractional part, even though it is inexact.
- ``exact-integer?`` is the strictest integer test: it returns ``#t`` only
  for exact integers, excluding inexact integers like ``3.0``.
- ``bigint?`` distinguishes arbitrary-precision integers from native 64-bit
  integers; note that both satisfy ``integer?`` and ``exact-integer?``.

The exactness predicates ``exact?`` and ``inexact?`` test whether a number's
value is represented exactly. Exact numbers include all integers, rationals,
and bignums. Inexact numbers include all floating-point reals and any complex
number with at least one inexact component.

**Numeric Value Predicates**

The numeric value predicates — ``zero?``, ``positive?``, ``negative?``,
``odd?``, and ``even?`` — test properties of a number's value rather than its
type. ``positive?`` and ``negative?`` require a real argument and will raise an
error if given a complex number with a non-zero imaginary part, since the
concepts of positive and negative are not defined for the complex numbers.
``odd?`` and ``even?`` require an integer argument per the numeric tower
definition, which means ``(odd? 3.0)`` is valid and returns ``#t``.

**Boolean Value Predicates**

The procedures ``true?`` and ``false?`` test for the literal boolean objects
``#t`` and ``#f`` respectively. These are distinct from the general notion of
*truthiness* used in conditional expressions: in Scheme, every value except
``#f`` is considered true in a boolean context, so ``(if 0 'yes 'no)`` yields
``'yes``. By contrast, ``(true? 0)`` returns ``#f``, since ``0`` is not the
literal ``#t`` object. These procedures are not part of R7RS and are provided
as a convenience.

Predicate Procedures
--------------------

Type-Identity Procedures
^^^^^^^^^^^^^^^^^^^^^^^^

.. _proc:number?:

.. function:: (number? obj)

    Returns ``#t`` if *obj* is a number of any numeric type (integer, rational,
    real, or complex), ``#f`` otherwise.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a number, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (number? 42)
      #t
      --> (number? 1/3)
      #t
      --> (number? 3.14)
      #t
      --> (number? 1+2i)
      #t
      --> (number? "42")
      #f


.. _proc:boolean?:

.. function:: (boolean? obj)

    Returns ``#t`` if *obj* is a boolean (``#t`` or ``#f``), ``#f`` otherwise.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a boolean, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (boolean? #t)
      #t
      --> (boolean? #f)
      #t
      --> (boolean? 0)
      #f


.. _proc:null?:

.. function:: (null? obj)

    Returns ``#t`` if *obj* is the empty list ``()``, ``#f`` otherwise.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is the empty list, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (null? '())
      #t
      --> (null? '(1 2 3))
      #f
      --> (null? #f)
      #f


.. _proc:pair?:

.. function:: (pair? obj)

    Returns ``#t`` if *obj* is a pair, ``#f`` otherwise.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a pair, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (pair? '(1 2))
      #t
      --> (pair? '(1 . 2))
      #t
      --> (pair? '())
      #f
      --> (pair? 42)
      #f


.. _proc:list?:

.. function:: (list? obj)

    Returns ``#t`` if *obj* is a proper list, ``#f`` otherwise. A proper list
    is either the empty list or a chain of pairs terminated by the empty list.
    Improper lists and circular lists return ``#f``.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a proper list, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (list? '(1 2 3))
      #t
      --> (list? '())
      #t
      --> (list? '(1 . 2))
      #f
      --> (list? 42)
      #f


.. _proc:procedure?:

.. function:: (procedure? obj)

    Returns ``#t`` if *obj* is a procedure (either a lambda or a built-in
    procedure), ``#f`` otherwise.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a procedure, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (procedure? car)
      #t
      --> (procedure? (lambda (x) x))
      #t
      --> (procedure? 42)
      #f


.. _proc:symbol?:

.. function:: (symbol? obj)

    Returns ``#t`` if *obj* is a symbol, ``#f`` otherwise.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a symbol, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (symbol? 'foo)
      #t
      --> (symbol? "foo")
      #f
      --> (symbol? 42)
      #f


.. _proc:string?:

.. function:: (string? obj)

    Returns ``#t`` if *obj* is a string, ``#f`` otherwise.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a string, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (string? "hello")
      #t
      --> (string? #\h)
      #f
      --> (string? 'hello)
      #f


.. _proc:char?:

.. function:: (char? obj)

    Returns ``#t`` if *obj* is a character, ``#f`` otherwise.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a character, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (char? #\a)
      #t
      --> (char? "a")
      #f
      --> (char? 97)
      #f


.. _proc:vector?:

.. function:: (vector? obj)

    Returns ``#t`` if *obj* is a vector, ``#f`` otherwise.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a vector, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (vector? #(1 2 3))
      #t
      --> (vector? '(1 2 3))
      #f
      --> (vector? 42)
      #f


.. _proc:bytevector?:

.. function:: (bytevector? obj)

    Returns ``#t`` if *obj* is a bytevector, ``#f`` otherwise.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a bytevector, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (bytevector? #u8(1 2 3))
      #t
      --> (bytevector? #(1 2 3))
      #f
      --> (bytevector? 42)
      #f


.. _proc:port?:

.. function:: (port? obj)

    Returns ``#t`` if *obj* is a port, ``#f`` otherwise.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a port, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (port? (current-input-port))
      #t
      --> (port? "myfile.txt")
      #f


.. _proc:set?:

.. function:: (set? obj)

    Returns ``#t`` if *obj* is a set, ``#f`` otherwise.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a set, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block::

      --> (set? #{1 2 3})
      #t
      --> (set? (set 1 2 3))
      #t
      --> (set? '(1 2 3))
      #f
      --> (set? #[1 "one"])
      #f


.. _proc:hash?:

.. function:: (hash? obj)

    Returns ``#t`` if *obj* is a hash, ``#f`` otherwise.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a hash, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block::

      --> (hash? #["a" 1 "b" 2])
      #t
      --> (hash? (hash "a" 1 "b" 2))
      #t
      --> (hash? #{1 2 3})
      #f
      --> (hash? '((a . 1) (b . 2)))
      #f


.. _proc:eof-object?:

.. function:: (eof-object? obj)

    Returns ``#t`` if *obj* is the end-of-file object, ``#f`` otherwise. The
    end-of-file object is returned by input procedures when the end of an input
    stream is reached.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is the end-of-file object, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (eof-object? (read (open-input-string "")))
      #t
      --> (eof-object? "")
      #f

Numeric identity procedures
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _proc:exact?:

.. function:: (exact? z)

    Returns ``#t`` if *z* is an exact number, ``#f`` otherwise. For complex
    numbers, returns ``#t`` only if both the real and imaginary parts are exact.

    :param z: A number to test.
    :type z: number
    :return: ``#t`` if *z* is exact, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (exact? 42)
      #t
      --> (exact? 1/3)
      #t
      --> (exact? 3.14)
      #f
      --> (exact? 1+2i)
      #t
      --> (exact? 1+2.0i)
      #f


.. _proc:inexact?:

.. function:: (inexact? z)

    Returns ``#t`` if *z* is an inexact number, ``#f`` otherwise. For complex
    numbers, returns ``#t`` if either the real or imaginary part is inexact.

    :param z: A number to test.
    :type z: number
    :return: ``#t`` if *z* is inexact, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (inexact? 3.14)
      #t
      --> (inexact? 42)
      #f
      --> (inexact? 1/3)
      #f
      --> (inexact? 1.0+2.0i)
      #t


.. _proc:complex?:

.. function:: (complex? obj)

    Returns ``#t`` if *obj* is a number, ``#f`` otherwise. Per R7RS, all
    numbers are complex numbers, so this procedure is equivalent to
    ``number?``.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a number, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (complex? 42)
      #t
      --> (complex? 1/3)
      #t
      --> (complex? 1+2i)
      #t
      --> (complex? "hello")
      #f


.. _proc:real?:

.. function:: (real? obj)

    Returns ``#t`` if *obj* is a real number, ``#f`` otherwise. Integers,
    rationals, and reals all return ``#t``. A complex number returns ``#t``
    only if its imaginary part is exactly zero.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a real number, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (real? 42)
      #t
      --> (real? 1/3)
      #t
      --> (real? 3.14)
      #t
      --> (real? 1+0i)
      #t
      --> (real? 1+2i)
      #f
      --> (real? "hello")
      #f


.. _proc:rational?:

.. function:: (rational? obj)

    Returns ``#t`` if *obj* is a rational number, ``#f`` otherwise. Exact
    integers and rationals always return ``#t``. Inexact reals return ``#t``
    if they are finite. A complex number returns ``#t`` only if its real part
    is exact and its imaginary part is zero. Non-numbers return ``#f`` without
    raising an error.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a rational number, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (rational? 42)
      #t
      --> (rational? 1/3)
      #t
      --> (rational? 3.14)
      #t
      --> (rational? +inf.0)
      #f
      --> (rational? +nan.0)
      #f
      --> (rational? "hello")
      #f


.. _proc:integer?:

.. function:: (integer? obj)

    Returns ``#t`` if *obj* is an integer, ``#f`` otherwise. This follows the
    R7RS numeric tower definition: a real number with no fractional part is
    considered an integer, so ``(integer? 3.0)`` returns ``#t``. Non-numbers
    return ``#f`` without raising an error.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is an integer, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (integer? 42)
      #t
      --> (integer? 3.0)
      #t
      --> (integer? 3.5)
      #f
      --> (integer? 1/3)
      #f
      --> (integer? "hello")
      #f


.. _proc:exact-integer?:

.. function:: (exact-integer? obj)

    Returns ``#t`` if *obj* is both an exact number and an integer, ``#f``
    otherwise. Unlike ``integer?``, this returns ``#f`` for inexact integers
    such as ``3.0``.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is an exact integer, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (exact-integer? 42)
      #t
      --> (exact-integer? 3.0)
      #f
      --> (exact-integer? 1/3)
      #f
      --> (exact-integer? "hello")
      #f


.. _proc:bigint?:

.. function:: (bigint? obj)

    Returns ``#t`` if *obj* is a bignum (an exact integer too large to be
    represented as a native 64-bit integer), ``#f`` otherwise. This is a
    non-standard predicate specific to this implementation.

    Note that ``(integer? obj)`` and ``(exact-integer? obj)`` both return
    ``#t`` for bignums; ``bigint?`` is provided for the rare cases where code
    needs to distinguish between native integers and arbitrary-precision ones.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is a bignum, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (bigint? 42)
      #f
      --> (bigint? 99999999999999999999999999999)
      #t
      --> (bigint? (expt 2 128))
      #t
      --> (bigint? 1/3)
      #f

Numeric predicate procedures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _proc:zero?:

.. function:: (zero? z)

    Returns ``#t`` if *z* is zero, ``#f`` otherwise. For complex numbers,
    returns ``#t`` only if both the real and imaginary parts are zero.

    :param z: A number to test.
    :type z: number
    :return: ``#t`` if *z* is zero, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (zero? 0)
      #t
      --> (zero? 0.0)
      #t
      --> (zero? 0+0i)
      #t
      --> (zero? 1)
      #f


.. _proc:positive?:

.. function:: (positive? x)

    Returns ``#t`` if *x* is greater than zero, ``#f`` otherwise. *x* must be
    a real number; an error is raised if *x* is a complex number with a
    non-zero imaginary part.

    :param x: A real number to test.
    :type x: real
    :return: ``#t`` if *x* is greater than zero, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (positive? 1)
      #t
      --> (positive? 0)
      #f
      --> (positive? -1)
      #f
      --> (positive? 0.1)
      #t


.. _proc:negative?:

.. function:: (negative? x)

    Returns ``#t`` if *x* is less than zero, ``#f`` otherwise. *x* must be a
    real number; an error is raised if *x* is a complex number with a non-zero
    imaginary part.

    :param x: A real number to test.
    :type x: real
    :return: ``#t`` if *x* is less than zero, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (negative? -1)
      #t
      --> (negative? 0)
      #f
      --> (negative? 1)
      #f
      --> (negative? -0.1)
      #t


.. _proc:odd?:

.. function:: (odd? n)

    Returns ``#t`` if *n* is an odd integer, ``#f`` otherwise. *n* must be an
    integer; an error is raised if *n* is a non-integer number such as a
    rational or inexact real with a fractional part.

    :param n: An integer to test.
    :type n: integer
    :return: ``#t`` if *n* is odd, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (odd? 1)
      #t
      --> (odd? 2)
      #f
      --> (odd? -3)
      #t
      --> (odd? 3.0)
      #t


.. _proc:even?:

.. function:: (even? n)

    Returns ``#t`` if *n* is an even integer, ``#f`` otherwise. *n* must be an
    integer; an error is raised if *n* is a non-integer number such as a
    rational or inexact real with a fractional part.

    :param n: An integer to test.
    :type n: integer
    :return: ``#t`` if *n* is even, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (even? 2)
      #t
      --> (even? 3)
      #f
      --> (even? 0)
      #t
      --> (even? 4.0)
      #t

Boolean Predicates
^^^^^^^^^^^^^^^^^^

.. _proc:false?:

.. function:: (false? obj)

    Returns ``#t`` if *obj* is the literal boolean ``#f``, ``#f`` otherwise.
    Note that this tests for the literal ``#f`` object specifically, not
    general falsiness — in this implementation, as in R7RS, ``#f`` is the only
    false value, but this procedure is distinct from simply using ``not``.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is ``#f``, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (false? #f)
      #t
      --> (false? #t)
      #f
      --> (false? 0)
      #f
      --> (false? '())
      #f


.. _proc:true?:

.. function:: (true? obj)

    Returns ``#t`` if *obj* is the literal boolean ``#t``, ``#f`` otherwise.
    Note that this tests for the literal ``#t`` object specifically, not
    general truthiness — any non-``#f`` value is truthy in Scheme, but this
    procedure returns ``#t`` only for the boolean ``#t`` itself.

    :param obj: The object to test.
    :type obj: any
    :return: ``#t`` if *obj* is ``#t``, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (true? #t)
      #t
      --> (true? #f)
      #f
      --> (true? 1)
      #f
      --> (true? '(1 2 3))
      #f

