/*
 * 'src/polymorph.c'
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

#include "polymorph.h"
#include "bytevectors.h"
#include "cell.h"
#include "pairs.h"
#include "strings.h"
#include "vectors.h"

#include <stdlib.h>
#include <unicode/utypes.h>
#include <unicode/ubrk.h>
#include <unicode/ustring.h>


static Cell* vector_reverse(const Cell* v)
{
    const int32_t len = v->count;
    Cell* result = make_cell_vector();
    for (int32_t i = len - 1; i >= 0; i--)
    {
        cell_add(result, v->cell[i]);
    }
    return result;
}


static Cell* bytevector_reverse(const Cell* v)
{
    (void)v;
    const bv_t type = v->bv->type;
    const int32_t len = v->count;
    Cell* result = make_cell_bytevector(type);
    switch (type) {
        case BV_U8:  REVERSE_CASE(uint8_t);  break;
        case BV_S8:  REVERSE_CASE(int8_t);   break;
        case BV_U16: REVERSE_CASE(uint16_t); break;
        case BV_S16: REVERSE_CASE(int16_t);  break;
        case BV_U32: REVERSE_CASE(uint32_t); break;
        case BV_S32: REVERSE_CASE(int32_t);  break;
        case BV_U64: REVERSE_CASE(uint64_t); break;
        case BV_S64: REVERSE_CASE(int64_t);  break;
        default: return make_cell_error("No f32 or f64 bv yet", TYPE_ERR);
    }
    return result;
}


/* fast-ascii and slow-Unicode reverse helpers for strings. */
static char* ascii_reverse(const char* input, const size_t len) {
    char* reversed = GC_MALLOC_ATOMIC(len + 1);
    if (!reversed) return nullptr;

    /* Simple swap loop. */
    for (size_t i = 0; i < len; i++) {
        reversed[i] = input[len - 1 - i];
    }
    reversed[len] = '\0';
    return reversed;
}


static char* unicode_reverse(const char* input, const int32_t byte_len) {
    UErrorCode status = U_ZERO_ERROR;

    /* Convert UTF-8 to UChar (UTF-16) because ICU Break Iterators work natively on UChar. */
    const int32_t uBufSize = byte_len + 1; // logical max
    UChar* uBuf = GC_MALLOC_ATOMIC(uBufSize * sizeof(UChar));
    int32_t uLen = 0;

    u_strFromUTF8(uBuf, uBufSize, &uLen, input, byte_len, &status);
    if (U_FAILURE(status)) {
        free(uBuf);
        return nullptr;
    }

    /* Create the Break Iterator (Character/Grapheme mode). */
    UBreakIterator* bi = ubrk_open(UBRK_CHARACTER, nullptr, uBuf, uLen, &status);
    if (U_FAILURE(status)) {
        free(uBuf);
        return nullptr;
    }

    /* Allocate Output Buffer (Same size as input + null). */
    char* reversed = malloc(byte_len + 1);
    char* revCursor = reversed;

    /* Iterate Backwards. */
    int32_t end = ubrk_last(bi);
    int32_t start = ubrk_previous(bi);

    while (start != UBRK_DONE) {
        /* We have a segment from 'start' to 'end' in the UTF-16 buffer
           Convert just this segment back to UTF-8 and append to our result. */
        int32_t destLen = 0;

        /* Convert this specific grapheme back to UTF-8. */
        u_strToUTF8(revCursor, byte_len - (int)(revCursor - reversed) + 1, &destLen,
                    uBuf + start, end - start, &status);

        revCursor += destLen; /* Advance our output pointer. */

        /* Move pointers back. */
        end = start;
        start = ubrk_previous(bi);
    }

    *revCursor = '\0';
    ubrk_close(bi);

    return reversed;
}


static Cell* string_reverse(const Cell* v)
{
    (void)v;
    const char* the_string = v->str;
    const int32_t len = v->count;

    char* result;
    if (is_pure_ascii(the_string, len)) {
        /* FAST PATH: No overhead, just swap bytes. */
        result =  ascii_reverse(the_string, len);
    } else {
        /* SLOW PATH: Load ICU, break iterators, handle emojis/accents. */
        result = unicode_reverse(the_string, len);
    }
    if (result == nullptr) {
        return make_cell_error(
            "rev: reverse operation failed",
            GEN_ERR);
    }
    return make_cell_string(result);
}


static Cell* list_idx(const Lex* e, const Cell* a)
{
    const Cell* v = builtin_list_to_vector(e, make_sexpr_len1(a->cell[0]));
    const int64_t start = a->cell[1]->integer_v;
    int64_t stop = v->count;
    int64_t step = 1;
    if (a->count > 2) {
        stop = a->cell[2]->integer_v;
    }
    if (a->count > 3) {
        step = a->cell[3]->integer_v;
    }

    Cell* result = make_cell_vector();
    for (int64_t i = start; i < stop; i+=step) {
        cell_add(result, v->cell[i]);
    }
    return builtin_vector_to_list(e, make_sexpr_len1(result));
}


static Cell* vector_idx(const Cell* a)
{
    const Cell* v = a->cell[0];
    const int64_t start = a->cell[1]->integer_v;
    int64_t stop = v->count;
    int64_t step = 1;
    if (a->count > 2) {
        stop = a->cell[2]->integer_v;
    }
    if (a->count > 3) {
        step = a->cell[3]->integer_v;
    }

    Cell* result = make_cell_vector();
    for (int64_t i = start; i < stop; i+=step) {
        cell_add(result, v->cell[i]);
    }
    return result;
}


Cell* builtin_len(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "len");
    if (err) return err;

    switch (a->cell[0]->type) {
    case CELL_PAIR:
        return builtin_list_length(e, a);
    case CELL_VECTOR:
        return builtin_vector_length(e, a);
    case CELL_BYTEVECTOR:
        return builtin_bytevector_length(e, a);
    case CELL_STRING:
        return builtin_string_length(e, a);
    default:
        return make_cell_error(
            fmt_err("len: no length for non-compound type: %s",
                cell_type_name(a->cell[0]->type)), TYPE_ERR);
    }
}


/* Polymorphic '*-ref'.
 * (at <seq object> i)
 * (at <seq object> start stop)
 * (at <seq object> start end step) */
Cell* builtin_idx(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 2, 4, "at");
    if (err) return err;

    switch (a->cell[0]->type) {
    case CELL_PAIR:
        if (a->count == 2) {
            return builtin_list_ref(e, a);
        }
        return list_idx(e, a);
    case CELL_VECTOR:
        if (a->count == 2) {
            return builtin_vector_ref(e, a);
        }
        return vector_idx(a);
    case CELL_BYTEVECTOR:
        return builtin_bytevector_ref(e, a);
    case CELL_STRING:
        return builtin_string_ref(e, a);
    default:
        return make_cell_error(
        fmt_err("at: cannot subscript non-compound type: %s",
            cell_type_name(a->cell[0]->type)), TYPE_ERR);
    }
}


Cell* builtin_rev(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "rev");
    if (err) return err;
    switch (a->cell[0]->type) {
    case CELL_PAIR:
        return builtin_list_reverse(e, a);
    case CELL_VECTOR:
        return vector_reverse(a->cell[0]);
    case CELL_BYTEVECTOR:
        return bytevector_reverse(a->cell[0]);
    case CELL_STRING:
        return string_reverse(a->cell[0]);
    default:
        return make_cell_error(
            fmt_err("rev: cannot reverse non-compound type: %s",
                cell_type_name(a->cell[0]->type)),
                TYPE_ERR);
    }
}
