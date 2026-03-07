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

String Constructor, Accessor, and Setter Procedures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _proc:string:

.. function:: (string char ...)

    Returns a newly allocated string composed of the given characters, in
    order. Analogous to ``list``. If no arguments are provided, returns an
    empty string.

    :param char: Zero or more characters to collect into a string.
    :type char: character
    :return: A new string containing the given characters.
    :rtype: string

    **Example:**

    .. code-block::

      --> (string #\h #\e #\l #\l #\o)
      "hello"
      --> (string #\λ #\space #\λ)
      "λ λ"
      --> (string)
      ""

.. _proc:string-append:

.. function:: (string-append string ...)

    Returns a newly allocated string whose characters are the concatenation of
    the characters of all *string* arguments, in order. If no arguments are
    provided, returns an empty string. If exactly one argument is provided, a
    fresh copy is returned.

    :param string: Zero or more strings to concatenate.
    :type string: string
    :return: A new string containing all characters of all *string* arguments.
    :rtype: string

    **Example:**

    .. code-block:: scheme

      --> (string-append "hello" " " "world")
      "hello world"
      --> (string-append "café" " " "日本語")
      "café 日本語"
      --> (string-append "foo")
      "foo"
      --> (string-append)
      ""


.. _proc:string-ref:

.. function:: (string-ref string k)

    Returns the character at index *k* of *string*, using zero-based character
    indexing. For strings containing multi-byte UTF-8 characters, *k* is a
    character index, not a byte offset. Raises an error if *k* is negative or
    out of range.

    :param string: The string to index into.
    :type string: string
    :param k: The zero-based character index of the character to retrieve.
    :type k: integer
    :return: The character at index *k*.
    :rtype: character

    **Example:**

    .. code-block::

      --> (string-ref "hello" 0)
      #\h
      --> (string-ref "hello" 4)
      #\o
      --> (string-ref "café" 3)
      #\é
      --> (string-ref "日本語" 1)
      #\本


.. _proc:make-string:

.. function:: (make-string k [char])

    Returns a newly allocated string of *k* characters. If *char* is given,
    every character is initialised to *char*. Otherwise every character is
    initialised to a space (``#\space``, U+0020). *k* must be a non-negative
    integer.

    :param k: The number of characters in the new string.
    :type k: integer
    :param char: The character to fill the string with. Defaults to
                 ``#\space``.
    :type char: character
    :return: A new string of *k* characters.
    :rtype: string

    **Example:**

    .. code-block::

      --> (make-string 5)
      "     "
      --> (make-string 5 #\x)
      "xxxxx"
      --> (make-string 3 #\λ)
      "λλλ"
      --> (make-string 0)
      ""

.. _proc:substring:

.. function:: (substring string start end)

    Returns a newly allocated string formed from the characters of *string*
    beginning at index *start* (inclusive) and ending at index *end*
    (exclusive). *start* and *end* are character indices. Raises an error if
    either index is out of range or if *start* is greater than *end*.

    Note that ``substring`` is equivalent to calling ``string-copy`` with the
    same arguments, and is provided for backward compatibility and stylistic
    flexibility.

    :param string: The string to extract a substring from.
    :type string: string
    :param start: The character index of the first character to include.
    :type start: integer
    :param end: The character index past the last character to include.
    :type end: integer
    :return: A new string containing the characters of *string* between
             *start* and *end*.
    :rtype: string

    **Example:**

    .. code-block:: scheme

      --> (substring "hello" 1 3)
      "el"
      --> (substring "hello" 0 5)
      "hello"
      --> (substring "café" 2 4)
      "fé"
      --> (substring "日本語" 1 3)
      "本語"


.. _proc:string-set!:

.. function:: (string-set! string k char)

    Stores *char* at character index *k* of *string*, mutating it in place.
    Correctly handles replacement of a character with one of a different
    UTF-8 byte width by reallocating and shifting the underlying buffer as
    needed. Raises an error if *k* is negative or out of range. Returns an
    unspecified value.

    :param string: The string to mutate.
    :type string: string
    :param k: The zero-based character index at which to store *char*.
    :type k: integer
    :param char: The character to store.
    :type char: character
    :return: Unspecified.

    **Example:**

    .. code-block::

      --> (define s (string-copy "hello"))
      --> (string-set! s 0 #\H)
      --> s
      "Hello"
      --> (define s2 (string-copy "café"))
      --> (string-set! s2 3 #\e)
      --> s2
      "cafe"
      --> (string-set! s2 3 #\é)
      --> s2
      "café"


.. _proc:string-copy:

.. function:: (string-copy string [start [end]])

    Returns a newly allocated copy of the characters of *string* between
    *start* (inclusive) and *end* (exclusive). *start* and *end* are character
    indices. *start* defaults to ``0`` and *end* defaults to the length of
    *string*. Raises an error if either index is out of range or if *start* is
    greater than *end*.

    :param string: The string to copy.
    :type string: string
    :param start: The index of the first character to include. Defaults to
                  ``0``.
    :type start: integer
    :param end: The index past the last character to include. Defaults to the
                length of *string*.
    :type end: integer
    :return: A new string containing the characters of *string* between
             *start* and *end*.
    :rtype: string

    **Example:**

    .. code-block:: scheme

      --> (string-copy "hello")
      "hello"
      --> (string-copy "hello" 1)
      "ello"
      --> (string-copy "hello" 1 3)
      "el"
      --> (string-copy "café" 2)
      "fé"

.. _proc:string-copy!:

.. function:: (string-copy! to at from [start [end]])

    Copies the characters of string *from* between *start* (inclusive) and
    *end* (exclusive) into string *to*, starting at character index *at*,
    mutating *to* in place. All indices are character indices. *start* defaults
    to ``0`` and *end* defaults to the length of *from*. If the source and
    destination ranges overlap, the copy is performed correctly. Returns an
    unspecified value.

    Raises an error if *at* is out of range for *to*, if *start* or *end* are
    out of range for *from*, or if the target region in *to* is too small to
    hold the copied characters.

    :param to: The destination string to copy characters into.
    :type to: string
    :param at: The character index in *to* at which to begin writing.
    :type at: integer
    :param from: The source string to copy characters from.
    :type from: string
    :param start: The index of the first character in *from* to copy. Defaults
                  to ``0``.
    :type start: integer
    :param end: The index past the last character in *from* to copy. Defaults
                to the length of *from*.
    :type end: integer
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

      --> (define s (string-copy "hello world"))
      --> (string-copy! s 6 "scheme")
      --> s
      "hello scheme"
      --> (define s2 (string-copy "café bar"))
      --> (string-copy! s2 5 "日本語" 0 2)
      --> s2
      "café 日本"


.. _proc:string-fill!:

.. function:: (string-fill! string fill [start [end]])

    Stores the character *fill* in every position of *string* between *start*
    (inclusive) and *end* (exclusive), mutating *string* in place. All indices
    are character indices. *start* defaults to ``0`` and *end* defaults to the
    length of *string*. Correctly handles replacement with a character of a
    different UTF-8 byte width by rebuilding the underlying buffer as needed.
    Returns an unspecified value.

    :param string: The string to fill.
    :type string: string
    :param fill: The character to store in each position.
    :type fill: character
    :param start: The index of the first character to fill. Defaults to ``0``.
    :type start: integer
    :param end: The index past the last character to fill. Defaults to the
                length of *string*.
    :type end: integer
    :return: Unspecified.

    **Example:**

    .. code-block::

      --> (define s (string-copy "hello"))
      --> (string-fill! s #\*)
      --> s
      "*****"
      --> (define s2 (string-copy "hello"))
      --> (string-fill! s2 #\x 1 3)
      --> s2
      "hxxlo"
      --> (define s3 (string-copy "hello"))
      --> (string-fill! s3 #\λ 0 2)
      --> s3
      "λλllo"

String Misc Procedures
^^^^^^^^^^^^^^^^^^^^^^

.. _proc:string-length:

.. function:: (string-length string)

    Returns the number of characters in *string* as an exact integer. Note
    that for strings containing multi-byte UTF-8 characters, this is the number
    of Unicode code points, not the number of underlying bytes.

    :param string: The string to measure.
    :type string: string
    :return: The number of characters in *string*.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (string-length "hello")
      5
      --> (string-length "")
      0
      --> (string-length "café")
      4
      --> (string-length "日本語")
      3

.. _proc:string->list:

.. function:: (string->list string [start [end]])

    Returns a newly allocated list of the characters of *string* between
    *start* (inclusive) and *end* (exclusive). *start* and *end* are character
    indices. *start* defaults to ``0`` and *end* defaults to the length of
    *string*. Raises an error if either index is out of range, or if *start*
    is greater than *end*.

    :param string: The string to convert.
    :type string: string
    :param start: The index of the first character to include. Defaults to
                  ``0``.
    :type start: integer
    :param end: The index past the last character to include. Defaults to the
                length of *string*.
    :type end: integer
    :return: A new list of the characters of *string* between *start* and
             *end*.
    :rtype: list

    **Example:**

    .. code-block::

      --> (string->list "hello")
      (#\h #\e #\l #\l #\o)
      --> (string->list "hello" 1)
      (#\e #\l #\l #\o)
      --> (string->list "hello" 1 3)
      (#\e #\l)
      --> (string->list "café" 2)
      (#\f #\é)
      --> (string->list "")
      ()


.. _proc:list->string:

.. function:: (list->string list)

    Returns a newly allocated string formed from the characters in *list*, in
    order. Every element of *list* must be a character; an error is raised
    otherwise.

    :param list: A proper list of characters.
    :type list: list
    :return: A new string containing the characters of *list*.
    :rtype: string

    **Example:**

    .. code-block::

      --> (list->string '(#\h #\e #\l #\l #\o))
      "hello"
      --> (list->string '(#\c #\a #\f #\é))
      "café"
      --> (list->string '())
      ""

.. _proc:string->number:

.. function:: (string->number string [radix])

    Returns the number represented by *string*, or ``#f`` if *string* is not a
    valid numeric representation. *radix* must be one of ``2``, ``8``, ``10``,
    or ``16``, and defaults to ``10``. An explicit radix prefix in *string*
    (e.g. ``"#o177"``) overrides the *radix* argument. Unlike most error
    conditions, an invalid numeric string never signals an error — ``#f`` is
    returned instead.

    The result is the most precise representation available; integers, rationals,
    reals, and complex numbers are all returned in their native types.

    :param string: A string to parse as a number.
    :type string: string
    :param radix: The base in which to interpret *string*. Must be ``2``, ``8``,
                  ``10``, or ``16``. Defaults to ``10``.
    :type radix: integer
    :return: The number represented by *string*, or ``#f`` if *string* is not a
             valid numeric representation.
    :rtype: number or boolean

    **Example:**

    .. code-block:: scheme

      --> (string->number "42")
      42
      --> (string->number "3.14")
      3.14
      --> (string->number "1/3")
      1/3
      --> (string->number "FF" 16)
      255
      --> (string->number "1010" 2)
      10
      --> (string->number "#o177")
      127
      --> (string->number "1+2i")
      1+2i
      --> (string->number "hello")
      #f
      --> (string->number "")
      #f


.. _proc:number->string:

.. function:: (number->string z [radix])

    Returns a string representation of *z* in the given *radix*. *radix* must
    be one of ``2``, ``8``, ``10``, or ``16``, and defaults to ``10``. The
    result never contains an explicit radix prefix. For inexact numbers with
    radix ``10``, the result contains a decimal point and uses the minimum
    number of digits necessary to uniquely identify the value.

    ``number->string`` and ``string->number`` are inverses: for any number *z*
    and valid radix *r*, ``(eqv? z (string->number (number->string z r) r))``
    is guaranteed to be ``#t``.

    :param z: The number to convert.
    :type z: number
    :param radix: The base in which to represent *z*. Must be ``2``, ``8``,
                  ``10``, or ``16``. Defaults to ``10``.
    :type radix: integer
    :return: A string representation of *z* in the given *radix*.
    :rtype: string

    **Example:**

    .. code-block:: scheme

      --> (number->string 42)
      "42"
      --> (number->string 255 16)
      "ff"
      --> (number->string 10 2)
      "1010"
      --> (number->string 127 8)
      "177"
      --> (number->string 3.14)
      "3.14"
      --> (number->string 1/3)
      "1/3"
      --> (number->string 1+2i)
      "1+2i"

.. _proc:string-split:

.. function:: (string-split string [delim])

    Returns a list of substrings of *string* obtained by splitting on
    occurrences of *delim*. *delim* is a string rather than a character,
    allowing for multi-character delimiters. Empty substrings produced by
    adjacent delimiters or leading/trailing delimiters are suppressed. If
    *delim* is omitted, *string* is split on spaces.

    Raises an error if *delim* is longer than *string*, which most likely
    indicates reversed argument order.

    :param string: The string to split.
    :type string: string
    :param delim: The delimiter string to split on. Defaults to ``" "``.
    :type delim: string
    :return: A list of substrings of *string* split by *delim*.
    :rtype: list

    **Example:**

    .. code-block:: scheme

      --> (string-split "hello world foo")
      ("hello" "world" "foo")
      --> (string-split "a,b,c" ",")
      ("a" "b" "c")
      --> (string-split "one::two::three" "::")
      ("one" "two" "three")
      --> (string-split "  hello  world  ")
      ("hello" "world")

String Case-sensitive Comparison Procedures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _proc:string=?:

.. function:: (string=? string1 string2 ...)

    Returns ``#t`` if all arguments contain the same sequence of characters,
    ``#f`` otherwise. Comparison is case-sensitive and based on Unicode scalar
    values. Zero or one argument returns ``#t``.

    :param string1: Two or more strings to compare.
    :type string1: string
    :return: ``#t`` if all arguments are equal, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (string=? "hello" "hello")
      #t
      --> (string=? "hello" "Hello")
      #f
      --> (string=? "café" "café")
      #t
      --> (string=? "a" "a" "a")
      #t


.. _proc:string<?:

.. function:: (string<? string1 string2 ...)

    Returns ``#t`` if the arguments are monotonically increasing in
    lexicographic order by Unicode scalar value, ``#f`` otherwise. Zero or one
    argument returns ``#t``.

    :param string1: Two or more strings to compare.
    :type string1: string
    :return: ``#t`` if the arguments are monotonically increasing, ``#f``
             otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (string<? "abc" "abd")
      #t
      --> (string<? "abc" "abc")
      #f
      --> (string<? "a" "b" "c")
      #t


.. _proc:string<=?:

.. function:: (string<=? string1 string2 ...)

    Returns ``#t`` if the arguments are monotonically non-decreasing in
    lexicographic order by Unicode scalar value, ``#f`` otherwise. Zero or one
    argument returns ``#t``.

    :param string1: Two or more strings to compare.
    :type string1: string
    :return: ``#t`` if the arguments are monotonically non-decreasing, ``#f``
             otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (string<=? "abc" "abd")
      #t
      --> (string<=? "abc" "abc")
      #t
      --> (string<=? "abd" "abc")
      #f
      --> (string<=? "a" "a" "b")
      #t


.. _proc:string>?:

.. function:: (string>? string1 string2 ...)

    Returns ``#t`` if the arguments are monotonically decreasing in
    lexicographic order by Unicode scalar value, ``#f`` otherwise. Zero or one
    argument returns ``#t``.

    :param string1: Two or more strings to compare.
    :type string1: string
    :return: ``#t`` if the arguments are monotonically decreasing, ``#f``
             otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (string>? "abd" "abc")
      #t
      --> (string>? "abc" "abc")
      #f
      --> (string>? "c" "b" "a")
      #t


.. _proc:string>=?:

.. function:: (string>=? string1 string2 ...)

    Returns ``#t`` if the arguments are monotonically non-increasing in
    lexicographic order by Unicode scalar value, ``#f`` otherwise. Zero or one
    argument returns ``#t``.

    :param string1: Two or more strings to compare.
    :type string1: string
    :return: ``#t`` if the arguments are monotonically non-increasing, ``#f``
             otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (string>=? "abd" "abc")
      #t
      --> (string>=? "abc" "abc")
      #t
      --> (string>=? "abc" "abd")
      #f
      --> (string>=? "c" "b" "b")
      #t

String Case-insensitive Comparison Procedures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _proc:string-ci=?:

.. function:: (string-ci=? string1 string2 ...)

    Returns ``#t`` if all arguments are equal under Unicode case-folding, ``#f``
    otherwise. Equivalent to applying ``string-foldcase`` to all arguments
    before comparing with ``string=?``. Zero or one argument returns ``#t``.

    :param string1: Two or more strings to compare.
    :type string1: string
    :return: ``#t`` if all arguments are equal ignoring case, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (string-ci=? "hello" "HELLO")
      #t
      --> (string-ci=? "café" "CAFÉ")
      #t
      --> (string-ci=? "hello" "world")
      #f
      --> (string-ci=? "a" "A" "a")
      #t


.. _proc:string-ci<?:

.. function:: (string-ci<? string1 string2 ...)

    Returns ``#t`` if the arguments are monotonically increasing in
    case-folded lexicographic order, ``#f`` otherwise. Zero or one argument
    returns ``#t``.

    :param string1: Two or more strings to compare.
    :type string1: string
    :return: ``#t`` if the arguments are monotonically increasing ignoring
             case, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (string-ci<? "abc" "ABD")
      #t
      --> (string-ci<? "ABC" "abd")
      #t
      --> (string-ci<? "abc" "ABC")
      #f


.. _proc:string-ci<=?:

.. function:: (string-ci<=? string1 string2 ...)

    Returns ``#t`` if the arguments are monotonically non-decreasing in
    case-folded lexicographic order, ``#f`` otherwise. Zero or one argument
    returns ``#t``.

    :param string1: Two or more strings to compare.
    :type string1: string
    :return: ``#t`` if the arguments are monotonically non-decreasing ignoring
             case, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (string-ci<=? "abc" "ABD")
      #t
      --> (string-ci<=? "abc" "ABC")
      #t
      --> (string-ci<=? "ABD" "abc")
      #f


.. _proc:string-ci>?:

.. function:: (string-ci>? string1 string2 ...)

    Returns ``#t`` if the arguments are monotonically decreasing in
    case-folded lexicographic order, ``#f`` otherwise. Zero or one argument
    returns ``#t``.

    :param string1: Two or more strings to compare.
    :type string1: string
    :return: ``#t`` if the arguments are monotonically decreasing ignoring
             case, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (string-ci>? "ABD" "abc")
      #t
      --> (string-ci>? "ABC" "abc")
      #f
      --> (string-ci>? "C" "b" "A")
      #t


.. _proc:string-ci>=?:

.. function:: (string-ci>=? string1 string2 ...)

    Returns ``#t`` if the arguments are monotonically non-increasing in
    case-folded lexicographic order, ``#f`` otherwise. Zero or one argument
    returns ``#t``.

    :param string1: Two or more strings to compare.
    :type string1: string
    :return: ``#t`` if the arguments are monotonically non-increasing ignoring
             case, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (string-ci>=? "ABD" "abc")
      #t
      --> (string-ci>=? "ABC" "abc")
      #t
      --> (string-ci>=? "abc" "ABD")
      #f
      --> (string-ci>=? "C" "b" "B")
      #t

String Case-transformation Procedures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

String Iteration Procedures
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _proc:string-map:

.. function:: (string-map proc string ...)

    Applies *proc* element-wise to the characters of each *string* argument
    and returns a new string of the resulting characters, in order. *proc* must
    accept as many arguments as there are strings and must return a character;
    an error is raised if it returns any other type. If more than one string is
    given and they differ in length, iteration stops when the shortest string is
    exhausted.

    :param proc: A procedure accepting as many character arguments as there are
                 strings, and returning a character.
    :type proc: procedure
    :param string: One or more strings to map over.
    :type string: string
    :return: A new string of the characters returned by *proc*.
    :rtype: string

    **Example:**

    .. code-block::

      --> (string-map char-upcase "hello")
      "HELLO"
      --> (string-map char-downcase "CAFÉ")
      "café"
      --> (string-map (lambda (c) (if (char=? c #\a) #\@ c)) "banana")
      "b@n@n@"
      --> (string-map (lambda (a b) (if (char<? a b) a b)) "hello" "world")
      "hello"


.. _proc:string-for-each:

.. function:: (string-for-each proc string ...)

    Applies *proc* element-wise to the characters of each *string* argument,
    in order, for its side effects. Unlike ``string-map``, the results of
    *proc* are discarded. *proc* must accept as many arguments as there are
    strings. If more than one string is given and they differ in length,
    iteration stops when the shortest string is exhausted. Returns an
    unspecified value.

    :param proc: A procedure accepting as many character arguments as there are
                 strings.
    :type proc: procedure
    :param string: One or more strings to iterate over.
    :type string: string
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

      --> (string-for-each display "hello")
      hello
      --> (define result '())
      --> (string-for-each
      ...   (lambda (c) (set! result (cons c result)))
      ...   "hello")
      --> result
      (#\o #\l #\l #\e #\h)


