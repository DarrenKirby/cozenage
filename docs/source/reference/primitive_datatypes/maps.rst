Maps
====

Overview
--------

A `map` is a collection of key–value associations. Each key in a map is associated with exactly one value, and no
duplicate keys are permitted. Conceptually, a map allows you to “look up” a value by providing its corresponding key.
Maps are sometimes referred to as dictionaries, tables, or associative arrays in other languages. In this
implementation, maps are constructed using the make-map procedure and manipulated with procedures such as ``map-get``,
``map-add!``, and ``map-remove!``.

In mathematical terms, a map represents a function from a set of keys to a set of values. Each key uniquely determines
its associated value. Unlike lists, where values are accessed by numeric position, maps are accessed by key. For
example, instead of asking “what is the third element?”, you ask “what value is associated with this key?” This
makes maps especially useful when data is naturally described by named or symbolic attributes, such as configuration
settings, record fields, caches, lookup tables, frequency counters, or indexing structures.

Internally, maps are implemented using a hash table. A hash table stores key–value pairs in an array and uses a hash
function to convert each key into a numeric index. When a key is inserted or looked up, the hash function determines
where it should be stored or searched for. Because this computation is fast and avoids scanning the entire collection,
hash tables typically provide very efficient average-case performance for insertion, deletion, and lookup operations.
In practice, these operations are close to constant time for well-distributed hash functions. This makes maps far more
efficient than linear data structures such as lists when frequent lookups are required.

In order for a key to be stored in a map, it must be hashable. Hashable objects are those whose value can be converted
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
map values may be any first-class object, including lists, vectors, sets, maps, procedures, numbers, strings, or
user-defined types. Only keys are restricted; values are unrestricted.

Maps should be distinguished from association lists (alists). An association list is simply a list of dotted pairs,
where each pair contains a key in its car and a value in its cdr. For example:

.. code-block:: scheme

    --> '((name . "Alice")
    ...      (age . 30)
    ...      (city . "Toronto"))

Alists are simple, portable, and easy to manipulate using standard list procedures. However, alist lookups require
scanning the list sequentially, which becomes slower as the list grows. Maps, by contrast, use hashing to provide
significantly faster lookups for larger collections. As a rule of thumb:

* Use an alist for small collections or when preserving insertion order is important.
* Use a map when you expect many lookups, larger data sets, or performance-sensitive operations.

Maps do not guarantee any ordering of keys or values. When retrieving keys or values using map-keys or map-values,
the order is indeterminate and should not be relied upon. If a specific ordering is required, convert the result to a
list and sort it explicitly.

As with sets, procedures ending in ! mutate the original map, while procedures without ! return newly allocated objects.
Understanding this distinction is important when writing programs that rely on persistent data structures versus
in-place modification.

Map Procedures
--------------

.. _proc:make-map:

.. function:: (make-map obj1 obj2 ...)

    Constructs a new map from key–value pairs supplied as arguments.

    Arguments must be provided in pairs, where each key is followed by its associated value.
    Keys must be hashable. Values may be any first-class object.

    :param obj1: First key.
    :type obj1: hashable
    :param obj2: Value associated with first key.
    :type obj2: any
    :param ...: Additional key–value pairs.
    :type ...: alternating hashable / any
    :return: A newly allocated map.
    :rtype: map

    Example:

    .. code-block:: text

      --> (make-map 1 "one" 2 "two")
      #{1 "one" 2 "two"}


.. _proc:map-copy:

.. function:: (map-copy map)

    Returns a shallow copy of map.

    The map structure is duplicated, but keys and values themselves are not copied.

    :param map: The map to copy.
    :type map: map
    :return: A newly allocated map containing the same key–value pairs.
    :rtype: map

    Example:

    .. code-block:: text

      --> (map-copy #{1 "one"})
      #{1 "one"}


.. _proc:map-clear!:

.. function:: (map-clear! map)

    Removes all key–value associations from map.

    This procedure mutates the original map.

    :param map: The map to clear.
    :type map: map
    :return: The same map object, now empty.
    :rtype: map

    Example:

    .. code-block:: text

      --> (define m (make-map 1 "one"))
      --> (map-clear! m)
        #{}
      --> m
        #{}


.. _proc:map-get:

.. function:: (map-get map key [default])

    Returns the value associated with key in map.

    If key is not present and no default value is supplied, an index error is raised.
    If default is provided, it is returned instead.

    :param map: The map to search.
    :type map: map
    :param key: The lookup key (must be hashable).
    :type key: hashable
    :param default: Optional value returned if key is not present.
    :type default: any
    :return: The associated value, or default if provided.
    :rtype: any

    Example:

    .. code-block:: text

      --> (map-get #{1 "one" 2 "two"} 1)
        "one"
      --> (map-get #{1 "one"} 3 "unknown")
        "unknown"


.. _proc:map-add!:

.. function:: (map-add! map key value)

    Adds or updates a key–value association in map.

    If the key already exists, its value is replaced.
    This procedure mutates the map.

    :param map: The map to modify.
    :type map: map
    :param key: The key (must be hashable).
    :type key: hashable
    :param value: The value to associate with key.
    :type value: any
    :return: The modified map.
    :rtype: map

    Example:

    .. code-block:: text

      --> (define m #{})
      --> (map-add! m 1 "one")
        #{1 "one"}
      --> (map-add! m 1 "uno")
        #{1 "uno"}


.. _proc:map-remove!:

.. function:: (map-remove! map key [sym])

    Removes key and its associated value from map.

    If key is not present and no optional symbol is supplied,
    an index error is raised. If a symbol is provided as the
    third argument, the procedure returns silently instead.

    This procedure mutates the map.

    :param map: The map to modify.
    :type map: map
    :param key: The key to remove.
    :type key: hashable
    :param sym: Optional symbol suppressing index error.
    :type sym: symbol
    :return: The modified map.
    :rtype: map

    Example:

    .. code-block:: text

      --> (define m #{1 "one" 2 "two"})
      --> (map-remove! m 2)
        #{1 "one"}


.. _proc:map-keys:

.. function:: (map-keys map)

    Returns a list containing all keys in map.

    The order of keys is indeterminate.

    :param map: The map to inspect.
    :type map: map
    :return: A list of keys.
    :rtype: list

    Example:

    .. code-block:: text

      --> (map-keys #{1 "one" 2 "two"})
        (1 2)


.. _proc:map-values:

.. function:: (map-values map)

    Returns a list containing all values in map.

    The order of values is indeterminate.

    :param map: The map to inspect.
    :type map: map
    :return: A list of values.
    :rtype: list

    Example:

    .. code-block:: text

      --> (map-values #{1 "one" 2 "two"})
        ("one" "two")


.. _proc:map->alist:

.. function:: (map->alist map)

    Returns an association list representing the contents of map.

    Each element of the returned list is a dotted pair
    whose car is a key and whose cdr is its associated value.

    :param map: The map to convert.
    :type map: map
    :return: An association list.
    :rtype: list

    Example:

    .. code-block:: text

      --> (map->alist #{1 "one"})
        ((1 . "one"))


.. _proc:alist->map:

.. function:: (alist->map alist)

    Constructs a new map from an association list.

    Each element of alist must be a dotted pair whose
    car is the key and whose cdr is the value.

    :param alist: Association list.
    :type alist: list of dotted pairs
    :return: A newly allocated map.
    :rtype: map

    Example:

    .. code-block:: text

      --> (alist->map '((1 . "one") (2 . "two")))
        #{1 "one" 2 "two"}


.. _proc:map-keys-map:

.. function:: (map-keys-map proc map)

    Applies proc to each key in map and returns a list of results.

    The order of application is indeterminate.

    :param proc: Procedure accepting one argument.
    :type proc: procedure
    :param map: The map to traverse.
    :type map: map
    :return: A list of results.
    :rtype: list

.. _proc:map-keys-foreach:

.. function:: (map-keys-foreach proc map)

    Applies proc to each key in map for side effects.

    Returns #void.

    :param proc: Procedure accepting one argument.
    :type proc: procedure
    :param map: The map to traverse.
    :type map: map
    :return: #void
    :rtype: void

.. _proc:map-values-map:

.. function:: (map-values-map proc map)

    Applies proc to each value in map and returns a list of results.

    :param proc: Procedure accepting one argument.
    :type proc: procedure
    :param map: The map to traverse.
    :type map: map
    :return: A list of results.
    :rtype: list

.. _proc:map-values-foreach:

.. function:: (map-values-foreach proc map)

    Applies proc to each value in map for side effects.

    Returns #void.

    :param proc: Procedure accepting one argument.
    :type proc: procedure
    :param map: The map to traverse.
    :type map: map
    :return: #void
    :rtype: void

.. _proc:map-items-map:

.. function:: (map-items-map proc map)

    Applies proc to each key–value pair in map
    and returns a list of results.

    The procedure must accept exactly two arguments:
    the key and the corresponding value.

    :param proc: Procedure accepting two arguments.
    :type proc: procedure
    :param map: The map to traverse.
    :type map: map
    :return: A list of results.
    :rtype: list

    Example:

    .. code-block:: text

      --> (map-items-map (lambda (k v) (string-append (number->string k) ":" v))
      ...      #{1 "one" 2 "two"})
      ("1:one" "2:two")


.. _proc:map-items-foreach:

.. function:: (map-items-foreach proc map)

    Applies proc to each key–value pair in map
    for side effects.

    Returns #void.

    :param proc: Procedure accepting two arguments.
    :type proc: procedure
    :param map: The map to traverse.
    :type map: map
    :return: #void
    :rtype: void
