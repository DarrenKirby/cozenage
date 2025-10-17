Numerics
========

Overview
--------

It is important to distinguish between mathematical numbers, the Scheme numbers that attempt to
model them, the machine representations used to implement the Scheme numbers, and notations used to
write numbers.

Cozenage implements four distinct object types to represent mathematical values:

- Integers
- Rationals
- Reals
- Complex

Mathematically, numbers are arranged into a tower of subtypes in which each level is a subset of the
level above it:

- number
- complex number
- real number
- rational number
- integer

For example, 3 is an integer. Therefore 3 is also a rational, a real, and a complex number. The same
is true of the Scheme numbers that model 3. For Scheme numbers, these types are defined by the
predicates ``number?``, ``complex?``, ``real?``, ``rational?``, and ``integer?``.

There is no simple relationship between a number’s type and its representation inside a computer.
Although Cozenage offers four different representations of 3:

.. code-block:: scheme

    3    ; integer
    3/1  ; rational
    3.0  ; real
    3+0i ; complex

these different representations denote the same integer. Cozenage’s numerical operations treat
numbers as abstract data, as independent of their representation as possible. This means, with a few
exceptions that will be noted, numeric procedures will work on any of the above representations, and
any combination thereof.
