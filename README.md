# Cozenage

*Getting close to being ready for prime time*

This started as a 'toy' Lisp, but I am steadily, if slowly, working towards a full
R7RS [Scheme implementation](https://standards.scheme.org/). After the bulk of the R7RS standard is implemented,
I plan to add several `(cozenage foo)` libraries to interface with the OS, and eventually morph this
into some kind of usable shell with Scheme syntax.

`Cozenage` requires one of [readline](https://tiswww.cwru.edu/php/chet/readline/rltop.html) or 
libedit for the REPL. It requires [ICU](https://github.com/unicode-org/icu) for Unicode.
These will almost certainly be installed already on any sort of development rig. It requires the [Boehm-Demers-Weiser Garbage Collector](https://github.com/bdwgc/bdwgc)
which may or may not be installed already on your system.

If you have cmake, run `make`.

If you do not have cmake, run `make nocmake`.

To build the tests run `make tests`. You will need `criterion` installed.

To delete objects and binaries run `make clean`

Then run `./cozenage` to enter the funhouse.

You can add the `-l/--library` flag with a comma-delimited list of libraries to load on startup
(ie: complex,inexact,char etc.). Otherwise, you can load them from the REPL using the usual Scheme syntax: 
`(import (scheme char))`. User-defined libraries are not yet supported.

Note that not all built-in procedures are implemented yet. Not all base libraries are implemented yet, and not all procedures in
the libraries I have started are implemented yet.

This is a list of what *is* implemented so far. I try to keep it up to date:

## Builtin Procedures (scheme base)

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

### Special forms

- `quote`
- `define`
- `lambda`
- `if`
- `when`
- `unless`
- `cond`
- `else`
- `import`

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
- `and`
- `or`
- `boolean`

### Pair/list procedures

- `cons`
- `car`
- `cdr`
- `list`
- `length`
- `list-ref`
- `append`
- `reverse`
- `list-tail`

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
- `bytevector-u8--ref`
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

### String procedures

- `symbol->string`
- `string->symbol`
- `string`
- `string-length`
- `string=?`
- `string<?`
- `string<=?`
- `string>?`
- `string>=?`

### Control features and list iteration

- `apply`
- `eval`
- `map`
- `filter`
- `foldl`

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

## Library procedures

### Complex Library (scheme complex)

- `real-part`
- `imag-part`
- `make-rectangular`
- `magnitude`
- `angle`
- `make-polar`

### File Library (scheme file)

- `file-exists?`
- `delete-file`
- `open-input-file`
- `open-binary-input-file`
- `open-output-file`
- `open-binary-output-file`

### Inexact Library (scheme inexact)

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
- `finite`

### Process-Context Library (scheme process-context)

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
