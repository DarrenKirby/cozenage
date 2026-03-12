Vectors
=======

Overview
--------

A `vector` is a fixed-length, indexed collection of elements. Unlike a list, which is composed of linked pairs and
optimized for sequential access, a vector stores its elements in contiguous memory and allows direct access by numeric
index. Vectors are written using the literal syntax #( ... ).

.. code-block:: scheme

    #(1 2 3 4)
    #("a" "b" "c")

You can think of a vector as an array. Each element is stored at a specific position, starting from index 0. Accessing
an element by index is efficient and does not require traversing the structure from the beginning, as lists do. For
example:

.. code-block:: scheme

    --> (define v #(10 20 30))
    v
    --> (vector-ref v 1)
    20

Vectors are implemented internally using C-level arrays. This means their elements are stored contiguously in memory,
allowing constant-time indexed access. If your program frequently accesses elements by position, modifies elements in
place, or needs predictable performance for indexed operations, vectors are generally more efficient than lists.

Lists, by contrast, are better suited for recursive processing, structural decomposition (using car and cdr), and
situations where the collection size changes frequently. Lists grow and shrink naturally, while vectors have a fixed
size once created. Although a new vector can be created with a different size, its length cannot be changed after
allocation.

Vectors are especially useful for:

* Storing data where position matters
* Numerical computations
* Implementing tables or buffers
* Situations requiring frequent indexed access
* Performance-sensitive code

Like lists, vectors can store any first-class object, including numbers, strings, procedures, and even other vectors.
However, unlike lists, they are not constructed from pairs and cannot be processed using car and cdr.

Because vectors are mutable, procedures ending in ! may modify their contents in place:

.. code-block:: scheme

    --> (define v #(1 2 3))
    v
    --> (vector-set! v 0 42)
    --> v
    #(42 2 3)

Vectors combine the flexibility of heterogeneous storage with the performance characteristics of arrays, making them
an important general-purpose data structure.

Vector Procedures
-----------------

.. _proc:vector:

vector
^^^^^^

.. function:: (vector obj ...)

    Returns a newly allocated vector whose elements are the given arguments, in
    order. If no arguments are provided, returns an empty vector. Analogous to
    ``list``.

    :param obj: Zero or more objects to store in the vector.
    :type obj: any
    :return: A new vector containing *obj* ...
    :rtype: vector

    **Example:**

    .. code-block:: scheme

      --> (vector 1 2 3)
      #(1 2 3)
      --> (vector 'a "hello" #t)
      #(a "hello" #t)
      --> (vector)
      #()


.. _proc:vector-length:

vector-length
^^^^^^^^^^^^^

.. function:: (vector-length vector)

    Returns the number of elements in *vector* as an exact integer.

    :param vector: The vector to measure.
    :type vector: vector
    :return: The number of elements in *vector*.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (vector-length #(1 2 3))
      3
      --> (vector-length #())
      0


.. _proc:vector-ref:

vector-ref
^^^^^^^^^^

.. function:: (vector-ref vector k)

    Returns the element at index *k* in *vector*, where the first element is at
    index ``0``. Raises an error if *k* is negative or out of range.

    :param vector: The vector to index into.
    :type vector: vector
    :param k: The zero-based index of the element to retrieve.
    :type k: integer
    :return: The element at index *k*.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (vector-ref #(a b c d) 0)
      a
      --> (vector-ref #(a b c d) 2)
      c


.. _proc:make-vector:

make-vector
^^^^^^^^^^^

.. function:: (make-vector k [fill])

    Returns a newly allocated vector of *k* elements. If *fill* is provided,
    every element is initialised to *fill*. Otherwise, every element is
    initialised to ``0``. *k* must be a non-negative integer.

    :param k: The number of elements in the new vector.
    :type k: integer
    :param fill: The value to fill each element with. Defaults to ``0``.
    :type fill: any
    :return: A new vector of *k* elements.
    :rtype: vector

    **Example:**

    .. code-block:: scheme

      --> (make-vector 3)
      #(0 0 0)
      --> (make-vector 4 'x)
      #(x x x x)
      --> (make-vector 0)
      #()

.. _proc:list->vector:

list->vector
^^^^^^^^^^^^

.. function:: (list->vector list)

    Returns a newly allocated vector containing the elements of *list*, in
    order. If *list* is empty, an empty vector is returned.

    :param list: A proper list of elements to store in the vector.
    :type list: list
    :return: A new vector containing the elements of *list*.
    :rtype: vector

    **Example:**

    .. code-block:: scheme

      --> (list->vector '(a b c))
      #(a b c)
      --> (list->vector '())
      #()
      --> (list->vector '(1 (2 3) 4))
      #(1 (2 3) 4)


.. _proc:vector->list:

vector->list
^^^^^^^^^^^^

.. function:: (vector->list vector [start [end]])

    Returns a newly allocated list of the elements of *vector* between *start*
    (inclusive) and *end* (exclusive). *start* defaults to ``0`` and *end*
    defaults to the length of *vector*. Order is preserved.

    :param vector: The vector to convert.
    :type vector: vector
    :param start: The index of the first element to include. Defaults to ``0``.
    :type start: integer
    :param end: The index past the last element to include. Defaults to the
                length of *vector*.
    :type end: integer
    :return: A new list of the elements of *vector* between *start* and *end*.
    :rtype: list

    **Example:**

    .. code-block:: scheme

      --> (vector->list #(a b c d))
      (a b c d)
      --> (vector->list #(a b c d) 1)
      (b c d)
      --> (vector->list #(a b c d) 1 3)
      (b c)
      --> (vector->list #())
      ()


.. _proc:vector-copy:

vector-copy
^^^^^^^^^^^

.. function:: (vector-copy vector [start [end]])

    Returns a newly allocated copy of the elements of *vector* between *start*
    (inclusive) and *end* (exclusive). *start* defaults to ``0`` and *end*
    defaults to the length of *vector*. The elements of the new vector are the
    same (in the sense of ``eqv?``) as the elements of the original.

    :param vector: The vector to copy.
    :type vector: vector
    :param start: The index of the first element to include. Defaults to ``0``.
    :type start: integer
    :param end: The index past the last element to include. Defaults to the
                length of *vector*.
    :type end: integer
    :return: A new vector containing the specified elements of *vector*.
    :rtype: vector

    **Example:**

    .. code-block:: scheme

      --> (vector-copy #(a b c d))
      #(a b c d)
      --> (vector-copy #(a b c d) 1)
      #(b c d)
      --> (vector-copy #(a b c d) 1 3)
      #(b c)


.. _proc:vector->string:

vector->string
^^^^^^^^^^^^^^

.. function:: (vector->string vector [start [end]])

    Returns a newly allocated string formed from the characters stored in
    *vector* between *start* (inclusive) and *end* (exclusive). Every element
    of *vector* in the specified range must be a character. *start* defaults to
    ``0`` and *end* defaults to the length of *vector*. Order is preserved.

    :param vector: A vector of characters.
    :type vector: vector
    :param start: The index of the first element to include. Defaults to ``0``.
    :type start: integer
    :param end: The index past the last element to include. Defaults to the
                length of *vector*.
    :type end: integer
    :return: A new string composed of the characters in *vector* between
             *start* and *end*.
    :rtype: string

    **Example:**

    .. code-block:: scheme

      --> (vector->string #(#\h #\e #\l #\l #\o))
      "hello"
      --> (vector->string #(#\h #\e #\l #\l #\o) 1)
      "ello"
      --> (vector->string #(#\h #\e #\l #\l #\o) 1 3)
      "el"


.. _proc:string->vector:

string->vector
^^^^^^^^^^^^^^

.. function:: (string->vector string [start [end]])

    Returns a newly allocated vector of the characters of *string* between
    *start* (inclusive) and *end* (exclusive). *start* defaults to ``0`` and
    *end* defaults to the length of *string*. Order is preserved. Character
    indices are by Unicode code point, not by byte offset.

    :param string: The string to convert.
    :type string: string
    :param start: The index of the first character to include. Defaults to
                  ``0``.
    :type start: integer
    :param end: The index past the last character to include. Defaults to the
                length of *string*.
    :type end: integer
    :return: A new vector of the characters of *string* between *start* and
             *end*.
    :rtype: vector

    **Example:**

    .. code-block:: scheme

      --> (string->vector "hello")
      #(#\h #\e #\l #\l #\o)
      --> (string->vector "hello" 1)
      #(#\e #\l #\l #\o)
      --> (string->vector "hello" 1 3)
      #(#\e #\l)

.. _proc:vector-set!:

vector-set!
^^^^^^^^^^^

.. function:: (vector-set! vector k obj)

    Stores *obj* at index *k* of *vector*, mutating it in place. Raises an
    error if *k* is negative or out of range. Returns an unspecified value.

    :param vector: The vector to mutate.
    :type vector: vector
    :param k: The zero-based index at which to store *obj*.
    :type k: integer
    :param obj: The value to store.
    :type obj: any
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

      --> (define v (vector 'a 'b 'c))
      --> (vector-set! v 1 'z)
      --> v
      #(a z c)


.. _proc:vector-append:

vector-append
^^^^^^^^^^^^^

.. function:: (vector-append vector ...)

    Returns a newly allocated vector whose elements are the concatenation of
    the elements of all *vector* arguments, in order. If no arguments are
    provided, returns an empty vector. If exactly one argument is provided, it
    is returned unchanged.

    :param vector: Zero or more vectors to concatenate.
    :type vector: vector
    :return: A new vector containing all elements of all *vector* arguments.
    :rtype: vector

    **Example:**

    .. code-block:: scheme

      --> (vector-append #(1 2) #(3 4))
      #(1 2 3 4)
      --> (vector-append #(a b) #(c d) #(e f))
      #(a b c d e f)
      --> (vector-append #())
      #()
      --> (vector-append)
      #()


.. _proc:vector-copy!:

vector-copy!
^^^^^^^^^^^^^

.. function:: (vector-copy! to at from [start [end]])

    Copies the elements of vector *from* between *start* (inclusive) and *end*
    (exclusive) into vector *to*, starting at index *at*, mutating *to* in
    place. *start* defaults to ``0`` and *end* defaults to the length of
    *from*. If the source and destination ranges overlap, the copy is performed
    correctly as if via a temporary intermediate vector. Returns an unspecified
    value.

    Raises an error if any index argument is out of range, or if the target
    range in *to* would exceed its length.

    :param to: The destination vector to copy elements into.
    :type to: vector
    :param at: The index in *to* at which to begin writing.
    :type at: integer
    :param from: The source vector to copy elements from.
    :type from: vector
    :param start: The index of the first element in *from* to copy. Defaults
                  to ``0``.
    :type start: integer
    :param end: The index past the last element in *from* to copy. Defaults to
                the length of *from*.
    :type end: integer
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

      --> (define v (vector 'a 'b 'c 'd 'e))
      --> (vector-copy! v 1 #(x y z))
      --> v
      #(a x y z e)
      --> (vector-copy! v 0 #(x y z) 1 3)
      --> v
      #(y z y z e)


.. _proc:vector-fill!:

vector-fill!
^^^^^^^^^^^^

.. function:: (vector-fill! vector fill [start [end]])

    Stores *fill* in every element of *vector* between *start* (inclusive) and
    *end* (exclusive), mutating *vector* in place. *start* defaults to ``0``
    and *end* defaults to the length of *vector*. If *start* and *end* are
    equal, the procedure has no effect. Returns an unspecified value.

    :param vector: The vector to fill.
    :type vector: vector
    :param fill: The value to store in each element.
    :type fill: any
    :param start: The index of the first element to fill. Defaults to ``0``.
    :type start: integer
    :param end: The index past the last element to fill. Defaults to the length
                of *vector*.
    :type end: integer
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

      --> (define v (vector 'a 'b 'c 'd 'e))
      --> (vector-fill! v 'z)
      --> v
      #(z z z z z)
      --> (vector-fill! v 'x 1 3)
      --> v
      #(z x x z z)
      --> (vector-fill! v 'y 2)
      --> v
      #(z x y y y)

.. _proc:vector-map:

vector-map
^^^^^^^^^^

.. function:: (vector-map proc vector ...)

    Applies *proc* element-wise to the elements of each *vector* argument and
    returns a new vector of the results, in order. *proc* must accept as many
    arguments as there are vectors. If more than one vector is given and they
    differ in length, iteration stops when the shortest vector is exhausted.

    :param proc: A procedure accepting as many arguments as there are vectors.
    :type proc: procedure
    :param vector: One or more vectors to map over.
    :type vector: vector
    :return: A new vector of the results of applying *proc* to each set of
             elements.
    :rtype: vector

    **Example:**

    .. code-block:: scheme

      --> (vector-map (lambda (x) (* x x)) #(1 2 3 4 5))
      #(1 4 9 16 25)
      --> (vector-map + #(1 2 3) #(10 20 30))
      #(11 22 33)
      --> (vector-map + #(1 2 3) #(10 20))
      #(11 22)
      --> (vector-map car #(()))
      #()


.. _proc:vector-for-each:

vector-for-each
^^^^^^^^^^^^^^^

.. function:: (vector-for-each proc vector ...)

    Applies *proc* element-wise to the elements of each *vector* argument, in
    order, for its side effects. Unlike ``vector-map``, the results of *proc*
    are discarded. *proc* must accept as many arguments as there are vectors.
    If more than one vector is given and they differ in length, iteration stops
    when the shortest vector is exhausted. Returns an unspecified value.

    :param proc: A procedure accepting as many arguments as there are vectors.
    :type proc: procedure
    :param vector: One or more vectors to iterate over.
    :type vector: vector
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

      --> (vector-for-each display #(1 2 3))
      123
      --> (define result '())
      --> (vector-for-each
      ...   (lambda (x) (set! result (cons (* x x) result)))
      ...   #(1 2 3))
      --> result
      (9 4 1)

