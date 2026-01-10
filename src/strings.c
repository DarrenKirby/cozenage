/*
 * 'src/strings.c'
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
#include "vectors.h"

#include <string.h>
#include <stdlib.h>
#include <gc/gc.h>
#include <unicode/utf8.h>
#include <unicode/ustring.h>
#include <unicode/uchar.h>
#include <unicode/umachine.h>


/* Helpers for UTF-8 strings. */

/* Returns the byte offset for the k-th character in a string. */
static int32_t get_utf8_byte_offset(const Cell* s, const int32_t char_idx) {
    if (s->ascii) return char_idx; /* Fast Path: 1 char = 1 byte */

    int32_t byte_offset = 0;
    /* ICU macro: str, current_offset, total_bytes, move_n_chars. */
    U8_FWD_N(s->str, byte_offset, s->count, char_idx);
    return byte_offset;
}


/* Encodes a Unicode codepoint into a byte buffer. Returns length (1-4). */
static int32_t encode_utf8(const uint32_t cp, char* out) {
    if (cp < 0x80) {
        out[0] = (char)cp;
        return 1;
    }

    int32_t i = 0;
    UBool is_err = 0;

    /* U8_APPEND(buffer, index, capacity, codepoint, error_flag). */
    U8_APPEND(out, i, 4, cp, is_err);

    if (is_err) {
        /* If the codepoint is invalid, we fall back to the Unicode
           Replacement Character: U+FFFD.
           In UTF-8, this is 3 bytes: 0xEF, 0xBF, 0xBD. */
        out[0] = (char)0xEF;
        out[1] = (char)0xBF;
        out[2] = (char)0xBD;
        return 3;
    }
    return i;
}


static int string_compare(const Cell* a, const Cell* b) {
    const int32_t min_len = (a->count < b->count) ? a->count : b->count;
    const int res = memcmp(a->str, b->str, min_len);
    if (res != 0) return res;
    /* If prefixes are identical, the shorter string comes first. */
    if (a->count < b->count) return -1;
    if (a->count > b->count) return 1;
    return 0;
}


void integer_to_binary_string(int64_t val, char* buf, size_t size) {
    if (val == 0) { strcpy(buf, "0"); return; }
    char temp[66];
    int i = 0;
    uint64_t v = (val < 0) ? -val : val;
    while (v > 0) {
        temp[i++] = (v & 1) ? '1' : '0';
        v >>= 1;
    }
    if (val < 0) temp[i++] = '-';

    /* Reverse into buf. */
    int j = 0;
    while (i > 0 && (size_t)j < size - 1) buf[j++] = temp[--i];
    buf[j] = '\0';
}


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

    const int32_t char_count = a->count;
    /* Allocate the Cell first so we can fill metadata directly. */
    Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));

    /* Worst-case allocation: 4 bytes per codepoint + null terminator. */
    char* buffer = GC_MALLOC_ATOMIC(char_count * 4 + 1);

    int32_t byte_idx = 0;
    int32_t is_ascii = 1;

    for (int i = 0; i < char_count; i++) {
        const uint32_t cp = (uint32_t)a->cell[i]->char_v;

        if (cp >= 0x80) is_ascii = 0;

        byte_idx += encode_utf8(cp, buffer + byte_idx);
    }

    buffer[byte_idx] = '\0';

    /* Set metadata directly. */
    v->type = CELL_STRING;
    v->str = buffer;
    v->count = byte_idx;        /* Byte length. */
    v->char_count = char_count; /* Char length (already known). */
    v->ascii = is_ascii;

    return v;
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


/* (string=? s1 s2 s3 ...) */
Cell* builtin_string_eq_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string=?");
    if (err) return err;

    /* Arity check: 0 or 1 args is technically true in R7RS,
       but most impls require at least 2 for a useful comparison. */
    if (a->count < 2) return True_Obj;

    for (int i = 0; i < a->count - 1; i++) {
        const Cell* lhs = a->cell[i];
        const Cell* rhs = a->cell[i+1];

        /* Pointer identity (Fastest). */
        if (lhs == rhs) continue;

        /* Byte length check (Fast)
           Eliminates the need for strlen(). */
        if (lhs->count != rhs->count) {
            return False_Obj;
        }

        /* Raw byte comparison (Very Fast)
           Since UTF-8 is unique for a given sequence of codepoints,
           memcmp is sufficient for string=? */
        if (memcmp(lhs->str, rhs->str, lhs->count) != 0) {
            return False_Obj;
        }
    }

    return True_Obj;
}


Cell* builtin_string_lt_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string<?");
    if (err) return err;
    if (a->count < 2) return True_Obj;

    for (int i = 0; i < a->count - 1; i++) {
        const Cell* lhs = a->cell[i];
        const Cell* rhs = a->cell[i+1];

        if (lhs == rhs) return False_Obj; /* A string is not less than itself. */

        if (string_compare(lhs, rhs) >= 0) {
            return False_Obj;
        }
    }
    return True_Obj;
}


Cell* builtin_string_lte_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string<?");
    if (err) return err;
    if (a->count < 2) return True_Obj;

    for (int i = 0; i < a->count - 1; i++) {
        const Cell* lhs = a->cell[i];
        const Cell* rhs = a->cell[i+1];

        if (string_compare(lhs, rhs) > 0) {
            return False_Obj;
        }
    }
    return True_Obj;
}


Cell* builtin_string_gt_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string<?");
    if (err) return err;
    if (a->count < 2) return True_Obj;

    for (int i = 0; i < a->count - 1; i++) {
        const Cell* lhs = a->cell[i];
        const Cell* rhs = a->cell[i+1];

        if (lhs == rhs) return False_Obj; /* A string is not less than itself. */

        if (string_compare(lhs, rhs) <= 0) {
            return False_Obj;
        }
    }
    return True_Obj;
}


Cell* builtin_string_gte_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string<?");
    if (err) return err;
    if (a->count < 2) return True_Obj;

    for (int i = 0; i < a->count - 1; i++) {
        const Cell* lhs = a->cell[i];
        const Cell* rhs = a->cell[i+1];

        if (string_compare(lhs, rhs) < 0) {
            return False_Obj;
        }
    }
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

    /* Handle (string-append) or (string-append "foo"). */
    if (a->count == 0) return make_cell_string("");
    if (a->count == 1) return a->cell[0];

    uint32_t total_bytes = 0;
    uint32_t total_chars = 0;
    int is_ascii = 1;

    /* Calculate totals from metadata (No strlen calls!). */
    for (int i = 0; i < a->count; i++) {
        total_bytes += a->cell[i]->count;
        total_chars += a->cell[i]->char_count;
        if (!a->cell[i]->ascii) is_ascii = 0;
    }

    /* Allocate the exact buffer once. */
    char* buffer = GC_MALLOC_ATOMIC(total_bytes + 1);
    char* current_ptr = buffer;

    /* Copy data directly. */
    for (int i = 0; i < a->count; i++) {
        const Cell* s = a->cell[i];
        memcpy(current_ptr, s->str, s->count);
        current_ptr += s->count;
    }
    *current_ptr = '\0';

    /* Construct Cell and set metadata manually to avoid rescanning. */
    Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));
    v->type = CELL_STRING;
    v->str = buffer;
    v->count = (int)total_bytes;
    v->char_count = (int)total_chars;
    v->ascii = is_ascii;

    return v;
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

    const Cell* s_cell = a->cell[0];
    if (s_cell->type != CELL_STRING)
        return make_cell_error(
            "string-ref: arg 1 must be a string",
            TYPE_ERR);
    if (a->cell[1]->type != CELL_INTEGER)
        return make_cell_error(
            "string-ref: arg 2 must be an integer",
            TYPE_ERR);

    const int32_t char_idx = (int32_t)a->cell[1]->integer_v;

    /* O(1) Bounds Validation using Metadata. */
    if (char_idx < 0 || char_idx >= s_cell->char_count) {
        return make_cell_error(
            "string-ref: index out of range",
            INDEX_ERR);
    }

    /* FAST PATH: ASCII. */
    if (s_cell->ascii) {
        return make_cell_char(s_cell->str[char_idx]);
    }

    /* SLOW PATH: UTF-8 */
    /* Find the byte offset of the desired character. */
    const int32_t byte_offset = get_utf8_byte_offset(s_cell, char_idx);

    UChar32 c;
    int32_t temp_offset = byte_offset;
    /* U8_NEXT reads the character and advances the offset. */
    U8_NEXT(s_cell->str, temp_offset, s_cell->count, c);

    /* ICU returns a negative value if the encoding is invalid. */
    if (c < 0) {
        return make_cell_error(
            "string-ref: malformed UTF-8 sequence",
            VALUE_ERR);
    }

    return make_cell_char(c);
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

    if (a->cell[0]->type != CELL_INTEGER)
        return make_cell_error(
            "make-string: arg 1 must be an integer",
            TYPE_ERR);

    const int32_t char_count = (int32_t)a->cell[0]->integer_v;
    if (char_count < 0)
        return make_cell_error(
            "make-string: length cannot be negative",
            VALUE_ERR);

    /* Default to space (U+0020) if no char provided. */
    const uint32_t fill_cp = (a->count == 2) ? (uint32_t)a->cell[1]->char_v : 0x0020;

    if (a->count == 2 && a->cell[1]->type != CELL_CHAR)
        return make_cell_error(
            "make-string: arg 2 must be a char",
            TYPE_ERR);

    /* Allocate the Cell and the Buffer. */
    Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));
    char* buffer;
    int32_t total_bytes;
    const int is_ascii = (fill_cp <= 0x7F);

    if (is_ascii) {
        total_bytes = char_count;
        buffer = GC_MALLOC_ATOMIC(total_bytes + 1);
        /* memset is much faster than a manual loop for ASCII. */
        memset(buffer, (char)fill_cp, total_bytes);
    } else {
        char encoded[4];
        int32_t char_len = encode_utf8(fill_cp, encoded);
        total_bytes = char_count * char_len;
        buffer = GC_MALLOC_ATOMIC(total_bytes + 1);

        /* Fill the buffer with the multibyte sequence. */
        for (int32_t i = 0; i < total_bytes; i += char_len) {
            memcpy(buffer + i, encoded, char_len);
        }
    }
    buffer[total_bytes] = '\0';

    /* Set metadata directly to skip the scan pass. */
    v->type = CELL_STRING;
    v->str = buffer;
    v->count = total_bytes;
    v->char_count = char_count;
    v->ascii = is_ascii;

    return v;
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

    Cell* s_cell = a->cell[0];
    if (s_cell->type != CELL_STRING)
        return make_cell_error(
            "string->list: arg 1 must be a string",
            TYPE_ERR);

    const int32_t str_len = s_cell->char_count;
    int32_t start = 0;
    int32_t end = str_len;

    /* Extract and Validate Indices. */
    if (a->count >= 2) {
        if (a->cell[1]->type != CELL_INTEGER)
            return make_cell_error(
                "string->list: start must be an integer",
                TYPE_ERR);
        start = (int32_t)a->cell[1]->integer_v;
    }
    if (a->count == 3) {
        if (a->cell[2]->type != CELL_INTEGER)
            return make_cell_error(
                "string->list: end must be an integer",
                TYPE_ERR);
        end = (int32_t)a->cell[2]->integer_v;
    }

    if (start < 0 || end > str_len || start > end) {
        return make_cell_error(
            "string->list: index out of range",
            INDEX_ERR);
    }

    /* Jump to the starting byte offset. */
    int32_t byte_index = get_utf8_byte_offset(s_cell, start);

    Cell* head = make_cell_nil();
    Cell* tail = nullptr;
    const int32_t remaining = end - start;

    /* Build the list. */
    for (int32_t i = 0; i < remaining; i++) {
        uint32_t cp;

        if (s_cell->ascii) {
            /* ASCII Fast Path: Direct access. */
            cp = (uint32_t)(unsigned char)s_cell->str[byte_index++];
        } else {
            /* UTF-8 Path: Use ICU to get codepoint. */
            UChar32 c;
            U8_NEXT(s_cell->str, byte_index, s_cell->count, c);
            cp = (uint32_t)c;
        }

        Cell* new_pair = make_cell_pair(make_cell_char((int)cp), make_cell_nil());
        new_pair->len = remaining - i;

        if (head->type == CELL_NIL) {
            head = new_pair;
            tail = new_pair;
        } else {
            tail->cdr = new_pair;
            tail = new_pair;
        }
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
    if (l->type != CELL_PAIR && l->type != CELL_NIL)
        return make_cell_error(
            "list->string: arg must be a list",
            TYPE_ERR);

    /* First pass: Validate types and calculate required memory. */
    int32_t total_bytes = 0;
    int32_t char_count = 0;
    int is_ascii = 1;
    const Cell* curr = l;

    while (curr->type == CELL_PAIR) {
        if (curr->car->type != CELL_CHAR)
            return make_cell_error(
                "list->string: all elements must be chars",
                TYPE_ERR);

        const uint32_t cp = (uint32_t)curr->car->char_v;
        if (cp >= 0x80) is_ascii = 0;

        total_bytes += U8_LENGTH(cp);
        char_count++;
        curr = curr->cdr;
    }

    /* Allocate and Second pass: Encode directly. */
    char* buffer = GC_MALLOC_ATOMIC(total_bytes + 1);
    int32_t byte_idx = 0;
    curr = l;

    while (curr->type == CELL_PAIR) {
        const uint32_t cp = (uint32_t)curr->car->char_v;

        if (cp < 0x80) {
            buffer[byte_idx++] = (char)cp;
        } else {
            UBool is_err = 0;
            U8_APPEND(buffer, byte_idx, total_bytes, cp, is_err);

            if (is_err) {
                /* If the codepoint is invalid, we fall back to the Unicode.
                   Replacement Character: U+FFFD.
                   In UTF-8, this is 3 bytes: 0xEF, 0xBF, 0xBD */
                buffer[0] = (char)0xEF;
                buffer[1] = (char)0xBF;
                buffer[2] = (char)0xBD;
                total_bytes = 3;
            }
        }
        curr = curr->cdr;
    }
    buffer[total_bytes] = '\0';

    /* Construct Cell with manual metadata. */
    Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));
    v->type = CELL_STRING;
    v->str = buffer;
    v->count = total_bytes;
    v->char_count = char_count;
    v->ascii = is_ascii;

    return v;
}


/* (substring string start end)
 * The substring procedure returns a newly allocated string formed from the characters of string
 * beginning with index start and ending with index end. This is equivalent to calling string-copy
 * with the same arguments, but is provided for backward compatibility and stylistic flexibility. */
Cell* builtin_substring(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 3, "substring");
    if (err) return err;

    Cell* s_cell = a->cell[0];
    if (s_cell->type != CELL_STRING)
        return make_cell_error(
            "substring: arg 1 must be a string",
            TYPE_ERR);

    const int32_t start = (int32_t)a->cell[1]->integer_v;
    const int32_t end   = (int32_t)a->cell[2]->integer_v;

    /* Bounds Validation. */
    if (start < 0 || end > s_cell->char_count || start > end)
        return make_cell_error(
            "substring: index out of range",
            INDEX_ERR);

    /* Find Byte Offsets. */
    const int32_t start_byte = get_utf8_byte_offset(s_cell, start);
    const int32_t end_byte = get_utf8_byte_offset(s_cell, end);
    int32_t byte_len = end_byte - start_byte;

    /* Create the New String. */
    char* buffer = GC_MALLOC_ATOMIC(byte_len + 1);
    memcpy(buffer, s_cell->str + start_byte, byte_len);
    buffer[byte_len] = '\0';

    /* Construct Cell and Populate Metadata. */
    Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));
    v->type = CELL_STRING;
    v->str = buffer;
    v->count = byte_len;
    v->char_count = end - start;
    v->ascii = s_cell->ascii;

    /* Note: If the parent wasn't ASCII, the substring MIGHT be ASCII,
       but it's safer/faster to just inherit the '0' flag unless you
       want to re-scan with is_pure_ascii(). */
    if (!v->ascii) {
        v->ascii = is_pure_ascii(v->str, v->count);
    }

    return v;
}


/* (string-set! string k char)
 * It is an error if k is not a valid index of string. The string-set! procedure stores char in
 * element k of string. */
Cell* builtin_string_set_bang(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 3, "string-set!");
    if (err) return err;

    Cell* s_cell = a->cell[0];
    if (s_cell->type != CELL_STRING)
        return make_cell_error(
            "string-set!: arg 1 must be a string",
            TYPE_ERR);
    if (a->cell[1]->type != CELL_INTEGER)
        return make_cell_error(
            "string-set!: arg 2 must be an integer",
            TYPE_ERR);
    if (a->cell[2]->type != CELL_CHAR)
        return make_cell_error(
            "string-set!: arg 3 must be a char",
            TYPE_ERR);

    const int32_t char_idx = (int32_t)a->cell[1]->integer_v;
    const uint32_t new_cp = (uint32_t)a->cell[2]->char_v;

    /* Bounds Validation. */
    if (char_idx < 0 || char_idx >= s_cell->char_count)
        return make_cell_error(
            "string-set!: index out of range",
            INDEX_ERR);

    /* FAST PATH: ASCII to ASCII. */
    if (s_cell->ascii && new_cp < 128) {
        s_cell->str[char_idx] = (char)new_cp;
        return USP_Obj;
    }

    /* SLOW PATH: UTF-8 Mutation. */
    int32_t old_char_offset = get_utf8_byte_offset(s_cell, char_idx);

    /* Determine old char byte length. */
    UChar32 dummy;
    int32_t temp_offset = old_char_offset;
    U8_NEXT(s_cell->str, temp_offset, s_cell->count, dummy);
    const int32_t old_char_len = temp_offset - old_char_offset;

    /* Determine new char byte length. */
    char new_encoded[4];
    int32_t new_char_len = encode_utf8(new_cp, new_encoded);

    if (old_char_len == new_char_len) {
        /* Same size? Just overwrite in place. */
        memcpy(s_cell->str + old_char_offset, new_encoded, new_char_len);
    } else {
        /* Different size? We must reallocate and shift. */
        const int32_t new_total_bytes = s_cell->count - old_char_len + new_char_len;
        char* new_buf = GC_MALLOC_ATOMIC(new_total_bytes + 1);

        /* Copy prefix. */
        memcpy(new_buf, s_cell->str, old_char_offset);
        /* Insert new char. */
        memcpy(new_buf + old_char_offset, new_encoded, new_char_len);
        /* Copy suffix. */
        const int32_t suffix_offset = old_char_offset + old_char_len;
        int32_t suffix_len = s_cell->count - suffix_offset;
        memcpy(new_buf + old_char_offset + new_char_len, s_cell->str + suffix_offset, suffix_len);

        new_buf[new_total_bytes] = '\0';
        s_cell->str = new_buf;
        s_cell->count = new_total_bytes;
    }

    /* Update Metadata. */
    if (new_cp >= 128) {
        s_cell->ascii = 0;
    } else if (!s_cell->ascii) {
        /* If the string was UTF-8, and we inserted an ASCII char,
           we could re-scan to see if it's now pure ASCII, but usually,
           it's better to just leave it as 0 for performance. */
        s_cell->ascii = is_pure_ascii(s_cell->str, s_cell->count);
    }

    return USP_Obj;
}


/* (string-copy string )
 * (string-copy string start )
 * (string-copy string start end )
 * Returns a newly allocated copy of the part of the given string between start and end. */
Cell* builtin_string_copy(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3, "string-copy");
    if (err) return err;

    Cell* s_cell = a->cell[0];
    if (s_cell->type != CELL_STRING)
        return make_cell_error(
            "string-copy: arg 1 must be a string",
            TYPE_ERR);

    /* Extract and Default Indices. */
    int32_t start = 0;
    int32_t end = s_cell->char_count;

    if (a->count >= 2) {
        if (a->cell[1]->type != CELL_INTEGER)
            return make_cell_error(
                "string-copy: arg 2 must be an integer",
                TYPE_ERR);
        start = (int32_t)a->cell[1]->integer_v;
    }
    if (a->count == 3) {
        if (a->cell[2]->type != CELL_INTEGER)
            return make_cell_error(
                "string-copy: arg 3 must be an integer",
                TYPE_ERR);
        end = (int32_t)a->cell[2]->integer_v;
    }

    /* Validation. */
    if (start < 0 || end > s_cell->char_count || start > end)
        return make_cell_error(
            "string-copy: index out of range",
            INDEX_ERR);

    /* Handle Full Copy Shortcut */
    /* If the user wants the whole string, just do a clean byte-copy and clone metadata. */
    if (start == 0 && end == s_cell->char_count) {
        char* new_str = GC_strndup(s_cell->str, s_cell->count);
        Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));
        v->type = CELL_STRING;
        v->str = new_str;
        v->count = s_cell->count;
        v->char_count = s_cell->char_count;
        v->ascii = s_cell->ascii;
        return v;
    }

    /* Substring Path (calculate byte offsets). */
    const int32_t byte_start = get_utf8_byte_offset(s_cell, start);
    const int32_t byte_end = get_utf8_byte_offset(s_cell, end);
    int32_t byte_len = byte_end - byte_start;

    char* buffer = GC_MALLOC_ATOMIC(byte_len + 1);
    memcpy(buffer, s_cell->str + byte_start, byte_len);
    buffer[byte_len] = '\0';

    /* Construct and set metadata. */
    Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));
    v->type = CELL_STRING;
    v->str = buffer;
    v->count = byte_len;
    v->char_count = end - start;
    v->ascii = s_cell->ascii;

    /* Re-verify ASCII only if parent was UTF-8 (slice might be ASCII). */
    if (!v->ascii) {
        v->ascii = is_pure_ascii(v->str, v->count);
    }

    return v;
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
    Cell* err = CHECK_ARITY_RANGE(a, 3, 5, "string-copy!");
    if (err) return err;

    /* 1. Extract Cells and Basic Info. */
    Cell* to_cell = a->cell[0];
    const Cell* from_cell = a->cell[2];
    if (to_cell->type != CELL_STRING || from_cell->type != CELL_STRING)
        return make_cell_error(
            "string-copy!: arguments must be strings",
            TYPE_ERR);

    const int32_t to_at = (int32_t)a->cell[1]->integer_v;
    const int32_t f_start = (a->count >= 4) ? (int32_t)a->cell[3]->integer_v : 0;
    const int32_t f_end = (a->count == 5) ? (int32_t)a->cell[4]->integer_v : from_cell->char_count;

    /* Validation using char_count metadata. */
    if (to_at < 0 || to_at > to_cell->char_count)
        return make_cell_error(
            "string-copy!: 'at' index out of range",
            INDEX_ERR);
    if (f_start < 0 || f_end > from_cell->char_count || f_start > f_end)
        return make_cell_error(
            "string-copy!: 'from' indices out of range",
            INDEX_ERR);

    int32_t num_chars = f_end - f_start;
    if (to_at + num_chars > to_cell->char_count)
        return make_cell_error(
            "string-copy!: target string too small",
            VALUE_ERR);

    /* FAST PATH: ASCII to ASCII */
    if (to_cell->ascii && from_cell->ascii) {
        /* No resizing needed, just a memmove (to handle overlap correctly). */
        memmove(to_cell->str + to_at, from_cell->str + f_start, num_chars);
        return to_cell;
    }

    /* SLOW PATH: UTF-8 Reconstruction. */
    /* Because char sizes differ, we calculate byte offsets and build a new buffer. */
    int32_t to_prefix_bytes = get_utf8_byte_offset(to_cell, to_at);
    const int32_t to_suffix_start = get_utf8_byte_offset(to_cell, to_at + num_chars);
    int32_t to_suffix_bytes = to_cell->count - to_suffix_start;

    const int32_t from_start_byte = get_utf8_byte_offset(from_cell, f_start);
    const int32_t from_end_byte   = get_utf8_byte_offset(from_cell, f_end);
    int32_t bytes_to_copy   = from_end_byte - from_start_byte;

    const int32_t total_bytes = to_prefix_bytes + bytes_to_copy + to_suffix_bytes;
    char* new_str = GC_MALLOC_ATOMIC(total_bytes + 1);

    memcpy(new_str, to_cell->str, to_prefix_bytes);
    memcpy(new_str + to_prefix_bytes, from_cell->str + from_start_byte, bytes_to_copy);
    memcpy(new_str + to_prefix_bytes + bytes_to_copy, to_cell->str + to_suffix_start, to_suffix_bytes);
    new_str[total_bytes] = '\0';

    /* Update Metadata. */
    to_cell->str = new_str;
    to_cell->count = total_bytes;
    if (!from_cell->ascii) to_cell->ascii = 0;

    return to_cell;
}

/* (string-fill! string fill)
 * (string-fill! string fill start)
 * (string-fill! string fill start end)
 * It is an error if fill is not a character. The string-fill! procedure stores fill in the elements
 * of string between start and end. */
Cell* builtin_string_fill_bang(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 2, 4, "string-fill!");
    if (err) return err;

    Cell* s = a->cell[0];
    const uint32_t fill_char = (uint32_t)a->cell[1]->char_v;
    if (s->type != CELL_STRING) return make_cell_error(
        "string-fill!: arg 1 must be string",
        TYPE_ERR);

    int32_t start = 0;
    int32_t end = s->char_count;

    /* Handle Optional Indices (Using the logic we fixed for vector-fill!). */
    if (a->count >= 3) {
        start = (int32_t)a->cell[2]->integer_v;
        if (start < 0 || start > s->char_count) return make_cell_error(
            "string-fill!: start out of bounds",
            INDEX_ERR);
    }
    if (a->count == 4) {
        end = (int32_t)a->cell[3]->integer_v;
        if (end < start || end > s->char_count) return make_cell_error(
            "string-fill!: end out of bounds",
            INDEX_ERR);
    }

    /* Determine Fill Character Properties. */
    char encoded[4];
    const int32_t char_len = encode_utf8(fill_char, encoded);
    int32_t num_chars_to_fill = end - start;

    /* FAST PATH: ASCII fill on ASCII string. */
    if (s->ascii && fill_char < 128) {
        memset(s->str + start, (char)fill_char, num_chars_to_fill);
        return USP_Obj;
    }

    /* SLOW PATH: UTF-8 Reconstruction */
    /* Since the byte-length of the fill char might differ from the existing chars,
       it's often safest/cleanest to build a new buffer. */

    int32_t prefix_bytes = get_utf8_byte_offset(s, start);
    const int32_t suffix_start_offset = get_utf8_byte_offset(s, end);
    int32_t suffix_bytes = s->count - suffix_start_offset;

    const int32_t new_total_bytes = prefix_bytes + (num_chars_to_fill * char_len) + suffix_bytes;
    char* new_str = GC_MALLOC_ATOMIC(new_total_bytes + 1);

    /* Copy original prefix. */
    memcpy(new_str, s->str, prefix_bytes);

    /* Write fill characters. */
    char* p = new_str + prefix_bytes;
    for (int32_t i = 0; i < num_chars_to_fill; i++) {
        for (int32_t b = 0; b < char_len; b++) {
            *p++ = encoded[b];
        }
    }

    /* Copy original suffix. */
    memcpy(p, s->str + suffix_start_offset, suffix_bytes);
    new_str[new_total_bytes] = '\0';

    /* Update Metadata. */
    s->str = new_str;
    s->count = new_total_bytes;
    if (fill_char >= 128) s->ascii = 0;

    return USP_Obj;
}


Cell* builtin_string_number(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2, "string->number");
    if (err) return err;

    const Cell* s_cell = a->cell[0];
    if (s_cell->type != CELL_STRING)
        return make_cell_error(
            "string->number: arg 1 must be a string",
            TYPE_ERR);

    /* Validate Radix. */
    int radix = 10;
    if (a->count == 2) {
        if (a->cell[1]->type != CELL_INTEGER)
            return make_cell_error(
                "string->number: radix must be integer",
                TYPE_ERR);
        radix = (int)a->cell[1]->integer_v;
        if (radix != 2 && radix != 8 && radix != 10 && radix != 16)
            return make_cell_error(
                "string->number: invalid radix (must be 2, 8, 10, 16)",
                VALUE_ERR);
    }

    /* Sanity Check: Numbers must be ASCII. */
    if (!s_cell->ascii) return False_Obj;

    /* Prepare Parsing Buffer. */
    /* Add space for the radix prefix (e.g., "#x") and null terminator. */
    size_t buf_size = s_cell->count + 4;
    char* parse_buf = GC_MALLOC_ATOMIC(buf_size);

    if (radix == 10) {
        memcpy(parse_buf, s_cell->str, s_cell->count + 1);
    } else {
        const char* prefix = (radix == 2) ? "#b" : (radix == 8) ? "#o" : "#x";

        if (strpbrk(s_cell->str, ".eEsSfFdDlL")) return False_Obj;

        /* Use the calculated buf_size here. */
        snprintf(parse_buf, buf_size, "%s%s", prefix, s_cell->str);
    }

    /* Use internal Lexer/Parser */
    TokenArray* ta = scan_all_tokens(parse_buf);
    if (!ta) return False_Obj;

    Cell* result = parse_tokens(ta);

    /* Validation of Result. */
    if (result->type == CELL_ERROR) return False_Obj;

    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const int num_mask = CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX;
    if (!(result->type & num_mask)) return False_Obj;

    return result;
}

Cell* builtin_number_string(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2, "number->string");
    if (err) return err;

    const Cell* num = a->cell[0];
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const int num_mask = CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX;
    if (!(num->type & num_mask)) {
        return make_cell_error(
            "number->string: arg 1 must be a number",
            TYPE_ERR);
    }

    int radix = 10;
    if (a->count == 2) {
        if (a->cell[1]->type != CELL_INTEGER)
            return make_cell_error(
                "number->string: radix must be an integer",
                TYPE_ERR);
        radix = (int)a->cell[1]->integer_v;
        if (radix != 2 && radix != 8 && radix != 10 && radix != 16)
            return make_cell_error(
                "number->string: invalid radix (2, 8, 10, 16)",
                VALUE_ERR);
    }

    char buf[128]; /* Large enough for a 64-bit binary string. */
    const char* result_str;

    /* Handle Integers with Radix. */
    if (num->type == CELL_INTEGER && radix != 10) {
        if (radix == 2) {
            integer_to_binary_string(num->integer_v, buf, sizeof(buf));
        } else {
            /* Use %lo for octal, %lx for hex.
               Cast to unsigned long to match the format specifiers. */
            const char* fmt = (radix == 8) ? "%lo" : "%lx";
            snprintf(buf, sizeof(buf), fmt, (unsigned long)num->integer_v);
        }
        result_str = buf;
    } else {
        result_str = cell_to_string(num, MODE_DISPLAY);
    }

    /* Since numbers are always ASCII, we can optimize the constructor. */
    return make_cell_string(result_str);
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

    UChar* dst = GC_MALLOC(sizeof(UChar) * src_len + 1);
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

    UChar* dst = GC_MALLOC(sizeof(UChar) * src_len + 1);
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

    UChar* dst = GC_MALLOC(sizeof(UChar) * src_len + 1);
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

Cell* builtin_string_split(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "string-split");
    if (err) return err;
    err = CHECK_ARITY_RANGE(a, 1, 2, "string-split");
    if (err) return err;

    char* sep;
    if (a->count == 2) {
        sep = a->cell[1]->str;
    } else {
        sep = " ";
    }

    char* src = GC_strdup(a->cell[0]->str);
    Cell* result = make_cell_vector();
    char* token;

    while ((token = strsep(&src, sep)) != NULL) {
        Cell* s = make_cell_string(token);
        if (strcmp(s->str, "") == 0) {
            continue;
        }
        cell_add(result, s);
    }

    return builtin_vector_to_list(e, make_sexpr_len1(result));
}
