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

