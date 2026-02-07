Symbols
=======

Overview
--------

In this interpreter, **symbols** are a fundamental and widely used Cozenage datatype. They play a central role in the
language’s syntax, semantics, and everyday programming style. Understanding symbols is essential to understanding
how Cozenage programs are written, read, and executed.

At a high level, a symbol is an object that represents a *name*. Unlike strings, which represent sequences of
characters for textual data, symbols are primarily used to represent identifiers, labels, and distinct named values
within programs.

What Is a Symbol?
^^^^^^^^^^^^^^^^^

A **symbol** is an atomic Cozenage object that has an associated *name*. Two symbols are considered identical if and
only if their names are spelled exactly the same way.

More precisely, symbols have the following defining property:

* Two symbols are the same object (in the sense of `eqv?`) if and only if their names are spelled the same way.

This means that symbols with the same name are not merely equal in value; they are *the same interned object*.

Because of this property, symbols are often used in places where other languages might use:

* enumerated values,
* named constants,
* keywords or tags,
* identifiers referring to variables, procedures, or syntactic forms.

Motivation for Symbols
^^^^^^^^^^^^^^^^^^^^^^

Symbols exist to efficiently represent *names* in a program.

In Cozenage, much of the language itself is built on symbols:

* variable names are symbols,
* procedure names are symbols,
* special forms such as `if`, `lambda`, and `define` are identified by symbols,
* quoted data often contains symbols as structural markers.

Using symbols instead of strings for these purposes has several advantages:

* **Efficiency**: Symbols can be compared very quickly, often by pointer comparison.
* **Uniqueness**: There is only one symbol object for a given name.
* **Clarity of intent**: Symbols represent identifiers or labels, not arbitrary text.

For example, a symbol such as `'red` can naturally represent a fixed choice or category in a program, similar to an
enum value in other languages.

Interning and Identity
^^^^^^^^^^^^^^^^^^^^^^

All symbols in this interpreter are **interned**.

Interning means that the interpreter maintains a global table of symbols. When a symbol with a given name is created
or read:

* if a symbol with that name already exists, the existing symbol object is reused;
* otherwise, a new symbol is created and stored in the table.

As a result:

* there is exactly one symbol object for each distinct name,
* symbols with the same name are guaranteed to be identical.

This property allows symbols to be compared directly using identity-based predicates such as `eq?` or `eqv?`. This is
both safe and idiomatic in Cozenage.

This behavior contrasts with strings, where two separate string objects may contain the same characters but still be
distinct objects.

Symbols and Equality
^^^^^^^^^^^^^^^^^^^^

Because symbols are interned, equality comparisons are straightforward:

* `eq?` and `eqv?` can be used reliably to test whether two symbols have the same name.
* There is no need to perform character-by-character comparisons.

This makes symbols especially suitable for use as keys, tags, and markers in data structures and control logic.

Writing and Reading Symbols
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The syntax for writing a symbol is exactly the same as the syntax for writing an **identifier** in Cozenage.

When symbols appear in source code or input, they are typically written as bare identifiers, such as:

* `x`
* `count`
* `lambda`
* `file-not-found`

When a symbol appears as a literal value rather than as an identifier to be evaluated, it is usually introduced using
quotation.

The rules for reading and writing symbols guarantee *round-trip correctness*:

* Any symbol that is produced as part of a literal expression, or read using the `read` procedure,
* and then written out using the `write` procedure,
* will be read back in as the identical symbol (in the sense of `eqv?`).

This property is critical for programs that manipulate code or structured symbolic data, such as interpreters,
compilers, and macro systems.

Symbols vs Strings
^^^^^^^^^^^^^^^^^^

Although symbols and strings may look similar, they serve very different purposes:

* **Strings** represent textual data intended for display, storage, or manipulation.
* **Symbols** represent names and identifiers intended to be compared and matched by identity.

Key differences include:

* Strings are mutable in some Cozenage systems; symbols are not.
* Two strings with the same contents may be distinct objects; two symbols with the same name are always the same object.
* Strings are typically compared using `string=?`; symbols are compared using `eq?` or `eqv?`.

Choosing between a symbol and a string is largely a question of intent: use strings for text, and symbols for names.

Symbols in Programs and Data
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Symbols are used both in executable code and in data structures.

In code, symbols name variables, procedures, and syntactic constructs.

In data, symbols are often used as:

* labels in lists or association lists,
* markers indicating meaning or structure,
* lightweight alternatives to numeric constants or booleans.

Because symbols are interned and immutable, they are safe to share freely throughout a program.

Summary
^^^^^^^

Symbols are a core Cozenage datatype used to represent names and identifiers. Their defining feature is interning: each
distinct name corresponds to exactly one symbol object. This guarantees fast, reliable identity comparison and makes
symbols ideal for use as identifiers, tags, and enumerated values.

For readers new to Cozenage, symbols can be thought of as *named constants with guaranteed identity*, forming the
backbone of both the language’s syntax and its symbolic data structures.

Symbol procedures
-----------------

.. _proc:symbol=?:

.. function:: (symbol=? sym sym ...)

    The symbol=? predicate returns #true if all symbols passed as arguments are equal, otherwise returns #false. Takes
    zero or more arguments. Returns #true if passed zero or one symbols. Raises a type error if passed any object
    other than a symbol.

    :param sym: the object to test
    :type sym: symbol
    :return: #true or #false
    :rtype: boolean

.. _proc:symbol->string:

.. function:: (symbol->string symbol)

    Returns the name of symbol as a string, but without adding escapes. Raises a type error when passed any object
    but a symbol.

    :param symbol: the symbol to extract a string from
    :type symbol: symbol
    :return: the name of symbol as a string
    :rtype: string


.. _proc:string->symbol:

.. function:: (string->symbol string)

    Returns the symbol whose name is string. Raises a type error when passed any object except a string.

    :param string: the string 'name' to create a symbol from
    :type string: string
    :return: the string text as a symbol
    :rtype: symbol
