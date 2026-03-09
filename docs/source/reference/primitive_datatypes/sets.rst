Sets
====

Overview
--------

A `set` is an unordered collection of unique elements. Unlike a list, which may contain duplicate values and whose order
is significant, a set guarantees that each element appears at most once and that element order has no meaning. The
literal syntax for sets is written using ``#{ ... }``. For example:

.. code-block::

    --> #{1 2 3}
    #{1 2 3}
    --> #{"foo" "bar" "baz"}
    #{"foo" "bar" "baz"}

In mathematics, sets are fundamental objects used to describe collections of distinct elements. Two sets are considered
equal if they contain the same elements, regardless of order. For example, the sets ``#{1, 2, 3}`` and ``#{3, 1, 2}`` are
identical in set theory. This same principle applies here: the internal ordering of a set is indeterminate and should
not be relied upon. Sets support classical mathematical operations such as union (∪), intersection (∩), difference (−),
subset (⊆), superset (⊇), and symmetric difference (Δ), all of which are provided as procedures.

In programming, sets are most useful when you care about membership rather than position. Common use cases include
removing duplicates from data, testing whether a value has already been seen, tracking visited items in a graph
traversal, implementing filters, and performing fast membership tests. Because sets are implemented using a hash table
internally, membership checks such as ``(set-member? s obj)`` are typically very fast. This makes sets significantly more
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

set
***

.. function:: (set obj ...)

    Returns a newly allocated set containing the given *obj* arguments as
    members. Duplicate values are silently collapsed — each distinct value
    appears only once. The set literal syntax ``#{obj ...}`` is equivalent.
    It is an error if any argument is not a hashable type.

    Hashable types include: numbers, strings, characters, symbols, and
    booleans. Mutable compound types such as lists and vectors are not
    hashable.

    :param obj: Zero or more hashable objects to include as members.
    :return: A newly allocated set.
    :rtype: set

    **Example:**

    .. code-block::

        --> (set 1 2 3)
        #{1 2 3}
        --> #{1 2 3}
        #{1 2 3}
        --> (set 1 1 2 2 3)
        #{1 2 3}
        --> (set)
        #{}


.. _proc:set-copy:

set-copy
********

.. function:: (set-copy set)

    Returns a newly allocated copy of *set*. The copy is shallow — the member
    objects themselves are not copied, only the references to them. It is an
    error if *set* is not a set.

    :param set: The set to copy.
    :type set: set
    :return: A newly allocated copy of *set*.
    :rtype: set

    **Example:**

    .. code-block::

        --> (define a #{1 2 3})
        --> (define b (set-copy a))
        --> (set-add! b 4)
        --> b
        #{1 2 3 4}
        --> a
        #{1 2 3}


.. _proc:set-clear!:

set-clear!
**********

.. function:: (set-clear! set)

    Removes all members from *set*, mutating it in place. Returns the same
    set object, now empty. It is an error if *set* is not a set.

    :param set: The set to clear.
    :type set: set
    :return: The mutated set, now empty.
    :rtype: set

    **Example:**

    .. code-block::

        --> (define s #{1 2 3})
        --> (set-clear! s)
        #{}
        --> s
        #{}


.. _proc:set-add!:

set-add!
********

.. function:: (set-add! set obj ...)

    Adds one or more objects to *set*, mutating it in place, and returns the
    mutated set. If a list or vector is passed as an argument, its *members*
    are added to the set individually rather than the list or vector itself.
    It is an error if any object, or any member of a list or vector argument,
    is not a hashable type.

    :param set: The set to add to.
    :type set: set
    :param obj: One or more hashable objects, lists, or vectors to add.
    :return: The mutated set.
    :rtype: set

    **Example:**

    .. code-block::

        --> (define s #{1 2 3})
        --> (set-add! s 4 5)
        #{1 2 3 4 5}
        --> (set-add! s '(6 7))
        #{1 2 3 4 5 6 7}
        --> (set-add! s #(8 9))
        #{1 2 3 4 5 6 7 8 9}
        --> (set-add! s 3)
        #{1 2 3 4 5 6 7 8 9}


.. _proc:set-remove!:

set-remove!
***********

.. function:: (set-remove! set obj [sym])

    Removes *obj* from *set*, mutating it in place, and returns the mutated
    set. Signals an index error if *obj* is not a member of *set*. If an
    optional symbol is supplied as a third argument, the error is suppressed
    and the procedure returns silently instead.

    :param set: The set to remove from.
    :type set: set
    :param obj: The object to remove.
    :param sym: An optional symbol. If provided, suppresses the error when
                *obj* is not a member.
    :type sym: symbol
    :return: The mutated set.
    :rtype: set

    **Example:**

    .. code-block::

        --> (define s #{1 2 3})
        --> (set-remove! s 2)
        #{1 3}
        --> (set-remove! s 99)
        error: index-error
        --> (set-remove! s 99 'ok)
        --> s
        #{1 3}


.. _proc:set-member?:

set-member?
***********

.. function:: (set-member? set obj)

    Returns ``#t`` if *obj* is a member of *set* (i.e. *obj* ∈ *set*),
    ``#f`` otherwise. It is an error if *set* is not a set.

    :param set: The set to test membership in.
    :type set: set
    :param obj: The object to test.
    :return: ``#t`` if *obj* ∈ *set*, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block::

        --> (set-member? #{1 2 3} 2)
        #t
        --> (set-member? #{1 2 3} 99)
        #f


.. _proc:set-disjoint?:

set-disjoint?
*************

.. function:: (set-disjoint? set1 set2)

    Returns ``#t`` if *set1* and *set2* share no members in common, ``#f``
    otherwise. Two sets are disjoint if their intersection is empty
    (i.e. *set1* ∩ *set2* = ∅). It is an error if either argument is not
    a set.

    :param set1: The first set.
    :type set1: set
    :param set2: The second set.
    :type set2: set
    :return: ``#t`` if *set1* ∩ *set2* = ∅, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block::

        --> (set-disjoint? #{1 2 3} #{4 5 6})
        #t
        --> (set-disjoint? #{1 2 3} #{3 4 5})
        #f


.. _proc:set-subset?:

set-subset?
***********

.. function:: (set-subset? set1 set2)

    Returns ``#t`` if every member of *set1* is also a member of *set2*
    (i.e. *set1* ⊆ *set2*), ``#f`` otherwise. A set is always a subset of
    itself. It is an error if either argument is not a set.

    :param set1: The candidate subset.
    :type set1: set
    :param set2: The candidate superset.
    :type set2: set
    :return: ``#t`` if *set1* ⊆ *set2*, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block::

        --> (set-subset? #{1 2} #{1 2 3})
        #t
        --> (set-subset? #{1 2 3} #{1 2 3})
        #t
        --> (set-subset? #{1 2 4} #{1 2 3})
        #f


.. _proc:set-superset?:

set-superset?
*************

.. function:: (set-superset? set1 set2)

    Returns ``#t`` if *set1* contains every member of *set2*
    (i.e. *set1* ⊇ *set2*), ``#f`` otherwise. A set is always a superset of
    itself. It is an error if either argument is not a set.

    :param set1: The candidate superset.
    :type set1: set
    :param set2: The candidate subset.
    :type set2: set
    :return: ``#t`` if *set1* ⊇ *set2*, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block::

        --> (set-superset? #{1 2 3} #{1 2})
        #t
        --> (set-superset? #{1 2 3} #{1 2 3})
        #t
        --> (set-superset? #{1 2 3} #{1 2 4})
        #f


.. _proc:set-union:

set-union
*********

.. function:: (set-union set1 set2)

    Returns a newly allocated set containing all members that appear in
    *set1*, *set2*, or both (i.e. *set1* ∪ *set2*). Neither argument is
    modified. It is an error if either argument is not a set.

    :param set1: The first set.
    :type set1: set
    :param set2: The second set.
    :type set2: set
    :return: A new set equal to *set1* ∪ *set2*.
    :rtype: set

    **Example:**

    .. code-block::

        --> (set-union #{1 2 3} #{3 4 5})
        #{1 2 3 4 5}
        --> (set-union #{1 2 3} #{})
        #{1 2 3}


.. _proc:set-union!:

set-union!
**********

.. function:: (set-union! set1 set2)

    Mutates *set1* to contain all members that appear in *set1*, *set2*, or
    both (i.e. *set1* ∪ *set2*), and returns the mutated *set1*. It is an
    error if either argument is not a set.

    :param set1: The set to mutate.
    :type set1: set
    :param set2: The set to union with.
    :type set2: set
    :return: *set1* mutated to equal *set1* ∪ *set2*.
    :rtype: set

    **Example:**

    .. code-block::

        --> (define s #{1 2 3})
        --> (set-union! s #{3 4 5})
        #{1 2 3 4 5}
        --> s
        #{1 2 3 4 5}


.. _proc:set-intersection:

set-intersection
****************

.. function:: (set-intersection set1 set2)

    Returns a newly allocated set containing only the members that appear in
    both *set1* and *set2* (i.e. *set1* ∩ *set2*). Neither argument is
    modified. It is an error if either argument is not a set.

    :param set1: The first set.
    :type set1: set
    :param set2: The second set.
    :type set2: set
    :return: A new set equal to *set1* ∩ *set2*.
    :rtype: set

    **Example:**

    .. code-block::

        --> (set-intersection #{1 2 3 4} #{3 4 5 6})
        #{3 4}
        --> (set-intersection #{1 2 3} #{4 5 6})
        #{}


.. _proc:set-intersection!:

set-intersection!
*****************

.. function:: (set-intersection! set1 set2)

    Mutates *set1* to contain only the members that appear in both *set1* and
    *set2* (i.e. *set1* ∩ *set2*), and returns the mutated *set1*. It is an
    error if either argument is not a set.

    :param set1: The set to mutate.
    :type set1: set
    :param set2: The set to intersect with.
    :type set2: set
    :return: *set1* mutated to equal *set1* ∩ *set2*.
    :rtype: set

    **Example:**

    .. code-block::

        --> (define s #{1 2 3 4})
        --> (set-intersection! s #{3 4 5 6})
        #{3 4}
        --> s
        #{3 4}


.. _proc:set-difference:

set-difference
**************

.. function:: (set-difference set1 set2)

    Returns a newly allocated set containing the members of *set1* that are
    not members of *set2* (i.e. *set1* − *set2*). Neither argument is
    modified. It is an error if either argument is not a set.

    Note that set difference is not commutative: *set1* − *set2* and
    *set2* − *set1* are generally different results.

    :param set1: The set to subtract from.
    :type set1: set
    :param set2: The set of members to exclude.
    :type set2: set
    :return: A new set equal to *set1* − *set2*.
    :rtype: set

    **Example:**

    .. code-block::

        --> (set-difference #{1 2 3 4} #{3 4 5 6})
        #{1 2}
        --> (set-difference #{3 4 5 6} #{1 2 3 4})
        #{5 6}


.. _proc:set-difference!:

set-difference!
***************

.. function:: (set-difference! set1 set2)

    Mutates *set1* to contain only the members of *set1* that are not members
    of *set2* (i.e. *set1* − *set2*), and returns the mutated *set1*. It is
    an error if either argument is not a set.

    :param set1: The set to mutate.
    :type set1: set
    :param set2: The set of members to exclude.
    :type set2: set
    :return: *set1* mutated to equal *set1* − *set2*.
    :rtype: set

    **Example:**

    .. code-block::

        --> (define s #{1 2 3 4})
        --> (set-difference! s #{3 4 5 6})
        #{1 2}
        --> s
        #{1 2}


.. _proc:set-sym-difference:

set-sym-difference
******************

.. function:: (set-sym-difference set1 set2)

    Returns a newly allocated set containing the members that appear in
    *set1* or *set2*, but not in both (i.e. *set1* Δ *set2*). This is
    equivalent to (*set1* ∪ *set2*) − (*set1* ∩ *set2*). Neither argument
    is modified. It is an error if either argument is not a set.

    :param set1: The first set.
    :type set1: set
    :param set2: The second set.
    :type set2: set
    :return: A new set equal to *set1* Δ *set2*.
    :rtype: set

    **Example:**

    .. code-block::

        --> (set-sym-difference #{1 2 3 4} #{3 4 5 6})
        #{1 2 5 6}
        --> (set-sym-difference #{1 2 3} #{1 2 3})
        #{}


.. _proc:set-sym-difference!:

set-sym-difference!
*******************

.. function:: (set-sym-difference! set1 set2)

    Mutates *set1* to contain the members that appear in *set1* or *set2*,
    but not in both (i.e. *set1* Δ *set2*), and returns the mutated *set1*.
    It is an error if either argument is not a set.

    :param set1: The set to mutate.
    :type set1: set
    :param set2: The second set.
    :type set2: set
    :return: *set1* mutated to equal *set1* Δ *set2*.
    :rtype: set

    **Example:**

    .. code-block::

        --> (define s #{1 2 3 4})
        --> (set-sym-difference! s #{3 4 5 6})
        #{1 2 5 6}
        --> s
        #{1 2 5 6}


.. _proc:set-map:

set-map
*******

.. function:: (set-map proc set)

    Applies *proc* to each member of *set* and returns a list of the results.
    The order in which *proc* is applied to the members is indeterminate, as
    sets are unordered. *proc* must accept exactly one argument. It is an
    error if *proc* is not a procedure or *set* is not a set.

    :param proc: A procedure of one argument.
    :type proc: procedure
    :param set: The set whose members to map over.
    :type set: set
    :return: A list of the results of applying *proc* to each member.
    :rtype: list

    **Example:**

    .. code-block::

        --> (set-map (lambda (x) (* x x)) #{1 2 3 4})
        (1 4 9 16)
        --> (set-map number->string #{1 2 3})
        ("1" "2" "3")


.. _proc:set-foreach:

set-foreach
***********

.. function:: (set-foreach proc set)

    Applies *proc* to each member of *set* for the purpose of side effects.
    The return values of *proc* are discarded. The order in which *proc* is
    applied to the members is indeterminate, as sets are unordered. Returns
    an unspecified value. *proc* must accept exactly one argument. It is an
    error if *proc* is not a procedure or *set* is not a set.

    :param proc: A procedure of one argument.
    :type proc: procedure
    :param set: The set whose members to iterate over.
    :type set: set
    :return: Unspecified.

    **Example:**

    .. code-block::

        --> (set-foreach display #{1 2 3})
        123


.. _proc:list->set:

list->set
*********

.. function:: (list->set list)

    Returns a newly allocated set containing all members of *list*.
    Duplicate values in *list* are collapsed — each distinct value appears
    only once in the result. It is an error if *list* is not a proper list,
    or if any member of *list* is not a hashable type.

    :param list: A proper list of hashable objects.
    :type list: list
    :return: A newly allocated set containing the members of *list*.
    :rtype: set

    **Example:**

    .. code-block::

        --> (list->set '(1 2 3))
        #{1 2 3}
        --> (list->set '(1 1 2 2 3 3))
        #{1 2 3}
        --> (list->set '())
        #{}


.. _proc:set->list:

set->list
*********

.. function:: (set->list set)

    Returns a newly allocated list containing all members of *set*. The order
    of the members in the returned list is indeterminate, as sets are
    unordered. It is an error if *set* is not a set.

    :param set: The set to convert.
    :type set: set
    :return: A newly allocated list containing the members of *set*.
    :rtype: list

    **Example:**

    .. code-block::

        --> (set->list #{1 2 3})
        (1 2 3)
        --> (set->list #{})
        ()
        --> (length (set->list #{1 2 3 4 5}))
        5

