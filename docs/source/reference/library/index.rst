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

Scheme R7RS Libraries
---------------------

- ``case-lambda`` - The ``(scheme case-lambda)`` library exports the case-lambda syntax.
- ``char`` - The ``(scheme char)`` library provides the procedures for dealing with characters that
involve potentially large tables when supporting all of Unicode.
- ``complex`` - The (scheme complex) library exports procedures which are typically only useful with
non-real numbers.
- ``cxr`` - The ``(scheme cxr)`` library exports twenty-four procedures which are the compositions of
from three to four car and cdr operations. For example caddar could be defined by:

    .. code-block:: scheme

        (define caddar
            (lambda (x) (car (cdr (cdr (car x))))))

The procedures car and cdr themselves and the four two-level compositions are included in
``scheme base``, and are exported by default.

- ``eval`` - The ``(scheme eval)`` library exports procedures for evaluating Scheme data as programs.
- ``file`` - The ``(scheme file)`` library provides procedures for accessing files.
- ``inexact`` - The ``(scheme inexact)`` library exports procedures which are typically only useful with
inexact values.

Cozenage Libraries
------------------

``bits`` - The ``(cozenage bits)`` library exports procedures which perform bitwise operations on
integers, and provides a mechanism for converting integers to and from bitstrings. A bitstring is
a symbol which represents a binary integer as a string of ones and zeros.
