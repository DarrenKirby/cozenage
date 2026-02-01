# Cozenage

*Getting close to being ready for prime time*

**Cozenage** is a Scheme-derived, Lisp-like programming language written in C, designed primarily as an educational project and exploratory implementation. It provides a small, expressive core language and is extended through a growing standard library and dynamically loadable modules.

While Cozenage borrows heavily from Scheme—particularly R5RS and R7RS—it is **not intended to be fully standard-compliant**. Where possible, Cozenage follows Scheme semantics and conventions, but it intentionally deviates from the standards in a number of non-trivial ways. These deviations are the result of pragmatic design choices, implementation constraints, or deliberate simplifications, and are documented where relevant.

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

- `len`
- `idx`
- `rev`

---

## Intentional omissions

- **First-class continuations (`call/cc`)**  
  These are unlikely to be implemented. Control-flow patterns typically expressed using continuations may instead be 
  provided as primitive syntax or built-in forms.

- **Hygienic macros**  
  While desirable, hygienic macros are complex and currently low priority. Cozenage instead provides Lisp-style, non-hygienic macros.

---

## Features in progress

- Arbitrary-precision rationals and reals  
  (big integers are already supported)

---

## Planned and future work

- Custom line editor (replace readline/libedit)
- Custom allocator / garbage collector
- Expanded bytevector library
- Native `set` and `map` types
- Exception handling (`try` / `catch`)
- Asynchronous ports (sockets)
- URL-backed ports
- User-defined library loading, both 'Scheme' libraries, and libraries written in C (ie: FFI).

---

## Philosophy

Cozenage is best understood as **a Scheme-inspired Lisp**, prioritizing internal consistency, approachability, and educational value over exhaustive standards compliance.

### Design goals

- **Clarity over completeness**  
  Prefer understandable, inspectable implementations over strict standards compliance.

- **A small, coherent core**  
  Keep the core language modest, with additional functionality layered on via libraries and modules.

- **Exploration and learning**  
  Cozenage exists as a vehicle for learning about language implementation, interpreter design, and runtime systems.

- **Practical Lisp semantics**  
  Favor straightforward, predictable behavior over obscure or highly abstract features.

---

## Dependencies

`Cozenage` requires one of [readline](https://tiswww.cwru.edu/php/chet/readline/rltop.html) or 
libedit for the REPL. It requires [ICU](https://github.com/unicode-org/icu) for UTF-8.
These will almost certainly be installed already on any sort of development rig. It requires the [Boehm-Demers-Weiser Garbage Collector](https://github.com/bdwgc/bdwgc)
which may or may not be installed already on your system. The [GNU GMP library](https://gmplib.org/) is required for the fledgling arbitrary size/precision numeric types I am currently implementing. The [GNU MPFR library](https://www.mpfr.org/) will be required soon, as I wire up bigfloats. 

## Building Cozenage

Cozenage has been built and tested on Linux, FreeBSD, and macOS. Building on Windows will almost
certainly break without a POSIX subsystem in place. See [this bug](https://github.com/DarrenKirby/cozenage/issues/1) if you are able to help.
The build system(s) specify the C23 standard, so the build might fail on older compilers.

If you have cmake, run `make`.

If you do not have cmake, or do not want to use it, run `make nocmake`.

There is also a bit more in depth guide as part of the [Cozenage documentation](https://darrenkirby.github.io/cozenage/howto/installation.html)

## Running Cozenage

To interpret a Scheme file (typically with an .scm or .ss extension, although Cozenage will attempt
to run any file argument as Scheme code) just add the file name after any options:

    $ ./cozenage -l system,file my_program.scm

To run the REPL just run the program with no arguments:

    $ ./cozenage -l system,file

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

