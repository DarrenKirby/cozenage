Sets
====

Overview
--------

A `set` is an unordered collection of unique elements. Unlike a list, which may contain duplicate values and whose order
is significant, a set guarantees that each element appears at most once and that element order has no meaning. The
literal syntax for sets is written using ``#{ ... }``. For example:

.. code-block:: scheme

    --> #{1 2 3]
    --> #{"foo" "bar" "baz"]

In mathematics, sets are fundamental objects used to describe collections of distinct elements. Two sets are considered
equal if they contain the same elements, regardless of order. For example, the sets {1, 2, 3} and {3, 1, 2} are
identical in set theory. This same principle applies here: the internal ordering of a set is indeterminate and should
not be relied upon. Sets support classical mathematical operations such as union (∪), intersection (∩), difference (−),
subset (⊆), superset (⊇), and symmetric difference (Δ), all of which are provided as procedures.

In programming, sets are most useful when you care about membership rather than position. Common use cases include
removing duplicates from data, testing whether a value has already been seen, tracking visited items in a graph
traversal, implementing filters, and performing fast membership tests. Because sets are implemented using a hash table
internally, membership checks such as (set-member? s obj) are typically very fast. This makes sets significantly more
efficient than lists for repeated lookup operations.

Not all objects may be stored in a set. An object must be hashable in order to be used as a set element. Hashable
objects are those whose value can be reliably converted into a stable hash code. The following types are hashable:

* string
* symbol
* integer
* rational
* real
* complex
* boolean
* char

Compound types such as lists, vectors, sets, and hashes are not currently hashable and therefore cannot be used as set
elements. Attempting to insert a non-hashable object into a set will raise a type error.

It is important to remember that sets are unordered. When converting a set to a list, iterating over it, or printing it,
the order of elements is not defined and may vary. Programs should never depend on any particular ordering of set
elements. If ordering is required, convert the set to a list and sort the result explicitly.

Sets may be created using the set constructor or converted from lists using list->set. There are both non-mutating and
mutating variants of many operations. Procedures ending in ! modify the original set in place, while those without !
return newly allocated sets. This follows a common Scheme convention indicating mutation.


Set Procedures
--------------


.. _proc:set:

.. function:: (set obj ...)

    Constructs a new set containing the given objects.

    Each argument must be hashable. Duplicate values are automatically removed.

    :param obj: Zero or more hashable objects.
    :type obj: any (hashable)
    :return: A newly allocated set.
    :rtype: set

    Example:

    .. code-block:: scheme

      --> (set 1 2 3 2)
      #{1 2 3}


.. _proc:set-copy:

.. function:: (set-copy set)

    Returns a shallow copy of set.

    The structure of the set is duplicated, but the elements themselves are not copied.

    :param set: The set to copy.
    :type set: set
    :return: A newly allocated set containing the same elements.
    :rtype: set

    Example:

    .. code-block:: scheme

        --> (define s1 #{1 2 3 4 5})
        s1
        --> (define s2 (set-copy s1))
        s2
        --> (set-remove! s2 5)
        #{4 3 1 2}
        --> s1
        #{4 5 3 1 2}  ; note that the two sets are independently modified


.. _proc:set-clear!:

.. function:: (set-clear! set)

    Removes all elements from set.

    This procedure mutates the original set.

    :param set: The set to clear.
    :type set: set
    :return: The same set object, now empty.
    :rtype: set

    Example:

    .. code-block:: scheme

      --> (define s (set 1 2 3))
      s
      --> s
      #{3 1 2}
      --> (set-clear! s)
      #{}


.. _proc:set-add!:

.. function:: (set-add! set obj ...)

    Adds one or more objects to set.

    If an argument is a list or vector, its elements are added individually.
    All objects must be hashable.

    This procedure mutates the set.

    :param set: The set to modify.
    :type set: set
    :param obj: One or more objects, or lists/vectors of objects.
    :type obj: any (hashable)
    :return: The modified set.
    :rtype: set

    Example:

    .. code-block:: scheme

      --> (define s (set 1 2))
      s
      --> (set-add! s 3)
      #[1 2 3}
      --> (set-add! s '(4 5))
      #{1 2 3 4 5}


.. _proc:set-remove!:

.. function:: (set-remove! set obj [sym])

    Removes obj from set.

    If obj is not present, an index error is raised unless an optional
    symbol is supplied as the third argument, in which case the procedure
    returns silently.

    This procedure mutates the set.

    :param set: The set to modify.
    :type set: set
    :param obj: The object to remove.
    :type obj: any
    :param sym: Optional symbol suppressing index error.
    :type sym: symbol
    :return: The modified set.
    :rtype: set

    Example:

    .. code-block:: scheme

      --> (define s (set 1 2 3))
      s
      --> (set-remove! s 2)
      #{1 3}


.. _proc:set-member?:

.. function:: (set-member? set obj)

    Tests whether obj is a member of set.

    :param set: The set to test.
    :type set: set
    :param obj: The object to search for.
    :type obj: any
    :return: #true if present, otherwise #false.
    :rtype: boolean

    Example:

    .. code-block:: scheme

      --> (set-member? #{1 2 3} 2)
      #true
      --> (set-member? #{1 2 3} 5)
      #false


.. _proc:set-disjoint?:

.. function:: (set-disjoint? set1 set2)

    Returns #true if set1 and set2 share no common elements.

    :param set1: First set.
    :type set1: set
    :param set2: Second set.
    :type set2: set
    :return: #true if disjoint, otherwise #false.
    :rtype: boolean

.. _proc:set-subset?:

.. function:: (set-subset? set1 set2)

    Returns #true if every element of set1 is contained in set2.

    :param set1: Candidate subset.
    :type set1: set
    :param set2: Candidate superset.
    :type set2: set
    :return: #true if set1 ⊆ set2, otherwise #false.
    :rtype: boolean

.. _proc:set-superset?:

.. function:: (set-superset? set1 set2)

    Returns #true if set1 contains every element of set2.

    :param set1: Candidate superset.
    :type set1: set
    :param set2: Candidate subset.
    :type set2: set
    :return: #true if set1 ⊇ set2, otherwise #false.
    :rtype: boolean

.. _proc:set-union:

.. function:: (set-union set1 set2)

    Returns a new set containing all elements of set1 and set2.

    :param set1: First set.
    :type set1: set
    :param set2: Second set.
    :type set2: set
    :return: A newly allocated set representing the union.
    :rtype: set

    Example:

    .. code-block:: scheme

      --> (set-union #{1 2} #{2 3})
      #{1 2 3}


.. _proc:set-union!:

.. function:: (set-union! set1 set2)

    Mutates set1 so that it becomes the union of set1 and set2.

    :param set1: Set to modify.
    :type set1: set
    :param set2: Set whose elements will be added.
    :type set2: set
    :return: The modified set1.
    :rtype: set

.. _proc:set-intersection:

.. function:: (set-intersection set1 set2)

    Returns a new set containing elements common to both sets.

    :param set1: First set.
    :type set1: set
    :param set2: Second set.
    :type set2: set
    :return: A newly allocated set representing the intersection.
    :rtype: set

.. _proc:set-intersection!:

.. function:: (set-intersection! set1 set2)

    Mutates set1 so that it contains only elements also found in set2.

    :param set1: Set to modify.
    :type set1: set
    :param set2: Comparison set.
    :type set2: set
    :return: The modified set1.
    :rtype: set

.. _proc:set-difference:

.. function:: (set-difference set1 set2)

    Returns a new set containing elements of set1 that are not in set2.

    :param set1: First set.
    :type set1: set
    :param set2: Second set.
    :type set2: set
    :return: A newly allocated set representing the difference.
    :rtype: set

.. _proc:set-difference!:

.. function:: (set-difference! set1 set2)

    Mutates set1 by removing elements also found in set2.

    :param set1: Set to modify.
    :type set1: set
    :param set2: Set whose elements will be removed.
    :type set2: set
    :return: The modified set1.
    :rtype: set

.. _proc:set-sym-difference:

.. function:: (set-sym-difference set1 set2)

    Returns a new set containing elements found in either set,
    but not in both (the symmetric difference).

    :param set1: First set.
    :type set1: set
    :param set2: Second set.
    :type set2: set
    :return: A newly allocated set representing the symmetric difference.
    :rtype: set

.. _proc:set-sym-difference!:

.. function:: (set-sym-difference! set1 set2)

    Mutates set1 so that it becomes the symmetric difference
    of set1 and set2.

    :param set1: Set to modify.
    :type set1: set
    :param set2: Comparison set.
    :type set2: set
    :return: The modified set1.
    :rtype: set

.. _proc:set-map:

.. function:: (set-map proc set)

    Applies proc to each element of set and returns a list of results.

    The order of application is indeterminate.

    :param proc: Procedure accepting one argument.
    :type proc: procedure
    :param set: Set to traverse.
    :type set: set
    :return: A list of results.
    :rtype: list

    Example:

    .. code-block:: scheme

      --> (set-map (lambda (x) (* x 2)) #{1 2 3})
      (6 2 4)
      --> (set-map square #{2 4 6})
      (16 36 4)


.. _proc:set-foreach:

.. function:: (set-foreach proc set)

    Applies proc to each element of set for side effects.

    Returns #void. The order of application is indeterminate.

    :param proc: Procedure accepting one argument.
    :type proc: procedure
    :param set: Set to traverse.
    :type set: set
    :return: #void
    :rtype: void

.. _proc:list->set:

.. function:: (list->set list)

    Constructs a new set from the elements of list.
    Duplicate elements are removed.

    :param list: A proper list.
    :type list: list
    :return: A newly allocated set.
    :rtype: set

    Example:

    .. code-block:: scheme

      --> (list->set '(1 2 2 3))
        #{3 1 2}


.. _proc:set->list:

.. function:: (set->list set)

    Returns a list containing all elements of set.

    The order of elements is indeterminate.

    :param set: The set to convert.
    :type set: set
    :return: A newly allocated list.
    :rtype: list

    Example:

    .. code-block:: scheme

      --> (set->list #{1 2 3})
      (3 1 2)
