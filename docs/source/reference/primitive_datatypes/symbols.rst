Symbols
=======

Overview
--------

Symbols are objects whose usefulness rests on the fact that two symbols are identical (in the sense
of eqv?) if and only if their names are spelled the same way. For instance, they can be used the way
enumerated values are used in other languages.

The rules for writing a symbol are exactly the same as the rules for writing an identifier.

It is guaranteed that any symbol that has been returned as part of a literal expression, or read
using the read procedure, and subsequently written out using the write procedure, will read back in
as the identical symbol (in the sense of eqv?).
