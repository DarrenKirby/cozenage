Strings
=======

Overview
--------

Strings are sequences of characters. Strings are written as sequences of characters enclosed within
quotation marks (").

The length of a string is the number of characters that it contains. This number is an exact,
non-negative integer that is fixed when the string is created. The valid indexes of a string are the
exact non-negative integers less than the length of the string. The first character of a string has
index 0, the second has index 1, and so on.

Some of the procedures that operate on strings ignore the difference between upper and lower case.
The names of the versions that ignore case end with “-ci” (for “case insensitive”).


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


