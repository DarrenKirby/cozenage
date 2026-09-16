Base CXR Library
==================

Overview
--------

The ``CxR`` procedures are a standard feature of Lisp and Scheme, providing convenient shorthands for composing up to
four ``car`` and ``cdr`` operations. The names of the procedures encode the sequence of operations: the letters
between the initial 'c' and the final 'r' correspond to the sequence of ``car`` (from 'a') and ``cdr`` (from 'd').

For example, the procedure ``caddr`` is equivalent to the expression ``(car (cdr (cdr x)))``. This allows for more
concise and often more readable code when accessing elements deep inside a nested list structure.

**Derivation Example:**

* To get the third element of a list `(1 2 3 4)`:
    * The long form is ``(car (cdr (cdr '(1 2 3 4))))`` which evaluates to ``3``.
    * The short form is ``(caddr '(1 2 3 4))`` which also evaluates to ``3``.

**Typical Use:**

These procedures are most useful for accessing fixed parts of a data structure represented as a list. For instance,
if a "person" record is structured as `'(firstname lastname (address-details))`, you could access the last name
with ``cadr`` and the address details with ``caddr``.

.. code-block:: scheme

   --> (define person '("John" "Doe" ("123 Lisp Lane" "Schemeville")))
   --> (cadr person)
     "Doe"
   --> (caaddr person)
     "123 Lisp Lane"

**Positional accessors**

This module also defines positional accessors from ``first`` to ``tenth``. Their usage is strait-forward and
intuitive:

.. code-block:: scheme

    --> (define l '(1 2 3 4 5 6 7 8 9 10))
    l
    --> (first l)
    1
    --> (fifth l)
    5
    --> (tenth l)
    10

Unlike the cxr procedures, the positional accessors can only be used on proper lists.

Procedure Definitions
---------------------

.. index:: caaaar
.. function:: (caaaar pair)

   Equivalent to ``(car (car (car (car pair))))``.

.. index:: caaar
.. function:: (caaar pair)

   Equivalent to ``(car (car (car pair)))``.

.. index:: caaddr
.. function:: (caaddr pair)

   Equivalent to ``(car (car (cdr (cdr pair))))``.

.. index:: cadaar
.. function:: (cadaar pair)

   Equivalent to ``(car (cdr (car (car pair))))``.

.. index:: cadar
.. function:: (cadar pair)

   Equivalent to ``(car (cdr (car pair)))``.

.. index:: cadddr
.. function:: (cadddr pair)

   Equivalent to ``(car (cdr (cdr (cdr pair))))``.

.. index:: cdaaar
.. function:: (cdaaar pair)

   Equivalent to ``(cdr (car (car (car pair))))``.

.. index:: cdaar
.. function:: (cdaar pair)

   Equivalent to ``(cdr (car (car pair)))``.

.. index:: cdaddr
.. function:: (cdaddr pair)

   Equivalent to ``(cdr (car (cdr (cdr pair))))``.

.. index:: cddaar
.. function:: (cddaar pair)

   Equivalent to ``(cdr (cdr (car (car pair))))``.

.. index:: cddar
.. function:: (cddar pair)

   Equivalent to ``(cdr (cdr (car pair)))``.

.. index:: cddddr
.. function:: (cddddr pair)

   Equivalent to ``(cdr (cdr (cdr (cdr pair))))``.

.. index:: caaard
.. function:: (caaadr pair)

   Equivalent to ``(car (car (car (cdr pair))))``.

.. index:: caadar
.. function:: (caadar pair)

   Equivalent to ``(car (car (cdr (car pair))))``.

.. index:: caadr
.. function:: (caadr pair)

   Equivalent to ``(car (car (cdr pair)))``.

.. index:: cadadr
.. function:: (cadadr pair)

   Equivalent to ``(car (cdr (car (cdr pair))))``.

.. index:: caddar
.. function:: (caddar pair)

   Equivalent to ``(car (cdr (cdr (car pair))))``.

.. index:: caddr
.. function:: (caddr pair)

   Equivalent to ``(car (cdr (cdr pair)))``.

.. index:: cdaadr
.. function:: (cdaadr pair)

   Equivalent to ``(cdr (car (car (cdr pair))))``.

.. index:: cdadar
.. function:: (cdadar pair)

   Equivalent to ``(cdr (car (cdr (car pair))))``.

.. index:: cdadr
.. function:: (cdadr pair)

   Equivalent to ``(cdr (car (cdr pair)))``.

.. index:: cddadr
.. function:: (cddadr pair)

   Equivalent to ``(cdr (cdr (car (cdr pair))))``.

.. index:: cdddar
.. function:: (cdddar pair)

   Equivalent to ``(cdr (cdr (cdr (car pair))))``.

.. index:: cdddr
.. function:: (cdddr pair)

   Equivalent to ``(cdr (cdr (cdr pair)))``.

