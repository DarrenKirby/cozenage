Scheme Complex Library
======================

Overview
--------

The Cozenage complex number library provides a comprehensive set of procedures for creating and manipulating complex numbers. Complex numbers are a fundamental mathematical tool with wide-ranging applications in engineering, physics, signal processing, and graphics.

Cozenage supports two common representations of complex numbers:

1.  **Rectangular Form:** Represented as ``a+bi``, where `a` is the **real part** and `b` is the **imaginary part**. This form is intuitive for addition and subtraction.
2.  **Polar Form:** Represented as ``r@θ``, where `r` is the **magnitude** (or modulus) and `θ` is the **angle** (or argument). This form simplifies multiplication and division.

The library provides procedures to create complex numbers from both representations and to extract their component parts. All complex number objects are of the type ``complex``.

Complex Number Procedures
-------------------------

.. _proc:make-rectangular:

.. function:: (make-rectangular x1 x2)

   Returns a complex number constructed from the real part *x1* and the imaginary part *x2*.

   :param x1: The real part.
   :type x1: real
   :param x2: The imaginary part.
   :type x2: real
   :return: The corresponding complex number.
   :rtype: complex

   **Example:**

   .. code-block:: scheme

      --> (make-rectangular 3 4)
        3+4i
      --> (make-rectangular -1.5 0)
        -1.5+0i

.. _proc:real-part:

.. function:: (real-part z)

   Returns the real part of the complex number *z*.

   :param z: The complex number to inspect.
   :type z: complex
   :return: The real part of the number.
   :rtype: real

   **Example:**

   .. code-block:: scheme

      --> (real-part 3+4i)
        3
      --> (real-part -1.2-5.7i)
        -1.2

.. _proc:imag-part:

.. function:: (imag-part z)

   Returns the imaginary part of the complex number *z*.

   :param z: The complex number to inspect.
   :type z: complex
   :return: The imaginary part of the number.
   :rtype: real

   **Example:**

   .. code-block:: scheme

      --> (imag-part 3+4i)
        4
      --> (imag-part -1.2-5.7i)
        -5.7

.. _proc:make-polar:

.. function:: (make-polar r theta)

   Returns a complex number constructed from the magnitude *r* and the angle *theta*.

   :param r: The magnitude (radius).
   :type r: real
   :param theta: The angle (in radians).
   :type theta: real
   :return: The corresponding complex number.
   :rtype: complex

   **Example:**

   .. code-block:: scheme

      --> (make-polar 5 0.927)
        3.00043+3.99974i
      --> (make-polar 1 (atan 0 -1)) ; pi
        -1+1.22465e-16i

.. _proc:magnitude:

.. function:: (magnitude z)

   Returns the magnitude (or modulus) of the complex number *z*. This is the distance from the origin (0,0) to the point (real-part, imag-part) in the complex plane.

   :param z: The complex number to inspect.
   :type z: complex
   :return: The magnitude of the number.
   :rtype: real

   **Example:**

   .. code-block:: scheme

      --> (magnitude 3+4i)
        5.0
      --> (magnitude (make-polar 10 2))
        10.0

.. _proc:angle:

.. function:: (angle z)

   Returns the angle (or argument) of the complex number *z* in radians.

   :param z: The complex number to inspect.
   :type z: complex
   :return: The angle of the number in radians.
   :rtype: real

   **Example:**

   .. code-block:: scheme

      --> (angle 3+4i)
        0.927295
      --> (angle (make-polar 5 1.5))
        1.5