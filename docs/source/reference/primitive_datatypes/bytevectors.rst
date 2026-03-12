Bytevectors
===========

Overview
--------

Bytevectors represent blocks of binary data as fixed-length sequences of typed
integer elements. Unlike vectors, which can hold any Scheme object, bytevectors
are specialised for compact, efficient storage of numeric data.

**Element Types**

This implementation extends the standard R7RS bytevector with support for
signed and unsigned integer elements of multiple widths, following the
conventions of SRFI-4. Each bytevector has a fixed element type, chosen at
construction time, which determines both the width of each element and whether
its values are interpreted as signed or unsigned. The supported types are:

.. list-table::
   :header-rows: 1
   :widths: auto

   * - Symbol
     - C type
     - Minimum value
     - Maximum value
   * - ``'u8``
     - uint8_t
     - 0
     - 255
   * - ``'s8``
     - int8_t
     - -128
     - 127
   * - ``'u16``
     - uint16_t
     - 0
     - 65,535
   * - ``'s16``
     - int16_t
     - -32,768
     - 32,767
   * - ``'u32``
     - uint32_t
     - 0
     - 4,294,967,295
   * - ``'s32``
     - int32_t
     - -2,147,483,648
     - 2,147,483,647
   * - ``'u64``
     - uint64_t
     - 0
     - 18,446,744,073,709,551,615
   * - ``'s64``
     - int64_t
     - -9,223,372,036,854,775,808
     - 9,223,372,036,854,775,807


If no type is specified at construction, the type defaults to ``'u8``, which
is the standard R7RS bytevector type.

**Length**

The length of a bytevector is the number of *elements* it contains, regardless
of the underlying byte width of each element. For example, a ``'u16``
bytevector of length 3 contains 3 elements, each occupying 2 bytes of storage.
The length is a non-negative exact integer that is fixed at construction time
and cannot be changed. Valid indices are exact non-negative integers less than
the length, starting at index zero.

**Literal Notation**

Bytevectors are written using the notation ``#<type>(elem ...)``, where
``<type>`` indicates the element type. For example:

.. code-block:: scheme

    #u8(0 10 5)      ; A u8 bytevector of length 3
    #s8(-1 0 1)      ; An s8 bytevector of length 3
    #u16(1000 2000)  ; A u16 bytevector of length 2
    #s32(-100000 0)  ; An s32 bytevector of length 2

The standard R7RS ``#u8(...)`` notation is equivalent to the ``'u8`` type.
Bytevector constants are self-evaluating and do not need to be quoted in
programs.

**Type Compatibility**

A bytevector's element type is fixed for its lifetime. Procedures that operate
on two bytevectors simultaneously, such as ``bytevector-copy!`` and
``bytevector-append``, require both bytevectors to be of the same type and will
raise an error if they are not.

**Value Range Checking**

All procedures that write to a bytevector validate that the value being written
falls within the valid range for the bytevector's element type. An error is
raised if a value is out of range. For example, attempting to store ``-1`` in
a ``'u8`` bytevector, or ``256`` in a ``'u8`` bytevector, will raise an error.

**Space Efficiency**

Bytevectors are significantly more space-efficient than vectors containing the
same values, as each element occupies exactly the number of bytes required by
its type rather than a full heap-allocated Scheme object. A ``'u8`` bytevector
of length 1024 occupies 1024 bytes of storage, whereas a vector of 1024 small
integers would require 1024 object pointers plus the objects themselves.

Bytevector Procedures
---------------------

.. _proc:bytevector:

bytevector
**********

.. function:: (bytevector byte ... [type])

    Returns a newly allocated bytevector containing *byte* ... as its elements.
    The optional *type* argument is a symbol specifying the element type of the
    bytevector; it must be one of ``'u8``, ``'s8``, ``'u16``, ``'s16``,
    ``'u32``, ``'s32``, ``'u64``, or ``'s64``. If omitted, the type defaults
    to ``'u8``. Each *byte* must be an exact integer within the valid range for
    the specified type; an error is raised otherwise.

    :param byte: Zero or more exact integers, each within the valid range for
                 *type*.
    :type byte: integer
    :param type: The element type of the bytevector. Must be one of ``'u8``,
                 ``'s8``, ``'u16``, ``'s16``, ``'u32``, ``'s32``, ``'u64``,
                 or ``'s64``. Defaults to ``'u8``.
    :type type: symbol
    :return: A new bytevector of the specified type containing *byte* ...
    :rtype: bytevector

    **Example:**

    .. code-block:: scheme

      --> (bytevector 1 2 3)
      #u8(1 2 3)
      --> (bytevector 1 2 3 's8)
      #s8(1 2 3)
      --> (bytevector 1000 2000 3000 'u16)
      #u16(1000 2000 3000)
      --> (bytevector -1 -2 -3 's16)
      #s16(-1 -2 -3)


.. _proc:bytevector-length:

bytevector-length
*****************

.. function:: (bytevector-length bytevector)

    Returns the number of elements in *bytevector* as an exact integer. Note
    that for multi-byte types (e.g. ``'u16``, ``'s32``), this is the number of
    elements, not the number of underlying bytes in the backing storage.

    :param bytevector: The bytevector to measure.
    :type bytevector: bytevector
    :return: The number of elements in *bytevector*.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (bytevector-length (bytevector 1 2 3))
      3
      --> (bytevector-length (bytevector 1000 2000 3000 'u16))
      3
      --> (bytevector-length (bytevector))
      0

.. _proc:bytevector-ref:

bytevector-ref
**************

.. function:: (bytevector-ref bytevector k)

    Returns the element at index *k* in *bytevector*, where the first element
    is at index ``0``. The value is returned as an exact integer. Raises an
    error if *k* is out of range.

    :param bytevector: The bytevector to index into.
    :type bytevector: bytevector
    :param k: The zero-based index of the element to retrieve.
    :type k: integer
    :return: The element at index *k* as an exact integer.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (bytevector-ref (bytevector 1 2 3) 0)
      1
      --> (bytevector-ref (bytevector 1 2 3) 2)
      3
      --> (bytevector-ref (bytevector -1 -2 -3 's8) 0)
      -1


.. _proc:bytevector-set!:

bytevector-set!
***************

.. function:: (bytevector-set! bytevector k byte)

    Stores *byte* at index *k* of *bytevector*, mutating it in place. *byte*
    must be an exact integer within the valid range for the type of
    *bytevector*. Raises an error if *k* is negative or out of range, or if
    *byte* is out of range for the bytevector's type. Returns an unspecified
    value.

    :param bytevector: The bytevector to mutate.
    :type bytevector: bytevector
    :param k: The zero-based index at which to store *byte*.
    :type k: integer
    :param byte: The value to store.
    :type byte: integer
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

      --> (define bv (bytevector 1 2 3))
      --> (bytevector-set! bv 1 99)
      --> bv
      #u8(1 99 3)
      --> (define bv16 (bytevector 100 200 300 'u16))
      --> (bytevector-set! bv16 0 1000)
      --> bv16
      #u16(1000 200 300)


.. _proc:make-bytevector:

make-bytevector
***************

.. function:: (make-bytevector k [byte [type]])

    Returns a newly allocated bytevector of *k* elements. If *byte* is given,
    every element is initialised to *byte*; otherwise every element is
    initialised to ``0``. The optional *type* argument is a symbol specifying
    the element type; it must be one of ``'u8``, ``'s8``, ``'u16``, ``'s16``,
    ``'u32``, ``'s32``, ``'u64``, or ``'s64``. If omitted, the type defaults
    to ``'u8``. *byte* must be within the valid range for *type*.

    .. note::

        Because *type* is the third argument, creating an empty bytevector of
        a non-default type requires a dummy fill value of ``0`` as the second
        argument, e.g. ``(make-bytevector 0 0 'u16)``.

    :param k: The number of elements in the new bytevector.
    :type k: integer
    :param byte: The value to initialise each element to. Defaults to ``0``.
    :type byte: integer
    :param type: The element type of the bytevector. Defaults to ``'u8``.
    :type type: symbol
    :return: A new bytevector of *k* elements.
    :rtype: bytevector

    **Example:**

    .. code-block:: scheme

      --> (make-bytevector 4)
      #u8(0 0 0 0)
      --> (make-bytevector 4 7)
      #u8(7 7 7 7)
      --> (make-bytevector 4 0 's16)
      #s16(0 0 0 0)
      --> (make-bytevector 3 -1 's8)
      #s8(-1 -1 -1)
      --> (make-bytevector 0)
      #u8()
      --> (make-bytevector 0 0 'u16)
      #u16()


.. _proc:bytevector-copy:

bytevector-copy
***************

.. function:: (bytevector-copy bytevector [start [end]])

    Returns a newly allocated bytevector containing the elements of *bytevector*
    between *start* (inclusive) and *end* (exclusive). The new bytevector has
    the same type as *bytevector*. *start* defaults to ``0`` and *end* defaults
    to the length of *bytevector*.

    :param bytevector: The bytevector to copy.
    :type bytevector: bytevector
    :param start: The index of the first element to include. Defaults to ``0``.
    :type start: integer
    :param end: The index past the last element to include. Defaults to the
                length of *bytevector*.
    :type end: integer
    :return: A new bytevector containing the specified elements.
    :rtype: bytevector

    **Example:**

    .. code-block:: scheme

      --> (bytevector-copy (bytevector 1 2 3 4 5))
      #u8(1 2 3 4 5)
      --> (bytevector-copy (bytevector 1 2 3 4 5) 2)
      #u8(3 4 5)
      --> (bytevector-copy (bytevector 1 2 3 4 5) 1 3)
      #u8(2 3)
      --> (bytevector-copy (bytevector 100 200 300 'u16) 1 3)
      #u16(200 300)


.. _proc:bytevector-copy!:

bytevector-copy!
****************

.. function:: (bytevector-copy! to at from [start [end]])

    Copies the elements of bytevector *from* between *start* (inclusive) and
    *end* (exclusive) into bytevector *to*, starting at index *at*, mutating
    *to* in place. *start* defaults to ``0`` and *end* defaults to the length
    of *from*. If the source and destination overlap, the copy is performed
    correctly as if via a temporary intermediate bytevector. *to* and *from*
    must be of the same bytevector type. Returns an unspecified value.

    :param to: The destination bytevector.
    :type to: bytevector
    :param at: The index in *to* at which to begin writing.
    :type at: integer
    :param from: The source bytevector.
    :type from: bytevector
    :param start: The index of the first element in *from* to copy. Defaults
                  to ``0``.
    :type start: integer
    :param end: The index past the last element in *from* to copy. Defaults to
                the length of *from*.
    :type end: integer
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

      --> (define bv (bytevector 1 2 3 4 5))
      --> (bytevector-copy! bv 1 (bytevector 10 20 30))
      #void
      --> bv
      #u8(1 10 20 30 5)
      --> (bytevector-copy! bv 0 (bytevector 10 20 30) 1 3)
      #void
      --> bv
      #u8(20 30 20 30 5)


.. _proc:bytevector-append:

bytevector-append
*****************

.. function:: (bytevector-append bytevector ...)

    Returns a newly allocated bytevector whose elements are the concatenation
    of the elements of all *bytevector* arguments, in order. All arguments must
    be bytevectors of the same type. If no arguments are provided, returns an
    empty ``u8`` bytevector.

    :param bytevector: Zero or more bytevectors of the same type to concatenate.
    :type bytevector: bytevector
    :return: A new bytevector containing all elements of all *bytevector*
             arguments.
    :rtype: bytevector

    **Example:**

    .. code-block:: scheme

      --> (bytevector-append (bytevector 1 2) (bytevector 3 4))
      #u8(1 2 3 4)
      --> (bytevector-append (bytevector 100 200 'u16) (bytevector 300 400 'u16))
      #u16(100 200 300 400)
      --> (bytevector-append)
      #u8()


.. _proc:utf8->string:

utf8->string
************

.. function:: (utf8->string bytevector [start [end]])

    Decodes the bytes of *bytevector* between *start* (inclusive) and *end*
    (exclusive) and returns the corresponding string. *bytevector* must be a
    ``u8`` bytevector. *start* and *end* are byte offsets. *start* defaults to
    ``0`` and *end* defaults to the length of *bytevector*.

    :param bytevector: A ``u8`` bytevector containing UTF-8 encoded bytes.
    :type bytevector: bytevector
    :param start: The byte offset of the first byte to decode. Defaults to
                  ``0``.
    :type start: integer
    :param end: The byte offset past the last byte to decode. Defaults to the
                length of *bytevector*.
    :type end: integer
    :return: A string decoded from the specified bytes.
    :rtype: string

    **Example:**

    .. code-block:: scheme

      --> (utf8->string (bytevector 104 101 108 108 111))
      "hello"
      --> (utf8->string (bytevector 104 101 108 108 111) 1)
      "ello"
      --> (utf8->string (bytevector 104 101 108 108 111) 1 3)
      "el"


.. _proc:string->utf8:

string->utf8
************

.. function:: (string->utf8 string [start [end]])

    Encodes the characters of *string* between *start* (inclusive) and *end*
    (exclusive) and returns the corresponding ``u8`` bytevector. *start* and
    *end* are byte offsets into the underlying UTF-8 representation of
    *string*. *start* defaults to ``0`` and *end* defaults to the byte length
    of *string*.

    :param string: The string to encode.
    :type string: string
    :param start: The byte offset of the first byte to encode. Defaults to
                  ``0``.
    :type start: integer
    :param end: The byte offset past the last byte to encode. Defaults to the
                byte length of *string*.
    :type end: integer
    :return: A ``u8`` bytevector containing the UTF-8 encoding of the specified
             portion of *string*.
    :rtype: bytevector

    **Example:**

    .. code-block:: scheme

      --> (string->utf8 "hello")
      #u8(104 101 108 108 111)
      --> (string->utf8 "hello" 1)
      #u8(101 108 108 111)
      --> (string->utf8 "hello" 1 3)
      #u8(101 108)

