# Cozenage

*Getting close to being ready for prime time*

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
(ie: complex,inexact,char etc.).

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

### Control features

- `apply`
- `eval`
- `map`

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
