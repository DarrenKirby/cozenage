Base Bits Library
=====================

The `bits` library implements bitwise operations on integers,
and also interprets a `bitstring` pseudo-type, which is string of ones and zeros implemented
as regular Scheme symbols prefaced by a lowercase 'b' character (so that the
parser doesn't interpret them as regular numeric digits). For example:

.. code-block:: scheme

    --> (import (cozenage bits))
    --> (bitstring->int 'b1001001001)
    -439
    --> (bitstring->int 'b01001001001
    585
    --> (int->bitstring 10)
    b01010
    --> (int->bitstring -120)
    b10001000

Notice that bitstrings are interpreted as twos-compliment values, so positive values
must have a leading `0`, and negative values must have a leading `1`.
