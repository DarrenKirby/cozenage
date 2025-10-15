# Cozenage

*Getting close to being ready for prime time*

## About

This started as a 'toy' Lisp, but I am steadily, if slowly, working towards a full
R7RS [Scheme implementation](https://standards.scheme.org/). After the bulk of the R7RS standard is implemented,
I plan to add several `(cozenage foo)` libraries to interface with the OS, and eventually, I would
like to implement some kind of 'shell mode' which would act like an interpretive shell 
with Scheme syntax, kicking all 'unbound symbols' down a level as potential shell commands.

This is an educational process for me. I started writing Cozenage to enhance my understanding of 
Scheme, C , and programming language fundamentals in general.
Not everything here is implemented in the most efficient or best way. Cozenage is a work in progress,
and as I learn new and better techniques I will come back and improve sections of this program. Perhaps
the most conspicuous deficiency is the lexer/parser. I am currently working through Robert Nystrom's
"Crafting Interpreters", and I will rewrite Cozenage's parser when I finish. At that time I will
also have to revisit my decision to implement the bare-bones 'AST' as a plain C array of Cell objects,
rather than as a 'proper list', as it is done in all dogmatic Lisp and Scheme variants.

A few things that I feel I have done well:

- Full tower of numeric types, allowing for operations on integers, rationals, reals, complex numbers, and any arbitrary
combination thereof.
- Symbol interning and the global environment using a fast hash table.
- Proper tail-call recursion for most special forms which prescribe it, including 'lambda', 'if', 'else', 
'cond', 'when', 'unless', 'and', and 'or', plus the procedure 'apply'. Coming soon: tail recursion for 
'let', 'let*', and 'letrec'. 

## Dependencies

`Cozenage` requires one of [readline](https://tiswww.cwru.edu/php/chet/readline/rltop.html) or 
libedit for the REPL. It requires [ICU](https://github.com/unicode-org/icu) for Unicode.
These will almost certainly be installed already on any sort of development rig. It requires the [Boehm-Demers-Weiser Garbage Collector](https://github.com/bdwgc/bdwgc)
which may or may not be installed already on your system.

## Building Cozenage

If you have cmake, run `make`.

If you do not have cmake, run `make nocmake`.

To build the tests run `make tests`. You will need `criterion` installed.

To delete objects and binaries run `make clean`

Run `./cozenage` to enter the funhouse.

You can add the `-l/--library` flag with a comma-delimited list of libraries to load on startup
(ie: complex,inexact,char etc.). Otherwise, you can load them from the REPL using the usual Scheme syntax: 
`(import (scheme char))`. User-defined libraries are not yet supported.

Note that not all built-in procedures are implemented yet. Not all base libraries are implemented yet, and not all procedures in
the libraries I have started are implemented yet.

This is a list of what *is* implemented so far. I try to keep it up to date:

## Builtin procedures (scheme base)

### Special forms/syntax

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

### Basic arithmetic 

- `+`
- `-`
- `*`
- `/`

### Comparison operators

- `=`
- `>`
- `<`
- `>=`
- `<=`

### Numeric predicate procedures

- `zero?`
- `positive?`
- `negative?`
- `odd?`
- `even?`

### Equality and equivalence comparators

- `eq?`
- `eqv?`
- `equal?`

### Generic numeric operations

- `abs`
- `expt`
- `remainder`
- `modulo`
- `quotient`
- `lcm`
- `gcd`
- `max`
- `min`
- `floor`
- `ceiling`
- `round`
- `truncate`
- `numerator`
- `denominator`
- `truncate-quotient`
- `truncate-remainder`
- `floor-remainder`
- `square`
- `exact`
- `inexact`

### Type identity predicate procedures

- `number?`
- `boolean?`
- `null?`
- `pair?`
- `list?`
- `procedure?`
- `symbol?`
- `string?`
- `char?`
- `vector?`
- `bytevector?`
- `port?`
- `eof-object?`

### Numeric identity predicate procedures

- `exact?`
- `inexact?`
- `complex?`
- `real?`
- `rational?`
- `integer?`
- `exact-integer?`

### Boolean and logical procedures

- `not`
- `boolean=?`

### Pair/list procedures

- `cons`
- `car`
- `cdr`
- `caar`
- `cadr`
- `cdar`
- `cddr`
- `set-car!`
- `set-cdr!`
- `list`
- `length`
- `list-ref`
- `append`
- `reverse`
- `list-tail`
- `list-set!`
- `make-list`
- `memq`
- `memv`
- `member`
- `assq`
- `assv`
- `assoc`
- `list-copy`
- `filter`
- `foldl`
- `map`

### Vector procedures

- `vector`
- `vector-length`
- `vector-ref`
- `make-vector`
- `list->vector`
- `vector->list`
- `vector-copy`
- `string->vector`
- `vector->string`

### Bytevector procedures

- `bytevector`
- `bytevector-length`
- `bytevector-u8-ref`
- `make-bytevector`
- `bytevector-copy`

### Char procedures

- `char->int`
- `int->char`
- `char=?`
- `char<?`
- `char<=?`
- `char>?`
- `char>=?`

### String/Symbol procedures

- `symbol=?`
- `symbol->string`
- `string->symbol`
- `string`
- `string-length`
- `string=?`
- `string<?`
- `string<=?`
- `string>?`
- `string>=?`
- `string-append`

### Control features and list iteration

- `apply`
- `map`

### input/output and ports

- `current-input-port`
- `current-output-port`
- `current-error-port`
- `input-port?`
- `output-port?`
- `textual-port?`
- `binary-port?`
- `input-port-open?`
- `output-port-open?`
- `close-port`
- `close-input-port`
- `close-output-port`
- `read-line`
- `write-string`
- `newline`

## R7RS Scheme library procedures

### Eval library (scheme eval)

- `eval`

### Complex library (scheme complex)

- `real-part`
- `imag-part`
- `make-rectangular`
- `magnitude`
- `angle`
- `make-polar`

### File library (scheme file)

- `file-exists?`
- `delete-file`
- `open-input-file`
- `open-binary-input-file`
- `open-output-file`
- `open-binary-output-file`

### Inexact library (scheme inexact)

- `cos`
- `acos`
- `sin`
- `asin`
- `tan`
- `atan`
- `exp`
- `log`
- `log2`
- `log10`
- `sqrt`
- `cbrt`
- `nan?`
- `infinite?`
- `finite?`

### Process-Context library (scheme process-context)

- `exit`
- `emergency-exit`
- `get-environment-variable`
- `get-environment-variables`

### Char library (scheme char)

- `char-alphabetic?`
- `char-whitespace?`
- `char-numeric?`
- `char-upper-case?`
- `char-lower-case?`
- `char-upcase`
- `char-downcase`
- `char-foldcase`
- `digit-value`
- `char-ci=?`
- `char-ci<?`
- `char-ci<=?`
- `char-ci>?`
- `char-ci>=?`
- `string-downcase`
- `string-upcase`
- `string-foldcase`
- `string-ci=?`
- `string-ci<?`
- `string-ci<=?`
- `string-ci>?`
- `string-ci>=?`

### Cxr library (scheme cxr)

- `caaar`
- `caadr`
- `cadar`
- `caddr`
- `cdaar`
- `cdadr`
- `cddar`
- `cdddr`
- `caaaar`
- `caaadr`
- `caadar`
- `caaddr`
- `cadaar`
- `cadadr`
- `caddar`
- `cadddr`
- `cdaaar`
- `cdaadr`
- `cdadar`
- `cdaddr`
- `cddaar`
- `cddadr`
- `cdddar`
- `cddddr`

## Cozenage library procedures (non-R7RS)

### Bits library (cozenage bits)

- `>>` (right shift)
- `<<` (left shift)
- `&` (bitwise AND)
- `|` (bitwise OR)
- `^` (bitwise XOR)
- `~` (bitwise NOT)
- `int->bitstring`
- `bitstring->int`

