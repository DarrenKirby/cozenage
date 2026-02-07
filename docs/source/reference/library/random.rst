Base Random Library
===================

The random library provides a set of high-quality primitives for stochastic processes and data
manipulation. Unlike basic modulo-based generators, this library utilizes unbiased sampling techniques
such as Lemire’s algorithm for integers and Fisher-Yates for permutations—to ensure a truly uniform
distribution across all ranges. These tools are essential for everything from simple game logic and
randomized testing to data science applications like bootstrapping and shuffling datasets.


Numerical Randomness
--------------------

.. _proc:rand-int:

.. function:: (rand-int [limit])

    Returns a uniformly distributed random integer in the range [0, limit). If limit is omitted, it defaults to the maximum value of a 32-bit unsigned integer (4,294,967,295).

    :param limit: The upper bound (exclusive).
    :type limit: integer
    :return: A random integer.
    :rtype: integer

    Example

    .. code-block:: scheme

      --> (rand-int 10)
        7

.. _proc:rand-dbl:

.. function:: (rand-dbl)

    Returns a uniformly distributed random real number in the range [0.0, 1.0). The result maintains 53 bits of precision, consistent with IEEE 754 double-precision floats.

    :return: A random real number.
    :rtype: real

.. _proc:rand-uniform:

.. function:: (rand-uniform min max)

    Returns a uniformly distributed random real number in the range [min, max).

    :param min: The lower bound (inclusive).
    :type min: integer, rational, or real
    :param max: The upper bound (exclusive).
    :type max: integer, rational, or real
    :return: A random real number.
    :rtype: real


Sequence Manipulation
---------------------

.. _proc:shuffle:

.. function:: (shuffle seq)

    Returns a new sequence containing the elements of seq in a randomized order. This uses an unbiased Fisher-Yates permutation. The original sequence remains unmodified.

    :param seq: The sequence to shuffle.
    :type seq: list or vector
    :return: A shuffled version of the input.
    :rtype: list or vector (matches input type)

    Example

    .. code-block:: scheme

      --> (shuffle '(1 2 3 4 5))
        (3 5 1 4 2)

.. _proc:rand-choice:

.. function:: (rand-choice seq)

    Returns a single element chosen uniformly at random from seq.

    :param seq: The sequence to choose from.
    :type seq: list or vector
    :return: A single element from the sequence.
    :rtype: any

.. _proc:rand-choices:

.. function:: (rand-choices seq k)

    Returns a new sequence containing k elements chosen uniformly at random from seq with replacement (meaning the same element can be picked multiple times).

    :param seq: The sequence to sample from.
    :type seq: list or vector
    :param k: The number of elements to return.
    :type k: integer
    :return: A new sequence of length k.
    :rtype: list or vector (matches input type)

    Example

    .. code-block:: scheme

      --> (rand-choices #(heads tails) 3)
        #(tails heads tails)

