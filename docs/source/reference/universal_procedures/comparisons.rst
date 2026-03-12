Comparison Procedures
=====================

Overview
--------

Comparison procedures test relationships between objects and return a boolean
value. This implementation provides three distinct categories of comparison:
*numeric ordering*, *object equality*, and *type-specific equality*, each
suited to different circumstances.

**Numeric Ordering**

The numeric ordering procedures — ``=``, ``<``, ``>``, ``<=``, and ``>=`` —
compare numbers by value and accept any number of arguments, testing the
relationship between each consecutive pair. They operate across the full
numeric tower, automatically promoting arguments to a common type before
comparing. ``=`` additionally accepts complex arguments, since complex numbers
support equality testing even though they have no natural ordering; the
remaining four require real arguments. These procedures are discussed in full
in the :doc:`../primitive_datatypes/numerics` section.

**Object Equality**

The three general equality predicates form a hierarchy of generality and
computational cost, and choosing the right one for a given situation is
important both for correctness and efficiency.

``eq?`` is the most primitive and most efficient. It tests *object identity*
— two objects are ``eq?`` only if they are the exact same object in memory.
This gives reliable results for symbols (which are interned and therefore
unique), booleans, the empty list, and any object compared with itself. It
should not be used for numbers, characters, or compound objects, where
distinct objects with identical values are not guaranteed to be
pointer-identical.

``eqv?`` extends ``eq?`` to cover *value equality* for atomic types: two
numbers are ``eqv?`` if they have the same type, exactness, and value; two
characters are ``eqv?`` if they have the same Unicode scalar value. For all
other types, ``eqv?`` falls back to pointer identity. Note that ``eqv?``
is stricter than ``=`` for numbers: ``(eqv? 1 1.0)`` returns ``#f`` because
the two arguments differ in type and exactness, even though ``(= 1 1.0)``
returns ``#t``.

``equal?`` is the most general predicate and performs *deep structural
comparison*. Two lists are ``equal?`` if they have the same length and all
corresponding elements are ``equal?`` recursively; likewise for vectors and
bytevectors. Two strings are ``equal?`` if they contain the same sequence of
characters. For numbers, ``equal?`` applies the same type-and-exactness
requirement as ``eqv?``. For all other atomic types it behaves identically
to ``eqv?``. Because ``equal?`` recurses into compound structures, it is
necessarily more expensive than ``eq?`` or ``eqv?`` for large objects. It
correctly handles circular and shared structure and will not loop infinitely
on cyclic lists.

As a rule of thumb: use ``eq?`` when comparing symbols or testing object
identity; use ``eqv?`` when comparing numbers or characters where type
matters; use ``equal?`` when comparing compound data structures by content.

Comparison Procedures
---------------------

Numeric Comparison Procedures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

equal (=)
~~~~~~~~~

.. _proc:numeric-eq:

.. function:: (= z1 z2 ...)

    Returns ``#t`` if all arguments are numerically equal, ``#f`` otherwise.
    Unlike ``eqv?`` and ``equal?``, this procedure compares by numeric value
    across types, so ``(= 1 1.0 1/1)`` returns ``#t``. Accepts complex number
    arguments in addition to real numbers.

    :param z1: Two or more numbers to compare.
    :type z1: number
    :return: ``#t`` if all arguments are numerically equal, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (= 1 1)
      #t
      --> (= 1 1.0)
      #t
      --> (= 1 1/1)
      #t
      --> (= 1 2)
      #f
      --> (= 1+0i 1)
      #t
      --> (= 1 1 1 1)
      #t

greater than (>)
~~~~~~~~~~~~~~~~

.. _proc:numeric-gt:

.. function:: (> x1 x2 ...)

    Returns ``#t`` if the arguments are monotonically decreasing, ``#f``
    otherwise. Arguments must be real numbers; complex numbers are not
    permitted.

    :param x1: Two or more real numbers to compare.
    :type x1: real
    :return: ``#t`` if the arguments are monotonically decreasing, ``#f``
             otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (> 3 2)
      #t
      --> (> 2 3)
      #f
      --> (> 3 2 1)
      #t
      --> (> 3 3)
      #f
      --> (> 1/2 1/3)
      #t

less than (<)
~~~~~~~~~~~~~

.. _proc:numeric-lt:

.. function:: (< x1 x2 ...)

    Returns ``#t`` if the arguments are monotonically increasing, ``#f``
    otherwise. Arguments must be real numbers; complex numbers are not
    permitted.

    :param x1: Two or more real numbers to compare.
    :type x1: real
    :return: ``#t`` if the arguments are monotonically increasing, ``#f``
             otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (< 1 2)
      #t
      --> (< 2 1)
      #f
      --> (< 1 2 3)
      #t
      --> (< 1 1)
      #f
      --> (< 1/3 1/2)
      #t

greater than or equal (>=)
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. _proc:numeric-gte:

.. function:: (>= x1 x2 ...)

    Returns ``#t`` if the arguments are monotonically non-increasing, ``#f``
    otherwise. Arguments must be real numbers; complex numbers are not
    permitted.

    :param x1: Two or more real numbers to compare.
    :type x1: real
    :return: ``#t`` if the arguments are monotonically non-increasing, ``#f``
             otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (>= 3 2)
      #t
      --> (>= 3 3)
      #t
      --> (>= 2 3)
      #f
      --> (>= 3 2 2 1)
      #t

less than or equal (<=)
~~~~~~~~~~~~~~~~~~~~~~~

.. _proc:numeric-lte:

.. function:: (<= x1 x2 ...)

    Returns ``#t`` if the arguments are monotonically non-decreasing, ``#f``
    otherwise. Arguments must be real numbers; complex numbers are not
    permitted.

    :param x1: Two or more real numbers to compare.
    :type x1: real
    :return: ``#t`` if the arguments are monotonically non-decreasing, ``#f``
             otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (<= 1 2)
      #t
      --> (<= 1 1)
      #t
      --> (<= 2 1)
      #f
      --> (<= 1 1 2 3)
      #t

General Comparison Procedures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

eq?
~~~

.. _proc:eq?:

.. function:: (eq? obj1 obj2)

    Returns ``#t`` if *obj1* and *obj2* are the exact same object, ``#f``
    otherwise. Comparison is by pointer identity — two objects are ``eq?``
    only if they occupy the same location in memory.

    ``eq?`` is the most efficient equality predicate but the least general.
    It gives reliable results for symbols, booleans, the empty list, and other
    unique objects. It should not be used to compare numbers, characters,
    strings, pairs, or vectors, since two distinct objects with the same value
    are not guaranteed to be pointer-identical.

    :param obj1: The first object to compare.
    :type obj1: any
    :param obj2: The second object to compare.
    :type obj2: any
    :return: ``#t`` if *obj1* and *obj2* are the same object, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (eq? 'foo 'foo)
      #t
      --> (eq? '() '())
      #t
      --> (eq? #t #t)
      #t
      --> (let ((x '(1 2))) (eq? x x))
      #t
      --> (eq? '(1 2) '(1 2))
      #f
      --> (eq? "hello" "hello")
      #f

eqv?
~~~~

.. _proc:eqv?:

.. function:: (eqv? obj1 obj2)

    Returns ``#t`` if *obj1* and *obj2* are equivalent, ``#f`` otherwise.
    ``eqv?`` extends ``eq?`` by additionally considering numbers of the same
    type and value, and characters with the same Unicode scalar value, as
    equivalent — even if they are distinct objects in memory.

    For types not covered by these rules (pairs, vectors, strings, and other
    compound objects), ``eqv?`` falls back to pointer identity. Use
    ``equal?`` for deep structural comparison of compound objects.

    Note that ``eqv?`` requires both objects to be of the same type before
    comparing values, so ``(eqv? 1 1.0)`` returns ``#f`` even though
    ``(= 1 1.0)`` returns ``#t``.

    :param obj1: The first object to compare.
    :type obj1: any
    :param obj2: The second object to compare.
    :type obj2: any
    :return: ``#t`` if *obj1* and *obj2* are equivalent, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (eqv? 42 42)
      #t
      --> (eqv? 1 1.0)
      #f
      --> (eqv? #\a #\a)
      #t
      --> (eqv? 'foo 'foo)
      #t
      --> (eqv? '() '())
      #t
      --> (eqv? '(1 2) '(1 2))
      #f

equal?
~~~~~~

.. _proc:equal?:

.. function:: (equal? obj1 obj2)

    Returns ``#t`` if *obj1* and *obj2* have the same structure and contents,
    ``#f`` otherwise. ``equal?`` performs a deep recursive comparison: two
    lists are ``equal?`` if they have the same length and each corresponding
    pair of elements is ``equal?``; two vectors are ``equal?`` if they have
    the same length and each corresponding pair of elements is ``equal?``; two
    strings are ``equal?`` if they contain the same sequence of characters;
    two bytevectors are ``equal?`` if they are of the same type and contain
    the same sequence of elements.

    For numbers, ``equal?`` requires both the same type and the same
    exactness, so ``(equal? 2 2/1)`` returns ``#f`` even though
    ``(= 2 2/1)`` returns ``#t``. For all other atomic types, ``equal?``
    behaves like ``eqv?``.

    ``equal?`` correctly handles circular and shared list structures using
    cycle detection, and will not loop infinitely on circular lists.

    :param obj1: The first object to compare.
    :type obj1: any
    :param obj2: The second object to compare.
    :type obj2: any
    :return: ``#t`` if *obj1* and *obj2* are structurally equal, ``#f``
             otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (equal? '(1 2 3) '(1 2 3))
      #t
      --> (equal? '(1 2 3) '(1 2 4))
      #f
      --> (equal? #(1 2 3) #(1 2 3))
      #t
      --> (equal? "hello" "hello")
      #t
      --> (equal? 2 2/1)
      #f
      --> (equal? '(1 (2 3)) '(1 (2 3)))
      #t

