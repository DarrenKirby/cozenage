Hashes
======

Overview
--------

A ``hash`` is a collection of key–value associations. Each key in a hash is associated with exactly one value, and no
duplicate keys are permitted. Conceptually, a hash allows you to “look up” a value by providing its corresponding key.
Hashes are sometimes referred to as dictionaries, hashmaps, tables, or associative arrays in other languages. In this
implementation, hashes are constructed using the ``hash`` procedure and manipulated with procedures such as ``hash-get``,
``hash-add!``, and ``hash-remove!``.

In mathematical terms, a hash represents a function from a set of keys to a set of values. Each key uniquely determines
its associated value. Unlike lists, where values are accessed by numeric position, hashes are accessed by key. For
example, instead of asking “what is the third element?”, you ask “what value is associated with this key?” This
makes hashes especially useful when data is naturally described by named or symbolic attributes, such as configuration
settings, record fields, caches, lookup tables, frequency counters, or indexing structures.

Internally, hashes are implemented using a hash table. A hash table stores key–value pairs in an array and uses a hash
function to convert each key into a numeric index. When a key is inserted or looked up, the hash function determines
where it should be stored or searched for. Because this computation is fast and avoids scanning the entire collection,
hash tables typically provide very efficient average-case performance for insertion, deletion, and lookup operations.
In practice, these operations are close to constant time for well-distributed hash functions. This makes hashes far more
efficient than linear data structures such as lists when frequent lookups are required.

In order for a key to be stored in a hash, it must be hashable. Hashable objects are those whose value can be converted
into a stable hash code suitable for indexing. The following types are hashable:

* string
* symbol
* integer
* rational
* real
* complex
* boolean
* char

Keys must be one of these types. Attempting to use a non-hashable object as a key will raise a type error. In contrast,
hash values may be any first-class object, including lists, vectors, sets, hashes, procedures, numbers, strings, or
user-defined types. Only keys are restricted; values are unrestricted.

Hashes should be distinguished from association lists (alists). An association list is simply a list of dotted pairs,
where each pair contains a key in its car and a value in its cdr. For example:

.. code-block:: scheme

    --> '((name . "Alice")
    ...    (age . 30)
    ...    (city . "Toronto"))
    ((name . "Alice") (age . 30) (city . "Toronto"))

Alists are simple, portable, and easy to manipulate using standard list procedures. However, alist lookups require
scanning the list sequentially, which becomes slower as the list grows. Hashes, by contrast, use hashing to provide
significantly faster lookups for larger collections. As a rule of thumb:

* Use an alist for small collections or when preserving insertion order is important.
* Use a hash when you expect many lookups, larger data sets, or performance-sensitive operations.

Hashes do not guarantee any ordering of keys or values. When retrieving keys or values using hash-keys or hash-values,
the order is indeterminate and should not be relied upon. If a specific ordering is required, convert the result to a
list and sort it explicitly.

As with sets, procedures ending in ! mutate the original hash, while procedures without ! return newly allocated objects.
Understanding this distinction is important when writing programs that rely on persistent data structures versus
in-place modification.

Hash Procedures
---------------

.. _proc:hash:

hash
****

.. function:: (hash key value ...)

    Returns a newly allocated hash containing the key–value pairs supplied as
    alternating arguments. Keys must be hashable types; values may be any
    object. If no arguments are supplied, an empty hash is returned. It is an
    error if an odd number of arguments is supplied, or if any key is not a
    hashable type.

    :param key: A hashable key.
    :param value: The value to associate with the preceding key.
    :return: A newly allocated hash.
    :rtype: hash

    **Example:**

    .. code-block:: scheme

        --> (hash 'name "Alice" 'age 30 'city "Toronto")
        #[name "Alice" age 30 city "Toronto"]
        --> (hash)
        #[]
        --> (hash-get (hash 'x 42) 'x)
        42


.. _proc:hash-copy:

hash-copy
*********

.. function:: (hash-copy hash)

    Returns a newly allocated copy of *hash*. The copy is shallow — the keys
    and values themselves are not copied, only the references to them.
    It is an error if *hash* is not a hash.

    :param hash: The hash to copy.
    :type hash: hash
    :return: A newly allocated copy of *hash*.
    :rtype: hash

    **Example:**

    .. code-block:: scheme

        --> (define h (hash 'a 1 'b 2))
        --> (define h2 (hash-copy h))
        --> (hash-add! h2 'c 3)
        --> (hash-keys h2)
        (a b c)
        --> (hash-keys h)
        (a b)


.. _proc:hash-clear!:

hash-clear!
***********

.. function:: (hash-clear! hash)

    Removes all key–value pairs from *hash*, mutating it in place. Returns
    the same hash object, now empty. It is an error if *hash* is not a hash.

    :param hash: The hash to clear.
    :type hash: hash
    :return: The mutated hash, now empty.
    :rtype: hash

    **Example:**

    .. code-block:: scheme

        --> (define h (hash 'a 1 'b 2 'c 3))
        --> (hash-clear! h)
        #[]
        --> h
        #[]


.. _proc:hash-get:

hash-get
********

.. function:: (hash-get hash key [default])

    Returns the value associated with *key* in *hash*. If *key* is not found
    and a *default* value is supplied, *default* is returned. If *key* is not
    found and no *default* is supplied, an index error is signalled. It is an
    error if *hash* is not a hash, or if *key* is not a hashable type.

    :param hash: The hash to look up.
    :type hash: hash
    :param key: The key to look up.
    :param default: A value to return if *key* is not found. Optional.
    :return: The value associated with *key*, or *default* if not found.

    **Example:**

    .. code-block:: scheme

        --> (define h (hash 'a 1 'b 2 'c 3))
        --> (hash-get h 'b)
        2
        --> (hash-get h 'z)
        Index error:  hash-get: object not found in hash
        --> (hash-get h 'z 'not-found)
        not-found


.. _proc:hash-add!:

hash-add!
*********

.. function:: (hash-add! hash key value)

    Adds *key* with associated *value* to *hash*, mutating it in place, and
    returns the mutated hash. If *key* already exists in *hash*, its value is
    updated. It is an error if *hash* is not a hash, or if *key* is not a
    hashable type.

    :param hash: The hash to add to.
    :type hash: hash
    :param key: The key to add.
    :param value: The value to associate with *key*.
    :return: The mutated hash.
    :rtype: hash

    **Example:**

    .. code-block:: scheme

        --> (define h (hash 'a 1))
        --> (hash-add! h 'b 2)
        --> h
        #[a 1 b 2]
        --> (hash-add! h 'a 99)
        --> (hash-get h 'a)
        99


.. _proc:hash-remove!:

hash-remove!
************

.. function:: (hash-remove! hash key [sym])

    Removes *key* and its associated value from *hash*, mutating it in place,
    and returns the mutated hash. Signals an index error if *key* is not found
    in *hash*. If an optional symbol is supplied as a third argument, the
    error is suppressed and the procedure returns silently instead. It is an
    error if *hash* is not a hash.

    :param hash: The hash to remove from.
    :type hash: hash
    :param key: The key to remove.
    :param sym: An optional symbol. If provided, suppresses the error when
                *key* is not found.
    :type sym: symbol
    :return: The mutated hash.
    :rtype: hash

    **Example:**

    .. code-block:: scheme

        --> (define h (hash 'a 1 'b 2 'c 3))
        --> (hash-remove! h 'b)
        --> h
        #[a 1 c 3]
        --> (hash-remove! h 'z)
        error: index-error
        --> (hash-remove! h 'z 'ok)
        --> h
        #[a 1 c 3]


.. _proc:hash-keys:

hash-keys
*********

.. function:: (hash-keys hash)

    Returns a newly allocated list containing all keys in *hash*. The order
    of keys in the returned list is indeterminate, as hashes are unordered.
    It is an error if *hash* is not a hash.

    :param hash: The hash to retrieve keys from.
    :type hash: hash
    :return: A list of all keys in *hash*.
    :rtype: list

    **Example:**

    .. code-block:: scheme

        --> (define h (hash 'a 1 'b 2 'c 3))
        --> (hash-keys h)
        (a b c)
        --> (hash-keys (hash))
        ()


.. _proc:hash-values:

hash-values
***********

.. function:: (hash-values hash)

    Returns a newly allocated list containing all values in *hash*. The order
    of values in the returned list is indeterminate, as hashes are unordered.
    It is an error if *hash* is not a hash.

    :param hash: The hash to retrieve values from.
    :type hash: hash
    :return: A list of all values in *hash*.
    :rtype: list

    **Example:**

    .. code-block:: scheme

        --> (define h (hash 'a 1 'b 2 'c 3))
        --> (hash-values h)
        (1 2 3)
        --> (hash-values (hash))
        ()


.. _proc:hash->alist:

hash->alist
***********

.. function:: (hash->alist hash)

    Returns a newly allocated association list (alist) containing all
    key–value pairs in *hash*. Each element of the returned list is a dotted
    pair of the form ``(key . value)``. The order of pairs in the returned
    list is indeterminate. It is an error if *hash* is not a hash.

    :param hash: The hash to convert.
    :type hash: hash
    :return: An alist of ``(key . value)`` pairs.
    :rtype: list

    **Example:**

    .. code-block:: scheme

        --> (define h (hash 'a 1 'b 2))
        --> (hash->alist h)
        ((a . 1) (b . 2))


.. _proc:alist->hash:

alist->hash
***********

.. function:: (alist->hash alist)

    Returns a newly allocated hash constructed from the key–value pairs in
    *alist*. Each element of *alist* must be a dotted pair of the form
    ``(key . value)``; the ``car`` becomes the key and the ``cdr`` becomes
    the value. It is an error if *alist* is not a proper list, or if any
    element is not a dotted pair.

    :param alist: An association list of ``(key . value)`` dotted pairs.
    :type alist: list
    :return: A newly allocated hash.
    :rtype: hash

    **Example:**

    .. code-block:: scheme

        --> (define al '((a . 1) (b . 2) (c . 3)))
        --> (define h (alist->hash al))
        --> h
        #[a 1 b 2 c 3]
        --> (hash-get h 'b)
        2


.. _proc:hash-keys-map:

hash-keys-map
*************

.. function:: (hash-keys-map proc hash)

    Applies *proc* to each key in *hash* and returns a list of the results.
    The order in which *proc* is applied is indeterminate, as hashes are
    unordered. *proc* must accept exactly one argument. It is an error if
    *proc* is not a procedure or *hash* is not a hash.

    :param proc: A procedure of one argument.
    :type proc: procedure
    :param hash: The hash whose keys to map over.
    :type hash: hash
    :return: A list of results of applying *proc* to each key.
    :rtype: list

    **Example:**

    .. code-block:: scheme

        --> (define h (hash 'a 1 'b 2 'c 3))
        --> (hash-keys-map symbol->string h)
        ("a" "b" "c")


.. _proc:hash-keys-foreach:

hash-keys-foreach
*****************

.. function:: (hash-keys-foreach proc hash)

    Applies *proc* to each key in *hash* for the purpose of side effects.
    The return values of *proc* are discarded. The order of applications is
    indeterminate. Returns an unspecified value. *proc* must accept exactly
    one argument. It is an error if *proc* is not a procedure or *hash* is
    not a hash.

    :param proc: A procedure of one argument.
    :type proc: procedure
    :param hash: The hash whose keys to iterate over.
    :type hash: hash
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

        --> (define h (hash 'a 1 'b 2 'c 3))
        --> (hash-keys-foreach display h)
        abc


.. _proc:hash-values-map:

hash-values-map
***************

.. function:: (hash-values-map proc hash)

    Applies *proc* to each value in *hash* and returns a list of the results.
    The order in which *proc* is applied is indeterminate, as hashes are
    unordered. *proc* must accept exactly one argument. It is an error if
    *proc* is not a procedure or *hash* is not a hash.

    :param proc: A procedure of one argument.
    :type proc: procedure
    :param hash: The hash whose values to map over.
    :type hash: hash
    :return: A list of results of applying *proc* to each value.
    :rtype: list

    **Example:**

    .. code-block:: scheme

        --> (define h (hash 'a 1 'b 2 'c 3))
        --> (hash-values-map (lambda (x) (* x 10)) h)
        (10 20 30)


.. _proc:hash-values-foreach:

hash-values-foreach
*******************

.. function:: (hash-values-foreach proc hash)

    Applies *proc* to each value in *hash* for the purpose of side effects.
    The return values of *proc* are discarded. The order of applications is
    indeterminate. Returns an unspecified value. *proc* must accept exactly
    one argument. It is an error if *proc* is not a procedure or *hash* is
    not a hash.

    :param proc: A procedure of one argument.
    :type proc: procedure
    :param hash: The hash whose values to iterate over.
    :type hash: hash
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

        --> (define h (hash 'a 1 'b 2 'c 3))
        --> (hash-values-foreach display h)
        123


.. _proc:hash-items-map:

hash-items-map
**************

.. function:: (hash-items-map proc hash)

    Applies *proc* to each key–value pair in *hash* and returns a list of the
    results. *proc* is called with two arguments for each entry: first the
    key, then the value. The order of applications is indeterminate. It is an
    error if *proc* is not a procedure or *hash* is not a hash.

    :param proc: A procedure of two arguments.
    :type proc: procedure
    :param hash: The hash whose items to map over.
    :type hash: hash
    :return: A list of results of applying *proc* to each key–value pair.
    :rtype: list

    **Example:**

    .. code-block:: scheme

        --> (define h (hash 'a 1 'b 2 'c 3))
        --> (hash-items-map
        ...   (lambda (k v) (cons k (* v 10)))
        ...   h)
        ((a . 10) (b . 20) (c . 30))


.. _proc:hash-items-foreach:

hash-items-foreach
******************

.. function:: (hash-items-foreach proc hash)

    Applies *proc* to each key–value pair in *hash* for the purpose of side
    effects. *proc* is called with two arguments for each entry: first the
    key, then the value. The return values of *proc* are discarded. The order
    of applications is indeterminate. Returns an unspecified value. It is an
    error if *proc* is not a procedure or *hash* is not a hash.

    :param proc: A procedure of two arguments.
    :type proc: procedure
    :param hash: The hash whose items to iterate over.
    :type hash: hash
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

        --> (define h (hash 'a 1 'b 2 'c 3))
        --> (hash-items-foreach
        ...   (lambda (k v)
        ...     (display k) (display ":") (display v) (display " "))
        ...   h)
        a:1 b:2 c:3

