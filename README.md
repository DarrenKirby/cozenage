# Cozenage

*Getting close to being ready for prime time*

## About

This started as a 'toy' Lisp, but I am steadily, if slowly, working towards a full
R5RS/R7RS [Scheme implementation](https://standards.scheme.org/). After the bulk of the standard is implemented,
I plan to add several `(cozenage foo)` libraries to interface with the OS, and eventually, I would
like to implement some kind of 'shell mode' which would act like an interpretive shell 
with Scheme syntax, kicking all 'unbound symbols' down a level as potential shell commands.

This is an educational process for me. I started writing Cozenage to enhance my understanding of 
Scheme, C , and programming language fundamentals in general.
Not everything here is implemented in the most efficient or best way. Cozenage is a work in progress,
and as I learn new and better techniques I will come back and improve sections of this program.

## Dependencies

`Cozenage` requires one of [readline](https://tiswww.cwru.edu/php/chet/readline/rltop.html) or 
libedit for the REPL. It requires [ICU](https://github.com/unicode-org/icu) for UTF-8.
These will almost certainly be installed already on any sort of development rig. It requires the [Boehm-Demers-Weiser Garbage Collector](https://github.com/bdwgc/bdwgc)
which may or may not be installed already on your system.

## Building Cozenage

Cozenage has been built and tested on Linux, FreeBSD, and macOS. Building on Windows will almost
certainly break without a POSIX subsystem in place. See [this bug](https://github.com/DarrenKirby/cozenage/issues/1) for details.
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

The status of the R7RS (scheme base) procedures, and the other scheme libraries 
are documented on another page: 

[R7RS built-in procedures](README.status.md)

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

