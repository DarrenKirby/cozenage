Procedures
==========

Overview
--------

In Scheme, procedures are a fundamental, primitive data type, just like numbers, strings, or
booleans. This concept is central to the language's power. Because procedures are data, they are
considered first-class citizens. This means they can be treated like any other value: you can store
them in variables, pass them as arguments to other procedures (creating "higher-order" procedures
like map), and even return them as the result of a procedure call.

The R7RS standard provides a rich library of hundreds of built-in procedures for tasks ranging from
arithmetic (``+``, ``*``) to list manipulation (``car``, ``cons``). While this standard library is
extensive, the true power of Scheme comes from creating new, user-defined procedures. The most
fundamental way to create a procedure is with the lambda special form, which constructs a new
procedure object. For convenience, the define syntax provides a "shorthand" for creating and naming
a procedure, but it is internally just an abbreviation for creating a lambda and binding it to a
name.
