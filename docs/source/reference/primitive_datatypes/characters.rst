Characters
==========

Overview
--------

A `character` represents a single textual unit. Characters are written using the ``#\`` prefix:

.. code-block:: scheme

    #\a
    #\A
    #\newline
    #\space

Characters are distinct from strings. A string may contain many characters, while a character represents exactly one.
For example:

.. code-block:: scheme

    --> #\a
    #\a
    --> "a"
    "a"

Although these may look similar, the first is a character and the second is a string containing one character.

Characters are primarily used when working with textual data at a fine-grained level, such as:

* Parsing input character by character
* Implementing lexical analyzers
* Performing character classification (alphabetic, numeric, whitespace, etc.)
* Converting between character codes and textual representations

Characters in Cozenage are stored internally as 32-bit Unicode code points (UChar32). This means each character directly
represents a full Unicode scalar value, making character comparisons, classification, and transformation straightforward
and unambiguous. Unlike UTF-8 strings, which are variable-length byte sequences, a character object always represents
exactly one Unicode code point.

This design allows characters such as λ, €, or Ω to behave identically to simple ASCII characters like A or ?.

Example:

.. code-block:: text

    --> #\λ
    #\λ
    --> (char? #\λ)
    #true

Character comparison procedures allow you to test equality and ordering, and classification procedures determine
properties such as whether a character is alphabetic, numeric, or whitespace.

Because characters are first-class objects, they can be stored in lists, vectors, sets, or maps, passed to procedures,
and returned as values.

Understanding the distinction between characters and strings is important: characters represent atomic textual units,
while strings represent ordered sequences of those units.

Named Character Literals
------------------------

The Cozenage parser supports a set of user-friendly named Unicode character literals. These are accepted during input
using the #\\name syntax. When printed, however, characters are displayed as their actual Unicode glyph, not their
named form.

For example:

.. code-block:: text

    --> #\lambda
    #\λ

This means the name is only recognized by the reader; internally and when printed, the true Unicode character is used.

Below is the complete list of supported named character literals.

Supported Named Character Literals
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

+-----------+---------+------------+
| Name      | Glyph   | Code Point |
+===========+=========+============+
| Alpha     | Α       | U+0391     |
+-----------+---------+------------+
| Beta      | Β       | U+0392     |
+-----------+---------+------------+
| Delta     | Δ       | U+0394     |
+-----------+---------+------------+
| Gamma     | Γ       | U+0393     |
+-----------+---------+------------+
| Iota      | Ι       | U+0399     |
+-----------+---------+------------+
| Lambda    | Λ       | U+039B     |
+-----------+---------+------------+
| Omega     | Ω       | U+03A9     |
+-----------+---------+------------+
| Omicron   | Ο       | U+039F     |
+-----------+---------+------------+
| Phi       | Φ       | U+03A6     |
+-----------+---------+------------+
| Pi        | Π       | U+03A0     |
+-----------+---------+------------+
| Psi       | Ψ       | U+03A8     |
+-----------+---------+------------+
| Rho       | Ρ       | U+03A1     |
+-----------+---------+------------+
| Sigma     | Σ       | U+03A3     |
+-----------+---------+------------+
| Theta     | Θ       | U+0398     |
+-----------+---------+------------+
| Xi        | Ξ       | U+039E     |
+-----------+---------+------------+
| alpha     | α       | U+03B1     |
+-----------+---------+------------+
| beta      | β       | U+03B2     |
+-----------+---------+------------+
| chi       | χ       | U+03C7     |
+-----------+---------+------------+
| copy      | ©       | U+00A9     |
+-----------+---------+------------+
| curren    | ¤       | U+00A4     |
+-----------+---------+------------+
| deg       | °       | U+00B0     |
+-----------+---------+------------+
| delta     | δ       | U+03B4     |
+-----------+---------+------------+
| divide    | ÷       | U+00F7     |
+-----------+---------+------------+
| epsilon   | ε       | U+03B5     |
+-----------+---------+------------+
| eta       | η       | U+03B7     |
+-----------+---------+------------+
| euro      | €       | U+20AC     |
+-----------+---------+------------+
| gamma     | γ       | U+03B3     |
+-----------+---------+------------+
| iota      | ι       | U+03B9     |
+-----------+---------+------------+
| iquest    | ¿       | U+00BF     |
+-----------+---------+------------+
| kappa     | κ       | U+03BA     |
+-----------+---------+------------+
| lambda    | λ       | U+03BB     |
+-----------+---------+------------+
| micro     | µ       | U+00B5     |
+-----------+---------+------------+
| mu        | μ       | U+03BC     |
+-----------+---------+------------+
| omega     | ω       | U+03C9     |
+-----------+---------+------------+
| para      | ¶       | U+00B6     |
+-----------+---------+------------+
| phi       | φ       | U+03C6     |
+-----------+---------+------------+
| pi        | π       | U+03C0     |
+-----------+---------+------------+
| plusnm    | ±       | U+00B1     |
+-----------+---------+------------+
| pound     | £       | U+00A3     |
+-----------+---------+------------+
| psi       | ψ       | U+03C8     |
+-----------+---------+------------+
| reg       | ®       | U+00AE     |
+-----------+---------+------------+
| rho       | ρ       | U+03C1     |
+-----------+---------+------------+
| sect      | §       | U+00A7     |
+-----------+---------+------------+
| sigma     | σ       | U+03C3     |
+-----------+---------+------------+
| tau       | τ       | U+03C4     |
+-----------+---------+------------+
| theta     | θ       | U+03B8     |
+-----------+---------+------------+
| times     | ×       | U+00D7     |
+-----------+---------+------------+
| xi        | ξ       | U+03BE     |
+-----------+---------+------------+
| yen       | ¥       | U+00A5     |
+-----------+---------+------------+
| zeta      | ζ       | U+03B6     |
+-----------+---------+------------+

Character Procedures
--------------------

.. _proc:char-alphabetic?:

.. function:: (char-alphabetic? char)

   Returns ``#true`` if *char* is an alphabetic character, and ``#false`` otherwise.

   :param char: The character to test.
   :type char: char
   :return: #true or #false.
   :rtype: boolean

   **Example:**

   .. code-block:: scheme

      --> (char-alphabetic? #\a)
        #true
      --> (char-alphabetic? #\Z)
        #true
      --> (char-alphabetic? #\7)
        #false

.. _proc:char-numeric?:

.. function:: (char-numeric? char)

   Returns ``#true`` if *char* is a numeric digit (0-9), and ``#false`` otherwise.

   :param char: The character to test.
   :type char: char
   :return: #true or #false.
   :rtype: boolean

   **Example:**

   .. code-block:: scheme

      --> (char-numeric? #\5)
        #true
      --> (char-numeric? #\x)
        #false

.. _proc:char-whitespace?:

.. function:: (char-whitespace? char)

   Returns ``#true`` if *char* is a whitespace character (like space, tab, or newline), and ``#false`` otherwise.

   :param char: The character to test.
   :type char: char
   :return: #true or #false.
   :rtype: boolean

   **Example:**

   .. code-block:: scheme

      --> (char-whitespace? #\space)
        #true
      --> (char-whitespace? #\newline)
        #true
      --> (char-whitespace? #\a)
        #false

.. _proc:char-upper-case?:

.. function:: (char-upper-case? char)

   Returns ``#true`` if *char* is an uppercase letter, and ``#false`` otherwise.

   :param char: The character to test.
   :type char: char
   :return: #true or #false.
   :rtype: boolean

   **Example:**

   .. code-block:: scheme

      --> (char-upper-case? #\A)
        #true
      --> (char-upper-case? #\a)
        #false

.. _proc:char-lower-case?:

.. function:: (char-lower-case? char)

   Returns ``#true`` if *char* is a lowercase letter, and ``#false`` otherwise.

   :param char: The character to test.
   :type char: char
   :return: #true or #false.
   :rtype: boolean

   **Example:**

   .. code-block:: scheme

      --> (char-lower-case? #\z)
        #true
      --> (char-lower-case? #\Z)
        #false

.. _proc:digit-value:

.. function:: (digit-value char)

   If *char* is a numeric digit, this procedure returns its integer value (0-9). If *char* is not a digit, it
   returns ``#false``.

   :param char: The character to convert.
   :type char: char
   :return: An integer from 0-9 or #false.
   :rtype: integer or boolean

   **Example:**

   .. code-block:: scheme

      --> (digit-value #\7)
        7
      --> (digit-value #\a)
        #false

Case Conversion Procedures
--------------------------

.. _proc:char-upcase:

.. function:: (char-upcase char)

   Returns the uppercase equivalent of *char*. If *char* is not a lowercase letter, it is returned unchanged.

   :param char: The character to convert.
   :type char: char
   :return: The uppercase version of the character.
   :rtype: char

   **Example:**

   .. code-block:: scheme

      --> (char-upcase #\a)
        #\A
      --> (char-upcase #\B)
        #\B

.. _proc:char-downcase:

.. function:: (char-downcase char)

   Returns the lowercase equivalent of *char*. If *char* is not an uppercase letter, it is returned unchanged.

   :param char: The character to convert.
   :type char: char
   :return: The lowercase version of the character.
   :rtype: char

   **Example:**

   .. code-block:: scheme

      --> (char-downcase #\A)
        #\a
      --> (char-downcase #\b)
        #\b

.. _proc:char-foldcase:

.. function:: (char-foldcase char)

   Applies Unicode case-folding to *char*. For most characters, this is the same as ``char-downcase``, but it handles
   special cases for robust case-insensitive comparison.

   :param char: The character to convert.
   :type char: char
   :return: The folded-case version of the character.
   :rtype: char

   **Example:**

   .. code-block:: scheme

      --> (char-foldcase #\A)
        #\a
      --> (char-foldcase #\ς) ; Greek final sigma
        #\σ

Case-Insensitive Comparison Procedures
--------------------------------------

.. _proc:char-ci-equal:
.. function:: (char-ci=? char1 char2 char3 ...)

   Returns ``#true`` if all characters are the same when ignoring case.

   **Example:**

   .. code-block:: scheme

      --> (char-ci=? #\a #\A)
        #true
      --> (char-ci=? #\z #\Z #\z)
        #true
      --> (char-ci=? #\a #\b)
        #false

.. function:: (char-ci<? char1 char2 char3 ...)
.. function:: (char-ci<=? char1 char2 char3 ...)
.. function:: (char-ci>? char1 char2 char3 ...)
.. function:: (char-ci>=? char1 char2 char3 ...)

   These procedures compare characters in a case-insensitive manner, returning ``#true`` if the arguments are in
   monotonically increasing, non-decreasing, decreasing, or non-increasing order, respectively.


.. _proc:string-ci-equal:
.. function:: (string-ci=? string1 string2 string3 ...)

   Returns ``#true`` if all strings are equal when ignoring case.

   **Example:**

   .. code-block:: scheme

      --> (string-ci=? "scheme" "Scheme" "SCHEME")
        #true
      --> (string-ci=? "hello" "goodbye")
        #false

.. _proc:string-ci-less:
.. function:: (string-ci<? string1 string2 string3 ...)

   Returns ``#true`` if the strings are in lexicographically increasing order, ignoring case.

   **Example:**

   .. code-block:: scheme

      --> (string-ci<? "alpha" "Beta")
        #true
      --> (string-ci<? "a" "B" "c")
        #true
      --> (string-ci<? "zeta" "alpha")
        #false

.. function:: (string-ci<=? string1 string2 string3 ...)
.. function:: (string-ci>? string1 string2 string3 ...)
.. function:: (string-ci>=? string1 string2 string3 ...)

   These procedures compare strings lexicographically in a case-insensitive manner, returning ``#true`` if the
   arguments are in monotonically non-decreasing, decreasing, or non-increasing order, respectively.
