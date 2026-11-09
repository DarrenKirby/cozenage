/*
 * 'src/types.h'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 Darren Kirby <darren@dragonbyte.ca>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COZENAGE_TYPES_H
#define COZENAGE_TYPES_H

#include "environment.h"

#include <complex.h>
#include <stdint.h>
#include <unicode/umachine.h>

#define INIT_HASH_CAPACITY 8
#define INIT_BV_CAPACITY 8
#define INIT_SYMBOL_TABLE_SIZE 256
#define GMP_BASE 10
#define WORD_SIZE 8

#define BASE_2   2
#define BASE_8   8
#define BASE_10 10
#define BASE_16 16

/* For Unicode checks. */
#define MAX_VALID_CODEPOINT 0x10FFFFu
#define SURROGATE_MIN       0xD800u
#define SURROGATE_MAX       0xDFFFu
#define UNICODE_REPLACEMENT 0xFFFDu
#define UTF8_MASK_TAG       0xC0u   /* top two bits of a byte */
#define UTF8_CONT_TAG       0x80u   /* 10xxxxxx marks a continuation byte */
#define UTF8_CONT_MASK      0x3Fu   /* payload bits of a continuation byte */
#define UTF8_CONT_BITS      6       /* ...and how many there are */
#define UTF8_MAX_LEN        4
#define ASCII_MAX           0x80

/* ASCII key codes. */
#define ASC_BELL 0x7
#define ASC_BACKSPACE 0x8
#define ASC_DELETE 0x7f
#define ASC_ESCAPE 0x1b
#define ASC_RETURN 0xd

static inline bool is_scalar_value(const uint32_t cp) {
    return cp <= MAX_VALID_CODEPOINT && (cp < SURROGATE_MIN || cp > SURROGATE_MAX);
}

/* Convenience macros for arity checks. */
#define CHECK_ARITY_EXACT(a, n, s) \
check_arg_arity((a), (n), -1, -1, (s))

#define CHECK_ARITY_MIN(a, n, s) \
check_arg_arity((a), -1, (n), -1, (s))

#define CHECK_ARITY_MAX(a, n, s) \
check_arg_arity((a), -1, -1, (n), (s))

#define CHECK_ARITY_RANGE(a, lo, hi, s) \
check_arg_arity((a), -1, (lo), (hi), (s))


/* Global copies of argc and argv for use by (command-line). */
extern int g_argc;
extern char** g_argv;
/* Global is_repl used by many. */
extern bool is_repl;

/* For named chars */
typedef struct {
    const char* name;
    UChar32 codepoint;
} NamedChar;

typedef enum {
    NUM_INT = 0,
    NUM_BIGINT,
    NUM_RATIONAL,
    NUM_BIGRAT,
    NUM_REAL,
    NUM_COMPLEX,
    NUM_TYPE_COUNT,
} NumType;


typedef Cell* (*BuiltinFn)(const Lex* e, const Cell* args);

const char* fmt_err(const char *fmt, ...);
//const char* cell_type_name(int t);
const char* cell_type_name(uint32_t t);
const char* cell_mask_types(uint32_t mask);
Cell* check_arg_types(const Cell* a, int mask, const char* fname);
Cell* check_arg_arity(const Cell* a, int exact, int min, int max, const char* fname);
int check_lambda_arity(const Cell* proc, int expected);
void numeric_promote(Cell** lhs, Cell** rhs);
Cell* make_sexpr_len1(const Cell* a);
Cell* make_sexpr_len2(const Cell* a, const Cell* b);
Cell* make_sexpr_len3(const Cell* a, const Cell* b, const Cell* c);
Cell* make_sexpr_len4(const Cell* a, const Cell* b, const Cell* c, const Cell* d);
Cell* make_list_from_sexpr(Cell* c);
Cell* make_sexpr_from_list(const Cell* v, bool recurse);
Cell* make_sexpr_from_array(int count, Cell** cells);
Cell* flatten_sexpr(const Cell* sexpr);
long double complex cell_to_c_complex(const Cell* c);
bool cell_is_real_zero(const Cell* c);
bool cell_is_integer(const Cell* c);
bool cell_is_real(const Cell* c);
bool cell_is_positive(const Cell* c);
bool cell_is_negative(const Cell* c);
bool cell_is_odd(const Cell* c);
bool cell_is_even(const Cell* c);
Cell* negate_numeric(const Cell* x);
Cell* simplify_rational(Cell* v);
void complex_apply(BuiltinFn fn, const Lex* e, Cell* result, const Cell* rhs);
long double cell_to_long_double(const Cell* c);
Cell* make_cell_from_double(long double d);
int compare_named_chars(const void* key, const void* element);
const NamedChar* find_named_char(const char* name);
Cell* list_get_nth_cell_ptr(const Cell* list, long n, bool tail);
int utf8_encode(UChar32 c, uint8_t* out_buf);
int utf8_decode(const uint8_t* s, size_t n, UChar32* cp);
int utf8_append(UChar32 c, char *out_buf);
int utf8_len(uint8_t first_byte);
int utf8_code_point_len(uint32_t cp);
int32_t get_utf8_byte_offset(const Cell* s, int32_t char_idx);
uint32_t utf8_next(const uint8_t **p, const uint8_t *end);
size_t utf8_prev_char(const char* str, size_t pos);
size_t utf8_next_char(const char* str, size_t pos, size_t len);
int32_t utf8_strlen(const char* s);
bool is_pure_ascii(const char *str, size_t len);
char* convert_to_utf8(const UChar* ustr);
UChar* convert_to_utf16(const char* str);

/* Only need these for the test builds. */
#ifdef CRITERION_TEST_BUILD
Cell* int_to_rat(const Cell* v);
Cell* int_to_real(const Cell* v);
#endif

#endif // COZENAGE_TYPES_H
