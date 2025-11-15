# Cozenage

*Getting close to being ready for prime time*

## About

``Cozenage`` is a Scheme-derived, LISP-like language which provides a modest core of built-in
procedures and is extended by a growing standard library of additional functionality implemented 
as 'hot-pluggable' dynamic modules. While Cozenage agrees with the formal R5RS/R7RS standards 
inasmuch as possible, it is not intended to be fully-compliant, and deviates from the standards in 
many non-trivial ways. These differences, and a justifications for them, will be documented as I find
the time. That said, anyone experienced with Scheme will find Cozenage quite familiar.

Cozenage has:

- The typical menu of disjoint primitive objects: ``number``, ``string``, ``char``, ``symbol``, ``pair``, ``null``, ``vector``, ``boolean``, ``port``, and ``procedure``.
- A selection of ``bytevector`` objects implemented using native C type arrays: ``u8``, ``s8``, ``u16``, ``s16``, ``u32``, and ``s32``.
- Full 'numeric tower' of derived types including ``integer``, ``rational``, ``real``, and ``complex`` numbers.
- Exact and inexact numbers.
- Proper tail-call optimization where applicable.
- Garbage collection.
- A REPL with multi-line input editing, command history, and tab-autocompletion.
- UTF-8 Unicode support.
- eval/apply procedures for interpreting data as executable code.

Typical Scheme features that Cozenage does NOT have:

- Arbitrary size and precision integers and real numbers. Currently, the magnitude of numbers is limited by the underlying C long long and long double types provided by the platform. While I do intend to implement 'bugnums' and 'bigfloats' at some point, it is somewhat low-priority currently.
- call/cc and first-class continuations. I'm not likely to ever implement this. Rather, I am more likely to implement control flow patterns typically implemented via call/cc as primitive syntax.  
- Hygienic macros. I will likely implement this at some point, but it is low-priority.
- Quasiquotation. Currently, only standard quotation is supported.
- Delayed, or lazy evaluation. On the todo, but low-priority.

## Dependencies

`Cozenage` requires one of [readline](https://tiswww.cwru.edu/php/chet/readline/rltop.html) or 
libedit for the REPL. It requires [ICU](https://github.com/unicode-org/icu) for UTF-8.
These will almost certainly be installed already on any sort of development rig. It requires the [Boehm-Demers-Weiser Garbage Collector](https://github.com/bdwgc/bdwgc)
which may or may not be installed already on your system.

## Building Cozenage

Cozenage has been built and tested on Linux, FreeBSD, and macOS. Building on Windows will almost
certainly break without a POSIX subsystem in place. See [this bug](https://github.com/DarrenKirby/cozenage/issues/1) if you are able to help.
The build system(s) specify the C23 standard, so the build might fail on older compilers.

If you have cmake, run `make`.

If you do not have cmake, run `make nocmake`.

There is also a bit more in depth guide as part of the [Cozenage documentation](https://darrenkirby.github.io/cozenage/howto/installation.html)

## Running Cozenage

To interpret a Scheme file (typically with an .scm or .ss extension, although Cozenage will attempt
to run any file argument as Scheme code) just add the file name after any options:

    $ ./cozenage -l write,file my_program.scm

To run the REPL just run the program with no arguments:

    $ ./cozenage -l write,file

## Status of built-in procedures and special forms

The status of the base procedures, and the other scheme libraries 
are documented on another page: 

[built-in and library procedures](README.status.md)

### Special forms/syntax

These are the special forms/syntax constructs that are implemented in Cozenage so far:

- `quote`
- `define`
- `lambda`
- `let`
- `let*`
- `letrec`
- `set!`
- `if`
- `when`
- `unless`
- `cond`
- `else`
- `begin`
- `import`
- `and`
- `or`

## Non-R7RS procedures and libraries

Cozenage has implemented several built-in procedures which extend it beyond the
R7RS specification. Some are loaded with `(scheme base)` by default, and some are
imported along with the relevant `(scheme foo)` libraries.

- `filter` - `(scheme base)`
- `foldl` - `(scheme base)`
- `zip` - `(scheme base)`


- `log2` imported with `(scheme inexact)`
- `log10` imported with `(scheme inexact)`
- `cbrt` imported with `(scheme inexact)`


- `current-dt-utc` imported with `(scheme time)`
- `current-dt-local` imported with `(scheme time)`

### Bits library (cozenage bits)

The `bits` library implements bitwise operations on integers,
and also interprets a `bitstring` pseudo-type, which is string of ones and zeros implemented 
as regular Scheme symbols prefaced by a lowercase 'b' character (so that the 
parser doesn't interpret them as regular numeric digits). For example:

    --> (import (cozenage bits))
    --> (bitstring->int 'b1001001001)
    -439
    --> (bitstring->int 'b01001001001
    585
    --> (int->bitstring 10)
    b01010
    --> (int->bitstring -120)
    b10001000

Notice that bitstrings are interpreted as twos-compliment values, so positive values
must have a leading `0`, and negative values must have a leading `1`.

- `>>` (right shift)
- `<<` (left shift)
- `&` (bitwise AND)
- `|` (bitwise OR)
- `^` (bitwise XOR)
- `~` (bitwise NOT)
- `int->bitstring`
- `bitstring->int`

