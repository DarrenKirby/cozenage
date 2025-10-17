Bytevectors
===========

Overview
--------

Bytevectors represent blocks of binary data. They are fixed-length sequences of bytes, where a byte
is an exact integer in the range from 0 to 255 inclusive. A bytevector is typically more
space-efficient than a vector containing the same values.

The length of a bytevector is the number of elements that it contains. This number is a non-negative
integer that is fixed when the bytevector is created. The valid indexes of a bytevector are the
exact non-negative integers less than the length of the bytevector, starting at index zero as with
vectors.

Bytevectors are written using the notation ``#u8(byte ...)``. For example, a bytevector of length 3
containing the byte 0 in element 0, the byte 10 in element 1, and the byte 5 in element 2 can be
written as follows: ``#u8(0 10 5)``.

Bytevector constants are self-evaluating, so they do not need to be quoted in programs.
