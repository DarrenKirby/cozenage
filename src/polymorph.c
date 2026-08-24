/*
 * 'src/polymorph.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 - 2026 Darren Kirby <darren@dragonbyte.ca>
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
#include <string.h>
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
    Cell* result = make_cell_bytevector(type, len);
    switch (type) {
        case BV_U8:  REVERSE_CASE(uint8_t);   break;
        case BV_S8:  REVERSE_CASE(int8_t);    break;
        case BV_U16: REVERSE_CASE(uint16_t);  break;
        case BV_S16: REVERSE_CASE(int16_t);   break;
        case BV_U32: REVERSE_CASE(uint32_t);  break;
        case BV_S32: REVERSE_CASE(int32_t);   break;
        case BV_U64: REVERSE_CASE(uint64_t);  break;
        case BV_S64: REVERSE_CASE(int64_t);   break;
        case BV_F32: REVERSE_CASE_FP(float);  break;
        case BV_F64: REVERSE_CASE_FP(double); break;
        default: return make_cell_error("Unknown bytevector type", TYPE_ERR);
    }
    return result;
}


/* fast-ascii and slow-Unicode reverse helpers for strings. */
static char* ascii_reverse(const char* input, const size_t len)
{
    char* reversed = GC_MALLOC_ATOMIC(len + 1);
    if (!reversed) return nullptr;

    /* Simple swap loop. */
    for (size_t i = 0; i < len; i++) {
        reversed[i] = input[len - 1 - i];
    }
    reversed[len] = '\0';
    return reversed;
}


static char* unicode_reverse(const char* input, const int32_t byte_len)
{
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
    char* reversed = GC_MALLOC(byte_len + 1);
    char* revCursor = reversed;

    /* Iterate Backwards. */
    int32_t end = ubrk_last(bi);
    int32_t start = ubrk_previous(bi);

    while (start != UBRK_DONE) {
        /* We have a segment from 'start' to 'end' in the UTF-16 buffer
           Convert just this segment back to UTF-8 and append to our result. */
        int32_t dest_len = 0;

        /* Convert this specific grapheme back to UTF-8. */
        u_strToUTF8(revCursor, byte_len - (int)(revCursor - reversed) + 1, &dest_len,
                    uBuf + start, end - start, &status);

        revCursor += dest_len; /* Advance our output pointer. */

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


static Cell* bytevector_idx(const Cell* a)
{
    const Cell* v = a->cell[0];
    const bv_t type = v->bv->type;
    const int64_t start = a->cell[1]->integer_v;
    int64_t stop = v->count;
    int64_t step = 1;

    if (a->count > 2) {
        stop = a->cell[2]->integer_v;
    }
    if (a->count > 3) {
        step = a->cell[3]->integer_v;
    }

    if (start < 0 || stop > v->count || start > stop || step <= 0) {
        return make_cell_error(
            "idx: bytevector index out of range",
            INDEX_ERR);
    }

    int64_t result_len = 0;
    for (int64_t i = start; i < stop; i += step) result_len++;

    Cell* result = make_cell_bytevector(type, (int32_t)result_len);
    for (int64_t i = start; i < stop; i += step) {
        if (type == BV_F32 || type == BV_F64) {
            BV_FP_OPS[type].append(result, BV_FP_OPS[type].get(v, (int)i));
            continue;
        }
        BV_INT_OPS[type].append(result, BV_INT_OPS[type].get(v, (int)i));
    }
    return result;
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


/* cmp functions for sort. */
static int cmp_char(const void* a, const void* b) {
    const Cell* l = *(const Cell**)a;
    const Cell* r = *(const Cell**)b;

    return (l->char_v > r->char_v) - (l->char_v < r->char_v);
}


static int cmp_integer(const void* a, const void* b) {
    /* Cast to double pointer, then dereference ONCE to get the Cell* */
    const Cell* l = *(const Cell**)a;
    const Cell* r = *(const Cell**)b;

    return (l->integer_v > r->integer_v) - (l->integer_v < r->integer_v);
}


static int cmp_real(const void* a, const void* b) {
    /* Cast to double pointer, then dereference ONCE to get the Cell* */
    const Cell* l = *(const Cell**)a;
    const Cell* r = *(const Cell**)b;

    return (l->real_v > r->real_v) - (l->real_v < r->real_v);
}


static int cmp_string(const void* a, const void* b) {
    /* Cast to double pointer, then dereference ONCE to get the Cell* */
    const Cell* l = *(const Cell**)a;
    const Cell* r = *(const Cell**)b;

    return strcmp(l->str, r->str);
}


static int cmp_symbol(const void* a, const void* b) {
    /* Cast to double pointer, then dereference ONCE to get the Cell* */
    const Cell* l = *(const Cell**)a;
    const Cell* r = *(const Cell**)b;

    return strcmp(l->sym, r->sym);
}


static int cmp_u8(const void* a, const void* b) {
    const uint8_t l = *(uint8_t*)a;
    const uint8_t r = *(uint8_t*)b;
    return (l > r) - (l < r);
}


static int cmp_s8(const void* a, const void* b) {
    const int8_t l = *(int8_t*)a;
    const int8_t r = *(int8_t*)b;
    return (l > r) - (l < r);
}


static int cmp_u16(const void* a, const void* b) {
    const uint16_t l = *(uint16_t*)a;
    const uint16_t r = *(uint16_t*)b;
    return (l > r) - (l < r);
}


static int cmp_s16(const void* a, const void* b) {
    const int16_t l = *(int16_t*)a;
    const int16_t r = *(int16_t*)b;
    return (l > r) - (l < r);
}


static int cmp_u32(const void* a, const void* b) {
    const uint32_t l = *(uint32_t*)a;
    const uint32_t r = *(uint32_t*)b;
    return (l > r) - (l < r);
}


static int cmp_s32(const void* a, const void* b) {
    const int32_t l = *(int32_t*)a;
    const int32_t r = *(int32_t*)b;
    return (l > r) - (l < r);
}

static int cmp_f32(const void* a, const void* b) {
    const float l = *(float*)a;
    const float r = *(float*)b;

    return (l > r) - (l < r);
}

static int cmp_f64(const void* a, const void* b) {
    const double l = *(double*)a;
    const double r = *(double*)b;

    return (l > r) - (l < r);
}


/* Bytevector sort. */
static Cell* sort_bytevector(Cell* bv) {
    const size_t nel = bv->count;
    const size_t width = get_bv_width(bv->bv->type);

    /* Function pointer for the correct comparator. */
    int (*cmp_func)(const void*, const void*);

    switch (bv->bv->type) {
        case BV_U8:
            cmp_func = cmp_u8;
            break;
        case BV_S8:
            cmp_func = cmp_s8;
            break;
        case BV_U16:
            cmp_func = cmp_u16;
            break;
        case BV_S16:
            cmp_func = cmp_s16;
            break;
        case BV_U32:
            cmp_func = cmp_u32;
            break;
        case BV_S32:
            cmp_func = cmp_s32;
            break;
        case BV_F32:
            cmp_func = cmp_f32;
            break;
        case BV_F64:
            cmp_func = cmp_f64;
            break;
        default:
            // Handle error or unsupported type
            return bv;
    }
    if (nel > 0 && width > 0) {
        qsort(bv->bv->data, nel, width, cmp_func);
    }

    return bv;
}


static Cell* sort_vector(Cell* vec, const Cell_t type) {
    const size_t nel = vec->count;

    /* Function pointer for the correct comparator. */
    int (*cmp_func)(const void*, const void*);

    switch (type) {
        case CELL_INTEGER:
            cmp_func = cmp_integer;
            break;
        case CELL_REAL:
            cmp_func = cmp_real;
            break;
        case CELL_STRING:
            cmp_func = cmp_string;
            break;
        case CELL_SYMBOL:
            cmp_func = cmp_symbol;
            break;
        case CELL_CHAR:
            cmp_func = cmp_char;
            break;
        default:
            return make_cell_error(
                fmt_err("sort: cannot sort %s\n", cell_type_name(type)),
                VALUE_ERR);
    }

    if (nel > 0) {
        constexpr size_t width = sizeof(Cell*);
        qsort(vec->cell, nel, width, cmp_func);
    }

    return vec;
}


/* The user-facing procedures. */

Cell* builtin_len(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "len");
    if (err) return err;

    switch (a->cell[0]->type) {
        case CELL_PAIR:
            if (a->cell[0]->len >= 0) return make_cell_integer(a->cell[0]->len);
            /* Still run the check, as the length might not be cached. */
            Cell* result = builtin_list_length(e, a);
            if (result->type == CELL_ERROR) {
                return make_cell_error(
                    "len: no length for improper or circular list",
                    VALUE_ERR);
            }
            return result;
        case CELL_VECTOR:
        case CELL_BYTEVECTOR:
            return make_cell_integer(a->cell[0]->count);
        case CELL_STRING:
            return make_cell_integer(a->cell[0]->char_count);
        case CELL_SET:
        case CELL_HASH:
            return make_cell_integer((long long)a->cell[0]->table->count);
        default:
            return make_cell_error(
                fmt_err("len: no length for non-compound type: %s",
                    cell_type_name(a->cell[0]->type)), TYPE_ERR);
        }
}


/* Polymorphic '*-ref'.
 * (idx <seq object> i)
 * (idx <seq object> start stop)
 * (idx <seq object> start end step) */
Cell* builtin_idx(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 2, 4, "idx");
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
            if (a->count == 2) {
                return builtin_bytevector_ref(e, a);
            }
            return bytevector_idx(a);
        case CELL_STRING:
            return builtin_string_ref(e, a);
        default:
            return make_cell_error(
            fmt_err("idx: cannot subscript non-ordered type: %s",
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
                fmt_err("rev: cannot reverse non-ordered type: %s",
                    cell_type_name(a->cell[0]->type)),
                    TYPE_ERR);
        }
}


Cell* builtin_sort(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "sort");
    if (err) return err;

    Cell* cp = cell_copy(a->cell[0]);

    if (cp->type == CELL_BYTEVECTOR) {
        return sort_bytevector(cp);
    }
    if (cp->type == CELL_VECTOR) {
        /* Ensure homogeneity by grabbing the type of
         * the first member, and enforcing it. */
        const Cell_t t = cp->cell[0]->type;

        for (int i = 1; i < cp->count; i++) {
            if (cp->cell[i]->type != t) {
                return make_cell_error(
                    "sort: cannot sort non-homogenous vector",
                    VALUE_ERR);
            }
        }
        return sort_vector(cp, t);
    }
    if (cp->type == CELL_PAIR) {
        if (cp->len == -1) {
            return make_cell_error("sort: cannot sort improper list", VALUE_ERR);
        }
        Cell* lv = builtin_list_to_vector(e, make_sexpr_len1(a->cell[0]));
        if (lv->type == CELL_ERROR) {
            return lv;
        }

        /* Ensure homogeneity by grabbing the type of
         * the first member, and enforcing it. */
        const Cell_t t = lv->cell[0]->type;

        for (int i = 1; i < lv->count; i++) {
            if (lv->cell[i]->type != t) {
                return make_cell_error(
                    "sort: cannot sort non-homogenous list",
                    VALUE_ERR);
            }
        }
        const Cell* sv = sort_vector(lv, t);
        return builtin_vector_to_list(e, make_sexpr_len1(sv));
    }

    return make_cell_error(
        "sort: can only sort container types",
        TYPE_ERR);
}


Cell* builtin_sort_bang(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "sort");
    if (err) return err;

    Cell* cp = a->cell[0];

    if (cp->type == CELL_BYTEVECTOR) {
        return sort_bytevector(cp);
    }
    if (cp->type == CELL_VECTOR) {
        /* Ensure homogeneity by grabbing the type of
         * the first member, and enforcing it. */
        const Cell_t t = cp->cell[0]->type;

        for (int i = 1; i < cp->count; i++) {
            if (cp->cell[i]->type != t) {
                return make_cell_error(
                    "sort: cannot sort non-homogenous vector",
                    VALUE_ERR);
            }
        }
        return sort_vector(cp, t);
    }
    if (cp->type == CELL_PAIR) {
        if (cp->len == -1) {
            return make_cell_error(
                "sort: cannot sort improper list",
                VALUE_ERR);
        }
        Cell* lv = builtin_list_to_vector(e, make_sexpr_len1(a->cell[0]));
        if (lv->type == CELL_ERROR) {
            return lv;
        }

        /* Ensure homogeneity by grabbing the type of
         * the first member, and enforcing it. */
        const Cell_t t = lv->cell[0]->type;

        for (int i = 1; i < lv->count; i++) {
            if (lv->cell[i]->type != t) {
                return make_cell_error(
                    "sort: cannot sort non-homogenous list",
                    VALUE_ERR);
            }
        }
        const Cell* sv = sort_vector(lv, t);
        return builtin_vector_to_list(e, make_sexpr_len1(sv));
    }

    return make_cell_error(
        "sort!: can only sort container types",
        TYPE_ERR);
}
