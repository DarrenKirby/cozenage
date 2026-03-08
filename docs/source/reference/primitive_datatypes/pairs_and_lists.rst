Pairs and Lists
===============

Overview
--------

Pairs and lists are among the most important and commonly used data structures in Cozenage. From a user’s perspective,
they provide the primary way to group values together, build sequences, and represent structured data.

Although **pairs** and **lists** have different *meanings* and *typical uses*, they are constructed from the same
underlying datatype. Understanding how they relate—and how they differ—is key to writing clear and correct Cozenage programs.

Pairs: The Fundamental Building Block
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A **pair** is a compound object that holds exactly two values:

* a *first* component, called the **car**
* a *second* component, called the **cdr**

Pairs are created using the `cons` procedure.

.. code-block:: scheme

    --> (cons 1 2)
    (1 . 2)

The printed form `(1 . 2)` is called a **dotted pair**. The dot visually separates the two parts of the pair.

Pairs are general-purpose containers: the two components can be *any* Cozenage objects, including numbers, strings,
symbols, other pairs, or even procedures.

Accessing the Parts of a Pair
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The two components of a pair can be accessed individually:

* `car` returns the first component
* `cdr` returns the second component

.. code-block:: scheme

    --> (define p (cons "a" "b"))
    --> (car p)
    "a"
    --> (cdr p)
    "b"

From a user’s perspective, this makes pairs useful for representing *two related values*.

Lists: A Special Kind of Structure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A **list** is not a separate datatype from a pair. Instead, a list is a *specific way of chaining pairs together*.

A list is defined as:

* either the empty list, or
* a pair whose `cdr` is itself a list.

This recursive definition means that lists are built from pairs, ending with a special object called the **empty list**.

The Empty List (Nil)
^^^^^^^^^^^^^^^^^^^^

The empty list is a distinguished object that represents a list with no elements. It is often written as `()`.

From a user’s perspective, the empty list:

* marks the end of a list,
* represents “no elements”, and
* is distinct from all other objects.

Example:

.. code-block:: scheme

    --> '()
    ()

The empty list is sometimes referred to as **nil** in documentation and discussion.

Constructing Lists
^^^^^^^^^^^^^^^^^^

Lists can be constructed explicitly using `cons` and the empty list:

.. code-block:: scheme

    --> (cons 1 (cons 2 (cons 3 '())))
    (1 2 3)

Because this pattern is so common, Cozenage provides the `list` procedure as a convenience:

.. code-block:: scheme

    --> (list 1 2 3)
    (1 2 3)

Although the printed form `(1 2 3)` looks different from dotted pairs, it is simply a shorthand for a chain of pairs
ending in the empty list.

Proper Lists
^^^^^^^^^^^^

A **proper list** is a list that:

* consists of zero or more pairs, and
* ends with the empty list `()`.

Examples of proper lists include:

.. code-block:: scheme

    --> '()
    ()
    --> '(a b c)
    (a b c)

Most list-processing procedures in Cozenage expect proper lists.

Improper Lists and Dotted Lists
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

An **improper list** is a structure that looks like a list, but does *not* end with the empty list.

For example:

.. code-block:: scheme

    --> (cons 1 (cons 2 3))
    (1 2 . 3)

This is sometimes called a **dotted list**. The final `cdr` is `3` instead of `()`.

Improper lists are still valid pair structures, but many list-oriented procedures do not accept them. From a user’s
perspective, improper lists are best used deliberately and sparingly, when a non-list tail is meaningful.

Semantic Differences: Pairs vs Lists
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Although pairs and lists share the same underlying representation, they have different *intended meanings*:

* **Pairs** represent a fixed association between two values.
* **Lists** represent ordered sequences of values of arbitrary length.

This distinction is semantic rather than structural. A list *is* a chain of pairs, but not every chain of pairs is
intended to be used as a list.

Choosing whether to treat a structure as a pair or as a list depends on how it is meant to be used in a program.

Association Lists (Alists)
^^^^^^^^^^^^^^^^^^^^^^^^^^

An **association list**, or **alist**, is a common Cozenage convention built on top of lists and pairs.

An alist is:

* a proper list,
* whose elements are themselves pairs,
* where each pair associates a *key* with a *value*.

Example:

.. code-block:: scheme

    --> (define colors
            '((red . "#ff0000")
            (green . "#00ff00")
            (blue . "#0000ff")))
    colors
    --> colors
    ((red . "#ff0000") (green . "#00ff00") (blue . "#0000ff"))

Each element of the list is a pair whose `car` is the key and whose `cdr` is the associated value.

Alists are commonly used for:

* simple lookup tables,
* configuration data,
* mappings from symbols to values.

Because symbols are interned and easily compared, they are often used as keys in alists.

Printed Representation and Readability
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Cozenage’s printed representations are designed to make pairs and lists readable and unambiguous:

* Proper lists are printed without dots.
* Dotted pairs and improper lists are printed with explicit dots.

This makes it easy to see the *structure* of a compound object directly from its printed form.

Summary
^^^^^^^

Pairs and lists form the backbone of structured data in Cozenage.

* A **pair** holds exactly two values.
* A **list** is a chain of pairs ending in the empty list.
* Proper lists, improper lists, and dotted pairs are all built from the same underlying structure.
* The empty list marks the end of a list and represents “no elements”.
* Association lists provide a simple and flexible way to map keys to values.

By understanding how pairs and lists relate—and how they differ semantically—Cozenage programmers gain a powerful and
expressive way to organize data.


List and pair procedures
------------------------

.. _proc:cons:

cons
^^^^

.. function:: (cons obj1 obj2)

   Constructs a pair.

   :param obj1: The object to place in the car cell.
   :type obj1: any
   :param obj2: The object to place in the cdr cell.
   :type obj2: any
   :return: A newly allocated pair.
   :rtype: pair

   **Example:**

   .. code-block:: scheme

      --> (cons 1 2)
        (1 . 2)

.. _proc:car:

car
^^^

.. function:: (car pair)

   Returns the first element of the pair.

   :param pair: The pair to operate on.
   :type pair: pair
   :return: The first element (car) of the pair.
   :rtype: any
   :raises: Type error if *pair* is not a pair.

   **Example:**

   .. code-block:: scheme

      --> (car (cons 1 2))
        1
      --> (car '(1 2 3))
        1

.. _proc:cdr:

cdr
^^^

.. function:: (cdr pair)

   Returns the second element of the pair.

   :param pair: The pair to operate on.
   :type pair: pair
   :return: The second element (cdr) of the pair.
   :rtype: any
   :raises: Type error if *pair* is not a pair.

   **Example:**

   .. code-block:: scheme

      --> (cdr (cons 1 2))
        2
      --> (cdr '(1 2 3 ))
        (2 3)

.. _proc:caar:

caar
^^^^

.. function:: (caar pair)

    Returns the car of the car of *pair*. Equivalent to ``(car (car pair))``.

    :param pair: A pair whose car is also a pair.
    :type pair: pair
    :return: The car of the car of *pair*.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (caar '((1 2) 3))
      1


.. _proc:cadr:

cadr
^^^^

.. function:: (cadr pair)

    Returns the car of the cdr of *pair*. Equivalent to ``(car (cdr pair))``.
    For a proper list, this returns the second element.

    :param pair: A pair whose cdr is also a pair.
    :type pair: pair
    :return: The car of the cdr of *pair*.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (cadr '(1 2 3))
      2


.. _proc:cdar:

cdar
^^^^

.. function:: (cdar pair)

    Returns the cdr of the car of *pair*. Equivalent to ``(cdr (car pair))``.

    :param pair: A pair whose car is also a pair.
    :type pair: pair
    :return: The cdr of the car of *pair*.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (cdar '((1 2) 3))
      (2)


.. _proc:cddr:

cddr
^^^^

.. function:: (cddr pair)

    Returns the cdr of the cdr of *pair*. Equivalent to ``(cdr (cdr pair))``.
    For a proper list, this returns the list with its first two elements removed.

    :param pair: A pair whose cdr is also a pair.
    :type pair: pair
    :return: The cdr of the cdr of *pair*.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (cddr '(1 2 3))
      (3)

.. _proc:list:

list
^^^^

.. function:: (list obj ...)

    Returns a newly allocated list whose elements are the given arguments, in order.
    If no arguments are provided, returns the empty list.

    :param obj: Zero or more objects to collect into a list.
    :type obj: any
    :return: A new list containing *obj* ...
    :rtype: list

    **Example:**

    .. code-block:: scheme

      --> (list 1 2 3)
      (1 2 3)
      --> (list 'a "hello" #t)
      (a "hello" #t)
      --> (list)
      ()


.. _proc:set-car!:

set-car!
^^^^^^^^

.. function:: (set-car! pair obj)

    Stores *obj* in the car field of *pair*, mutating it in place.
    Returns an unspecified value.

    :param pair: The pair to mutate.
    :type pair: pair
    :param obj: The new value for the car field.
    :type obj: any
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

      --> (define p (cons 1 2))
      --> p
      (1 . 2)
      --> (set-car! p 99)
      --> p
      (99 . 2)


.. _proc:set-cdr!:

set-cdr!
^^^^^^^^

.. function:: (set-cdr! pair obj)

    Stores *obj* in the cdr field of *pair*, mutating it in place.
    Returns an unspecified value.

    :param pair: The pair to mutate.
    :type pair: pair
    :param obj: The new value for the cdr field.
    :type obj: any
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

      --> (define p (cons 1 2))
      --> p
      (1 . 2)
      --> (set-cdr! p '(2 3))
      --> p
      (1 2 3)


.. _proc:length:

length
^^^^^^

.. function:: (length list)

    Returns the number of elements in *list*. Raises an error if *list* is an
    improper list or a circular list.

    :param list: The list to measure.
    :type list: list
    :return: The number of elements in *list*.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (length '(1 2 3))
      3
      --> (length '())
      0


.. _proc:list-ref:

list-ref
^^^^^^^^

.. function:: (list-ref list k)

    Returns the *k*-th element of *list*, where the first element is at index 0.
    Equivalent to ``(car (list-tail list k))``. Raises an error if *k* is
    negative or out of range.

    :param list: The list to index into.
    :type list: list
    :param k: The zero-based index of the element to retrieve.
    :type k: integer
    :return: The element at index *k*.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (list-ref '(a b c d) 0)
      a
      --> (list-ref '(a b c d) 2)
      c

.. _proc:append:

append
^^^^^^

.. function:: (append list ... [obj])

    Returns a list consisting of the elements of each *list* argument followed by
    the elements of the next. All arguments except the last must be proper lists.
    The last argument may be of any type; if it is not a proper list, the result
    will be an improper list.

    The resulting list is newly allocated, except that it shares structure with
    the last argument. If no arguments are provided, the empty list is returned.
    If exactly one argument is provided, it is returned unchanged.

    :param list: Zero or more proper lists to append.
    :type list: list
    :param obj: The final argument, which may be of any type.
    :type obj: any
    :return: A new list formed by appending all arguments, sharing structure with
             the last argument.
    :rtype: list

    **Example:**

    .. code-block:: scheme

      --> (append '(1 2) '(3 4))
      (1 2 3 4)
      --> (append '(a b) '(c d) '(e f))
      (a b c d e f)
      --> (append '(1 2) 3)
      (1 2 . 3)
      --> (append '() '(1 2))
      (1 2)
      --> (append)
      ()
      --> (append '(1 2))
      (1 2)


.. _proc:reverse:

reverse
^^^^^^^

.. function:: (reverse list)

    Returns a newly allocated list consisting of the elements of *list* in
    reverse order. Raises an error if *list* is an improper list.

    :param list: The list to reverse.
    :type list: list
    :return: A new list with the elements of *list* in reverse order.
    :rtype: list

    **Example:**

    .. code-block:: scheme

      --> (reverse '(1 2 3))
      (3 2 1)
      --> (reverse '(a (b c) d))
      (d (b c) a)
      --> (reverse '())
      ()


.. _proc:list-tail:

list-tail
^^^^^^^^^

.. function:: (list-tail list k)

    Returns the sublist of *list* obtained by omitting the first *k* elements.
    Raises an error if *k* is negative or greater than the length of *list*.

    :param list: The list to take a tail of.
    :type list: list
    :param k: The number of leading elements to omit.
    :type k: integer
    :return: The sublist of *list* starting at index *k*.
    :rtype: list

    **Example:**

    .. code-block:: scheme

      --> (list-tail '(a b c d) 0)
      (a b c d)
      --> (list-tail '(a b c d) 2)
      (c d)
      --> (list-tail '(a b c d) 4)
      ()


.. _proc:make-list:

make-list
^^^^^^^^^

.. function:: (make-list k [fill])

    Returns a newly allocated list of *k* elements. If *fill* is provided, every
    element is initialised to *fill*. Otherwise, every element is initialised to
    ``0``. Raises an error if *k* is not a positive integer.

    :param k: The number of elements in the new list.
    :type k: integer
    :param fill: The value to fill each element with. Defaults to ``0``.
    :type fill: any
    :return: A new list of *k* elements.
    :rtype: list

    **Example:**

    .. code-block:: scheme

      --> (make-list 4)
      (0 0 0 0)
      --> (make-list 3 'x)
      (x x x)
      --> (make-list 5 #f)
      (#f #f #f #f #f)


.. _proc:list-set!:

list-set!
^^^^^^^^^

.. function:: (list-set! list k obj)

    Stores *obj* in element *k* of *list*, mutating the list in place. Raises an
    error if *k* is negative or out of range. Returns an unspecified value.

    :param list: The list to mutate.
    :type list: list
    :param k: The zero-based index of the element to replace.
    :type k: integer
    :param obj: The new value to store at index *k*.
    :type obj: any
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

      --> (define lst '(a b c d))
      --> (list-set! lst 2 'z)
      --> lst
      (a b z d)

.. _proc:memq:

memq
^^^^

.. function:: (memq obj list)

    Returns the first sublist of *list* whose car is *obj*, or ``#f`` if *obj*
    does not occur in *list*. Uses ``eq?`` (pointer equality) for comparisons.

    :param obj: The object to search for.
    :type obj: any
    :param list: The list to search.
    :type list: list
    :return: The first sublist whose car is *obj*, or ``#f`` if not found.
    :rtype: list or boolean

    **Example:**

    .. code-block:: scheme

      --> (memq 'b '(a b c))
      (b c)
      --> (memq 'd '(a b c))
      #f
      --> (memq '() '(a () b))
      (() b)


.. _proc:memv:

memv
^^^^

.. function:: (memv obj list)

    Returns the first sublist of *list* whose car is *obj*, or ``#f`` if *obj*
    does not occur in *list*. Uses ``eqv?`` for comparisons, making it suitable
    for numbers and characters where ``eq?`` may not be reliable.

    :param obj: The object to search for.
    :type obj: any
    :param list: The list to search.
    :type list: list
    :return: The first sublist whose car is *obj*, or ``#f`` if not found.
    :rtype: list or boolean

    **Example:**

    .. code-block:: scheme

      --> (memv 2 '(1 2 3))
      (2 3)
      --> (memv 5 '(1 2 3))
      #f


.. _proc:member:

member
^^^^^^

.. function:: (member obj list [compare])

    Returns the first sublist of *list* whose car is *obj*, or ``#f`` if *obj*
    does not occur in *list*. Uses ``equal?`` for comparisons by default. If
    *compare* is provided, it is used in place of ``equal?``; it is called with
    *obj* and the current list element as arguments, and should return a true
    value on a match.

    :param obj: The object to search for.
    :type obj: any
    :param list: The list to search.
    :type list: list
    :param compare: A comparison procedure of two arguments. Defaults to ``equal?``.
    :type compare: procedure
    :return: The first sublist whose car matches *obj*, or ``#f`` if not found.
    :rtype: list or boolean

    **Example:**

    .. code-block:: scheme

      --> (member '(b) '((a) (b) (c)))
      ((b) (c))
      --> (member 2.0 '(1 2 3))
      #f
      --> (member 2.0 '(1 2 3) =)
      (2 3)


.. _proc:assq:

assq
^^^^

.. function:: (assq obj alist)

    Returns the first pair in association list *alist* whose car is *obj*, or
    ``#f`` if no such pair exists. Uses ``eq?`` (pointer equality) for
    comparisons. Raises an error if *alist* is not a proper association list.

    :param obj: The key to search for.
    :type obj: any
    :param alist: An association list to search.
    :type alist: list
    :return: The first pair whose car is *obj*, or ``#f`` if not found.
    :rtype: pair or boolean

    **Example:**

    .. code-block:: scheme

      --> (assq 'b '((a 1) (b 2) (c 3)))
      (b 2)
      --> (assq 'd '((a 1) (b 2) (c 3)))
      #f


.. _proc:assv:

assv
^^^^

.. function:: (assv obj alist)

    Returns the first pair in association list *alist* whose car is *obj*, or
    ``#f`` if no such pair exists. Uses ``eqv?`` for comparisons, making it
    suitable for numeric keys where ``eq?`` may not be reliable. Raises an error
    if *alist* is not a proper association list.

    :param obj: The key to search for.
    :type obj: any
    :param alist: An association list to search.
    :type alist: list
    :return: The first pair whose car is *obj*, or ``#f`` if not found.
    :rtype: pair or boolean

    **Example:**

    .. code-block:: scheme

      --> (assv 2 '((1 a) (2 b) (3 c)))
      (2 b)
      --> (assv 5 '((1 a) (2 b) (3 c)))
      #f


.. _proc:assoc:

assoc
^^^^^

.. function:: (assoc obj alist [compare])

    Returns the first pair in association list *alist* whose car is *obj*, or
    ``#f`` if no such pair exists. Uses ``equal?`` for comparisons by default.
    If *compare* is provided, it is used in place of ``equal?``; it is called
    with the current alist key and *obj* as arguments, and should return a true
    value on a match. Raises an error if *alist* is not a proper association
    list, or if *compare* is not a procedure.

    :param obj: The key to search for.
    :type obj: any
    :param alist: An association list to search.
    :type alist: list
    :param compare: A comparison procedure of two arguments. Defaults to ``equal?``.
    :type compare: procedure
    :return: The first pair whose car matches *obj*, or ``#f`` if not found.
    :rtype: pair or boolean

    **Example:**

    .. code-block:: scheme

      --> (assoc '(b) '((a 1) ((b) 2) (c 3)))
      ((b) 2)
      --> (assoc 2.0 '((1 a) (2 b) (3 c)) =)
      (2 b)
      --> (assoc 'z '((a 1) (b 2)))
      #f


.. _proc:list-copy:

list-copy
^^^^^^^^^

.. function:: (list-copy obj)

    Returns a newly allocated copy of *obj* if it is a list. Only the pairs
    themselves are copied; the cars of the result are the same (in the sense of
    ``eqv?``) as the cars of *obj*. If *obj* is an improper list, the result is
    likewise improper, with the final cdr shared with the original. If *obj* is
    not a list, it is returned unchanged.

    :param obj: The object to copy.
    :type obj: any
    :return: A fresh copy of *obj* if it is a list, or *obj* itself if it is not
             a list.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (define original '(1 2 3))
      --> (define copy (list-copy original))
      --> copy
      (1 2 3)
      --> (set-car! copy 99)
      --> copy
      (99 2 3)
      --> original
      (1 2 3)
      --> (list-copy 42)
      42
      --> (list-copy '(1 2 . 3))
      (1 2 . 3)


.. _proc:map:

map
^^^

.. function:: (map proc list ...)

    Applies *proc* element-wise to the elements of each *list* argument and
    returns a newly allocated list of the results, in order. *proc* must accept
    as many arguments as there are lists. If more than one list is given and
    they differ in length, iteration stops when the shortest list is exhausted.

    :param proc: A procedure accepting as many arguments as there are lists.
    :type proc: procedure
    :param list: One or more proper lists to map over.
    :type list: list
    :return: A new list of the results of applying *proc* to each set of
             elements.
    :rtype: list

    **Example:**

    .. code-block:: scheme

      --> (map car '((a b) (c d) (e f)))
      (a c e)
      --> (map (lambda (x) (* x x)) '(1 2 3 4 5))
      (1 4 9 16 25)
      --> (map + '(1 2 3) '(10 20 30))
      (11 22 33)
      --> (map + '(1 2 3) '(10 20))
      (11 22)


.. _proc:for-each:

for-each
^^^^^^^^

.. function:: (for-each proc list ...)

    Applies *proc* element-wise to the elements of each *list* argument, in
    order, for its side effects. Unlike ``map``, the results of *proc* are
    discarded. *proc* must accept as many arguments as there are lists. If more
    than one list is given and they differ in length, iteration stops when the
    shortest list is exhausted. Returns an unspecified value.

    :param proc: A procedure accepting as many arguments as there are lists.
    :type proc: procedure
    :param list: One or more proper lists to iterate over.
    :type list: list
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

      --> (for-each display '(1 2 3))
      123
      --> (define result '())
      --> (for-each
      ...   (lambda (x) (set! result (cons (* x x) result)))
      ...   '(1 2 3))
      --> result
      (9 4 1)


.. _proc:filter:

filter
^^^^^^

.. function:: (filter proc list)

    Returns a newly allocated list containing only the elements of *list* for
    which *proc* returns a true value, preserving the original order. *list*
    must be a proper list.

    :param proc: A predicate procedure of one argument.
    :type proc: procedure
    :param list: The list to filter.
    :type list: list
    :return: A new list of elements from *list* for which *proc* returned a true
             value.
    :rtype: list

    **Example:**

    .. code-block:: scheme

      --> (filter odd? '(1 2 3 4 5))
      (1 3 5)
      --> (filter string? '(1 "a" 2 "b" 3))
      ("a" "b")
      --> (filter (lambda (x) (> x 3)) '(1 2 3 4 5))
      (4 5)
      --> (filter odd? '())
      ()


.. _proc:foldl:

foldl
^^^^^

.. function:: (foldl proc init list ...)

    Applies *proc* to an accumulator and the corresponding elements of each
    *list*, processing from left to right. *proc* is called as
    ``(proc acc elem1 elem2 ...)``, where ``acc`` is the current accumulator
    value (initially *init*) and ``elem1``, ``elem2``, ... are the current
    elements drawn from each *list* argument. The return value of each call
    becomes the new accumulator. When the shortest list is exhausted, the final
    accumulator value is returned.

    If any *list* is empty, *init* is returned immediately. All *list* arguments
    must be proper lists.

    Note that the accumulator is passed as the *first* argument to *proc*,
    followed by the list elements.

    :param proc: A procedure of ``(1 + n)`` arguments, where ``n`` is the number
                 of list arguments.
    :type proc: procedure
    :param init: The initial accumulator value.
    :type init: any
    :param list: One or more proper lists to fold over.
    :type list: list
    :return: The final accumulator value after processing all elements.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (foldl + 0 '(1 2 3 4 5))
      15
      --> (foldl cons '() '(1 2 3))
      (((()) . 1) . 2) . 3)
      --> (foldl (lambda (acc x) (+ acc (* x x))) 0 '(1 2 3))
      14
      --> (foldl + 0 '(1 2 3) '(10 20 30))
      66


.. _proc:foldr:

foldr
^^^^^

.. function:: (foldr proc init list ...)

    Applies *proc* to the corresponding elements of each *list* and an
    accumulator, processing from right to left. *proc* is called as
    ``(proc elem1 elem2 ... acc)``, where ``elem1``, ``elem2``, ... are the
    current elements drawn from each *list* argument and ``acc`` is the current
    accumulator value (initially *init*). The return value of each call becomes
    the new accumulator. When the shortest list is exhausted, the final
    accumulator value is returned.

    If any *list* is empty, *init* is returned immediately. All *list* arguments
    must be proper lists.

    Note that the accumulator is passed as the *last* argument to *proc*,
    preceded by the list elements. This is the opposite convention to
    ``foldl``.

    :param proc: A procedure of ``(1 + n)`` arguments, where ``n`` is the number
                 of list arguments.
    :type proc: procedure
    :param init: The initial accumulator value.
    :type init: any
    :param list: One or more proper lists to fold over.
    :type list: list
    :return: The final accumulator value after processing all elements.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (foldr cons '() '(1 2 3))
      (1 2 3)
      --> (foldr + 0 '(1 2 3 4 5))
      15
      --> (foldr (lambda (x acc) (cons (* x x) acc)) '() '(1 2 3))
      (1 4 9)
      --> (foldr cons '() '(1 2 3) '(10 20 30))
      ((1 . 10) (2 . 20) (3 . 30))


.. _proc:zip:

zip
^^^

.. function:: (zip list ...)

    Returns a list of sublists, where the *i*-th sublist contains the *i*-th
    element from each *list* argument. If the input lists are of different
    lengths, the result is truncated to the length of the shortest list. If any
    list is empty, or if no arguments are provided, the empty list is returned.
    All arguments must be proper lists.

    :param list: One or more proper lists to zip together.
    :type list: list
    :return: A new list of sublists, each containing the corresponding elements
             from the input lists.
    :rtype: list

    **Example:**

    .. code-block:: scheme

      --> (zip '(1 2 3) '(a b c))
      ((1 a) (2 b) (3 c))
      --> (zip '(1 2 3) '(a b c) '(x y z))
      ((1 a x) (2 b y) (3 c z))
      --> (zip '(1 2 3) '(a b))
      ((1 a) (2 b))
      --> (zip '())
      ()


.. _proc:count:

count
^^^^^

.. function:: (count proc list)

    Returns the number of elements in *list* for which the predicate *proc*
    returns ``#t``. *proc* must be a predicate procedure that returns a boolean
    value; an error is raised if it returns a non-boolean result.

    :param proc: A predicate procedure of one argument.
    :type proc: procedure
    :param list: The list to count elements in.
    :type list: list
    :return: The number of elements in *list* for which *proc* returned ``#t``.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (count odd? '(1 2 3 4 5))
      3
      --> (count string? '(1 "a" 2 "b"))
      2
      --> (count (lambda (x) (> x 3)) '(1 2 3 4 5))
      2
      --> (count odd? '())
      0


.. _proc:count-equal:

count-equal
^^^^^^^^^^^

.. function:: (count-equal obj list)

    Returns the number of occurrences of *obj* in *list*, using ``equal?`` for
    comparisons.

    :param obj: The object to count occurrences of.
    :type obj: any
    :param list: The list to search.
    :type list: list
    :return: The number of elements in *list* that are equal to *obj*.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (count-equal 1 '(1 2 1 3 1))
      3
      --> (count-equal 'x '(a b c))
      0
      --> (count-equal '(1 2) '((1 2) (3 4) (1 2)))
      2

