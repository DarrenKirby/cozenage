# Cozenage

*Getting close to being ready for prime time*

If you have cmake, run `make`.
If you do not have cmake, run `make -f Makefile.no_cmake`

Then run `./cozenage` to enter the funhouse.

## Status of Builtin Procedures

### Basic arithmatic operators
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

### Equality and equivalence comparators
- `eq?`
- `eqv?`
- `equal?`

### Generic numeric operations
- `abs`
- `expt`
- `^`
- `remainder`
- `modulo`
- `%`
- `quotient`
- `lcm`
- `gcd`

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

### Numeric identity procedures
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

### Pair/list constructors and selectors
- `cons`
- `car`
- `cdr`
- `list`
- `length`
- `list-ref`

### Vector constructors, selectors, and procedures
- `vector`
- `vector-length`
- `vector-ref`


