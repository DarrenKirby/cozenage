# Cozenage Procedure Implementation Status

This document tracks the implementation status of procedures in the Cozenage interpreter.

---

## Core Interpreter Procedures

| Status | Procedure                 |
|:------:|:--------------------------|
|   ✅    | `*`                       |
|   ✅    | `+`                       |
|   ✅    | `-`                       |
|   ✅    | `/`                       |
|   ✅    | `<`                       |
|   ✅    | `<=`                      |
|   ✅    | `=`                       |
|   ✅    | `>`                       |
|   ✅    | `>=`                      |
|   ✅    | `abs`                     |
|   ✅    | `append`                  |
|   ✅    | `apply`                   |
|   ✅    | `assoc`                   |
|   ✅    | `assq`                    |
|   ✅    | `assv`                    |
|   ✅    | `binary-port?`            |
|   ✅    | `boolean=?`               |
|   ✅    | `boolean?`                |
|   ✅    | `bytevector`              |
|   ✅    | `bytevector-append`       |
|   ✅    | `bytevector-copy`         |
|   ✅    | `bytevector-copy!`        |
|   ✅    | `bytevector-length`       |
|   ✅    | `bytevector-u8-ref`       |
|   ✅    | `bytevector-u8-set!`      |
|   ✅    | `bytevector?`             |
|   ❌    | `call-with-input-file`    |
|   ❌    | `call-with-output-file`   |
|   ✅    | `caar`                    |
|   ✅    | `cadr`                    |
|   ✅    | `car`                     |
|   ✅    | `cdar`                    |
|   ✅    | `cddr`                    |
|   ✅    | `cdr`                     |
|   ✅    | `ceiling`                 |
|   ✅    | `char->integer`           |
|   ✅    | `char-ready?`             |
|   ✅    | `char<=?`                 |
|   ✅    | `char<?`                  |
|   ✅    | `char=?`                  |
|   ✅    | `char>=?`                 |
|   ✅    | `char>?`                  |
|   ✅    | `char?`                   |
|   ✅    | `char-alphabetic?`        |
|   ✅    | `char-ci<=?`              |
|   ✅    | `char-ci<?`               |
|   ✅    | `char-ci=?`               |
|   ✅    | `char-ci>=?`              |
|   ✅    | `char-ci>?`               |
|   ✅    | `char-downcase`           |
|   ✅    | `char-foldcase`           |
|   ✅    | `char-lower-case?`        |
|   ✅    | `char-numeric?`           |
|   ✅    | `char-upcase`             |
|   ✅    | `char-upper-case?`        |
|   ✅    | `char-whitespace?`        |
|   ✅    | `close-input-port`        |
|   ✅    | `close-output-port`       |
|   ✅    | `close-port`              |
|   ✅    | `command-line`            |
|   ✅    | `complex?`                |
|   ✅    | `cons`                    |
|   ✅    | `current-error-port`      |
|   ✅    | `current-input-port`      |
|   ✅    | `current-output-port`     |
|   ✅    | `denominator`             |
|   ✅    | `digit-value`             |
|   ✅    | `display`                 |
|   ✅    | `eof-object`              |
|   ✅    | `eof-object?`             |
|   ✅    | `eq?`                     |
|   ✅    | `equal?`                  |
|   ✅    | `eqv?`                    |
|   ❌    | `error`                   |
|   ❌    | `error-object-irritants`  |
|   ❌    | `error-object-message`    |
|   ✅    | `error-object?`           |
|   ✅    | `even?`                   |
|   ✅    | `eval`                    |
|   ✅    | `exact`                   |
|   ✅    | `exact-integer?`          |
|   ✅    | `exact?`                  |
|   ✅    | `expt`                    |
|   ✅    | `exit`                    |
|   ✅    | `features`                |
|   ✅    | `file-error?`             |
|   ✅    | `filter`                  |
|   ✅    | `floor`                   |
|   ✅    | `flush-output-port`       |
|   ✅    | `for-each`                |
|   ✅    | `foldl`                   |
|   ✅    | `gcd`                     |
|   ❌    | `get-output-bytevector`   |
|   ❌    | `get-output-string`       |
|   ✅    | `inexact`                 |
|   ✅    | `inexact?`                |
|   ✅    | `input-port-open?`        |
|   ✅    | `input-port?`             |
|   ✅    | `integer->char`           |
|   ✅    | `integer?`                |
|   ✅    | `lcm`                     |
|   ✅    | `length`                  |
|   ✅    | `list`                    |
|   ✅    | `list->string`            |
|   ✅    | `list->vector`            |
|   ✅    | `list-copy`               |
|   ✅    | `list-ref`                |
|   ✅    | `list-set!`               |
|   ✅    | `list-tail`               |
|   ✅    | `list?`                   |
|   ✅    | `load`                    |
|   ✅    | `make-bytevector`         |
|   ✅    | `make-list`               |
|   ✅    | `make-string`             |
|   ✅    | `make-vector`             |
|   ✅    | `map`                     |
|   ✅    | `max`                     |
|   ✅    | `member`                  |
|   ✅    | `memq`                    |
|   ✅    | `memv`                    |
|   ✅    | `min`                     |
|   ✅    | `modulo`                  |
|   ✅    | `negative?`               |
|   ✅    | `newline`                 |
|   ✅    | `not`                     |
|   ✅    | `null?`                   |
|   ✅    | `number->string`          |
|   ✅    | `number?`                 |
|   ✅    | `numerator`               |
|   ✅    | `odd?`                    |
|   ❌    | `open-input-bytevector`   |
|   ❌    | `open-input-string`       |
|   ❌    | `open-output-bytevector`  |
|   ❌    | `open-output-string`      |
|   ✅    | `open-binary-input-file`  |
|   ✅    | `open-binary-output-file` |
|   ✅    | `open-input-file`         |
|   ✅    | `open-output-file`        |
|   ✅    | `output-port-open?`       |
|   ✅    | `output-port?`            |
|   ✅    | `pair?`                   |
|   ✅    | `peek-char`               |
|   ❌    | `peek-u8`                 |
|   ✅    | `port?`                   |
|   ✅    | `positive?`               |
|   ✅    | `procedure?`              |
|   ✅    | `quotient`                |
|   ❌    | `raise`                   |
|   ✅    | `rational?`               |
|   ❌    | `read`                    |
|   ❌    | `read-bytevector`         |
|   ❌    | `read-bytevector!`        |
|   ✅    | `read-char`               |
|   ✅    | `read-error?`             |
|   ✅    | `read-line`               |
|   ✅    | `read-string`             |
|   ❌    | `read-u8`                 |
|   ✅    | `real?`                   |
|   ✅    | `remainder`               |
|   ✅    | `reverse`                 |
|   ✅    | `round`                   |
|   ✅    | `set-car!`                |
|   ✅    | `set-cdr!`                |
|   ✅    | `square`                  |
|   ✅    | `string`                  |
|   ✅    | `string->list`            |
|   ✅    | `string->number`          |
|   ✅    | `string->symbol`          |
|   ✅    | `string->utf8`            |
|   ✅    | `string->vector`          |
|   ✅    | `string-append`           |
|   ✅    | `string-copy`             |
|   ✅    | `string-copy!`            |
|   ❌    | `string-fill!`            |
|   ✅    | `string-for-each`         |
|   ✅    | `string-length`           |
|   ✅    | `string-map`              |
|   ✅    | `string-ref`              |
|   ❌    | `string-set!`             |
|   ✅    | `string<=?`               |
|   ✅    | `string<?`                |
|   ✅    | `string=?`                |
|   ✅    | `string>=?`               |
|   ✅    | `string>?`                |
|   ✅    | `string?`                 |
|   ✅    | `string-ci<=?`            |
|   ✅    | `string-ci<?`             |
|   ✅    | `string-ci=?`             |
|   ✅    | `string-ci>=?`            |
|   ✅    | `string-ci>?`             |
|   ✅    | `string-downcase`         |
|   ✅    | `string-foldcase`         |
|   ✅    | `string-upcase`           |
|   ✅    | `sqrt`                    |
|   ✅    | `substring`               |
|   ✅    | `symbol->string`          |
|   ✅    | `symbol=?`                |
|   ✅    | `symbol?`                 |
|   ✅    | `textual-port?`           |
|   ✅    | `truncate`                |
|   ✅    | `u8-ready?`               |
|   ✅    | `utf8->string`            |
|   ✅    | `vector`                  |
|   ✅    | `vector->list`            |
|   ✅    | `vector->string`          |
|   ✅    | `vector-append`           |
|   ✅    | `vector-copy`             |
|   ✅    | `vector-copy!`            |
|   ✅    | `vector-fill!`            |
|   ✅    | `vector-for-each`         |
|   ✅    | `vector-length`           |
|   ✅    | `vector-map`              |
|   ✅    | `vector-ref`              |
|   ✅    | `vector-set!`             |
|   ✅    | `vector?`                 |
|   ❌    | `with-exception-handler`  |
|   ❌    | `with-input-from-file`    |
|   ❌    | `with-output-to-file`     |
|   ✅    | `write`                   |
|   ✅    | `write-bytevector`        |
|   ✅    | `write-char`              |
|   ✅    | `write-string`            |
|   ✅    | `write-u8`                |
|   ✅    | `zero?`                   |
|   ✅    | `zip`                     |
---

## `(base math)`

| Status | Procedure            |
|:------:|:---------------------|
|   ✅    | `angle`              |
|   ✅    | `imag-part`          |
|   ✅    | `magnitude`          |
|   ✅    | `make-polar`         |
|   ✅    | `make-rectangular`   |
|   ✅    | `real-part`          |
|   ✅    | `acos`               |
|   ✅    | `asin`               |
|   ✅    | `atan`               |
|   ✅    | `cos`                |
|   ✅    | `exp`                |
|   ✅    | `finite?`            |
|   ✅    | `infinite?`          |
|   ✅    | `log`                |
|   ✅    | `nan?`               |
|   ✅    | `sin`                |
|   ✅    | `tan`                |
|   ✅    | `floor/`             |
|   ✅    | `truncate/`          |
|   ✅    | `truncate-quotient`  |
|   ✅    | `truncate-remainder` |
|   ✅    | `rationalize`        |
|   ✅    | `exact-integer-sqrt` |
|   ✅    | `floor-quotient`     |
|   ✅    | `floor-remainder`    |

---

## `(base cxr)`

| Status | Procedure |
|:------:|:----------|
|   ✅    | `caaaar`  |
|   ✅    | `caaadr`  |
|   ✅    | `caaar`   |
|   ✅    | `caadar`  |
|   ✅    | `caaddr`  |
|   ✅    | `caadr`   |
|   ✅    | `cadaar`  |
|   ✅    | `cadadr`  |
|   ✅    | `cadar`   |
|   ✅    | `caddar`  |
|   ✅    | `cadddr`  |
|   ✅    | `caddr`   |
|   ✅    | `cdaaar`  |
|   ✅    | `cdaadr`  |
|   ✅    | `cdaar`   |
|   ✅    | `cdadar`  |
|   ✅    | `cdaddr`  |
|   ✅    | `cdadr`   |
|   ✅    | `cddaar`  |
|   ✅    | `cddadr`  |
|   ✅    | `cddar`   |
|   ✅    | `cdddar`  |
|   ✅    | `cddddr`  |
|   ✅    | `cdddr`   |


---

## `(base file)`

| Status | Procedure             |
|:------:|:----------------------|
|   ✅    | `reg-file?`           |
|   ✅    | `directory?`          |
|   ✅    | `symlink?`            |
|   ✅    | `char-device?`        |
|   ✅    | `block-device?`       |
|   ✅    | `fifo?`               |
|   ✅    | `socket?`             |
|   ✅    | `getcwd`              |
|   ✅    | `rmdir`               |
|   ✅    | `mkdir`               |
|   ✅    | `unlink!`             |
|   ✅    | `file-exists?`        |


---

## `(base system)`

|  Status  | Procedure                   |
|:--------:|:----------------------------|
|    ✅     | `get-pid`                   |
|    ✅     | `get-ppid`                  |
|    ✅     | `get-environment-variable`  |
|    ✅     | `get-environment-variables` |


---

## `(base time)`

| Status | Procedure            |
|:------:|:---------------------|
|   ✅    | `current-jiffy`      |
|   ✅    | `current-second`     |
|   ✅    | `jiffies-per-second` |
|   ✅    | `current-dt-utc`     |
|   ✅    | `current-dt-local`   |

---

