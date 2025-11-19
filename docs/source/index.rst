.. image:: images/raven.jpg
    :alt: A jet black raven on a white field.


*Getting close to being ready for prime time*

About
-----

``Cozenage`` is a Scheme-derived, LISP-like language which provides a modest core of built-in
procedures and is extended by a growing standard library of additional functionality implemented
as 'hot-pluggable' dynamic modules. While Cozenage agrees with the formal R5RS/R7RS standards
inasmuch as possible, it is not intended to be fully-compliant, and deviates from the standards in
many non-trivial ways. These differences, and a justifications for them, will be documented as I find
the time. That said, anyone experienced with Scheme will find Cozenage quite familiar.

Cozenage has:

- The typical menu of disjoint primitive objects: ``number``, ``string``, ``char``, ``symbol``, ``pair``, ``null``, ``vector``, ``boolean``, ``port``, and ``procedure``.
- A selection of ``bytevector`` objects implemented using native C type arrays: ``u8``, ``s8``, ``u16``, ``s16``, ``u32``, and ``s32``.
- Full 'numeric tower' of derived types including ``integer``, ``rational``, ``real``, and ``complex`` numbers.
- Exact and inexact numbers.
- Proper tail-call optimization where applicable.
- Garbage collection.
- A REPL with multi-line input editing, command history, and tab-autocompletion.
- UTF-8 Unicode support.
- eval/apply procedures for interpreting data as executable code.

Typical Scheme features that Cozenage does NOT have:

- Arbitrary size and precision integers and real numbers. Currently, the magnitude of numbers is limited by the underlying C long long and long double types provided by the platform. While I do intend to implement 'bugnums' and 'bigfloats' at some point, it is somewhat low-priority currently.
- call/cc and first-class continuations. I'm not likely to ever implement this. Rather, I am more likely to implement control flow patterns typically implemented via call/cc as primitive syntax.
- Hygienic macros. I will likely implement this at some point, but it is low-priority.
- Quasiquotation. Currently, only standard quotation is supported.
- Delayed, or lazy evaluation. On the todo, but low-priority.

.. attention::

   This project is under active development. The documentation and the code itself changes
   every day. Be sure to pull from Github and build often to get the latest version.

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

