Numerics
========

Overview
--------

It is important to distinguish between **mathematical numbers**, the **Cozenage numbers** that attempt to model them,
the **machine representations** used to implement those numbers, and the **notations** used to write numeric literals
in source code.

From a user’s perspective, Cozenage numbers are abstract values that behave according to mathematical rules as closely
as practical, while hiding most details of how they are represented internally.

Numeric Types in Cozenage
^^^^^^^^^^^^^^^^^^^^^^^^^

Cozenage implements four distinct object types to represent mathematical values:

* **Integers**
* **Rationals**
* **Reals**
* **Complex numbers**

Each of these types corresponds to a well-known mathematical category, and each has a clear meaning independent of Cozenage.

The Numeric Tower
^^^^^^^^^^^^^^^^^

Mathematically, numbers are arranged into a *tower of subtypes*, in which each level is a subset of the level above it:

* number
* complex number
* real number
* rational number
* integer

This means that every integer is also a rational number, every rational number is also a real number, and every real
number is also a complex number.

For example, the mathematical number 3 is:

* an integer,
* a rational number (3 = 3/1),
* a real number (3 = 3.0)
* a complex number (3 + 0i).

The same is true of the Cozenage numbers that model 3.

For Cozenage numbers, these categories are identified by the predicates `number?`, `complex?`, `real?`, `rational?`,
and `integer?`. It is important to note that these predicates response to a number's mathematical value, and not
the number's representation:

.. code-block:: scheme

    --> (integer? 3.1)
    #false
    --> (integer? 3.0)
    #true
    --> (integer? 3+0i)
    #true
    --> (integer? 3+1i)
    #false
    --> (rational? 3)
    #true
    --> (real? 3)
    #true
    --> (complex? 3)
    #true

`number?` is essentially an alias for `complex?`, and will return true for any numeric object.

 .. code-block:: scheme

     --> (number? 3)
     #true
     --> (number? 1/3)
     #true
     --> (number? 23.364)
     #true
     --> (number? 23-45i)
     #true
     --> (number? "1000") ; "1000" is a string
     #false

Mathematical Meaning of the Numeric Types
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following informal definitions describe what each numeric type represents mathematically.

**Integers**

Integers are whole numbers with no fractional part. They include positive numbers, negative numbers, and zero.

Examples:

.. code-block:: scheme

    0
    42
    -7

**Rationals**

Rational numbers are numbers that can be expressed as the ratio of two integers, where the denominator is non-zero.
This includes all integers, since any integer *n* can be written as *n/1*.

Examples:

.. code-block:: scheme

    1/2
    -7/3
    3/1

**Reals**

Real numbers represent values along the continuous number line. In Cozenage, reals are typically written using decimal
notation and may represent values that cannot be expressed exactly as rational numbers.

Examples:

.. code-block:: scheme

    3.0
    2.71828
    -0.5

**Complex Numbers**

Complex numbers consist of a real part and an imaginary part, usually written in the form *a + bi*, where *i* is the
square root of −1.

Examples:

.. code-block:: scheme

    3+0i
    1+2i
    -4.5-0.1i

Representation Independence
^^^^^^^^^^^^^^^^^^^^^^^^^^^

There is no simple relationship between a number’s *type* and its *representation* inside a computer.

Although Cozenage offers four different representations of the number 3:

.. code-block:: scheme

    3    ; integer
    3/1  ; rational
    3.0  ; real
    3+0i ; complex

these different representations denote the same mathematical integer.

Cozenage’s numerical operations treat numbers as abstract data, as independent of their representation as possible.
This means, with a few exceptions that will be noted elsewhere, numeric procedures will work on any of the above
representations, and on any combination of them.

For example, arithmetic procedures do not require operands to have the same numeric type.

Numeric Literal Syntax
^^^^^^^^^^^^^^^^^^^^^^

Cozenage provides several ways to write numeric literals. These notations affect how numbers are read, but not the
abstract numeric value they represent.

Default Base (Decimal)
^^^^^^^^^^^^^^^^^^^^^^

By default, numeric literals are interpreted as base-10 (decimal) numbers.

Examples:

.. code-block:: scheme

    10
    255
    -42

Explicit Radix Prefixes
^^^^^^^^^^^^^^^^^^^^^^^

Cozenage also allows numeric literals to specify an explicit base using a prefix:

* `#b` for binary (base 2)
* `#o` for octal (base 8)
* `#d` for decimal (base 10)
* `#x` for hexadecimal (base 16)

Examples:

.. code-block:: scheme

    #b1010   ; binary 10
    #o377    ; octal 255
    #d42     ; decimal 42
    #xFF     ; hexadecimal 255

All of these literals denote exact integer values, regardless of the base used to write them.

Exact and Inexact Numbers
^^^^^^^^^^^^^^^^^^^^^^^^^

Cozenage distinguishes between **exact** and **inexact** numbers.

* **Exact numbers** represent values with no loss of precision.
* **Inexact numbers** represent values that may be approximations.

Integers and rational numbers are typically exact. Real numbers written with decimal notation are often inexact.

Examples:

.. code-block:: scheme

    1/3     ; exact rational
    3       ; exact integer
    3.0     ; inexact real

Exactness Prefixes
^^^^^^^^^^^^^^^^^^

Numeric literals may explicitly specify exactness using prefixes:

* `#e` forces an exact number
* `#i` forces an inexact number

Examples:

.. code-block:: scheme

    #e3.0   ; exact representation of 3
    #i3     ; inexact representation of 3

These prefixes control how a number is interpreted when it is read, not how it is printed. The REPL (and output
procedures such as display and write) always represent numbers in base 10 decimal form.

Mixed Numeric Computations
^^^^^^^^^^^^^^^^^^^^^^^^^^

When arithmetic combines exact and inexact numbers, the result is generally inexact. This reflects the idea that an
approximate input produces an approximate result. Operations on inexact numbers are said to be contagious, in that an
operation that includes even one inexact input will produce an inexact result. While this may sound disconcerting at
first, it is important to understand that an inexact value is typically correct to within the limits of machine
representation. Outside of rigorous symbolic or high-precision scientific computation, inexact numbers are entirely
appropriate and expected for the vast majority of practical programs.

Example:

.. code-block:: scheme

    --> (+ 1 2.0)
    3.0

Summary
^^^^^^^

Cozenage numbers model mathematical numbers using a small set of well-defined numeric types. Integers, rationals, reals,
and complex numbers form a numeric tower in which each category is a subset of the one above it.

Numeric literals provide flexible notation, including different bases and explicit control over exactness. By treating
numbers as abstract values rather than fixed representations, Cozenage allows numeric code to be written in a clear and
mathematically natural style.

Numeric procedures
------------------

.. _proc:add:

addition
^^^^^^^^

.. function:: (+ n ...)

    Returns the sum of all arguments. Applies numeric promotion as needed across
    the numeric tower. If no arguments are provided, returns the additive
    identity ``0``. If exactly one argument is provided, it is returned
    unchanged.

    :param n: Zero or more numbers to sum.
    :type n: number
    :return: The sum of all arguments, or ``0`` if no arguments are given.
    :rtype: number

    **Example:**

    .. code-block:: scheme

      --> (+)
      0
      --> (+ 5)
      5
      --> (+ 1 2 3)
      6
      --> (+ 1/3 1/6)
      1/2
      --> (+ 1 1.5)
      2.5
      --> (+ 1+2i 3+4i)
      4+6i


.. _proc:sub:

subtraction
^^^^^^^^^^^

.. function:: (- n1 [n2 ...])

    When called with a single argument, returns the negation of *n1*. When
    called with two or more arguments, returns the result of subtracting each
    successive argument from *n1*. Applies numeric promotion as needed across
    the numeric tower.

    :param n1: The number to negate, or the minuend in a subtraction.
    :type n1: number
    :param n2: One or more numbers to subtract from *n1*.
    :type n2: number
    :return: The negation of *n1*, or the result of subtracting all subsequent
             arguments from *n1*.
    :rtype: number

    **Example:**

    .. code-block:: scheme

      --> (- 5)
      -5
      --> (- 10 3)
      7
      --> (- 10 3 2)
      5
      --> (- 1/2)
      -1/2
      --> (- 1+2i)
      -1-2i


.. _proc:mul:

multiplication
^^^^^^^^^^^^^^

.. function:: (* n ...)

    Returns the product of all arguments. Applies numeric promotion as needed
    across the numeric tower. If no arguments are provided, returns the
    multiplicative identity ``1``. If exactly one argument is provided, it is
    returned unchanged.

    :param n: Zero or more numbers to multiply.
    :type n: number
    :return: The product of all arguments, or ``1`` if no arguments are given.
    :rtype: number

    **Example:**

    .. code-block:: scheme

      --> (*)
      1
      --> (* 5)
      5
      --> (* 2 3 4)
      24
      --> (* 1/3 3)
      1
      --> (* 2 1.5)
      3.0
      --> (* 1+2i 3+4i)
      -5+10i


.. _proc:div:

division
^^^^^^^^

.. function:: (/ n1 [n2 ...])

    When called with a single argument, returns the reciprocal of *n1*. When
    called with two or more arguments, returns the result of dividing *n1* by
    each successive argument. Applies numeric promotion as needed across the
    numeric tower.

    Integer division produces a rational result when the quotient is not exact
    (e.g. ``(/ 10 3)`` yields ``10/3``). Division of an integer by an integer
    that divides evenly yields an integer. Dividing by integer zero raises an
    error. Dividing ``0.0`` by ``0.0`` returns ``+nan.0`` per IEEE 754.

    :param n1: The number to take the reciprocal of, or the dividend.
    :type n1: number
    :param n2: One or more divisors.
    :type n2: number
    :return: The reciprocal of *n1*, or the result of dividing *n1* by all
             subsequent arguments.
    :rtype: number

    **Example:**

    .. code-block:: scheme

      --> (/ 2)
      1/2
      --> (/ 1.0)
      1.0
      --> (/ 10 2)
      5
      --> (/ 10 3)
      10/3
      --> (/ 1 3 3)
      1/9
      --> (/ 1+2i)
      1/5-2/5i

.. _proc:abs:

abs
^^^

.. function:: (abs x)

    Returns the absolute value of *x*. For complex numbers, returns the
    magnitude ``√(a² + b²)`` as an inexact real number. For all other numeric
    types, returns a non-negative value of the same type as *x*.

    :param x: The number to take the absolute value of.
    :type x: number
    :return: The absolute value or magnitude of *x*.
    :rtype: number

    **Example:**

    .. code-block:: scheme

      --> (abs -7)
      7
      --> (abs 3.14)
      3.14
      --> (abs -1/3)
      1/3
      --> (abs 3+4i)
      5.0


.. _proc:expt:

expt
^^^^

.. function:: (expt z1 z2)

    Returns *z1* raised to the power *z2*. The type of the result depends on
    the types of the arguments:

    - If either argument is ``0`` (exactly), ``(expt 0 z2)`` returns ``0`` and
      ``(expt z1 0)`` returns ``1``.
    - If *z1* is a non-negative real and *z2* is negative, an exact rational is
      returned.
    - If *z1* is a negative real and *z2* is a non-integer, the result is
      a complex number.
    - If *z1* is a bignum, *z2* must be an exact integer and must not exceed
      the platform ``INT_MAX``.

    :param z1: The base.
    :type z1: number
    :param z2: The exponent.
    :type z2: number
    :return: *z1* raised to the power *z2*.
    :rtype: number

    **Example:**

    .. code-block:: scheme

      --> (expt 2 10)
      1024
      --> (expt 2 -3)
      1/8
      --> (expt 2.0 10)
      1024.0
      --> (expt -1 1/2)
      0.0+1.0i
      --> (expt 1+2i 2)
      -3.0+4.0i
      --> (expt 0 0)
      1


.. _proc:modulo:

modulo
^^^^^^

.. function:: (modulo n1 n2)

    Returns the remainder of dividing *n1* by *n2*, with the result having the
    same sign as *n2* (the divisor). Both arguments must be integers.

    :param n1: The dividend.
    :type n1: integer
    :param n2: The divisor.
    :type n2: integer
    :return: The modulo of *n1* and *n2*, with the sign of *n2*.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (modulo 10 3)
      1
      --> (modulo -10 3)
      2
      --> (modulo 10 -3)
      -2
      --> (modulo -10 -3)
      -1


.. _proc:quotient:

quotient
^^^^^^^^

.. function:: (quotient n1 n2)

    Returns the integer quotient of dividing *n1* by *n2*, truncating toward
    zero. Both arguments must be integers.

    :param n1: The dividend.
    :type n1: integer
    :param n2: The divisor.
    :type n2: integer
    :return: The integer quotient of *n1* and *n2*, truncated toward zero.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (quotient 10 3)
      3
      --> (quotient -10 3)
      -3
      --> (quotient 10 -3)
      -3
      --> (quotient -10 -3)
      3


.. _proc:remainder:

remainder
^^^^^^^^^

.. function:: (remainder n1 n2)

    Returns the remainder of dividing *n1* by *n2*, with the result having the
    same sign as *n1* (the dividend). Both arguments must be integers.

    :param n1: The dividend.
    :type n1: integer
    :param n2: The divisor.
    :type n2: integer
    :return: The remainder of *n1* divided by *n2*, with the sign of *n1*.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (remainder 10 3)
      1
      --> (remainder -10 3)
      -1
      --> (remainder 10 -3)
      1
      --> (remainder -10 -3)
      -1

.. _proc:max:

max
^^^

.. function:: (max x1 x2 ...)

    Returns the largest of its arguments. All arguments must be real numbers;
    complex numbers are not permitted.

    :param x1: One or more real numbers to compare.
    :type x1: real
    :return: The largest argument.
    :rtype: real

    **Example:**

    .. code-block:: scheme

      --> (max 1 2 3)
      3
      --> (max -5 0 5)
      5
      --> (max 1/2 0.4 3/5)
      0.6
      --> (max 42)
      42


.. _proc:min:

min
^^^

.. function:: (min x1 x2 ...)

    Returns the smallest of its arguments. All arguments must be real numbers;
    complex numbers are not permitted.

    :param x1: One or more real numbers to compare.
    :type x1: real
    :return: The smallest argument.
    :rtype: real

    **Example:**

    .. code-block:: scheme

      --> (min 1 2 3)
      1
      --> (min -5 0 5)
      -5
      --> (min 1/2 0.4 3/5)
      0.4
      --> (min 42)
      42


.. _proc:floor:

floor
^^^^^

.. function:: (floor x)

    Returns the largest integer not larger than *x*. If *x* is an exact integer
    or bignum, it is returned unchanged.

    :param x: A real number.
    :type x: real
    :return: The largest integer not larger than *x*.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (floor 3.7)
      3
      --> (floor -3.7)
      -4
      --> (floor 7/2)
      3
      --> (floor 4)
      4


.. _proc:ceiling:

ceiling
^^^^^^^

.. function:: (ceiling x)

    Returns the smallest integer not smaller than *x*. If *x* is an exact
    integer or bignum, it is returned unchanged.

    :param x: A real number.
    :type x: real
    :return: The smallest integer not smaller than *x*.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (ceiling 3.2)
      4
      --> (ceiling -3.2)
      -3
      --> (ceiling 7/2)
      4
      --> (ceiling 4)
      4


.. _proc:round:

round
^^^^^

.. function:: (round x)

    Returns the closest integer to *x*, rounding halfway values to the nearest
    even integer. If *x* is an exact integer or bignum, it is returned
    unchanged.

    :param x: A real number.
    :type x: real
    :return: The closest integer to *x*.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (round 3.2)
      3
      --> (round 3.7)
      4
      --> (round 3.5)
      4
      --> (round 4.5)
      4
      --> (round -3.5)
      -4


.. _proc:truncate:

truncate
^^^^^^^^

.. function:: (truncate x)

    Returns the integer closest to *x* whose absolute value is not larger than
    that of *x*; i.e., rounds toward zero. If *x* is an exact integer or
    bignum, it is returned unchanged.

    :param x: A real number.
    :type x: real
    :return: *x* rounded toward zero.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (truncate 3.7)
      3
      --> (truncate -3.7)
      -3
      --> (truncate 7/2)
      3
      --> (truncate 4)
      4

.. _proc:numerator:

numerator
^^^^^^^^^

.. function:: (numerator q)

    Returns the numerator of *q*, computed as if *q* were represented as a
    fraction in lowest terms. If *q* is an exact integer or bignum, it is
    returned unchanged (as its own numerator over an implicit denominator of
    ``1``).

    :param q: A rational number or exact integer.
    :type q: rational or integer
    :return: The numerator of *q* in lowest terms.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (numerator 3/7)
      3
      --> (numerator 6/4)
      3
      --> (numerator 5)
      5


.. _proc:denominator:

denominator
^^^^^^^^^^^

.. function:: (denominator q)

    Returns the denominator of *q*, computed as if *q* were represented as a
    fraction in lowest terms. If *q* is an exact integer, returns ``1``.

    :param q: A rational number or exact integer.
    :type q: rational or integer
    :return: The denominator of *q* in lowest terms.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (denominator 3/7)
      7
      --> (denominator 6/4)
      2
      --> (denominator 5)
      1


.. _proc:rationalize:

rationalize
^^^^^^^^^^^

.. function:: (rationalize x y)

    Returns the simplest rational number that differs from *x* by no more than
    *y*. A rational *p/q* is considered simpler than *p2/q2* if both
    ``|p| ≤ |p2|`` and ``|q| ≤ |q2|`` when both fractions are in lowest terms.
    Note that ``0`` is the simplest rational of all.

    The search is limited to denominators up to ``10^16``. If no suitably simple
    rational is found within that range, a best-effort rational approximation of
    *x* is returned based on the full precision of the floating-point
    representation.

    :param x: The target value.
    :type x: real
    :param y: The maximum permitted difference from *x*.
    :type y: real
    :return: The simplest rational number within *y* of *x*.
    :rtype: rational

    **Example:**

    .. code-block:: scheme

      --> (rationalize 0.1 1/10)
      0
      --> (rationalize 1/3 1/10)
      1/3
      --> (rationalize 3.14159 1/100)
      22/7
      --> (rationalize 0.1 0)
      3602879701896397/36028797018963968


.. _proc:square:

square
^^^^^^

.. function:: (square z)

    Returns the square of *z*. Equivalent to ``(* z z)``.

    :param z: A number to square.
    :type z: number
    :return: The square of *z*.
    :rtype: number

    **Example:**

    .. code-block:: scheme

      --> (square 5)
      25
      --> (square -3)
      9
      --> (square 1/3)
      1/9
      --> (square 2.0)
      4.0
      --> (square 2+3i)
      -5+12i


.. _proc:sqrt:

sqrt
^^^^

.. function:: (sqrt z)

    Returns the principal square root of *z*. The result will have either a
    positive real part, or a zero real part and a non-negative imaginary part.
    If *z* is an exact non-negative integer whose square root is itself an exact
    integer, the result is returned as an exact integer. If *z* is a negative
    real number, the result is a complex number with zero real part.

    :param z: A number to take the square root of.
    :type z: number
    :return: The principal square root of *z*.
    :rtype: number

    **Example:**

    .. code-block:: scheme

      --> (sqrt 4)
      2
      --> (sqrt 2)
      1.4142135623730951
      --> (sqrt -1)
      0+1.0i
      --> (sqrt -4)
      0+2.0i
      --> (sqrt 1+2i)
      1.272+0.786i


.. _proc:exact-integer-sqrt:

exact-integer-sqrt
^^^^^^^^^^^^^^^^^^

.. function:: (exact-integer-sqrt k)

    Returns a list of two exact non-negative integers ``(s r)`` such that
    ``k = s² + r``, where *s* is the largest integer for which ``s² ≤ k``.
    *k* must be a positive exact integer.

    :param k: A positive exact integer.
    :type k: integer
    :return: A list of two exact integers ``(s r)`` where ``s`` is the integer
             square root of *k* and ``r`` is the remainder.
    :rtype: list

    **Example:**

    .. code-block:: scheme

      --> (exact-integer-sqrt 14)
      (3 5)
      --> (exact-integer-sqrt 16)
      (4 0)
      --> (exact-integer-sqrt 2)
      (1 1)

.. _proc:exact:

exact
^^^^^

.. function:: (exact z)

    Returns an exact representation of *z*. If *z* is already exact, it is
    returned unchanged. For inexact real numbers, the exact flag is set on the
    value. For complex numbers, the exact flag is applied to both the real and
    imaginary parts individually.

    :param z: A number to convert to an exact representation.
    :type z: number
    :return: An exact representation of *z*.
    :rtype: number

    **Example:**

    .. code-block:: scheme

      --> (exact 1)
      1
      --> (exact 1.5)
      1.5
      --> (exact 1/3)
      1/3


.. _proc:inexact:

inexact
^^^^^^^

.. function:: (inexact z)

    Returns an inexact representation of *z*. If *z* is already inexact, it is
    returned unchanged. For exact numbers, the value is promoted to an inexact
    real via numeric promotion. For complex numbers, the exact flag is cleared
    on both the real and imaginary parts individually.

    :param z: A number to convert to an inexact representation.
    :type z: number
    :return: An inexact representation of *z*.
    :rtype: number

    **Example:**

    .. code-block:: scheme

      --> (inexact 1)
      1.0
      --> (inexact 1/3)
      0.3333333333333333
      --> (inexact 1.5)
      1.5


.. _proc:infinite?:


infinite?
^^^^^^^^^

.. function:: (infinite? z)

    Returns ``#t`` if *z* is a real infinite value (``+inf.0`` or ``-inf.0``),
    or a complex number whose real or imaginary part (or both) is infinite.
    Returns ``#f`` for all other numbers, including exact integers, rationals,
    and bignums.

    :param z: The number to test.
    :type z: number
    :return: ``#t`` if *z* is infinite, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (infinite? +inf.0)
      #t
      --> (infinite? -inf.0)
      #t
      --> (infinite? 1.0)
      #f
      --> (infinite? 42)
      #f


.. _proc:finite?:

finite?
^^^^^^^

.. function:: (finite? z)

    Returns ``#t`` if *z* is a finite number. Returns ``#f`` for ``+inf.0``,
    ``-inf.0``, and ``+nan.0``. For complex numbers, returns ``#t`` only if
    both the real and imaginary parts are finite. Exact integers, rationals,
    and bignums are always considered finite.

    :param z: The number to test.
    :type z: number
    :return: ``#t`` if *z* is finite, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (finite? 42)
      #t
      --> (finite? 1.5)
      #t
      --> (finite? +inf.0)
      #f
      --> (finite? +nan.0)
      #f


.. _proc:nan?:

nan?
^^^^

.. function:: (nan? z)

    Returns ``#t`` if *z* is ``+nan.0``, or a complex number whose real or
    imaginary part (or both) is ``+nan.0``. Returns ``#f`` for all other
    numbers, including exact integers, rationals, and bignums.

    :param z: The number to test.
    :type z: number
    :return: ``#t`` if *z* is ``+nan.0``, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (nan? +nan.0)
      #t
      --> (nan? 1.0)
      #f
      --> (nan? 42)
      #f

.. _proc:gcd:

gcd
^^^

.. function:: (gcd n ...)

    Returns the greatest common divisor of its arguments. The result is always
    non-negative. If no arguments are provided, returns ``0``, which is the
    identity value for GCD. If a single argument is provided, its absolute
    value is returned.

    :param n: Zero or more exact integers.
    :type n: integer
    :return: The greatest common divisor of all arguments, or ``0`` if no
             arguments are given.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (gcd)
      0
      --> (gcd 12)
      12
      --> (gcd 12 8)
      4
      --> (gcd 32 -36)
      4
      --> (gcd 12 8 6)
      2


.. _proc:lcm:

lcm
^^^

.. function:: (lcm n ...)

    Returns the least common multiple of its arguments. The result is always
    non-negative. If no arguments are provided, returns ``1``, which is the
    identity value for LCM. If a single argument is provided, its absolute
    value is returned.

    :param n: Zero or more exact integers.
    :type n: integer
    :return: The least common multiple of all arguments, or ``1`` if no
             arguments are given.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (lcm)
      1
      --> (lcm 6)
      6
      --> (lcm 4 6)
      12
      --> (lcm -4 6)
      12
      --> (lcm 4 6 10)
      60

