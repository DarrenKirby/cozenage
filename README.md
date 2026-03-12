# Cozenage

**Cozenage** is a Scheme-derived, Lisp-like programming language written in C, designed primarily as an educational 
project and exploratory implementation. It provides a small, expressive core language and is extended through a 
growing standard library of dynamically loadable modules.

While Cozenage borrows heavily from Scheme, particularly R5RS and R7RS, it is **not intended to be fully standard-compliant**. 
Where possible, Cozenage follows Scheme semantics and conventions, but it intentionally deviates from the standards in a 
number of non-trivial ways. These deviations are the result of pragmatic design choices, implementation constraints, 
or deliberate simplifications, and are documented where relevant.

Anyone with experience in Scheme or other Lisp dialects should find Cozenage immediately familiar.

---

## Language features

### Core object types

Cozenage provides the usual set of disjoint primitive objects common to Scheme-like languages:

- number
- string
- character
- symbol
- pair
- null
- vector
- boolean
- port
- procedure

And two primitive object types not specified by R7RS:

- set
- hash

Both sets and hashes are implemented as hash tables for fast O(1) amortized lookups.

### Numeric system

- Full numeric tower:
    - integer
    - rational
    - real
    - complex
- Exact and inexact numbers
- Native big integer support
- Work in progress: arbitrary-precision rationals and reals

### Bytevectors

Native bytevector types backed by C arrays:

- u8, s8
- u16, s16
- u32, s32
- u64, s64

### Ports and I/O

- File-backed text and binary ports
- Memory-backed string and bytevector (u8) ports
- UTF-8 Unicode support

### Evaluation model

- Proper tail-call optimization
- `eval` / `apply`
- Lisp-style `defmacro` non-hygienic macros
- Delayed evaluation and streams (via the `lazy` library)

### Runtime and tooling

- Garbage collection (Boehm GC)
- Interactive REPL with:
    - multi-line editing
    - persistent history
    - tab completion

### Polymorphic procedures

Several common procedures are polymorphic across compound types, including:

- `len` - returns the number of objects in lists, vectors, bytevectors, strings, sets, and maps. 
- `idx` - returns the object at the supplied index for lists, vectors, bytevectors, and strings.
- `rev` - returns a reversed copy of lists, vectors, bytevectors, and strings.

---

## Intentional omissions

- **First-class continuations (`call/cc`)**  
  These are unlikely to be implemented. Control-flow patterns typically expressed using continuations may instead be 
  provided as primitive syntax or built-in forms.

- **Hygienic macros**  
  While desirable, hygienic macros are complex and currently low priority. Cozenage instead provides Lisp-style, non-hygienic macros.

---

## Features in progress, and Planned and future work

- Arbitrary-precision rationals and reals (big integers are already supported)
- Custom allocator / garbage collector
- Expanded bytevector library
- Exception handling (`try` / `catch`)
- Asynchronous ports (sockets)
- URL-backed ports
- User-defined library loading, both 'Scheme' libraries, and libraries written in C (ie: FFI).

---

## Philosophy

Cozenage is best understood as **a Scheme-inspired Lisp**, prioritizing internal consistency, approachability, and
educational value over exhaustive standards compliance.

---

## Dependencies

`Cozenage` requires [ICU](https://github.com/unicode-org/icu) for UTF-8 support. It requires the [Boehm-Demers-Weiser Garbage Collector](https://github.com/bdwgc/bdwgc).
The [GNU GMP library](https://gmplib.org/) is required for arbitrary size integers ('big ints'). 

## Obtaining Cozenage

### Git clone

The simplest and easiest way to get Cozenage is to simply clone the GitHub repository:

    $ git clone https://github.com/DarrenKirby/cozenage.git

This command will download the source tree and git metadata to a directory ``cozenage`` in the PWD. The git source will
contain at least two branches. ``main`` is the currently stable branch. This branch will *always* match the code in the
most current release available from GitHub. The ``develop`` branch contains the code under active development. While
the code from this branch is guaranteed to build and run, this is the branch that I push the most recent new features to,
and it is not as thoroughly tested as ``main``. If you want the latest, this is the branch to build.

### Downloading static packages

If you don't want to bother with git you can download a zip file or tar file (compressed with ``.gz`` or ``.xz`` 
compression) from GitHub. The latest of these source packages will always match the code in the current ``main`` branch.  

## Building Cozenage

Cozenage has been built and tested on Linux, FreeBSD, and macOS. Building on Windows will almost
certainly break without a POSIX subsystem in place. See [this bug](https://github.com/DarrenKirby/cozenage/issues/1) if you are able to help.
The build system(s) specify the C23 standard, so the build might fail on older compilers.

If you have cmake, run `make`.

If you do not have cmake, or do not want to use it, run `make nocmake`.

To build with debugging symbols, run ``make DEBUG=1``.

There is a more in-depth guide to building as part of the [Cozenage documentation](https://darrenkirby.github.io/cozenage/howto/installation.html)

## Running Cozenage

To interpret a Scheme file (typically with an .scm or .ss extension, although Cozenage will attempt
to run any file argument as Scheme code) just add the file name after any options:

    $ ./cozenage my_program.scm

To run the REPL just run the program with no arguments:

    $ ./cozenage

## Status of built-in procedures and special forms

The status of the base procedures
are documented on another page: 

[built-in and library procedures](README.status.md)

### Special forms/syntax

These are the special forms/syntax constructs that are implemented in Cozenage so far.

Implemented as primitives:

- `quote`
- `define`
- `lambda`
- `let`
- `letrec`
- `set!`
- `if`
- `begin`
- `import`
- `and`
- `defmacro`

Implemented as transforms/expands:

- `when`
- `unless`
- `cond`
- `else`
- `let*`
- `letrec*`
- `or`
- `case`
- `do`
- `Named let`
- `quasiquote`
- `unquote`
- `unquote-splicing`

Implemented as primitives in the ``lazy`` library:

- `delay`
- `delay-force`
- `stream`

