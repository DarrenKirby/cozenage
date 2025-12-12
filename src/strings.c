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
#include "lexer.h"
#include "repr.h"
#include "parser.h"

#include <string.h>
#include <stdlib.h>
#include <gc/gc.h>
#include <unicode/utf8.h>
#include <unicode/ustring.h>
#include <unicode/ucnv.h>
#include <unicode/uchar.h>


/*-------------------------------------------------------*
 *     String constructors, selectors, and procedures    *
 * ------------------------------------------------------*/

/* (string char ... )
* Returns a newly allocated string composed of the arguments. It is analogous to list. */
Cell* builtin_string(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_CHAR, "string");
    if (err) return err;

    /* This is the number of chars in the 'a' Sexpr */
    const int str_len = a->count;
    /* 4-bytes per char, plus 1 for null. */
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
Cell* builtin_string_length(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "string-length");
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "string-length: arg 1 must be a string",
            TYPE_ERR);
    }
    return make_cell_integer(a->cell[0]->char_count);
}

/* */
Cell* builtin_string_eq_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string=?");
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1, "string=?");
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* Quick exit before conversion: if the len is not the same,
         * the strings are not the same. */
        if (strlen(lhs) != strlen(rhs)) {
            return False_Obj;
        }
        /* Convert to UTF-16. */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        if (u_strcmpCodePointOrder(U_lhs, U_rhs) != 0)  {
            return False_Obj;
        }
    }
    /* If we get here, we're equal. */
    return True_Obj;
}

Cell* builtin_string_lt_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string<?");
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1, "string<?");
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        if (u_strcmpCodePointOrder(U_lhs, U_rhs) >= 0)  {
            return False_Obj;
        }
    }
    /* If we get here, s1 < s2 < sn ... */
    return True_Obj;
}

Cell* builtin_string_lte_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string<=");
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1, "string<=");
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        if (u_strcmpCodePointOrder(U_lhs, U_rhs) > 0)  {
            return False_Obj;
        }
    }
    /* If we get here, s1 <= s2 <= sn ... */
    return True_Obj;
}

Cell* builtin_string_gt_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string>?");
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1, "string>?");
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        if (u_strcmpCodePointOrder(U_lhs, U_rhs) <= 0)  {
            return False_Obj;
        }
    }
    /* If we get here, s1 > s2 > sn ... */
    return True_Obj;
}

Cell* builtin_string_gte_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string>=?");
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1, "string>=?");
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        if (u_strcmpCodePointOrder(U_lhs, U_rhs) >= 0)  {
            return False_Obj;
        }
    }
    /* If we get here, s1 >= s2 >= sn ... */
    return True_Obj;
}

/* (string-append string ...)
 * Returns a newly allocated string whose characters are the concatenation of the characters in the
 * given strings. */
Cell* builtin_string_append(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string-append");
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1, "string-append");
    if (err) return err;

    /* (string-append "foo") -> "foo. */
    if (a->count == 1) {
        return a->cell[0];
    }

    /* Calculate length needed for buffer. */
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
Cell* builtin_string_ref(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "string-ref");
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "string-ref: arg 1 must be a string",
            TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error(
            "string-ref: arg 2 must be an integer",
            TYPE_ERR);
    }

    int32_t byte_index = 0;
    int32_t current_char_index = 0;
    UChar32 c = 0;
    const int32_t char_index = (int)a->cell[1]->integer_v;
    const char* s = a->cell[0]->str;

    while (current_char_index <= char_index) {

        /* Check if we hit the end of the string before finding the character. */
        if (s[byte_index] == '\0') {
            return make_cell_error(
                "string-ref: index out of range",
                INDEX_ERR);
        }

        U8_NEXT(s, byte_index, -1, c);  /* -1: null terminated. */

        /* If this was the character we were looking for, return it. */
        if (current_char_index == char_index) {
            return make_cell_char(c);
        }

        current_char_index++;
    }
    return make_cell_error(
        "string-ref: invalid or malformed string",
        VALUE_ERR);
}

/* (make-string k)
 * (make-string k char)
 * The make-string procedure returns a newly allocated string of length k. If char is given, then
 * all the characters of the string are initialized to char, otherwise the contents of the string
 * are initialized to a space: " " aka 0x0020 */
Cell* builtin_make_string(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2, "make-string");
    if (err) return err;
    if (a->cell[0]->type != CELL_INTEGER) {
        return make_cell_error(
            "make-string: arg 1 must be an integer",
            TYPE_ERR);
    }

    UChar32 fill_char;
    const int32_t str_len = (int)a->cell[0]->integer_v;
    if (a->count == 1) {
        fill_char = 0x0020; /* fill char is a space. */
    } else {
        if (a->cell[1]->type != CELL_CHAR) {
            return make_cell_error(
                "make-string: arg 2 must be a char",
                TYPE_ERR);
        }
        fill_char = a->cell[1]->char_v;
    }

    const int32_t bytes_per_char = U8_LENGTH(fill_char);

    /* Calculate total bytes needed for the string data. */
    const int32_t total_string_bytes = str_len * bytes_per_char;

    /* Allocate memory, adding +1 for the null terminator. */
    char *new_string = GC_MALLOC_ATOMIC(total_string_bytes + 1);
    if (!new_string) {
        fprintf(stderr, "ENOMEM: out of memory");
        exit(EXIT_FAILURE);
    }

    int32_t byte_index = 0;
    for (int32_t i = 0; i < str_len; i++) {
        /* U8_APPEND will write the char's bytes and
         * advance byte_index by the correct amount (4, in this case). */
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
Cell* builtin_string_list(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3, "string->list");
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "string->list: arg 1 must be a string",
            TYPE_ERR);
    }

    const int32_t str_len = a->cell[0]->char_count;
    int32_t start = 0;
    int32_t end = str_len;
    const char* s = a->cell[0]->str;

    if (a->count == 2 || a->count == 3) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error(
                "string->list: arg 2 must be an integer",
                TYPE_ERR);
        }
        start = (int)a->cell[1]->integer_v;
    }
    if (a->count == 3) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error(
                "string->list: arg 3 must be an integer",
                TYPE_ERR);
        }
        end = (int)a->cell[2]->integer_v;
    }

    if (start < 0 || start > str_len ||
        end < 0   || end > str_len   ||
        start > end) {
        return make_cell_error(
            "string->list: index out of range",
            INDEX_ERR);
        }

    /* Build the list. */
    int32_t byte_index = 0;
    int32_t char_index = 0;
    int32_t current_list_len = end - start; /* Start with the total length. */
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
                /* This is the first node. */
                head = new_pair;
                tail = new_pair;
            } else {
                /* This is a subsequent node. */
                tail->cdr = new_pair;
                tail = new_pair;
            }
        }
        char_index++;
    }
    return head;
}

/* (list->string list)
 * It is an error if any element of list is not a character. list->string returns a newly allocated
 * string formed from the elements in the list. Order is preserved. string->list and list->string
 * are inverses so far as equal? is concerned. */
Cell* builtin_list_string(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "list->string");
    if (err) return err;

    const Cell* l = a->cell[0];
    const int l_len = a->cell[0]->len;
    UChar32 char_array[l_len];

    for (int32_t i = 0; i < l_len; i++) {
        if (l->car->type != CELL_CHAR) {
            return make_cell_error(
                "list->string: All list elements must be chars",
                TYPE_ERR);
        }
        char_array[i] = l->car->char_v;
        l = l->cdr;
    }

    const int32_t srcByteLength = l_len * (int32_t)sizeof(UChar32);

    UErrorCode status = U_ZERO_ERROR;

    /* Pre-flight to get the required buffer size. */

    /* Explicit endianness-check required, as ICU assumes big-endian if no BOM. */
    // ReSharper disable once CppDFAUnreachableCode
    const char* fromConverterName = U_IS_BIG_ENDIAN ? "UTF-32BE" : "UTF-32LE";
    /* Call with NULL destination to get the size*/
    const int32_t requiredByteCapacity = ucnv_convert(
        "UTF-8",                 /* toConverterName */
        fromConverterName,       /* fromConverterName */
        nullptr,                 /* target (NULL for pre-flight) */
        0,                       /* targetCapacity (0 for pre-flight) */
        // ReSharper disable once CppRedundantCastExpression
        (const char*)char_array, /* source */
        srcByteLength,           /* sourceLength (in bytes!) */
        &status
    );

    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status)) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Unicode error: %s", u_errorName(status));
        return make_cell_error(buf, GEN_ERR);
    }

    /* Reset status after expected "overflow". */
    status = U_ZERO_ERROR;

    /* Allocate and perform the real conversion. */
    char *utf8Buffer = GC_MALLOC(requiredByteCapacity + 1);
    if (!utf8Buffer) {
        fprintf(stderr, "ENOMEM: malloc failed\n");
        exit(EXIT_FAILURE);
    }

    ucnv_convert(
        "UTF-8",
        fromConverterName,
        utf8Buffer,
        requiredByteCapacity,
        // ReSharper disable once CppRedundantCastExpression
        (const char*)char_array,
        srcByteLength,
        &status
    );

    if (U_FAILURE(status)) {
        char buf[256];
        snprintf(buf, sizeof(buf), "ICU conversion failed: %s\n", u_errorName(status));
        free(utf8Buffer);
        return make_cell_error(buf, GEN_ERR);
    }

    /* Null-terminate the resulting UTF-8 string. */
    utf8Buffer[requiredByteCapacity] = '\0';
    return make_cell_string(utf8Buffer);
}

/* (substring string start end)
 * The substring procedure returns a newly allocated string formed from the characters of string
 * beginning with index start and ending with index end. This is equivalent to calling string-copy
 * with the same arguments, but is provided for backward compatibility and stylistic flexibility. */
Cell* builtin_substring(const Lex* e, const Cell* a)
{
    /* Just check that we have 3 args and kick it to string-copy. */
    Cell* err = CHECK_ARITY_EXACT(a, 3, "substring");
    if (err) return err;
    return builtin_string_copy(e, a);
}

/* (string-set! string k char)
 * It is an error if k is not a valid index of string. The string-set! procedure stores char in
 * element k of string. */
Cell* builtin_string_set_bang(const Lex* e, const Cell* a)
{
    /* TODO */
    (void)e;
    (void)a;
    return make_cell_error("Not implemented yet", VALUE_ERR);
}

/* (string-copy string )
 * (string-copy string start )
 * (string-copy string start end )
 * Returns a newly allocated copy of the part of the given string between start and end. */
Cell* builtin_string_copy(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3, "string_copy");
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "string-copy: arg 1 must be a string",
            TYPE_ERR);
    }
    const char* str = a->cell[0]->str;

    /* Simplest case, copy entire string. */
    if (a->count == 1) {
        return make_cell_string(GC_strdup(str));
    }

    /* Difficult cases, true substrings with start and end indices. */
    const int32_t total_byte_len = (int)strlen(a->cell[0]->str);
    int32_t start_idx = 0;
    int32_t end_idx = total_byte_len;

    /* Check args for non-default indices. */
    if (a->count == 2 || a->count == 3) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error(
                "string-copy: arg 2 must be an integer",
                TYPE_ERR);
        }
        start_idx = (int)a->cell[1]->integer_v;
        if (a->count == 3) {
            if (a->cell[2]->type != CELL_INTEGER) {
                return make_cell_error(
                    "string-copy: arg 3 must be an integer",
                    TYPE_ERR);
            }
            end_idx = (int)a->cell[2]->integer_v;
        }
    }

    /* Validate for legal indices. */
    const int32_t char_length = a->cell[0]->char_count;
    if (start_idx < 0) {
        return make_cell_error(
            "string-copy: start index must be non-negative",
            INDEX_ERR);
    }
    if (end_idx < 0) {
        return make_cell_error(
            "string-copy: end index must be non-negative",
            INDEX_ERR);
    }
    if (start_idx > char_length) {
        return make_cell_error(
            "string-copy: start index is out of bounds",
            INDEX_ERR);
    }
    if (end_idx > char_length) {
        return make_cell_error(
            "string-copy: end index is out of bounds",
            INDEX_ERR);
    }
    if (start_idx > end_idx) {
        return make_cell_error(
            "string-copy: start index cannot be greater than end index",
            INDEX_ERR);
    }

    /* Calculate the UTF8 codepoint indices. */
    int32_t byte_start_idx = 0;
    U8_FWD_N(str, byte_start_idx, total_byte_len, start_idx);
    const int32_t code_points_to_copy = end_idx - start_idx;
    int32_t byte_end_idx = byte_start_idx;
    U8_FWD_N(str, byte_end_idx, total_byte_len, code_points_to_copy);

    /* Allocate and copy. */
    int32_t bytes_to_copy = byte_end_idx - byte_start_idx;
    char *new_str = GC_MALLOC_ATOMIC(bytes_to_copy + 1);
    memcpy(new_str, str + byte_start_idx, bytes_to_copy);
    new_str[bytes_to_copy] = '\0';
    return make_cell_string(new_str);
}

/* (string-copy! to at from )
 * (string-copy! to at from start )
 * (string-copy! to at from start end )
 * It is an error if at is less than zero or greater than the length of to. It is also an error if
 * (- (string-length to) at) is less than (- end start). Copies the characters of string from
 * between start and end to string to , starting at 'at' . The order in which characters are copied
 * is unspecified, except that if the source and destination overlap, copying takes place as if the
 * source is first copied into a temporary string and then into the destination. This can be
 * achieved without allocating storage by making sure to copy in the correct direction in such
 * circumstances. */
Cell* builtin_string_copy_bang(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 3, 5, "string_copy!");
    if (err) return err;

    /* Validate arg Types. */
    if (a->cell[0]->type != CELL_STRING)
        return make_cell_error(
            "string-copy!: arg 1 must be a string (to)",
            TYPE_ERR);
    if (a->cell[1]->type != CELL_INTEGER)
        return make_cell_error(
            "string-copy!: arg 2 must be an integer (at)",
            TYPE_ERR);
    if (a->cell[2]->type != CELL_STRING)
        return make_cell_error(
            "string-copy!: arg 3 must be a string (from)",
            TYPE_ERR);

    /* Get 'to' string and 'at' index. */
    Cell* to_cell = a->cell[0];
    const char* to_str = to_cell->str;
    const int32_t to_char_len = to_cell->char_count;
    const int32_t to_byte_len = (int32_t)strlen(to_str);
    const int32_t to_start_idx = (int32_t)a->cell[1]->integer_v;

    /* Get 'from' string and 'start'/'end' indices. */
    const char* from_str = a->cell[2]->str;
    const int32_t from_char_len = a->cell[2]->char_count;
    const int32_t from_byte_len = (int32_t)strlen(from_str);

    int32_t from_start_idx = 0;
    int32_t from_end_idx = from_char_len; /* Default to char length. */

    if (a->count >= 4) {
        if (a->cell[3]->type != CELL_INTEGER)
            return make_cell_error(
                "string-copy!: arg 4 must be an integer (start)",
                TYPE_ERR);
        from_start_idx = (int32_t)a->cell[3]->integer_v;
    }
    if (a->count == 5) {
        if (a->cell[4]->type != CELL_INTEGER)
            return make_cell_error(
                "string-copy!: arg 5 must be an integer (end)",
                TYPE_ERR);
        from_end_idx = (int32_t)a->cell[4]->integer_v;
    }

    /* R7RS Index Validation. */
    if (to_start_idx < 0 || to_start_idx > to_char_len)
        return make_cell_error(
            "string-copy!: 'at' index is out of bounds for 'to' string",
            INDEX_ERR);
    if (from_start_idx < 0 || from_start_idx > from_char_len)
        return make_cell_error(
            "string-copy!: 'start' index is out of bounds for 'from' string",
            INDEX_ERR);
    if (from_end_idx < 0 || from_end_idx > from_char_len)
        return make_cell_error(
            "string-copy!: 'end' index is out of bounds for 'from' string",
            INDEX_ERR);
    if (from_start_idx > from_end_idx)
        return make_cell_error(
            "string-copy!: 'start' index cannot be greater than 'end' index",
            INDEX_ERR);

    /* R7RS Destination Bounds Check. */
    const int32_t code_points_to_copy = from_end_idx - from_start_idx;
    const int32_t to_available_chars = to_char_len - to_start_idx;

    if (to_available_chars < code_points_to_copy) {
        return make_cell_error(
            "string-copy!: 'from' substring is larger than available space in 'to' string",
            VALUE_ERR);
    }

    /* Calculate all BYTE offsets. */

    /* 'to' prefix (before 'at'). */
    int32_t to_prefix_byte_end = 0;
    U8_FWD_N(to_str, to_prefix_byte_end, to_byte_len, to_start_idx);

    /* 'from' substring. */
    int32_t from_byte_start = 0;
    U8_FWD_N(from_str, from_byte_start, from_byte_len, from_start_idx);
    int32_t from_byte_end = from_byte_start;
    U8_FWD_N(from_str, from_byte_end, from_byte_len, code_points_to_copy);
    const int32_t bytes_to_copy = from_byte_end - from_byte_start;

    /* 'to' suffix (after copied part). */
    int32_t to_suffix_byte_start = to_prefix_byte_end;
    U8_FWD_N(to_str, to_suffix_byte_start, to_byte_len, code_points_to_copy);
    const int32_t to_suffix_bytes = to_byte_len - to_suffix_byte_start;

    /* Build new string. */
    const int32_t to_prefix_bytes = to_prefix_byte_end;
    const int32_t new_total_bytes = to_prefix_bytes + bytes_to_copy + to_suffix_bytes;

    char* new_str = GC_MALLOC_ATOMIC(new_total_bytes + 1);
    if (!new_str) {
        fprintf(stderr, "ENOMEM: malloc failed\n");
        exit(EXIT_FAILURE);
    }

    /* Copy part 1: 'to' prefix. */
    memcpy(new_str, to_str, to_prefix_bytes);

    /* Copy part 2: 'from' substring. */
    memcpy(new_str + to_prefix_bytes, from_str + from_byte_start, bytes_to_copy);

    /* Copy part 3: 'to' suffix. */
    memcpy(new_str + to_prefix_bytes + bytes_to_copy, to_str + to_suffix_byte_start, to_suffix_bytes);

    new_str[new_total_bytes] = '\0';

    /* Mutate the Cell and return. */
    to_cell->str = new_str;
    return to_cell;
}

/* (string-fill! string fill)
 * (string-fill! string fill start)
 * (string-fill! string fill start end)
 * It is an error if fill is not a character. The string-fill! procedure stores fill in the elements
 * of string between start and end. */
Cell* builtin_string_fill(const Lex* e, const Cell* a)
{
    /* TODO */
    (void)e;
    (void)a;
    return make_cell_error("Not implemented yet", VALUE_ERR);
}

Cell* builtin_string_number(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2, "string->number");
    if (err) return err;

    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "string->number: arg1 must be a string",
            VALUE_ERR);
    }
    const char* the_raw_num = a->cell[0]->str;

    int radix = 10;
    if (a->count == 2) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error(
                "string->number: radix arg must be an integer",
                VALUE_ERR);
        }
        radix = (int)a->cell[1]->integer_v;
        if (!(radix == 2 || radix == 8 || radix == 10 || radix == 16)) {
            return make_cell_error(
                "string->number: radix arg must be one of 2, 8, 10, or 16",
                VALUE_ERR);
        }
        /*  */
        if (strchr(the_raw_num, '.') != NULL) {
            return make_cell_error(
                "Cannot use radix arg with real number",
                VALUE_ERR);
        }
    }

    char the_num[256];
    if (radix != 10) {
        switch (radix) {
            case 2: snprintf(the_num, sizeof(the_num), "#b%s", the_raw_num); break;
            case 8: snprintf(the_num, sizeof(the_num), "#o%s", the_raw_num); break;
            case 16: snprintf(the_num, sizeof(the_num), "#x%s", the_raw_num); break;
            default: ;
        }
    } else {
        snprintf(the_num, sizeof(the_num), "%s", the_raw_num);
    }

    TokenArray* ta = scan_all_tokens(the_num);
    Cell* result = parse_tokens(ta);

    /* Return false on parse errors. */
    if (result->type == CELL_ERROR) {
        return False_Obj;
    }

    /* Ditto if it's not actually a number. */
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const int mask = CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX;
    if (!(result->type & mask)) {
        return False_Obj;
    }
    return result;
}

/* TODO - the radix??? */
Cell* builtin_number_string(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2, "number->string");
    if (err) return err;
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const int mask = CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX;
    if (!(a->cell[0]->type & mask)) {
        return make_cell_error(
            "number->string: arg 1 must be a number",
            TYPE_ERR);
    }
    const char* result = cell_to_string(a->cell[0], MODE_DISPLAY);
    return make_cell_string(result);
}

/* These procedures apply the Unicode full string uppercasing, lowercasing, and case-folding
 * algorithms to their arguments and return the result. In certain cases, the result differs in
 * length from the argument. If the result is equal to the argument in the sense of string=?, the
 * argument may be returned. */

/* (string-downcase string) */
Cell* builtin_string_downcase(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string-downcase");
    if (err) return err;
    err = CHECK_ARITY_EXACT(a, 1, "string-downcase");
    if (err) return err;

    UErrorCode status = U_ZERO_ERROR;
    UChar* src = convert_to_utf16(a->cell[0]->str);
    if (!src) return make_cell_error(
        "string-downcase: malformed UTF-8 string",
        VALUE_ERR);

    const int32_t src_len = u_countChar32(src, -1);

    UChar* dst = GC_MALLOC(sizeof(UChar) * src_len + 1);;
    const int32_t dest_len = u_strToLower(dst,
        src_len + 1,
        src, -1, nullptr, &status);

    if (dest_len < src_len) {
        return make_cell_error(
            "string-downcase: some chars not copied!!!",
            GEN_ERR);
    }

    char* result = convert_to_utf8(dst);
    if (!result) {
        return make_cell_error(
            "string-downcase: malformed UTF-8 string",
            VALUE_ERR);
    }
    return make_cell_string(result);
}

/* (string-upcase string) */
Cell* builtin_string_upcase(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string-upcase");
    if (err) return err;
    err = CHECK_ARITY_EXACT(a, 1, "string-upcase");
    if (err) return err;

    UErrorCode status = U_ZERO_ERROR;
    UChar* src = convert_to_utf16(a->cell[0]->str);
    if (!src) return make_cell_error(
        "string-upcase: malformed UTF-8 string",
        VALUE_ERR);

    const int32_t src_len = u_countChar32(src, -1);

    UChar* dst = GC_MALLOC(sizeof(UChar) * src_len + 1);;
    const int32_t dest_len = u_strToUpper(dst,
        src_len + 1,
        src, -1, nullptr, &status);

    if (dest_len < src_len) {
        return make_cell_error(
            "string-upcase: some chars not copied!!!",
            GEN_ERR);
    }

    char* result = convert_to_utf8(dst);
    if (!result) {
        return make_cell_error(
            "string-upcase: malformed UTF-8 string",
            VALUE_ERR);
    }
    return make_cell_string(result);
}

/* (string-foldcase string) */
Cell* builtin_string_foldcase(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string-foldcase");
    if (err) return err;
    err = CHECK_ARITY_EXACT(a, 1, "string-foldcase");
    if (err) return err;

    UErrorCode status = U_ZERO_ERROR;
    UChar* src = convert_to_utf16(a->cell[0]->str);
    if (!src) return make_cell_error(
        "string-foldcase: malformed UTF-8 string",
        VALUE_ERR);

    const int32_t src_len = u_countChar32(src, -1);

    UChar* dst = GC_MALLOC(sizeof(UChar) * src_len + 1);;
    const int32_t dest_len = u_strFoldCase(dst, src_len + 1, src, -1,
        U_FOLD_CASE_DEFAULT, &status);

    if (dest_len < src_len) {
        return make_cell_error(
            "string-foldcase: some chars not copied!!!",
            GEN_ERR);
    }

    char* result = convert_to_utf8(dst);
    if (!result) {
        return make_cell_error(
            "string-foldcase: malformed UTF-8 string",
            VALUE_ERR);
    }
    return make_cell_string(result);
}

/* (string-ci=? string1 string2 string3 ... )
* Returns #t if, after case-folding, all the strings are the same length and contain the same
* characters in the same positions, otherwise returns #f.*/
Cell* builtin_string_equal_ci(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string-ci=?");
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1, "string-ci=?");
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* quick exit before conversion: if the len is not the same,
         * the strings are not the same. */
        if (strlen(lhs) != strlen(rhs)) {
            return False_Obj;
        }
        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        UErrorCode status = U_ZERO_ERROR;
        if (u_strCaseCompare(U_lhs, -1, U_rhs, -1,
            U_FOLD_CASE_DEFAULT, &status) != 0)  {
            return False_Obj;
        }
    }
    /* If we get here, we're equal */
    return True_Obj;
}

/* (string-ci<? string1 string2 string3 ... ) */
Cell* builtin_string_lt_ci(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string-ci<?");
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1, "string-ci<?");
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        UErrorCode status = U_ZERO_ERROR;
        if (u_strCaseCompare(U_lhs, -1, U_rhs, -1,
            U_FOLD_CASE_DEFAULT, &status) >= 0)  {
            return False_Obj;
        }
    }
    /* If we get here, s1 < s2 < sn ... */
    return True_Obj;
}

/* (string<=? string1 string2 string3 ... ) */
Cell* builtin_string_lte_ci(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string-ci<=?");
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1, "string-ci<=?");
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        UErrorCode status = U_ZERO_ERROR;
        if (u_strCaseCompare(U_lhs, -1, U_rhs, -1,
            U_FOLD_CASE_DEFAULT, &status) > 0)  {
            return False_Obj;
        }
    }
    /* If we get here, s1 <= s2 <= sn ... */
    return True_Obj;
}

/* (string-ci>? string1 string2 string3 ... ) */
Cell* builtin_string_gt_ci(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string-ci>?");
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1, "string-ci>?");
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        UErrorCode status = U_ZERO_ERROR;
        if (u_strCaseCompare(U_lhs, -1, U_rhs, -1,
            U_FOLD_CASE_DEFAULT, &status) <= 0)  {
            return False_Obj;
        }
    }
    /* If we get here, s1 > s2 > sn ... */
    return True_Obj;
}

/* (string-ci>=? string1 string2 string3 ... ) */
Cell* builtin_string_gte_ci(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string-ci>=?");
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1, "string-ci>=?");
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->str;
        const char* rhs = a->cell[i+1]->str;

        /* convert to UTF-16 */
        const UChar* U_lhs = convert_to_utf16(lhs);
        const UChar* U_rhs = convert_to_utf16(rhs);
        UErrorCode status = U_ZERO_ERROR;
        if (u_strCaseCompare(U_lhs, -1, U_rhs, -1,
            U_FOLD_CASE_DEFAULT, &status) >= 0)  {
            return False_Obj;
        }
    }
    /* If we get here, s1 >= s2 >= sn ... */
    return True_Obj;
}
