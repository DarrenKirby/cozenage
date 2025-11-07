# Cozenage R7RS Implementation Status

This document tracks the implementation status of the R7RS standard libraries in the Cozenage Scheme interpreter.

---

## `(scheme base)`

| Status | Procedure                        |
|:------:|:---------------------------------|
|   ✅    | `*`                              |
|   ✅    | `+`                              |
|   ✅    | `-`                              |
|   ✅    | `/`                              |
|   ✅    | `<`                              |
|   ✅    | `<=`                             |
|   ✅    | `=`                              |
|   ✅    | `>`                              |
|   ✅    | `>=`                             |
|   ✅    | `abs`                            |
|   ✅    | `append`                         |
|   ✅    | `apply`                          |
|   ✅    | `assoc`                          |
|   ✅    | `assq`                           |
|   ✅    | `assv`                           |
|   ✅    | `binary-port?`                   |
|   ✅    | `boolean=?`                      |
|   ✅    | `boolean?`                       |
|   ✅    | `bytevector`                     |
|   ❌    | `bytevector-append`              |
|   ✅    | `bytevector-copy`                |
|   ❌    | `bytevector-copy!`               |
|   ✅    | `bytevector-length`              |
|   ✅    | `bytevector-u8-ref`              |
|   ❌    | `bytevector-u8-set!`             |
|   ✅    | `bytevector?`                    |
|   ✅    | `caar`                           |
|   ✅    | `cadr`                           |
|   ❌    | `call-with-current-continuation` |
|   ❌    | `call-with-port`                 |
|   ❌    | `call-with-values`               |
|   ❌    | `call/cc`                        |
|   ✅    | `car`                            |
|   ✅    | `cdar`                           |
|   ✅    | `cddr`                           |
|   ✅    | `cdr`                            |
|   ✅    | `ceiling`                        |
|   ✅    | `char->integer`                  |
|   ❌    | `char-ready?`                    |
|   ✅    | `char<=?`                        |
|   ✅    | `char<?`                         |
|   ✅    | `char=?`                         |
|   ✅    | `char>=?`                        |
|   ✅    | `char>?`                         |
|   ✅    | `char?`                          |
|   ✅    | `close-input-port`               |
|   ✅    | `close-output-port`              |
|   ✅    | `close-port`                     |
|   ✅    | `complex?`                       |
|   ✅    | `cons`                           |
|   ✅    | `current-error-port`             |
|   ✅    | `current-input-port`             |
|   ✅    | `current-output-port`            |
|   ✅    | `denominator`                    |
|   ❌    | `dynamic-wind`                   |
|   ✅    | `eof-object`                     |
|   ✅    | `eof-object?`                    |
|   ✅    | `eq?`                            |
|   ✅    | `equal?`                         |
|   ✅    | `eqv?`                           |
|   ❌    | `error`                          |
|   ❌    | `error-object-irritants`         |
|   ❌    | `error-object-message`           |
|   ✅    | `error-object?`                  |
|   ✅    | `even?`                          |
|   ✅    | `exact`                          |
|   ❌    | `exact-integer-sqrt`             |
|   ✅    | `exact-integer?`                 |
|   ✅    | `exact?`                         |
|   ✅    | `expt`                           |
|   ❌    | `features`                       |
|   ✅    | `file-error?`                    |
|   ✅    | `floor`                          |
|   ❌    | `floor-quotient`                 |
|   ✅    | `floor-remainder`                |
|   ❌    | `floor/`                         |
|   ❌    | `flush-output-port`              |
|   ✅    | `for-each`                       |
|   ✅    | `gcd`                            |
|   ❌    | `get-output-bytevector`          |
|   ❌    | `get-output-string`              |
|   ❌    | `guard`                          |
|   ✅    | `inexact`                        |
|   ✅    | `inexact?`                       |
|   ✅    | `input-port-open?`               |
|   ✅    | `input-port?`                    |
|   ✅    | `integer->char`                  |
|   ✅    | `integer?`                       |
|   ✅    | `lcm`                            |
|   ✅    | `length`                         |
|   ✅    | `list`                           |
|   ✅    | `list->string`                   |
|   ✅    | `list->vector`                   |
|   ✅    | `list-copy`                      |
|   ✅    | `list-ref`                       |
|   ✅    | `list-set!`                      |
|   ✅    | `list-tail`                      |
|   ✅    | `list?`                          |
|   ✅    | `make-bytevector`                |
|   ✅    | `make-list`                      |
|   ❌    | `make-parameter`                 |
|   ✅    | `make-string`                    |
|   ✅    | `make-vector`                    |
|   ✅    | `map`                            |
|   ✅    | `max`                            |
|   ✅    | `member`                         |
|   ✅    | `memq`                           |
|   ✅    | `memv`                           |
|   ✅    | `min`                            |
|   ✅    | `modulo`                         |
|   ✅    | `negative?`                      |
|   ✅    | `newline`                        |
|   ✅    | `not`                            |
|   ✅    | `null?`                          |
|   ❌    | `number->string`                 |
|   ✅    | `number?`                        |
|   ✅    | `numerator`                      |
|   ✅    | `odd?`                           |
|   ❌    | `open-input-bytevector`          |
|   ❌    | `open-input-string`              |
|   ❌    | `open-output-bytevector`         |
|   ❌    | `open-output-string`             |
|   ✅    | `output-port-open?`              |
|   ✅    | `output-port?`                   |
|   ✅    | `pair?`                          |
|   ❌    | `parameterize`                   |
|   ✅    | `peek-char`                      |
|   ❌    | `peek-u8`                        |
|   ✅    | `port?`                          |
|   ✅    | `positive?`                      |
|   ✅    | `procedure?`                     |
|   ✅    | `quotient`                       |
|   ❌    | `raise`                          |
|   ❌    | `raise-continuable`              |
|   ✅    | `rational?`                      |
|   ❌    | `rationalize`                    |
|   ❌    | `read-bytevector`                |
|   ❌    | `read-bytevector!`               |
|   ✅    | `read-char`                      |
|   ✅    | `read-error?`                    |
|   ✅    | `read-line`                      |
|   ❌    | `read-string`                    |
|   ❌    | `read-u8`                        |
|   ✅    | `real?`                          |
|   ✅    | `remainder`                      |
|   ✅    | `reverse`                        |
|   ✅    | `round`                          |
|   ✅    | `set-car!`                       |
|   ✅    | `set-cdr!`                       |
|   ✅    | `square`                         |
|   ✅    | `string`                         |
|   ✅    | `string->list`                   |
|   ❌    | `string->number`                 |
|   ✅    | `string->symbol`                 |
|   ❌    | `string->utf8`                   |
|   ✅    | `string->vector`                 |
|   ✅    | `string-append`                  |
|   ✅    | `string-copy`                    |
|   ✅    | `string-copy!`                   |
|   ❌    | `string-fill!`                   |
|   ✅    | `string-for-each`                |
|   ✅    | `string-length`                  |
|   ✅    | `string-map`                     |
|   ✅    | `string-ref`                     |
|   ❌    | `string-set!`                    |
|   ✅    | `string<=?`                      |
|   ✅    | `string<?`                       |
|   ✅    | `string=?`                       |
|   ✅    | `string>=?`                      |
|   ✅    | `string>?`                       |
|   ✅    | `string?`                        |
|   ✅    | `substring`                      |
|   ✅    | `symbol->string`                 |
|   ✅    | `symbol=?`                       |
|   ✅    | `symbol?`                        |
|   ✅    | `textual-port?`                  |
|   ✅    | `truncate`                       |
|   ✅    | `truncate-quotient`              |
|   ✅    | `truncate-remainder`             |
|   ❌    | `truncate/`                      |
|   ❌    | `u8-ready?`                      |
|   ❌    | `utf8->string`                   |
|   ❌    | `values`                         |
|   ✅    | `vector`                         |
|   ✅    | `vector->list`                   |
|   ✅    | `vector->string`                 |
|   ✅    | `vector-append`                  |
|   ✅    | `vector-copy`                    |
|   ❌    | `vector-copy!`                   |
|   ✅    | `vector-fill!`                   |
|   ✅    | `vector-for-each`                |
|   ✅    | `vector-length`                  |
|   ✅    | `vector-map`                     |
|   ✅    | `vector-ref`                     |
|   ✅    | `vector-set!`                    |
|   ✅    | `vector?`                        |
|   ❌    | `with-exception-handler`         |
|   ❌    | `write-bytevector`               |
|   ❌    | `write-char`                     |
|   ✅    | `write-string`                   |
|   ❌    | `write-u8`                       |
|   ✅    | `zero?`                          |

---

## `(scheme case-lambda)`

| Status | Procedure |
|:---:|:---|
| ❌ | `case-lambda` |

---

## `(scheme char)`

| Status | Procedure |
|:---:|:---|
| ✅ | `char-alphabetic?` |
| ✅ | `char-ci<=?` |
| ✅ | `char-ci<?` |
| ✅ | `char-ci=?` |
| ✅ | `char-ci>=?` |
| ✅ | `char-ci>?` |
| ✅ | `char-downcase` |
| ✅ | `char-foldcase` |
| ✅ | `char-lower-case?` |
| ✅ | `char-numeric?` |
| ✅ | `char-upcase` |
| ✅ | `char-upper-case?` |
| ✅ | `char-whitespace?` |
| ✅ | `digit-value` |
| ✅ | `string-ci<=?` |
| ✅ | `string-ci<?` |
| ✅ | `string-ci=?` |
| ✅ | `string-ci>=?` |
| ✅ | `string-ci>?` |
| ✅ | `string-downcase` |
| ✅ | `string-foldcase` |
| ✅ | `string-upcase` |

---

## `(scheme complex)`

| Status | Procedure |
|:---:|:---|
| ✅ | `angle` |
| ✅ | `imag-part` |
| ✅ | `magnitude` |
| ✅ | `make-polar` |
| ✅ | `make-rectangular` |
| ✅ | `real-part` |

---

## `(scheme cxr)`

| Status | Procedure |
|:---:|:---|
| ✅ | `caaaar` |
| ✅ | `caaadr` |
| ✅ | `caaar` |
| ✅ | `caadar` |
| ✅ | `caaddr` |
| ✅ | `caadr` |
| ✅ | `cadaar` |
| ✅ | `cadadr` |
| ✅ | `cadar` |
| ✅ | `caddar` |
| ✅ | `cadddr` |
| ✅ | `caddr` |
| ✅ | `cdaaar` |
| ✅ | `cdaadr` |
| ✅ | `cdaar` |
| ✅ | `cdadar` |
| ✅ | `cdaddr` |
| ✅ | `cdadr` |
| ✅ | `cddaar` |
| ✅ | `cddadr` |
| ✅ | `cddar` |
| ✅ | `cdddar` |
| ✅ | `cddddr` |
| ✅ | `cdddr` |

---

## `(scheme eval)`

| Status | Procedure |
|:---:|:---|
| ❌ | `environment` |
| ✅ | `eval` |

---

## `(scheme file)`

| Status | Procedure |
|:---:|:---|
| ❌ | `call-with-input-file` |
| ❌ | `call-with-output-file` |
| ✅ | `delete-file` |
| ✅ | `file-exists?` |
| ✅ | `open-binary-input-file` |
| ✅ | `open-binary-output-file` |
| ✅ | `open-input-file` |
| ✅ | `open-output-file` |
| ❌ | `with-input-from-file` |
| ❌ | `with-output-to-file` |

---

## `(scheme inexact)`

| Status | Procedure |
|:---:|:---|
| ✅ | `acos` |
| ✅ | `asin` |
| ✅ | `atan` |
| ✅ | `cos` |
| ✅ | `exp` |
| ✅ | `finite?` |
| ✅ | `infinite?` |
| ✅ | `log` |
| ✅ | `nan?` |
| ✅ | `sin` |
| ✅ | `sqrt` |
| ✅ | `tan` |

---

## `(scheme lazy)`

| Status | Procedure |
|:---:|:---|
| ❌ | `delay` |
| ❌ | `delay-force` |
| ❌ | `force` |
| ❌ | `make-promise` |
| ❌ | `promise?` |

---

## `(scheme load)`

| Status | Procedure |
|:---:|:---|
| ✅ | `load` |

---

## `(scheme process-context)`

| Status | Procedure |
|:---:|:---|
| ❌ | `command-line` |
| ✅ | `exit` |
| ✅ | `get-environment-variable` |
| ✅ | `get-environment-variables` |
| ❌ | `emergency-exit` |

---

## `(scheme read)`

| Status | Procedure |
|:---:|:---|
| ❌ | `read` |

---

## `(scheme repl)`

| Status | Procedure |
|:---:|:---|
| ❌ | `interaction-environment` |

---

## `(scheme time)`

| Status | Procedure |
|:---:|:---|
| ✅ | `current-jiffy` |
| ✅ | `current-second` |
| ✅ | `jiffies-per-second` |

---

## `(scheme write)`

| Status | Procedure |
|:---:|:---|
| ✅ | `display` |
| ❌ | `write-shared` |
| ❌ | `write` |
| ❌ | `write-simple` |

