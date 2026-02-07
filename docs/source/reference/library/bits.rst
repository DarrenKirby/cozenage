Base Bits Library
=====================

The `bits` library implements bitwise operations on integers,
and also interprets a `bitstring` pseudo-type, which is string of ones and zeros implemented
as regular Scheme symbols prefaced by a lowercase 'b' character (so that the
parser doesn't interpret them as regular numeric digits). For example:


.. code-block:: scheme

    --> (import (base bits))
    --> (bitstring->int 'b1001001001)
    -439
    --> (bitstring->int 'b01001001001
    585
    --> (int->bitstring 10)
    b01010
    --> (int->bitstring -120)
    b10001000
    --> (bs+ 'b01010 'b01010)
    b010100

Notice that bitstrings are interpreted as twos-compliment values, so positive values
must have a leading `0`, and negative values must have a leading `1`.

Bitwise operations on integers behave as expected similar to most other languages:


.. code-block:: scheme

    --> (import (base bits))
    --> (>> 1000 2)
    250
    --> (<< 1000 2)
    4000
    --> (band 5 3)
    1
    --> (band 3 2)
    2
    --> (bor 5 3)
    7
    --> (bor 2 8)
    10
    --> (bxor 5 3)
    6
    --> (bxor 2 10)
    8
    --> (bnot 7)
    -8
    --> (> (band 10 1) 0)  ; test for oddness
    #false
    --> (> (band 11 1) 0)
    #true
    --> (= (band 11 1) 0)  ; test for evenness
    #false
    --> (= (band 10 1) 0)
    #true

Operands to these procedures may also be supplied as bitstrings. If either of the operands are supplied as a bitstring,
the result will be a bitstring:


.. code-block:: scheme

    --> (import (base bits))
    --> (<< 'b010 2)
    b01000
    --> (<< 2 'b01010)
    b0100000000000
    --> (band 'b01010 'b0101)
    b0
    --> (bor 'b01010 'b0101)
    b01111
    --> (bnot 'b01010)
    b10101

There are also four procedures exported for performing basic arithmetic on bitstrings:

.. code-block:: scheme

    --> (import (base bits))
    --> (bs+ 'b01010 'b01010)
    b010100
    --> (bs- 'b01010 'b010)
    b01000
    --> (bs* 'b01010 'b010)
    b010100
    --> (bs/ 'b01010 'b010)
    b0101


.. note::

   The overhead required for conversions make using bitstrings slower in all practical cases. Therefore,
   practical use-cases should avoid them. They are implemented for purely pedagogical reasons.


Procedures exported by bits
---------------------------

.. _proc:right-shift:

.. function:: (>> n1 n2)

   Returns the value of n1 right-shifted by n2.

   :param n1: The value to right-shift.
   :type n1: integer or bitstring.
   :param n2: The amount to right-shift by.
   :type n2: integer or bitstring.
   :return: The shifted value.
   :rtype: integer, if n1 AND n2 are integers, otherwise, a bitstring.


.. _proc:left-shift:

.. function:: (<< n1 n2)

   Returns the value of n1 left-shifted by n2.

   :param n1: The value to left-shift.
   :type n1: integer or bitstring.
   :param n2: The amount to left-shift by.
   :type n2: integer or bitstring.
   :return: The shifted value.
   :rtype: integer, if n1 AND n2 are integers, otherwise, a bitstring.

.. _proc:band:

.. function:: (band n1 n2)

    Returns the bitwise AND of the two values.

    :param n1: The first value to compare.
    :type n1: integer or bitstring.
    :param n2: The second value to compare.
    :type n2: integer or bitstring.
    :return: The bitwise AND result.
    :rtype: integer, if n1 AND n2 are integers, otherwise, a bitstring.

.. _proc:bor:

.. function:: (bor n1 n2)

    Returns the bitwise OR of the two values.

    :param n1: The first value to compare.
    :type n1: integer or bitstring.
    :param n2: The second value to compare.
    :type n2: integer or bitstring.
    :return: The bitwise OR result.
    :rtype: integer, if n1 AND n2 are integers, otherwise, a bitstring.

.. _proc:bxor:

.. function:: (bxor n1 n2)

    Returns the bitwise XOR (exclusive OR) of the two values.

    :param n1: The first value to compare.
    :type n1: integer or bitstring.
    :param n2: The second value to compare.
    :type n2: integer or bitstring.
    :return: The bitwise XOR result.
    :rtype: integer, if n1 AND n2 are integers, otherwise, a bitstring.

.. _proc:bnot:

.. function:: (bnot n)

    Returns the bitwise NOT of the value.

    :param n: The value to invert.
    :type n: integer or bitstring.
    :return: The bitwise NOT result.
    :rtype: integer, if n is an integer, otherwise, a bitstring.

.. _proc:bs-plus:

.. function:: (bs+ bitstring1 bitstring2)

    Returns the sum of the two arguments as a bitstring.

    :param bitstring1: The first addend.
    :type bitstring1: bitstring.
    :param bitstring2: The second addend.
    :type bitstring2: bitstring.
    :return: The sum as a bitstring.
    :rtype: bitstring.

.. _proc:bs-minus:

.. function:: (bs- bitstring1 bitstring2)

    Returns the difference of the two arguments as a bitstring.

    :param bitstring1: The value to subtract from.
    :type bitstring1: bitstring.
    :param bitstring2: The value to subtract.
    :type bitstring2: bitstring.
    :return: The difference as a bitstring.
    :rtype: bitstring.

.. _proc:bs-mul:

.. function:: (bs* bitstring1 bitstring2)

    Returns the product of the two arguments as a bitstring.

    :param bitstring1: The first factor.
    :type bitstring1: bitstring.
    :param bitstring2: The second factor.
    :type bitstring2: bitstring.
    :return: The product as a bitstring.
    :rtype: bitstring.

.. _proc:bs-div:

.. function:: (bs/ bitstring1 bitstring2)

    Returns the integer quotient of the two arguments as a bitstring.

    :param bitstring1: The dividend.
    :type bitstring1: bitstring.
    :param bitstring2: The divisor.
    :type bitstring2: bitstring.
    :return: The quotient as a bitstring.
    :rtype: bitstring.

.. _proc:int-to-bitstring:

.. function:: (int->bitstring n)

    Returns n represented as a two's-complement bitstring.

    :param n: The integer to convert.
    :type n: integer.
    :return: The bitstring representation.
    :rtype: bitstring.

.. _proc:bitstring-to-int:

.. function:: (bitstring->int bitstring)

    Returns the value represented by bitstring as an integer.

    :param bitstring: The bitstring to convert.
    :type bitstring: bitstring.
    :return: The integer value.
    :rtype: integer.

