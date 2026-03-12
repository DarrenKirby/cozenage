Primitive datatypes
===================

Cozenage provides all the primitive Scheme datatypes as would be expected, as well as two other primitive types not
specified by the R7RS standard. All primitive types are disjoint, which
is to say that any object will return ``#true`` for one, and only one, of the following predicates:

* ``boolean?``
* ``char?``
* ``null?``
* ``pair?``
* ``procedure?``
* ``symbol?``
* ``bytevector?``
* ``eof-object?``
* ``number?``
* ``port?``
* ``string?``
* ``vector?``
* ``set?``
* ``hash?``

It is common in Scheme documentation and literature to refer to these datatypes as **objects** of the given type, and
to use the generic term **object** to refer to an instantiation of any Scheme type. These types/objects can be
classified as either **atomic** or **compound**. The atomic types, ``boolean``, ``char``, ``null``, ``procedure``,
``symbol``, ``eof-object``, ``number``, and ``port`` are indivisible units that represent specific values of each type.
The compound types represent either homogenic or heterogenic 'containers' which can hold either atomic types, or in
some cases, atomic types and nested compound types. For example, ``string`` is a homogenic compound type that can hold
only chars, and ``bytevectors`` are homogenic types that can hold only integers. On the other hand, ``pair`` and
``vector`` are heterogenic types which can hold any combination of atomic types and/or nested compound types. A ``set``
can hold any hashable type, and a ``hash`` can use any hashable type as key, and any object type as value.

All primitive types and objects are first-class, which means they can be bound to names (ie: stored in a variable),
passed to procedures as arguments, and returned from procedures as values.

An important concept in Scheme is that of the external representation of an object as a sequence of characters. For
example, an external representation of the integer 28 is the sequence of characters ``28``, and an external
representation of a list consisting of the integers 8 and 13 is the sequence of characters ``(8 13)``.

The external representation of an object is not necessarily unique. The integer 28 also has representations
``#e28.000`` and ``#x1c``, and the list in the previous paragraph also has the representations
``( 08 13 )`` and ``(8 . (13 . ()))``.

All objects have some representation when displayed at the REPL, but not all of those
representations are meaningful to ``read``. Types with a defined external representation
can be written as literals directly in source code or at the REPL, and ``read`` can parse
them back into live objects — either via the ``quote`` special form or as a plain literal.

The following types have a defined external representation:

- Booleans (``#t``, ``#f``)
- Numbers
- Characters
- Strings
- Symbols
- Lists and pairs
- Vectors
- Bytevectors
- Sets
- Hashes

Types without a defined external representation such as procedures and ports are still
displayed at the REPL using an implementation-specific notation (for example,
``#<lambda 'fact'>``), but these representations are a display convenience only. They
cannot be read back by ``read`` or used as literals in source code.

Mutability of Objects
----------------------------

In Scheme, every object has a property of being either immutable or mutable. An immutable object's value can never be
changed after it has been created. In contrast, a mutable object's contents can be modified after its creation using
dedicated procedures. This distinction is fundamental to writing safe and predictable Scheme code. Many of the
primitive types are always immutable, including booleans (``#t``, ``#f``), numbers (e.g., ``42``, ``3.14``),
characters (e.g., ``#\\a``),
and symbols (e.g., ``'foo``).

The most common source of confusion regarding mutability arises with compound types like lists, vectors, and strings.
When you write a literal expression—that is, data that is quoted with ``'`` or ``quote`` you are creating a representation of
the data. The R7RS standard specifies that it is an error to attempt to modify an object created from a literal.
For example: ``(list-set! '(1 2 3 4) 0 99)`` is an error because ``'(1 2 3 4)`` is a literal constant. Binding it to a variable
with ``(define my-list '(1 2 3 4))`` does not change the fact that ``my-list`` is bound to an immutable object.
Attempting to mutate it is considered undefined behavior.

To create a mutable object that you can safely modify, you must use a constructor procedure. These procedures
explicitly allocate fresh, mutable objects. For lists, you would use ``list`` or ``cons``; for vectors, ``vector``; and
for strings, ``string``. For example, if you create a list with (define my-list (list 1 2 3 4)), you are guaranteed
to have a new, mutable list. You can then safely use procedures like ``list-set!``, ``set-car!``, or ``set-cdr!``
to modify my-list's contents. The key takeaway is this: use constructors like list, vector, and string when you need to
create objects that you intend to mutate. Relying on quoted literals for mutable data is not portable and may result in
an error.

.. toctree::
   :maxdepth: 1
   :caption: Contents:

   booleans
   bytevectors
   characters
   numerics
   pairs_and_lists
   ports
   procedures
   strings
   symbols
   vectors
   sets
   hashes
