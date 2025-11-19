Characters
==========

Overview
--------

Characters are objects that represent printed characters such as letters and digits.

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

   If *char* is a numeric digit, this procedure returns its integer value (0-9). If *char* is not a digit, it returns ``#false``.

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

   **Representative Example:**

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

   **Representative Example:**

   .. code-block:: scheme

      --> (string-ci=? "scheme" "Scheme" "SCHEME")
        #true
      --> (string-ci=? "hello" "goodbye")
        #false

.. _proc:string-ci-less:
.. function:: (string-ci<? string1 string2 string3 ...)

   Returns ``#true`` if the strings are in lexicographically increasing order, ignoring case.

   **Representative Example:**

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
