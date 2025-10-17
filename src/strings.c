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
#include "types.h"
#include <string.h>
#include <_stdlib.h>
#include <gc/gc.h>
#include <unicode/utf8.h>
#include <unicode/ustring.h>


/* Helper for other string procedures.
 * Like strlen, but works with UTF8 */
static int32_t string_length(const Cell* string) {
    const char* s = string->str;
    const int32_t len_bytes = (int)strlen(s);

    int32_t i = 0;
    int32_t code_point_count = 0;
    UChar32 c;

    /* Iterate through the string one code point at a time */
    while (i < len_bytes) {
        U8_NEXT(s, i, len_bytes, c);
        /* A negative value for 'c' indicates an invalid UTF-8 sequence */
        if (c < 0) {
            return -1;
        }
        code_point_count++;
    }
    return code_point_count;
}

/*-------------------------------------------------------*
 *     String constructors, selectors, and procedures    *
 * ------------------------------------------------------*/

/* (string char ... )
* Returns a newly allocated string composed of the arguments. It is analogous to list. */
Cell* builtin_string(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_CHAR);
    if (err) return err;

    const int str_len = a->count;
    char* the_string = GC_MALLOC_ATOMIC(str_len * 4 + 1);
    int32_t j = 0;
    for (int i = 0; i < str_len; i++) {
        const Cell* char_cell = a->cell[i];
        const UChar32 code_point = char_cell->char_v;
        U8_APPEND_UNSAFE(the_string, j, code_point);
    }
    the_string[j] = '\0';
    return make_cell_string(the_string);
}

/* (string-length string)
 * Returns the number of characters in the given string. */
Cell* builtin_string_length(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error("string-length: arg 1 must be a string", TYPE_ERR);
    }
    return make_cell_integer(string_length(a->cell[0]));
}

/* */
Cell* builtin_string_eq_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING);
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* quick exit before conversion: if the len is not the same,
         * the strings are not the same */
        if (strlen(lhs) != strlen(rhs)) {
            return make_cell_boolean(0);
        }
        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        if (u_strcmpCodePointOrder(U_lhs, U_rhs) != 0)  {
            return make_cell_boolean(0);
        }
    }
    /* If we get here, we're equal */
    return make_cell_boolean(1);
}

Cell* builtin_string_lt_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING);
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
            return make_cell_boolean(0);
        }
    }
    /* If we get here, s1 < s2 < sn ... */
    return make_cell_boolean(1);
}

Cell* builtin_string_lte_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING);
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
            return make_cell_boolean(0);
        }
    }
    /* If we get here, s1 <= s2 <= sn ... */
    return make_cell_boolean(1);
}

Cell* builtin_string_gt_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING);
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
            return make_cell_boolean(0);
        }
    }
    /* If we get here, s1 > s2 > sn ... */
    return make_cell_boolean(1);
}

Cell* builtin_string_gte_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING);
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
            return make_cell_boolean(0);
        }
    }
    /* If we get here, s1 >= s2 >= sn ... */
    return make_cell_boolean(1);
}

/* (string-append string ...)
 * Returns a newly allocated string whose characters are the concatenation of the characters in the
 * given strings. */
Cell* builtin_string_append(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING);
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
    return make_cell_string(convert_to_utf8(result));
}

/* (string-ref string k)
* It is an error if k is not a valid index of string. The string-ref procedure returns character k
* of string using zero-origin indexing. There is no requirement for this procedure to execute in
* constant time. */
Cell* builtin_string_ref(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error("string-ref: arg 1 must be a string", TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error("string-ref: arg 2 must be an integer", TYPE_ERR);
    }

    int32_t byte_index = 0;
    int32_t current_char_index = 0;
    UChar32 c = 0;
    const int32_t char_index = (int)a->cell[1]->integer_v;
    const char* s = a->cell[0]->str;

    while (current_char_index <= char_index) {

        /* Check if we hit the end of the string before finding the character. */
        if (s[byte_index] == '\0') {
            return make_cell_error("string-ref: index out of range", INDEX_ERR);
        }

        U8_NEXT(s, byte_index, -1, c);  /* -1: null terminated */

        /* If this was the character we were looking for, return it */
        if (current_char_index == char_index) {
            return make_cell_char(c);
        }

        current_char_index++;
    }
    return make_cell_error("string-ref: invalid or malformed string", VALUE_ERR);
}

/* (make-string k)
 * (make-string k char)
 * The make-string procedure returns a newly allocated string of length k. If char is given, then
 * all the characters of the string are initialized to char, otherwise the contents of the string
 * are initialized to a space: " " aka 0x0020 */
Cell* builtin_make_string(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_INTEGER) {
        return make_cell_error("make-string: arg 1 must be an integer", TYPE_ERR);
    }

    UChar32 fill_char;
    const int32_t str_len = (int)a->cell[0]->integer_v;
    if (a->count == 1) {
        fill_char = 0x0020;
    } else {
        if (a->cell[1]->type != CELL_CHAR) {
            return make_cell_error("make-string: arg 2 must be a char", TYPE_ERR);
        }
        fill_char = a->cell[1]->char_v;
    }

    const int32_t bytes_per_char = U8_LENGTH(fill_char);

    /* Calculate total bytes needed for the string data */
    const int32_t total_string_bytes = str_len * bytes_per_char;

    /* Allocate memory, adding +1 for the null terminator */
    char *new_string = GC_MALLOC_ATOMIC(total_string_bytes + 1);
    if (!new_string) {
        fprintf(stderr, "ENOMEM: out of memory");
        exit(EXIT_FAILURE);
    }

    int32_t byte_index = 0;
    for (int32_t i = 0; i < str_len; i++) {
        /* U8_APPEND will write the char's bytes and
         * advance byte_index by the correct amount (4, in this case) */
        U8_APPEND_UNSAFE(new_string, byte_index, fill_char);
    }
    /* Don't forget the null terminator! */
    new_string[total_string_bytes] = '\0';

    return make_cell_string(new_string);
}

/* (string->list string)
 * (string->list string start )
 * (string->list string start end)
 * The string->list procedure returns a newly allocated list of the characters of string between
 * start and end. */
Cell* builtin_string_list(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error("string->list: arg 1 must be a string", TYPE_ERR);
    }

    const int32_t str_len = string_length(a->cell[0]);
    int32_t start = 0;
    int32_t end = str_len;
    const char* s = a->cell[0]->str;

    if (a->count == 2 || a->count == 3) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error("string->list: arg 2 must be an integer", TYPE_ERR);
        }
        start = (int)a->cell[1]->integer_v;
    }
    if (a->count == 3) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error("string->list: arg 3 must be an integer", TYPE_ERR);
        }
        end = (int)a->cell[2]->integer_v;
    }

    if (start < 0 || start > str_len ||
        end < 0   || end > str_len   ||
        start > end) {
        return make_cell_error("string->list: index out of range", INDEX_ERR);
        }

    /* Build the list */
    int32_t byte_index = 0;
    int32_t char_index = 0;
    int32_t current_list_len = end - start; // Start with the total length
    UChar32 c = 0;
    Cell* head = make_cell_nil();
    Cell* tail = nullptr;

    while (char_index < end && s[byte_index] != '\0') {
        U8_NEXT(s, byte_index, -1, c);

        if (char_index >= start) {
            Cell* new_pair = make_cell_pair(make_cell_char(c), make_cell_nil());
            new_pair->len = current_list_len;
            current_list_len--;

            if (head->type == CELL_NIL) {
                /* This is the first node */
                head = new_pair;
                tail = new_pair;
            } else {
                /* This is a subsequent node */
                tail->cdr = new_pair;
                tail = new_pair;
            }
        }
        char_index++;
    }
    return head;
}
