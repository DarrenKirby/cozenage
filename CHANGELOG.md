# Changelog

## [0.16.0] - 2026-03-12

### Added
- `set` primitive type and literal syntax
- `hash` primitive type and literal syntax
- Finished builtin procedure and special forms documentation
- Implement `call-with-port`

### Changed
- Removed dependancy on Readline/libedit, and implemented line editing/history/tab completion from scratch
- Streamlined CMakeLists.txt build file
- Add `install` and `docs` targets to Makefile
- Substantially finished `base lazy` library
- Added more procedures to `base file` and `base system` libraries
- Added more tests to the suite, though the codebase is still woefully undercovered
- Add Unicode functions to replace ICU macros

### Fixed
- Change symbol=? to allow zero arguments as per R7RS
- Change all bitwise procedure names from chars; | won't parse correctly
- Fix critical bugs in `expt`, `gcd`, and `lcm`

## [0.10.0] - 2026-02-02

### Added
- Initial release