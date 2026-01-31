.. image:: images/raven.jpg
    :alt: A jet black raven on a white field.


Cozenage
========

**Cozenage** is a Scheme-derived, Lisp-like programming language written in C, designed primarily as an educational project and exploratory implementation. It provides a small, expressive core language and is extended through a growing standard library and dynamically loadable modules.

While Cozenage borrows heavily from Scheme—particularly R5RS and R7RS—it is **not intended to be fully standard-compliant**. Where possible, Cozenage follows Scheme semantics and conventions, but it intentionally deviates from the standards in a number of non-trivial ways. These deviations are the result of pragmatic design choices, implementation constraints, or deliberate simplifications, and are documented where relevant.

Anyone with experience in Scheme or other Lisp dialects should find Cozenage immediately familiar.

Design goals
------------

- **Clarity over completeness**
  Prefer understandable, inspectable implementations over strict standards compliance.

- **A small, coherent core**
  Keep the core language modest and expressive, with additional functionality layered on via libraries and modules.

- **Exploration and learning**
  Cozenage exists as a vehicle for learning about language implementation, interpreter design, and runtime systems.

- **Practical Lisp semantics**
  Favor straightforward, predictable behavior over obscure or highly abstract features.

Language features
-----------------

Core object types
^^^^^^^^^^^^^^^^^

Cozenage provides the usual set of disjoint primitive objects common to Scheme-like languages:

- number
- string
- character
- symbol
- pair
- null
- vector
- boolean
- port
- procedure

Numeric system
^^^^^^^^^^^^^^

- Full numeric tower:

  - integer
  - rational
  - real
  - complex

- Exact and inexact numbers
- Native big integer support
- Work in progress: arbitrary-precision rationals and reals

Bytevectors
^^^^^^^^^^^

Native bytevector types backed by C arrays:

- u8, s8
- u16, s16
- u32, s32
- u64, s64

Ports and I/O
^^^^^^^^^^^^^

- File-backed text and binary ports
- Memory-backed string and bytevector (u8) ports
- UTF-8 Unicode support

Evaluation model
^^^^^^^^^^^^^^^^

- Proper tail-call optimization
- ``eval`` / ``apply``
- Lisp-style ``defmacro`` non-hygienic macros
- Delayed evaluation and streams (via the ``lazy`` library)

Runtime and tooling
^^^^^^^^^^^^^^^^^^^

- Garbage collection (Boehm GC)
- Interactive REPL with:

  - multi-line editing
  - persistent history
  - tab completion

Polymorphic procedures
^^^^^^^^^^^^^^^^^^^^^^

- ``len``
- ``idx``
- ``rev``

Intentional omissions
---------------------

- **First-class continuations (``call/cc``)**
  These are unlikely to be implemented. Control-flow patterns typically expressed using continuations may instead be provided as primitive syntax or built-in forms.

- **Hygienic macros**
  While desirable, hygienic macros are complex and currently low priority. Cozenage instead provides Lisp-style, non-hygienic macros.

Features in progress
--------------------

- Arbitrary-precision rationals and reals
  (big integers are already supported)

Planned and future work
-----------------------

- Custom line editor (replace readline/libedit)
- Custom allocator / garbage collector
- Expanded bytevector library
- Native ``set`` and ``map`` types
- Exception handling (``try`` / ``catch``)
- Asynchronous ports (sockets)
- URL-backed ports
- User-defined library loading, both 'Scheme' libraries, and libraries written in C (ie: FFI).

Philosophy
----------

Cozenage is best understood as **a Scheme-inspired Lisp**, prioritizing internal consistency, approachability, and educational value over exhaustive standards compliance.

.. attention::

   This project is under active development. The documentation and the code itself changes
   every day. Be sure to pull from Github and build often to get the latest version.


Documentation
-------------

This documentation is organized into three main types. *Howtos* are goal-oriented documentation.
They have specific goals, and demonstrate how to achieve that goal. *Tutorials* are
knowledge-oriented documentation. They don't have specific goals other than to introduce and
demonstrate various concepts to work towards building general knowledge. The *Reference* is a
single document which exhaustively details all the types, procedures, syntax, special forms,
libraries, and even the underlying C internals of Cozenage. If you know what you are looking for, see
if there is a Howto on the topic, else search for the topic in the reference. If you have no
specific goal, and just want to learn more about Scheme and/or Cozenage, try a tutorial.


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   howto/index
   tutorial/index
   reference/index

