/*
 * 'src/types.h'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025  Darren Kirby <darren@dragonbyte.ca>
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
#include <stdio.h>
#include <unicode/umachine.h>


/* Convenience macros for readability */
#define CHECK_ARITY_EXACT(a, n) \
check_arg_arity((a), (n), -1, -1)

#define CHECK_ARITY_MIN(a, n) \
check_arg_arity((a), -1, (n), -1)

#define CHECK_ARITY_MAX(a, n) \
check_arg_arity((a), -1, -1, (n))

#define CHECK_ARITY_RANGE(a, lo, hi) \
check_arg_arity((a), -1, (lo), (hi))


/* For named chars */
typedef struct {
    const char* name;
    UChar32     codepoint;
} NamedChar;

typedef Cell* (*BuiltinFn)(const Lex* e, const Cell* args);

const char* cell_type_name(int t);
const char* cell_mask_types(int mask);
Cell* check_arg_types(const Cell* a, int mask);
Cell* check_arg_arity(const Cell* a, int exact, int min, int max);
void numeric_promote(Cell** lhs, Cell** rhs);
Cell* make_sexpr_len1(const Cell* a);
Cell* make_sexpr_len2(const Cell* a, const Cell* b);
Cell* make_sexpr_len4(const Cell* a, const Cell* b, const Cell* c, const Cell* d);
Cell* make_sexpr_from_list(Cell* v);
Cell* make_sexpr_from_array(int count, Cell** cells);
Cell* flatten_sexpr(const Cell* sexpr);
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
char* GC_strdup(const char* s);
char* GC_strndup(const char* s, size_t n);
int compare_named_chars(const void* key, const void* element);
const NamedChar* find_named_char(const char* name);
Cell* list_get_nth_cell_ptr(const Cell* list, long n);
char* convert_to_utf8(const UChar* ustr);
UChar* convert_to_utf16(const char* str);

#endif //COZENAGE_TYPES_H
