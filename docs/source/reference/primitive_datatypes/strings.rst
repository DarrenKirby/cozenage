Strings
=======

Overview
--------

A `string` is a sequence of characters. Strings are written between double quotation marks:

.. code-block:: scheme

    "hello"
    "Scheme"
    "123"

Strings are commonly used to represent textual data, such as names, messages, file contents, and user input. Like
vectors, strings are indexed collections, and individual characters can be accessed by position using string-ref.

.. code-block:: scheme

    --> (define s "hello")
    --> (string-ref s 1)
    #\e

Internally, strings are implemented as contiguous arrays of characters at the C level. This makes indexing operations
efficient and predictable. Accessing a character at a given index takes constant time, unlike lists, which require
traversal.

Cozenage strings are internally stored as UTF-8 encoded byte sequences, allowing them to represent the full range of
Unicode characters. This means strings can contain not only ASCII text, but accented letters, mathematical symbols,
emoji, and characters from writing systems around the world. UTF-8 is a variable-length encoding, so some characters
occupy more than one byte. While this detail is handled automatically by the implementation, it is important to remember
that the number of bytes in a string is not necessarily the same as the number of characters. All standard string
operations operate on characters rather than raw bytes, ensuring correct behavior even when multi-byte Unicode
characters are present.

Example:

.. code-block:: scheme

    --> "λ"
    "λ"
    --> (string-length "λ")
    1

Even though the character λ occupies multiple bytes in UTF-8, it is correctly treated as a single character.

Strings have a fixed length once created. Although individual characters may be modified using mutation procedures such
as string-set!, the overall length of the string cannot change. To create a string of a different length, a new string
must be allocated.

Strings are useful for:

* Representing human-readable text
* Parsing and formatting data
* Storing identifiers or keys
* File and network I/O
* Interfacing with external systems

Because strings are sequences of characters, many procedures operate on them collectively, including substring
extraction, comparison, and conversion to and from lists of characters.

It is important to remember that strings are distinct from symbols. A string represents textual data, while a symbol
represents an identifier. Although they may look similar when printed, they serve different purposes and behave
differently in the language.

String Procedures
-----------------

.. _proc:string-upcase:

.. function:: (string-upcase string)

   Returns a new string with all alphabetic characters in *string* converted to uppercase.

   :param string: The string to convert.
   :type string: string
   :return: A new, uppercased string.
   :rtype: string

   **Example:**

   .. code-block:: scheme

      --> (string-upcase "Hello World")
        "HELLO WORLD"

.. _proc:string-downcase:

.. function:: (string-downcase string)

   Returns a new string with all alphabetic characters in *string* converted to lowercase.

   :param string: The string to convert.
   :type string: string
   :return: A new, downcased string.
   :rtype: string

   **Example:**

   .. code-block:: scheme

      --> (string-downcase "Hello World")
        "hello world"

.. _proc:string-foldcase:

.. function:: (string-foldcase string)

   Returns a new string with all characters in *string* converted to their folded-case equivalents. This is the
   preferred method for preparing strings for case-insensitive comparison.

   :param string: The string to convert.
   :type string: string
   :return: A new, case-folded string.
   :rtype: string

   **Example:**

   .. code-block:: scheme

      --> (string-foldcase "Der Fluß")
        "der fluss"


