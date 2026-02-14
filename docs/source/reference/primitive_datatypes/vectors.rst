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