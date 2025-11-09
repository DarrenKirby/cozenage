Scheme Inexact Library
======================

Overview
--------

This library provides a standard set of procedures for working with inexact (floating-point) numbers. These procedures are essential for scientific and engineering applications that require trigonometry, logarithms, and exponentiation.

Cozenage, following the R7RS standard, represents special floating-point values such as positive infinity, negative infinity, and Not a Number as ``+inf.0``, ``-inf.0``, and ``+nan.0`` respectively. This library includes predicates to test for these specific values.

Inexact Procedures
------------------

.. _proc:acos:

.. function:: (acos z)

   Returns the arc cosine of *z* in radians.

   :param z: A number between -1 and 1, inclusive.
   :type z: number
   :return: The arc cosine in radians.
   :rtype: inexact

   **Example:**

   .. code-block:: scheme

      --> (acos -1)
        3.141592653589793
      --> (acos 0.5)
        1.0471975511965979

.. _proc:asin:

.. function:: (asin z)

   Returns the arc sine of *z* in radians.

   :param z: A number between -1 and 1, inclusive.
   :type z: number
   :return: The arc sine in radians.
   :rtype: inexact

   **Example:**

   .. code-block:: scheme

      --> (asin 1)
        1.5707963267948966
      --> (asin 0)
        0.0

.. _proc:atan:

.. function:: (atan y)
.. function:: (atan y x)

   Returns the arc tangent of *y/x* in radians. If only one argument *y* is given, it returns the arc tangent of *y*.

   :param y: The numerator.
   :type y: real
   :param x: The denominator.
   :type x: real
   :return: The arc tangent in radians.
   :rtype: inexact

   **Example:**

   .. code-block:: scheme

      --> (atan 1)
        0.7853981633974483
      --> (atan -1 0)
        -1.5707963267948966

.. _proc:cos:

.. function:: (cos z)

   Returns the cosine of *z*, where *z* is in radians.

   :param z: An angle in radians.
   :type z: number
   :return: The cosine of the angle.
   :rtype: inexact

   **Example:**

   .. code-block:: scheme

      --> (cos 3.1415926535)
        -1.0
      --> (cos 0)
        1.0

.. _proc:exp:

.. function:: (exp z)

   Returns *e* raised to the power of *z*, where *e* is the base of the natural logarithms.

   :param z: The exponent.
   :type z: number
   :return: e raised to the power of z.
   :rtype: inexact

   **Example:**

   .. code-block:: scheme

      --> (exp 1)
        2.718281828459045
      --> (exp 0)
        1.0

.. _proc:finite?:

.. function:: (finite? z)

   Returns ``#true`` if *z* is a finite number, and ``#false`` otherwise. A number is finite if it is not positive or negative infinity and not NaN.

   :param z: The number to test.
   :type z: number
   :return: #true or #false.
   :rtype: boolean

   **Example:**

   .. code-block:: scheme

      --> (finite? 123.45)
        #true
      --> (finite? +inf.0)
        #false
      --> (finite? +nan.0)
        #false

.. _proc:infinite?:

.. function:: (infinite? z)

   Returns ``#true`` if *z* is positive or negative infinity, and ``#false`` otherwise.

   :param z: The number to test.
   :type z: number
   :return: #true or #false.
   :rtype: boolean

   **Example:**

   .. code-block:: scheme

      --> (infinite? (/ 1.0 0.0))
        #true
      --> (infinite? -inf.0)
        #true
      --> (infinite? 1000)
        #false

.. _proc:log:

.. function:: (log z)
.. function:: (log z b)

   Returns the natural logarithm of *z*. If the optional base *b* is provided, it returns the logarithm of *z* to the base *b*.

   :param z: The number.
   :type z: number
   :param b: The base (optional).
   :type b: number
   :return: The logarithm of z.
   :rtype: inexact

   **Example:**

   .. code-block:: scheme

      --> (log (exp 1))
        1.0
      --> (log 100 10)
        2.0

.. _proc:nan?:

.. function:: (nan? z)

   Returns ``#true`` if *z* is Not a Number (NaN), and ``#false`` otherwise.

   :param z: The number to test.
   :type z: number
   :return: #true or #false.
   :rtype: boolean

   **Example:**

   .. code-block:: scheme

      --> (nan? (/ 0.0 0.0))
        #true
      --> (nan? 123)
        #false

.. _proc:sin:

.. function:: (sin z)

   Returns the sine of *z*, where *z* is in radians.

   :param z: An angle in radians.
   :type z: number
   :return: The sine of the angle.
   :rtype: inexact

   **Example:**

   .. code-block:: scheme

      --> (sin 1.57079632679)
        1.0
      --> (sin 0)
        0.0

.. _proc:sqrt:

.. function:: (sqrt z)

   Returns the principal square root of *z*.

   :param z: The number.
   :type z: number
   :return: The square root of z.
   :rtype: number

   **Example:**

   .. code-block:: scheme

      --> (sqrt 16)
        4.0
      --> (sqrt 2)
        1.4142135623730951

.. _proc:tan:

.. function:: (tan z)

   Returns the tangent of *z*, where *z* is in radians.

   :param z: An angle in radians.
   :type z: number
   :return: The tangent of the angle.
   :rtype: inexact

   **Example:**

   .. code-block:: scheme

      --> (tan 0)
        0.0
      --> (tan 0.785398)
        0.9999996208688432

Cozenage Extensions
-------------------

The following procedures are not part of the R7RS standard but are included in Cozenage for convenience.

.. _proc:log2:

.. function:: (log2 z)

   Returns the base-2 logarithm of *z*.

   :param z: The number.
   :type z: number
   :return: The base-2 logarithm of z.
   :rtype: inexact

   **Example:**

   .. code-block:: scheme

      --> (log2 256)
        8.0

.. _proc:log10:

.. function:: (log10 z)

   Returns the base-10 logarithm of *z*.

   :param z: The number.
   :type z: number
   :return: The base-10 logarithm of z.
   :rtype: inexact

   **Example:**

   .. code-block:: scheme

      --> (log10 1000)
        3.0

.. _proc:cbrt:

.. function:: (cbrt z)

   Returns the cube root of *z*.

   :param z: The number.
   :type z: number
   :return: The cube root of z.
   :rtype: inexact

   **Example:**

   .. code-block:: scheme

      --> (cbrt 27)
        3.0
      --> (cbrt -64)
        -4.0