Hashes
======

Overview
--------

A `hash` is a collection of key–value associations. Each key in a hash is associated with exactly one value, and no
duplicate keys are permitted. Conceptually, a hash allows you to “look up” a value by providing its corresponding key.
Hashes are sometimes referred to as dictionaries, hashs, tables, or associative arrays in other languages. In this
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
    ...      (age . 30)
    ...      (city . "Toronto"))

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

.. function:: (hash obj1 obj2 ...)

    Constructs a new hash from key–value pairs supplied as arguments.

    Arguments must be provided in pairs, where each key is followed by its associated value.
    Keys must be hashable. Values may be any first-class object.

    :param obj1: First key.
    :type obj1: hashable
    :param obj2: Value associated with first key.
    :type obj2: any
    :param ...: Additional key–value pairs.
    :type ...: alternating hashable / any
    :return: A newly allocated hash.
    :rtype: hash

    Example:

    .. code-block:: text

      --> (hash 1 "one" 2 "two")
      #[1 "one" 2 "two"]


.. _proc:hash-copy:

hash-copy
*********

.. function:: (hash-copy hash)

    Returns a shallow copy of hash.

    The hash structure is duplicated, but keys and values themselves are not copied.

    :param hash: The hash to copy.
    :type hash: hash
    :return: A newly allocated hash containing the same key–value pairs.
    :rtype: hash

    Example:

    .. code-block:: text

      --> (hash-copy #[1 "one"])
      #[1 "one"]


.. _proc:hash-clear!:

hash-clear
**********

.. function:: (hash-clear! hash)

    Removes all key–value associations from hash.

    This procedure mutates the original hash.

    :param hash: The hash to clear.
    :type hash: hash
    :return: The same hash object, now empty.
    :rtype: hash

    Example:

    .. code-block:: text

      --> (define m (make-hash 1 "one"))
      --> (hash-clear! m)
        #[]
      --> m
        #[]


.. _proc:hash-get:

hash-get
********

.. function:: (hash-get hash key [default])

    Returns the value associated with key in hash.

    If key is not present and no default value is supplied, an index error is raised.
    If default is provided, it is returned instead.

    :param hash: The hash to search.
    :type hash: hash
    :param key: The lookup key (must be hashable).
    :type key: hashable
    :param default: Optional value returned if key is not present.
    :type default: any
    :return: The associated value, or default if provided.
    :rtype: any

    Example:

    .. code-block:: text

      --> (hash-get #[1 "one" 2 "two"] 1)
        "one"
      --> (hash-get #[1 "one"] 3 "unknown")
        "unknown"


.. _proc:hash-add!:

hash-add!
*********

.. function:: (hash-add! hash key value)

    Adds or updates a key–value association in hash.

    If the key already exists, its value is replaced.
    This procedure mutates the hash.

    :param hash: The hash to modify.
    :type hash: hash
    :param key: The key (must be hashable).
    :type key: hashable
    :param value: The value to associate with key.
    :type value: any
    :return: The modified hash.
    :rtype: hash

    Example:

    .. code-block:: text

      --> (define m #[])
      --> (hash-add! m 1 "one")
        #[1 "one"]
      --> (hash-add! m 1 "uno")
        #[1 "uno"]


.. _proc:hash-remove!:

hash-remove!
************

.. function:: (hash-remove! hash key [sym])

    Removes key and its associated value from hash.

    If key is not present and no optional symbol is supplied,
    an index error is raised. If a symbol is provided as the
    third argument, the procedure returns silently instead.

    This procedure mutates the hash.

    :param hash: The hash to modify.
    :type hash: hash
    :param key: The key to remove.
    :type key: hashable
    :param sym: Optional symbol suppressing index error.
    :type sym: symbol
    :return: The modified hash.
    :rtype: hash

    Example:

    .. code-block:: text

      --> (define m #[1 "one" 2 "two"])
      --> (hash-remove! m 2)
        #[1 "one"]


.. _proc:hash-keys:

hash-keys
*********

.. function:: (hash-keys hash)

    Returns a list containing all keys in hash.

    The order of keys is indeterminate.

    :param hash: The hash to inspect.
    :type hash: hash
    :return: A list of keys.
    :rtype: list

    Example:

    .. code-block:: text

      --> (hash-keys #[1 "one" 2 "two"])
        (1 2)


.. _proc:hash-values:

hash-values
***********

.. function:: (hash-values hash)

    Returns a list containing all values in hash.

    The order of values is indeterminate.

    :param hash: The hash to inspect.
    :type hash: hash
    :return: A list of values.
    :rtype: list

    Example:

    .. code-block:: text

      --> (hash-values #[1 "one" 2 "two"])
        ("one" "two")


.. _proc:hash->alist:

hash->alist
***********

.. function:: (hash->alist hash)

    Returns an association list representing the contents of hash.

    Each element of the returned list is a dotted pair
    whose car is a key and whose cdr is its associated value.

    :param hash: The hash to convert.
    :type hash: hash
    :return: An association list.
    :rtype: list

    Example:

    .. code-block:: text

      --> (hash->alist #[1 "one"])
        ((1 . "one"))


.. _proc:alist->hash:

alist->hash
***********

.. function:: (alist->hash alist)

    Constructs a new hash from an association list.

    Each element of alist must be a dotted pair whose
    car is the key and whose cdr is the value.

    :param alist: Association list.
    :type alist: list of dotted pairs
    :return: A newly allocated hash.
    :rtype: hash

    Example:

    .. code-block:: text

      --> (alist->hash '((1 . "one") (2 . "two")))
        #[1 "one" 2 "two"]


.. _proc:hash-keys-map:

hash-keys-map
*************

.. function:: (hash-keys-map proc hash)

    Applies proc to each key in hash and returns a list of results. ``proc`` must accept one argument.

    The order of application is indeterminate.

    :param proc: Procedure accepting one argument.
    :type proc: procedure
    :param hash: The hash to traverse.
    :type hash: hash
    :return: A list of results.
    :rtype: list

.. _proc:hash-keys-foreach:

hash-keys-foreach
*****************

.. function:: (hash-keys-foreach proc hash)

    Applies proc to each key in hash for side effects.  ``proc`` must accept one argument.

    Returns #void.

    :param proc: Procedure accepting one argument.
    :type proc: procedure
    :param hash: The hash to traverse.
    :type hash: hash
    :return: #void
    :rtype: void

.. _proc:hash-values-map:

hash-values-map
***************

.. function:: (hash-values-map proc hash)

    Applies proc to each value in hash and returns a list of results. ``proc`` must accept one argument.

    :param proc: Procedure accepting one argument.
    :type proc: procedure
    :param hash: The hash to traverse.
    :type hash: hash
    :return: A list of results.
    :rtype: list

.. _proc:hash-values-foreach:

hash-values-foreach
*******************

.. function:: (hash-values-foreach proc hash)

    Applies proc to each value in hash for side effects. ``proc`` must accept one argument.

    Returns #void.

    :param proc: Procedure accepting one argument.
    :type proc: procedure
    :param hash: The hash to traverse.
    :type hash: hash
    :return: #void
    :rtype: void

.. _proc:hash-items-map:

hash-items-map
**************

.. function:: (hash-items-map proc hash)

    Applies proc to each key–value pair in hash
    and returns a list of results.

    The procedure must accept exactly two arguments:
    the key and the corresponding value.

    :param proc: Procedure accepting two arguments.
    :type proc: procedure
    :param hash: The hash to traverse.
    :type hash: hash
    :return: A list of results.
    :rtype: list

    Example:

    .. code-block:: text

      --> (hash-items-hash (lambda (k v) (string-append (number->string k) ":" v))
      ...      #[1 "one" 2 "two"])
      ("1:one" "2:two")


.. _proc:hash-items-foreach:

hash-items-foreach
******************

.. function:: (hash-items-foreach proc hash)

    Applies proc to each key–value pair in hash
    for side effects.

    The procedure must accept exactly two arguments:
    the key and the corresponding value.

    Returns #void.

    :param proc: Procedure accepting two arguments.
    :type proc: procedure
    :param hash: The hash to traverse.
    :type hash: hash
    :return: #void
    :rtype: void
