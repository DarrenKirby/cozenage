Vectors
=======

Overview
--------

Vectors are heterogeneous structures whose elements are indexed by integers. A vector typically
occupies less space than a list of the same length, and the average time needed to access a randomly
chosen element is typically less for the vector than for the list.

The length of a vector is the number of elements that it contains. This number is a non-negative
integer that is fixed when the vector is created. The valid indexes of a vector are the exact
non-negative integers less than the length of the vector. The first element in a vector is indexed
by zero, and the last element is indexed by one less than the length of the vector.

Vectors are written using the notation ``#(obj ... )``. For example, a vector of length 3 containing
the number zero in element 0, the list (2 2 2 2) in element 1, and the string "Anna" in element 2
can be written as follows: ``#(0 (2 2 2 2) "Anna")``.

Vector constants are self-evaluating, so they do not need to be quoted in programs.
