Pairs and Lists
===============

Overview
--------

A pair (sometimes called a dotted pair) is a record structure with two fields called the car and cdr
fields (for historical reasons). Pairs are created by the procedure cons. The car and cdr fields are
accessed by the procedures car and cdr. The car and cdr fields are assigned by the procedures
set-car! and set-cdr!.

Pairs are used primarily to represent lists. A list can be defined recursively as either the empty
list or a pair whose cdr is a list. More precisely, the set of lists is defined as the smallest set
X such that:

- The empty list is in X.
- If list is in X , then any pair whose cdr field contains list is also in X.

The objects in the car fields of successive pairs of a list are the elements of the list. For
example, a two-element list is a pair whose car is the first element and whose cdr is a pair whose
car is the second element and whose cdr is the empty list. The length of a list is the number of
elements, which is the same as the number of pairs.The objects in the car fields of successive pairs
of a list are the elements of the list. For example, a two-element list is a pair whose car is the
first element and whose cdr is a pair whose car is the second element and whose cdr is the empty
list. The length of a list is the number of elements, which is the same as the number of pairs.

The empty list is a special object of its own type. It is not a pair, it has no elements, and its
length is zero.

Lists are often referred to as 'proper lists', as opposed to 'improper lists', which are not
terminated with a ``nil`` value.

List and pair procedures
------------------------

.. _proc:cons:

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
