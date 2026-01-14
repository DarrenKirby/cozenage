Notation and Lexicography
=========================

This page describes a few lexicographic tidbits related to Scheme itself, and also details the notation used in the
rest of this document to represent prescribed values, types, and objects required by certain procedures and other
interfaces.

Identifiers
-----------

Identifiers have two uses within Scheme programs:

* Any identifier can be used as a variable or as a syntactic keyword.
* When an identifier appears as a literal or within a literal, it is being used to denote a symbol.

An identifier is any sequence of letters, digits, and “extended identifier characters” provided that it does not have
a prefix which is a valid number. However, the . token (a single period) used in the list syntax is not an identifier.

Cozenage supports the following extended identifier characters:

``! $ %  & * + - . / : < = > ? @ ^ _ ~``

Alternatively, an identifier can be represented by a sequence of zero or more characters enclosed within vertical
bars (|), analogous to string literals. Any character, including whitespace characters, but excluding the backslash
and vertical line characters, can appear verbatim in such an identifier.

Here are some examples of valid identifiers:

* ``+soup+``
* ``->string``
* ``a34kTMNs``
* ``list->vector``
* ``q``
* ``V17a``
* ``\|two words\|``
* ``the-word-recursion-has-many-meanings``

The syntax distinguishes between upper and lower case in identifiers and in characters specified using their names.
However, it does not distinguish between upper and lower case in numbers, nor in ⟨inline hex escapes⟩ used in the
syntax of identifiers, characters, or strings. None of the identifiers defined in this documentation contain upper-case
characters, even when they appear to do so as a result of the English-language convention of capitalizing the first
word of a sentence.

Whitespace
----------------

Whitespace characters include the space, tab, and newline characters. Whitespace is used for improved readability and as
necessary to separate tokens from each other, a token being an indivisible lexical unit such as an identifier or number,
but is otherwise insignificant. Whitespace can occur between any two tokens, but not within a token. Whitespace occurring
inside a string or inside a symbol delimited by vertical lines is significant.

Line comments and block comments
--------------------------------

The lexical syntax includes several comment forms. Comments are treated exactly like whitespace.

A semicolon ``;`` indicates the start of a line comment. The comment continues to the end of the line on which the semicolon
appears.

Block comments are indicated with properly nested ``#|`` and ``|#`` pairs:

.. code-block:: scheme

    #|
       The FACT procedure computes the factorial
       of a non-negative integer.
    |#
    (define fact
      (lambda (n)
        (if (= n 0)
            1        ; Base case: return 1
            (* n (fact (- n 1))))))

Other notations
---------------

``( )``
    Parentheses are used for grouping and to notate lists.

``'``
    The apostrophe (single quote) character is used to indicate literal data.

``ˋ``
    The grave accent (backquote) character is used to indicate partly constant data.

``,`` ``,@``
    The character comma and the sequence comma at-sign are used in conjunction with quasiquotation to denote ``unquote`` and ``unquote-splicing``.

``"``
    The quotation mark character is used to delimit strings.

``\``
    Backslash is used in the syntax for character constants and as an escape character within string constants and identifiers.

``#``
    The number sign is used for a variety of purposes depending on the character that immediately follows it:

``#t`` ``#f``
    These are the boolean constants, along with the alternatives ``#true`` and ``#false``.

``#\``
    This introduces a character constant.

``#(``
    This introduces a vector constant. Vector constants are terminated by ``)``.

``#u8(`` ``#s8(`` ``#u16(`` ``#s16(`` ``#u32(`` ``#s32(`` ``#u64(`` ``#s64(``
    These introduce a bytevector constant. Bytevector constants are terminated by ``)``.

``#e`` ``#i`` ``#b`` ``#o`` ``#d`` ``#x``
    These are used in the notation for numbers.


Notation used in procedure documentation
----------------------------------------

Every procedure exported by Cozenage has specific documentation appearing in later pages, which describes its arguments,
return values, and provides a prose-description of its intended uses. This documentation is of the form:

.. function:: (some-procedure arg1 arg2 arg3 ... )

   Returns ``#true`` if arg1 through arg3 frabdnocate the spelektamazoik, and ``#false`` otherwise.

   :param type: The file to load.
   :type char: obj
   :return: #true or #false.
   :rtype: boolean

The argument types, return values, and domain(s) of each procedure are specified by the following notations:

* `alist` - an association list ie, a proper list were each value is a pair
* `boolean` - ``#t`` or ``#f``; a boolean value
* `byte` - an integer, 0-255 inclusive
* `bytevector` - a bytevector
* `char` - a character
* `start` / `end` - an exact, non-negative integer
* `k`, `k1`, `kj` - an exact, non-negative integer
* `letter` - an alphabetic character (ie: 'd', not a Scheme char)
* `list`, `list1`, `listj` - a proper list
* `n`, `n1`, `nj` - an integer
* `obj` - any Scheme object
* `pair` - a proper or improper list
* `port` - an input or output port
* `proc` - a procedure, either built-in, or a user-defined lambda
* `q`, `q1`, `qj` - a rational number
* `string` - a string
* `symbol` - a symbol
* `thunk` - a procedure with zero arguments
* `vector` - a vector
* `x`, `x1`, `xj` - a real number
* `y`, `y1`, `yj` - a real number
* `z`, `z1`, `zj` - a complex number

The names `start` and `end` are used as indexes into strings, vectors, and bytevectors. Their use implies the following:

* It is an error if `start` is greater than `end`.
* It is an error if `end` is greater than the length of the string, vector, or bytevector.
* If `start` is omitted, it is assumed to be zero.
* If `end` is omitted, it assumed to be the length of the string, vector, or bytevector.
* The index `start` is always inclusive and the index `end` is always exclusive. As an example, consider a string:
  If `start` and `end` are the same, an empty substring is referred to, and if `start` is zero and `end` is the length of
  string, then the entire string is referred to.

