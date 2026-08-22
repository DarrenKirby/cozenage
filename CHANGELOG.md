# Changelog

## [0.19.1] - 2026-08-21

### Added

- Test suite for the `base math` library
- Dockerfile which helps test building against musl.libc, but may be appreciated by others

### Fixed

- Remove fprintf statements that dump to stderr from numeric parser

## [0.19.0] - 2026-08-03

### Added

- Add `sort` and `sort!`, and write helper comparison funcs
- Add Racket-style list accessors first thru tenth
- Add f32 and f64 bytevector support
- Add bigint_abs and bigrat_abs
- Add bigint literal parsing
- Add bigrat support (arbitrary size rational numbers)
- Add explicit check for duplicated variables in `letrec` and `letrec*`
- Add explicit check for duplicated formals in define/lambda/let

### Changed

- Refactor type promotion matrix
- Move bytevector type/range checker funcs to bytevector.c
- Sort the completion generator candidates.

### Fixed

- Add code to distinguish #f[alse] from #f[32|64]
- Fixed crash when using -ref to grab last cdr of improper list

## [0.18.2] - 2026-05-29

### Added

- Add support for `only`, `except`, `rename`, and `prefix` import modifiers
- Add Makefile targets to build and clean HTML docs 
- Add bigint support to number->string, base 10 only

### Changed

- Rename `time` library to `datetime`.
- Revert `cond`, `when`, and `unless` back to primitives for performance
- Complete refactor of module/library loading mechanism

### Fixed

- Fix crash when import set not in S-expr
- Fix line edit buffer redraw bugs
- Fix static buffer bigint repr bug

## [0.16.0] - 2026-03-12

### Added
- `set` primitive type and literal syntax
- `hash` primitive type and literal syntax
- Finished builtin procedure and special forms documentation
- Implement `call-with-port`

### Changed
- Removed dependency on Readline/libedit, and implemented line editing/history/tab completion from scratch
- Streamlined CMakeLists.txt build file
- Add `install` and `docs` targets to Makefile
- Substantially finished `base lazy` library
- Added more procedures to `base file` and `base system` libraries
- Added more tests to the suite, though the codebase is still woefully undercovered
- Add Unicode functions to replace ICU macros

### Fixed
- Change `symbol=?` to allow zero arguments as per R7RS
- Change all bitwise procedure names from chars; | won't parse correctly
- Fix critical bugs in `expt`, `gcd`, and `lcm`

## [0.10.0] - 2026-02-02

### Added
- Initial release

