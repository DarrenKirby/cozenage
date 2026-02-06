# Cozenage Procedure Implementation Status

This document tracks the implementation status of procedures in the Cozenage interpreter.

---

## Core Interpreter Procedures

| Status | Procedure                    |
|:------:|:-----------------------------|
|   ✅    | `*`                          |
|   ✅    | `+`                          |
|   ✅    | `-`                          |
|   ✅    | `/`                          |
|   ✅    | `<`                          |
|   ✅    | `<=`                         |
|   ✅    | `=`                          |
|   ✅    | `>`                          |
|   ✅    | `>=`                         |
|   ✅    | `abs`                        |
|   ✅    | `append`                     |
|   ✅    | `apply`                      |
|   ✅    | `assoc`                      |
|   ✅    | `assq`                       |
|   ✅    | `assv`                       |
|   ✅    | `binary-port?`               |
|   ✅    | `boolean=?`                  |
|   ✅    | `boolean?`                   |
|   ✅    | `bytevector`                 |
|   ✅    | `bytevector-append`          |
|   ✅    | `bytevector-copy`            |
|   ✅    | `bytevector-copy!`           |
|   ✅    | `bytevector-length`          |
|   ✅    | `bytevector-ref`             |
|   ✅    | `bytevector-set!`            |
|   ✅    | `bytevector?`                |
|   ✅    | `call-with-input-file`       |
|   ✅    | `call-with-output-file`      |
|   ✅    | `call-with-port`             |
|   ✅    | `caar`                       |
|   ✅    | `cadr`                       |
|   ✅    | `car`                        |
|   ✅    | `cdar`                       |
|   ✅    | `cddr`                       |
|   ✅    | `cdr`                        |
|   ✅    | `ceiling`                    |
|   ✅    | `char->integer`              |
|   ✅    | `char-ready?`                |
|   ✅    | `char<=?`                    |
|   ✅    | `char<?`                     |
|   ✅    | `char=?`                     |
|   ✅    | `char>=?`                    |
|   ✅    | `char>?`                     |
|   ✅    | `char?`                      |
|   ✅    | `char-alphabetic?`           |
|   ✅    | `char-ci<=?`                 |
|   ✅    | `char-ci<?`                  |
|   ✅    | `char-ci=?`                  |
|   ✅    | `char-ci>=?`                 |
|   ✅    | `char-ci>?`                  |
|   ✅    | `char-downcase`              |
|   ✅    | `char-foldcase`              |
|   ✅    | `char-lower-case?`           |
|   ✅    | `char-numeric?`              |
|   ✅    | `char-upcase`                |
|   ✅    | `char-upper-case?`           |
|   ✅    | `char-whitespace?`           |
|   ✅    | `close-input-port`           |
|   ✅    | `close-output-port`          |
|   ✅    | `close-port`                 |
|   ✅    | `command-line`               |
|   ✅    | `complex?`                   |
|   ✅    | `cons`                       |
|   ✅    | `count`                      |
|   ✅    | `count-equal`                |
|   ✅    | `current-error-port`         |
|   ✅    | `current-input-port`         |
|   ✅    | `current-output-port`        |
|   ✅    | `denominator`                |
|   ✅    | `digit-value`                |
|   ✅    | `display`                    |
|   ✅    | `displayln`                  |
|   ✅    | `eof-object`                 |
|   ✅    | `eof-object?`                |
|   ✅    | `eq?`                        |
|   ✅    | `equal?`                     |
|   ✅    | `eqv?`                       |
|   ✅    | `error-object?`              |
|   ✅    | `even?`                      |
|   ✅    | `eval`                       |
|   ✅    | `exact`                      |
|   ✅    | `exact-integer?`             |
|   ✅    | `exact?`                     |
|   ✅    | `expt`                       |
|   ✅    | `exit`                       |
|   ✅    | `false?`                     |
|   ✅    | `features`                   |
|   ✅    | `file-error?`                |
|   ✅    | `filter`                     |
|   ✅    | `floor`                      |
|   ✅    | `flush-output-port`          |
|   ✅    | `for-each`                   |
|   ✅    | `foldl`                      |
|   ✅    | `foldr`                      |
|   ✅    | `gcd`                        |
|   ✅    | `get-output-bytevector`      |
|   ✅    | `get-output-string`          |
|   ✅    | `idx` - polymorphic          |
|   ✅    | `inexact`                    |
|   ✅    | `inexact?`                   |
|   ✅    | `input-port-open?`           |
|   ✅    | `input-port?`                |
|   ✅    | `integer->char`              |
|   ✅    | `integer?`                   |
|   ✅    | `lcm`                        |
|   ✅    | `len` - polymorphic          |
|   ✅    | `length`                     |
|   ✅    | `list`                       |
|   ✅    | `list->string`               |
|   ✅    | `list->vector`               |
|   ✅    | `list-copy`                  |
|   ✅    | `list-ref`                   |
|   ✅    | `list-set!`                  |
|   ✅    | `list-tail`                  |
|   ✅    | `list?`                      |
|   ✅    | `load`                       |
|   ✅    | `make-bytevector`            |
|   ✅    | `make-list`                  |
|   ✅    | `make-string`                |
|   ✅    | `make-vector`                |
|   ✅    | `map`                        |
|   ✅    | `max`                        |
|   ✅    | `member`                     |
|   ✅    | `memq`                       |
|   ✅    | `memv`                       |
|   ✅    | `min`                        |
|   ✅    | `modulo`                     |
|   ✅    | `negative?`                  |
|   ✅    | `newline`                    |
|   ✅    | `not`                        |
|   ✅    | `null?`                      |
|   ✅    | `number->string`             |
|   ✅    | `number?`                    |
|   ✅    | `numerator`                  |
|   ✅    | `odd?`                       |
|   ✅    | `open-input-bytevector`      |
|   ✅    | `open-input-string`          |
|   ✅    | `open-output-bytevector`     |
|   ✅    | `open-output-string`         |
|   ✅    | `open-input-file`            |
|   ✅    | `open-output-file`           |
|   ✅    | `open-binary-input-file`     |
|   ✅    | `open-binary-output-file`    |
|   ✅    | `open-and-trunc-output-file` |
|   ✅    | `output-port-open?`          |
|   ✅    | `output-port?`               |
|   ✅    | `pair?`                      |
|   ✅    | `peek-char`                  |
|   ✅    | `peek-u8`                    |
|   ✅    | `port?`                      |
|   ✅    | `positive?`                  |
|   ✅    | `procedure?`                 |
|   ✅    | `quotient`                   |
|   ✅    | `raise`                      |
|   ✅    | `rational?`                  |
|   ✅    | `read`                       |
|   ✅    | `read-bytevector`            |
|   ✅    | `read-bytevector!`           |
|   ✅    | `read-char`                  |
|   ✅    | `read-error?`                |
|   ✅    | `read-line`                  |
|   ✅    | `read-string`                |
|   ✅    | `read-u8`                    |
|   ✅    | `real?`                      |
|   ✅    | `rev` - polymorphic          |
|   ✅    | `remainder`                  |
|   ✅    | `reverse`                    |
|   ✅    | `round`                      |
|   ✅    | `set-car!`                   |
|   ✅    | `set-cdr!`                   |
|   ✅    | `square`                     |
|   ✅    | `string`                     |
|   ✅    | `string->list`               |
|   ✅    | `string->number`             |
|   ✅    | `string->symbol`             |
|   ✅    | `string->utf8`               |
|   ✅    | `string->vector`             |
|   ✅    | `string-append`              |
|   ✅    | `string-copy`                |
|   ✅    | `string-copy!`               |
|   ✅    | `string-fill!`               |
|   ✅    | `string-for-each`            |
|   ✅    | `string-length`              |
|   ✅    | `string-map`                 |
|   ✅    | `string-ref`                 |
|   ✅    | `string-set!`                |
|   ✅    | `string<=?`                  |
|   ✅    | `string<?`                   |
|   ✅    | `string=?`                   |
|   ✅    | `string>=?`                  |
|   ✅    | `string>?`                   |
|   ✅    | `string?`                    |
|   ✅    | `string-ci<=?`               |
|   ✅    | `string-ci<?`                |
|   ✅    | `string-ci=?`                |
|   ✅    | `string-ci>=?`               |
|   ✅    | `string-ci>?`                |
|   ✅    | `string-downcase`            |
|   ✅    | `string-foldcase`            |
|   ✅    | `string-split`               |
|   ✅    | `string-upcase`              |
|   ✅    | `sqrt`                       |
|   ✅    | `substring`                  |
|   ✅    | `symbol->string`             |
|   ✅    | `symbol=?`                   |
|   ✅    | `symbol?`                    |
|   ✅    | `textual-port?`              |
|   ✅    | `true?`                      |
|   ✅    | `truncate`                   |
|   ✅    | `u8-ready?`                  |
|   ✅    | `utf8->string`               |
|   ✅    | `vector`                     |
|   ✅    | `vector->list`               |
|   ✅    | `vector->string`             |
|   ✅    | `vector-append`              |
|   ✅    | `vector-copy`                |
|   ✅    | `vector-copy!`               |
|   ✅    | `vector-fill!`               |
|   ✅    | `vector-for-each`            |
|   ✅    | `vector-length`              |
|   ✅    | `vector-map`                 |
|   ✅    | `vector-ref`                 |
|   ✅    | `vector-set!`                |
|   ✅    | `vector?`                    |
|   ✅    | `with-input-from-file`       |
|   ✅    | `with-output-to-file`        |
|   ✅    | `write`                      |
|   ✅    | `writeln`                    |
|   ✅    | `write-bytevector`           |
|   ✅    | `write-char`                 |
|   ✅    | `write-string`               |
|   ✅    | `write-u8`                   |
|   ✅    | `zero?`                      |
|   ✅    | `zip`                        |
---

## `(base math)`

| Status | Procedure               |
|:------:|:------------------------|
|   ✅    | `angle`                 |
|   ✅    | `imag-part`             |
|   ✅    | `magnitude`             |
|   ✅    | `make-polar`            |
|   ✅    | `make-rectangular`      |
|   ✅    | `real-part`             |
|   ✅    | `acos`                  |
|   ✅    | `asin`                  |
|   ✅    | `atan`                  |
|   ✅    | `cos`                   |
|   ✅    | `exp`                   |
|   ✅    | `finite?`               |
|   ✅    | `infinite?`             |
|   ✅    | `log`                   |
|   ✅    | `nan?`                  |
|   ✅    | `sin`                   |
|   ✅    | `tan`                   |
|   ✅    | `floor/`                |
|   ✅    | `truncate/`             |
|   ✅    | `truncate-quotient`     |
|   ✅    | `truncate-remainder`    |
|   ✅    | `rationalize`           |
|   ✅    | `exact-integer-sqrt`    |
|   ✅    | `floor-quotient`        |
|   ✅    | `floor-remainder`       |


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

| Status | Procedure          |
|:------:|:-------------------|
|   ✅    | `reg-file?`        |
|   ✅    | `directory?`       |
|   ✅    | `symlink?`         |
|   ✅    | `char-device?`     |
|   ✅    | `block-device?`    |
|   ✅    | `fifo?`            |
|   ✅    | `socket?`          |
|   ✅    | `rmdir!`           |
|   ✅    | `mkdir`            |
|   ✅    | `unlink!`          |
|   ✅    | `file-exists?`     |
|   ✅    | `stat`             |
|   ✅    | `file-size`        |
|   ✅    | `file-mtime`       |
|   ✅    | `file-atime`       |
|   ✅    | `file-ctime`       |
|   ✅    | `file-readable?`   |
|   ✅    | `file-writable?`   |
|   ✅    | `file-executable?` |


---

## `(base system)`

|   Status   | Procedure       |
|:----------:|:----------------|
|     ✅      | `get-pid`       |
|     ✅      | `get-ppid`      |
|     ✅      | `get-env-var`   |
|     ✅      | `get-env-vars`  |
|     ✅      | `get-uid`       |
|     ✅      | `get-gid`       |
|     ✅      | `set-uid!`      |
|     ✅      | `set-gid!`      |
|     ✅      | `get-euid`      |
|     ✅      | `get-egid`      |
|     ✅      | `get-username`  |
|     ✅      | `get-groups`    |
|     ✅      | `get-cwd`       |
|     ✅      | `chdir`         |
|     ✅      | `uname`         |
|     ✅      | `uptime`        |
|     ✅      | `chmod!`        |
|     ✅      | `get-hostname`  |
|     ✅      | `get-home`      |
|     ✅      | `get-path`      |
|     ✅      | `is-root?`      |
|     ✅      | `sleep`         |
|     ✅      | `cpu-count`     |
|     ✅      | `system`        |

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

## `(base lazy`

| Status | Procedure          |
|:------:|:-------------------|
|   ✅    | `delay` - SF       |
|   ✅    | `delay-force` - SF |
|   ✅    | `stream` - SF      |
|   ✅    | `force`            |
|   ✅    | `make-promise`     |
|   ✅    | `head`             |
|   ✅    | `tail`             |
|   ✅    | `at`               |
|   ✅    | `take`             |
|   ✅    | `drop`             |
|   ✅    | `promise?`         |
|   ✅    | `stream?`          |
|   ✅    | `stream-null?`     |

---

## `(base bits`

| Status | Procedure        |
|:------:|:-----------------|
|   ✅    | `>>`             |
|   ✅    | `<<`             |
|   ✅    | `&`              |
|   ✅    | `\|`             |
|   ✅    | `~`              |
|   ✅    | `^`              |
|   ✅    | `bs+`            |
|   ✅    | `bs-`            |
|   ✅    | `bs*`            |
|   ✅    | `bs/`            |
|   ✅    | `bitstring->int` |
|   ✅    | `int->bitstring` |
