Cozenage and Scheme libraries
=============================

The Scheme R7RS specification provides additional libraries beyond ``scheme base``, which is loaded
into the environment by default. These libraries are factored so as to separate features which might
not be supported by all implementations, or which might be expensive to load.

The ``scheme`` library prefix is used for all standard libraries, and is reserved for use by future
standards.

Cozenage also supplies libraries beyond the R7RS-specified implementations, all of which use the
``cozenage`` library prefix.

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
