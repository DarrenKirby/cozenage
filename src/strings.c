/*
 * 'strings.c'
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

#include "strings.h"
#include <string.h>
#include <gc/gc.h>
#include <unicode/utf8.h>
#include <unicode/ustring.h>


/*-------------------------------------------------------*
 *     String constructors, selectors, and procedures    *
 * ------------------------------------------------------*/

Cell* builtin_string_to_symbol(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_STR) {
        return make_val_err("string->symbol: arg 1 must be a string", TYPE_ERR);
    }
    return make_val_sym(a->cell[0]->str);
}

Cell* builtin_symbol_to_string(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_SYM) {
        return make_val_err("symbol->string: arg 1 must be a symbol", TYPE_ERR);
    }
    return make_val_str(a->cell[0]->sym);
}

Cell* builtin_string(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_CHAR);
    if (err) return err;

    const int str_len = a->count;
    char* the_string = GC_MALLOC_ATOMIC(str_len * 4 + 1);
    int32_t j = 0;
    for (int i = 0; i < str_len; i++) {
        const Cell* char_cell = a->cell[i];
        const UChar32 code_point = char_cell->c_val;
        U8_APPEND_UNSAFE(the_string, j, code_point);
    }
    the_string[j] = '\0';
    return make_val_str(the_string);
}

Cell* builtin_string_length(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_STR) {
        return make_val_err("string-length: arg 1 must be a string", TYPE_ERR);
    }

    const char* s = a->cell[0]->str;
    const int32_t len_bytes = (int)strlen(s);

    int32_t i = 0;
    int32_t code_point_count = 0;
    UChar32 c;

    /* Iterate through the string one code point at a time */
    while (i < len_bytes) {
        U8_NEXT(s, i, len_bytes, c);
        /* A negative value for 'c' indicates an invalid UTF-8 sequence */
        if (c < 0) {
            return make_val_err("string-length: invalid UTF-8 sequence in string", VALUE_ERR);
        }
        code_point_count++;
    }
    return make_val_int(code_point_count);
}

Cell* builtin_string_eq_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* quick exit before conversion: if the len is not the same,
         * the strings are not the same */
        if (strlen(lhs) != strlen(rhs)) {
            return make_val_bool(0);
        }
        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        if (u_strcmpCodePointOrder(U_lhs, U_rhs) != 0)  {
            return make_val_bool(0);
        }
    }
    /* If we get here, we're equal */
    return make_val_bool(1);
}

Cell* builtin_string_lt_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        if (u_strcmpCodePointOrder(U_lhs, U_rhs) >= 0)  {
            return make_val_bool(0);
        }
    }
    /* If we get here, s1 < s2 < sn ... */
    return make_val_bool(1);
}

Cell* builtin_string_lte_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        if (u_strcmpCodePointOrder(U_lhs, U_rhs) > 0)  {
            return make_val_bool(0);
        }
    }
    /* If we get here, s1 <= s2 <= sn ... */
    return make_val_bool(1);
}

Cell* builtin_string_gt_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        if (u_strcmpCodePointOrder(U_lhs, U_rhs) <= 0)  {
            return make_val_bool(0);
        }
    }
    /* If we get here, s1 > s2 > sn ... */
    return make_val_bool(1);
}

Cell* builtin_string_gte_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        if (u_strcmpCodePointOrder(U_lhs, U_rhs) >= 0)  {
            return make_val_bool(0);
        }
    }
    /* If we get here, s1 >= s2 >= sn ... */
    return make_val_bool(1);
}

Cell* builtin_string_append(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;

    /* (string-append "foo") -> "foo */
    if (a->count == 1) {
        return a->cell[0];
    }

    /* Calculate length needed for buffer */
    uint32_t total_len = 0;
    for (int i = 0; i < a->count; i++) {
        total_len += strlen(a->cell[i]->str);
    }

    UChar* result = GC_MALLOC(sizeof(UChar) * (total_len + 1));
    if (strcmp(a->cell[0]->str, "") != 0) {
        result = convert_to_utf16(a->cell[0]->str);
    } else {
        result[0] = L'\0';
    }

    for (int i = 1; i < a->count; i++) {
        const char* rhs = a->cell[i]->str;
        const UChar* U_rhs = convert_to_utf16(rhs);
        if (U_rhs && U_rhs[0] != L'\0') {
            u_strcat(result, U_rhs);
        }
    }
    return make_val_str(convert_to_utf8(result));
}
