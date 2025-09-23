# Cozenage

*Getting close to being ready for prime time*

If you have cmake, run `make`.
If you do not have cmake, run `make nocmake`.
To build the tests run `make tests`. You will need `criterion` installed.
To delete objects and binaries run `make clean`

Then run `./cozenage` to enter the funhouse.

## Builtin Procedures (scheme base)

### Basic arithmetic operators
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

## Complex Library (scheme complex)

- `real-part`
- `imag-part`
- `make-rectangular`
- `magnitude`
- `angle`
- `make-polar`

## File Library (scheme file)
- `file-exists?`
- `delete-file`

## Inexact Library (scheme inexact)
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

## Process-Context Library (scheme process-context)
- `exit`
- `emergency-exit`
- `get-environment-variable`
- `get-environment-variables`
