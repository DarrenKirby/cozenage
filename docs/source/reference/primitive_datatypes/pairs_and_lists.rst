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
