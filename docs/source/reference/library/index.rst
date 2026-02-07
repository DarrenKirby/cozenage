Cozenage loadable module libraries
==================================

Cozenage supplies libraries beyond the core implementation, all of which use the
``base`` library prefix.

The procedures exported by these various libraries are documented in this section of the Cozenage
Reference document. They are:

Cozenage 'base' standard libraries
----------------------------------

- ``bits`` - The ``(base bits)`` library exports procedures which perform bitwise operations on
    integers, and provides a mechanism for converting integers to and from bitstrings. A bitstring is
    a symbol which represents a binary integer as a string of ones and zeros.
- ``cxr`` - The ``(base cxr)`` library exports twenty-four procedures which are the compositions of
    from three to four car and cdr operations. For example caddar could be defined by:

    .. code-block:: scheme

        (define caddar
            (lambda (x) (car (cdr (cdr (car x))))))

    The procedures ``car`` and ``cdr`` themselves and the four two-level compositions are included in
    the core interpreter and are exported by default.

- ``file`` - The ``(base file)`` library provides procedures for accessing and querying files and directories.
- ``math`` - The ``(base math)`` library exports specialized math procedures.
- ``random`` - The ``(base random)`` library exports procedures for generating random numbers, shuffling containers,
  and generating random samples from collections.
- ``system`` - The ``(base system)`` library exports procedures for interfacing with the local operating system.
- ``time`` - The ``(base time)`` library provides access to time-related values.


.. toctree::
   :maxdepth: 1
   :caption: Contents:

   bits
   cxr
   file
   math
   random
   system
   time
