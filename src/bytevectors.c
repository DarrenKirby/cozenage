/*
 * 'src/bytevectors.c'
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

#include "bytevectors.h"
#include "strings.h"
#include "types.h"

#include <float.h>
#include <math.h>
#include <gc/gc.h>
#include <inttypes.h>


static long double get_f32(const Cell* bv, const int i) {
    return ((float*)bv->bv->data)[i];
}


static void set_f32(Cell* bv, const int i, const long double val) {
    ((float*)bv->bv->data)[i] = (float)val;
}


static void append_f32(Cell* bv, const long double val) {
    if (bv->count == bv->bv->capacity) {
        bv->bv->capacity *= 2;
        bv->bv->data = GC_realloc(bv->bv->data, bv->bv->capacity * sizeof(float));
    }
    ((float*)bv->bv->data)[bv->count++] = (float)val;
}


static void repr_f32(const Cell* bv, str_buf_t *sb) {
    sb_append_fmt(sb, "#%s(", "f32");
    for (int i = 0; i < bv->count; i++) {
        sb_append_fmt(sb, "%.7g", ((float*)bv->bv->data)[i]);
        if (i != bv->count - 1) sb_append_char(sb, ' ');
    }
    sb_append_char(sb, ')');
}


static long double get_f64(const Cell* bv, const int i) {
    return ((double*)bv->bv->data)[i];
}


static void set_f64(Cell* bv, const int i, const long double val) {
    ((double*)bv->bv->data)[i] = (double)val;
}


static void append_f64(Cell* bv, const long double val) {
    if (bv->count == bv->bv->capacity) {
        bv->bv->capacity *= 2;
        bv->bv->data = GC_realloc(bv->bv->data, bv->bv->capacity * sizeof(double));
    }
    ((double*)bv->bv->data)[bv->count++] = (double)val;
}


static void repr_f64(const Cell* bv, str_buf_t *sb) {
    sb_append_fmt(sb, "#%s(", "f64");
    for (int i = 0; i < bv->count; i++) {
        sb_append_fmt(sb, "%.15g", ((double*)bv->bv->data)[i]);
        if (i != bv->count - 1) sb_append_char(sb, ' ');
    }
    sb_append_char(sb, ')');
}


DEFINE_BV_TYPE(u8,  uint8_t,  "%u")
DEFINE_BV_TYPE(s8,  int8_t,   "%d")
DEFINE_BV_TYPE(u16, uint16_t, "%u")
DEFINE_BV_TYPE(s16, int16_t,  "%d")
DEFINE_BV_TYPE(u32, uint32_t, "%u")
DEFINE_BV_TYPE(s32, int32_t,  "%d")
DEFINE_BV_TYPE(u64, uint64_t, "%u")
DEFINE_BV_TYPE(s64, int64_t,  "%d")

#define INVALID 255


const bv_int_ops_t BV_INT_OPS[] = {
    [BV_U8]  = { get_u8,  set_u8,  repr_u8,  append_u8  },
    [BV_S8]  = { get_s8,  set_s8,  repr_s8,  append_s8  },
    [BV_U16] = { get_u16, set_u16, repr_u16, append_u16 },
    [BV_S16] = { get_s16, set_s16, repr_s16, append_s16 },
    [BV_U32] = { get_u32, set_u32, repr_u32, append_u32 },
    [BV_S32] = { get_s32, set_s32, repr_s32, append_s32 },
    [BV_U64] = { get_u64, set_u64, repr_u64, append_u64 },
    [BV_S64] = { get_s64, set_s64, repr_s64, append_s64 },
};

const bv_fp_ops_t BV_FP_OPS[] = {
    [BV_F32] = { get_f32, set_f32, repr_f32, append_f32 },
    [BV_F64] = { get_f64, set_f64, repr_f64, append_f64 },
};


static bv_t get_type(const Cell* t_sym)
{
    bv_t type;
    if (t_sym == make_cell_symbol("u8")) { type = BV_U8; }
    else if (t_sym == make_cell_symbol("s8")) { type = BV_S8; }
    else if (t_sym == make_cell_symbol("u16")) { type = BV_U16; }
    else if (t_sym == make_cell_symbol("s16")) { type = BV_S16; }
    else if (t_sym == make_cell_symbol("u32")) { type = BV_U32; }
    else if (t_sym == make_cell_symbol("s32")) { type = BV_S32; }
    else if (t_sym == make_cell_symbol("u64")) { type = BV_U64; }
    else if (t_sym == make_cell_symbol("s64")) { type = BV_S64; }
    else if (t_sym == make_cell_symbol("f32")) { type = BV_F32; }
    else if (t_sym == make_cell_symbol("f64")) { type = BV_F64; }
    else { type = INVALID; }
    return type;
}


static char* get_type_string(const bv_t type)
{
    char* t_string;
    switch (type) {
        case BV_U8: {t_string = "u8"; break; }
        case BV_S8: {t_string = "s8"; break; }
        case BV_U16: {t_string = "u16"; break; }
        case BV_S16: {t_string = "s16"; break; }
        case BV_U32: {t_string = "u32"; break; }
        case BV_S32: {t_string = "s32"; break; }
        case BV_U64: {t_string = "u64"; break; }
        case BV_S64: {t_string = "s64"; break; }
        case BV_F32: {t_string = "f32"; break; }
        case BV_F64: {t_string = "f64"; break; }
        default: { t_string = "unknown"; break; }
    }
    return t_string;
}


/* Adds an integer value to a bytevector object. */
Cell* int_byte_check(const bv_t type, const int64_t value)
{
    if (type == BV_U64) {
        if (value < 0) {
            return make_cell_error(
                fmt_err("byte value '%ll' invalid for u64 bytevector", value),
                VALUE_ERR);
        }
        return nullptr;
    }

    int64_t min;
    int64_t max;
    switch (type) {
        case BV_U8: { min = 0; max = UINT8_MAX; break; }
        case BV_S8: { min = INT8_MIN; max = INT8_MAX; break;  }
        case BV_U16: { min = 0; max = UINT16_MAX; break; }
        case BV_S16: { min = INT16_MIN; max = INT16_MAX; break;  }
        case BV_U32: { min = 0; max = UINT32_MAX; break;}
        case BV_S32: { min = INT32_MIN; max = INT32_MAX; break; }
        case BV_S64: { min = INT64_MIN; max = INT64_MAX; break; }
        default: { min = 0; max = UINT8_MAX; break; }
    }

    if (value < min || value > max) {
        return make_cell_error(
            fmt_err("byte value '%" PRId64 "' invalid for %s bytevector", value, get_type_string(type)),
            VALUE_ERR);
    }
    return nullptr;
}


/* Adds a floating point value to a bytevector object. */
Cell* fp_byte_check(const bv_t type, const long double value)
{
    switch (type) {
        case BV_F32:
            if (isfinite(value) && fabsl(value) > FLT_MAX)
                return make_cell_error(fmt_err(
                    "bad value for f32 bytevector: '%ld'", value),
                    VALUE_ERR);

            break;

        case BV_F64:
            if (isfinite(value) && fabsl(value) > DBL_MAX)
                return make_cell_error(fmt_err(
                    "bad value for f64 bytevector: '%ld'", value),
                    VALUE_ERR);
            break;

        default:
            ;
    }
    return nullptr;
}


// /* Helper to get element width - you'll likely use this in many places */
size_t get_bv_width(const bv_t type)
{
    switch (type) {
        case BV_U8:  case BV_S8:  return sizeof(int8_t);
        case BV_U16: case BV_S16: return sizeof(int16_t);
        case BV_U32: case BV_S32: return sizeof(int32_t);
        case BV_F32: return sizeof(float);
        case BV_F64: return sizeof(double);
        default: return 0;
    }
}


/*------------------------------------------------------------*
 *     Byte vector constructors, selectors, and procedures    *
 * -----------------------------------------------------------*/


/* (bytevector byte ... )
 * (bytevector byte ... symbol)
 * Returns a newly allocated bytevector containing its arguments. If the optional symbol arg is passed (one of 'u8, 's8,
 * 'u16, 's16 etc...) then the bytevector will be initialized as that type. Default is u8. */
Cell* builtin_bytevector(const Lex* e, const Cell* a)
{
    (void)e;
    /* Return empty u8 bv if no args. */
    if (a->count == 0) {
        return make_cell_bytevector(BV_U8, 0);
    }

    /* See if there's a type argument. */
    int32_t num_bytes = a->count;
    bv_t type = BV_U8;
    if (a->cell[num_bytes - 1]->type == CELL_SYMBOL)
    {
        type = get_type(a->cell[num_bytes - 1]);
        if (type == INVALID) {
            return make_cell_error(
                fmt_err("bytevector: invalid bytevector type: %s",
                    a->cell[num_bytes - 1]->sym),
                TYPE_ERR);
        }
        /* If it's a legit bytevector type arg,
         * don't add it to the bytevector */
        num_bytes--;
    }

    Cell* bv = make_cell_bytevector(type, num_bytes);

    if (type == BV_F32 || type == BV_F64) {
        for (int i = 0; i < num_bytes; i++) {
            long double byte;
            if (a->cell[i]->type == CELL_INTEGER) {
                byte = a->cell[i]->integer_v;
            } else if (a->cell[i]->type == CELL_REAL) {
                byte = a->cell[i]->real_v;
            } else {
                return make_cell_error(
                    "bytevector: floating point bytevector args must be integers or reals",
                    VALUE_ERR);
            }
            Cell* err = fp_byte_check(type, byte);
            if (err) return err;
            BV_FP_OPS[bv->bv->type].append(bv, byte);
        }
        return bv;
    }

    for (int i = 0; i < num_bytes; i++) {
        if (a->cell[i]->type != CELL_INTEGER) {
            return make_cell_error(
                "bytevector: args must be integers",
                VALUE_ERR);
        }
        const int64_t byte = a->cell[i]->integer_v;
        Cell* err = int_byte_check(type, byte);
        if (err) return err;
        BV_INT_OPS[bv->bv->type].append(bv, byte);
    }
    return bv;
}


/* (bytevector-length bytevector)
 * Returns the length in bytes of bytevector as an exact integer. */
Cell* builtin_bytevector_length(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "bytevector-length");
    if (err) return err;
    err = check_arg_types(a, CELL_BYTEVECTOR, "bytevector-length");
    if (err) return err;

    return make_cell_integer(a->cell[0]->count);
}


/* (bytevector-ref bytevector k)
 * Returns the kth byte of bytevector. */
Cell* builtin_bytevector_ref(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "bytevector-ref");
    if (err) return err;
    if (a->cell[0]->type != CELL_BYTEVECTOR) {
        return make_cell_error(
            "bytevector-ref: arg 1 must be a bytevector",
            TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error(
            "bytevector-ref: arg 2 must be an integer",
            TYPE_ERR);
    }
    const Cell* bv = a->cell[0];
    const int i = (int)a->cell[1]->integer_v;
    if (i < 0) {
        return make_cell_error(
            "bytevector-ref: indices cannot be negative",
            VALUE_ERR);
    }

    if (i >= a->cell[0]->count) {
        return make_cell_error(
            "bytevector-ref: index out of bounds",
            INDEX_ERR);
    }

    if (bv->bv->type == BV_F32 || bv->bv->type == BV_F64) {
        return make_cell_real(BV_FP_OPS[bv->bv->type].get(bv, i));
    }
    return make_cell_integer(BV_INT_OPS[bv->bv->type].get(bv, i));
}


/* (bytevector-set! bytevector k byte)
 * It is an error if k is not a valid index of the bytevector.
 * This procedure stores byte in the kth position of bytevector. */
Cell* builtin_bytevector_set_bang(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 3, "bytevector-set!");
    if (err) return err;
    if (a->cell[0]->type != CELL_BYTEVECTOR) {
        return make_cell_error(
            "bytevector-set!: arg 1 must be a vector",
            TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error(
            "bytevector-set!: arg 2 must be an exact non-negative integer",
            TYPE_ERR);
    }

    const int idx = (int)a->cell[1]->integer_v;
    Cell* bv = a->cell[0];
    const uint8_t type = bv->bv->type;

    if (idx < 0 || idx >= bv->count) {
        return make_cell_error(
            "bytevector-set!: index out of range",
            INDEX_ERR);
    }

    if (type == BV_F32 || type == BV_F64) {
        long double byte = 0;
        if (a->cell[2]->type == CELL_REAL) {
            byte = a->cell[2]->real_v;

        }
        if (a->cell[2]->type == CELL_INTEGER) {
            byte = a->cell[2]->integer_v;
        }
        err = fp_byte_check(type, byte);
        if (err) return err;
        BV_FP_OPS[type].set(bv, idx, byte);
        return USP_Obj;
    }

    if (a->cell[2]->type != CELL_INTEGER) {
        return make_cell_error(
            "bytevector-set: byte arg must be an integer",
            TYPE_ERR);
    }
    const int64_t byte = (int)a->cell[2]->integer_v;
    err = int_byte_check(type, byte);
    if (err) return err;
    BV_INT_OPS[type].set(bv, idx, byte);
    return USP_Obj;
}


/* (make-bytevector k)
 * (make-bytevector k byte)
 * (make-bytevector k byte symbol)
 * The make-bytevector procedure returns a newly allocated bytevector of length k. If byte is given,
 * then all elements of the bytevector are initialized to byte, otherwise the contents of each
 * element are set to 0. The optional third symbol argument is one of 'u8 's8 'u16 's16 'u32 or 's32,
 * the default is a regular u8 bytevector.*/
Cell* builtin_make_bytevector(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3, "make-bytevector");
    if (err) return err;
    if (a->cell[0]->type != CELL_INTEGER) {
        return make_cell_error(
            "make-bytevector: arg 1 must be an integer",
            TYPE_ERR);
    }
    const long long n = a->cell[0]->integer_v;
    if (n < 0) {
        return make_cell_error(
            "make-bytevector: arg 1 must be non-negative",
            VALUE_ERR);
    }
    /* Check for bv type. */
    bv_t type;
    if (a->count == 3) {
        const Cell* t_sym = a->cell[2];
        if (t_sym->type != CELL_SYMBOL) {
            return make_cell_error(
                "make-bytevector: arg 3 must be a symbol",
                TYPE_ERR);
        }
        type = get_type(t_sym);
        if (type == INVALID) {
            return make_cell_error(
                "arg 3 must be one of 'u8, 's8, 'u16, 's16, 'u32, 's32', 'u64', or 's64' ",
                VALUE_ERR);
        }
    } else {
        type = BV_U8;
    }

    Cell *vec = make_cell_bytevector(type, n);

    /* Float vector path. */
    if (type == BV_F32 || type == BV_F64) {
        long double fill = 0;
        if (a->count > 1) {
            if (a->cell[1]->type == CELL_INTEGER) {
                fill = a->cell[1]->integer_v;
            } else if (a->cell[1]->type == CELL_REAL) {
                fill = a->cell[1]->real_v;
            } else {
                return make_cell_error(fmt_err(
                    "make-bytevector: Fill value must be integer or real",
                    get_type_string(type)), VALUE_ERR);
            }
        }
        err = fp_byte_check(type, fill);
        if (err) return err;
        for (int i = 0; i < n; i++) {
            BV_FP_OPS[type].append(vec, fill);
        }
        return vec;
    }

    /* Integer vector path. */
    int64_t fill = 0;
    if (a->count > 1) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error(
                "make-bytevector: Fill value must be an integer",
                TYPE_ERR);
        }
        fill = a->cell[1]->integer_v;
    }
    err = int_byte_check(type, fill);
    if (err) return err;

    for (int i = 0; i < n; i++) {
        BV_INT_OPS[vec->bv->type].append(vec, fill);
    }
    return vec;
}


/* (bytevector-copy bytevector)
 * (bytevector-copy bytevector start)
 * (bytevector-copy bytevector start end)
 * Returns a newly allocated bytevector containing the bytes in bytevector between start and end. */
Cell* builtin_bytevector_copy(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3, "bytevector-copy");
    if (err) return err;
    if (a->cell[0]->type != CELL_BYTEVECTOR) {
        return make_cell_error(
            "bytevector-copy: arg 1 must be a bytevector",
            TYPE_ERR);
    }

    int start = 0;
    int end = a->cell[0]->count;
    if (a->count == 2) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error(
                "bytevector-copy: start arg must be an integer",
                TYPE_ERR);
        }
        start = (int)a->cell[1]->integer_v;
    }

    const Cell* bv = a->cell[0];
    const bv_t type = bv->bv->type;
    if (a->count == 3) {
        if (a->cell[1]->type != CELL_INTEGER || a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error(
                "bytevector-copy: start/end args must be integers",
                TYPE_ERR);
        }
        start = (int)a->cell[1]->integer_v;
        end = (int)a->cell[2]->integer_v;
    }

    if (start < 0) {
        return make_cell_error(
            "bytevector-copy: start index must be non-negative",
            VALUE_ERR);
    }
    if (end > a->cell[0]->count) {
        return make_cell_error(
            "bytevector-copy: end index out of range",
            VALUE_ERR);
    }

    Cell* vec = make_cell_bytevector(type, end - start);

    /* No need to check type and range, we
     * already know they are good. */
    for (int i = start; i < end; i++) {
        if (type == BV_F32 || type == BV_F64) {
            BV_FP_OPS[bv->bv->type].append(vec, BV_FP_OPS[type].get(bv, i));
            continue;
        }
        BV_INT_OPS[vec->bv->type].append(vec, BV_INT_OPS[type].get(bv, i));
    }
    return vec;
}


/* (bytevector-copy! to at from)
 * (bytevector-copy! to at from start)
 * (bytevector-copy! to at from start end)
 * Copies the bytes of bytevector 'from' between start and end to bytevector 'to' , starting at 'at' . The order in which
 * bytes are copied is unspecified, except that if the source and destination overlap, copying takes place as if the
 * source is first copied into a temporary bytevector and then into the destination. */
Cell* builtin_bytevector_copy_bang(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 3, 5, "bytevector-copy!");
    if (err) return err;

    /* Validate arg types. */
    if (a->cell[0]->type != CELL_BYTEVECTOR)
        return make_cell_error(
            "bytevector-copy!: arg 1 must be a bytevector (to)",
            TYPE_ERR);
    if (a->cell[1]->type != CELL_INTEGER)
        return make_cell_error(
            "bytevector-copy!: arg 2 must be an integer (at)",
            TYPE_ERR);
    if (a->cell[2]->type != CELL_BYTEVECTOR)
        return make_cell_error(
            "bytevector-copy!: arg 3 must be a bytevector (from)",
            TYPE_ERR);

    /* Get 'to' bytevector and 'at' index. */
    Cell* to_bv = a->cell[0];
    const int32_t to_bv_len = to_bv->count;
    const int32_t to_start_idx = (int32_t)a->cell[1]->integer_v;

    /* Get 'from' bytevector and 'start'/'end' indices. */
    const Cell* from_bv = a->cell[2];
    const int32_t from_bv_len = from_bv->count;

    int32_t from_start_idx = 0;
    int32_t from_end_idx = from_bv_len; /* Default to char length. */

    /* Ensure bytevectors are compatible. */
    const uint8_t from_type = from_bv->bv->type;
    const uint8_t to_type = to_bv->bv->type;
    if (from_type != to_type)
    {
        return make_cell_error(
            fmt_err("Incompatible bytevector types: '%s' and '%s'",
            get_type_string(from_type), get_type_string(to_type)), TYPE_ERR);
    }

    if (a->count >= 4) {
        if (a->cell[3]->type != CELL_INTEGER)
            return make_cell_error(
                "bytevector-copy!: arg 4 must be an integer (start)",
                TYPE_ERR);
        from_start_idx = (int32_t)a->cell[3]->integer_v;
    }
    if (a->count == 5) {
        if (a->cell[4]->type != CELL_INTEGER)
            return make_cell_error(
                "bytevector-copy!: arg 5 must be an integer (end)",
                TYPE_ERR);
        from_end_idx = (int32_t)a->cell[4]->integer_v;
    }

    /* R7RS Index Validation. */
    if (to_start_idx < 0 || to_start_idx > to_bv_len)
        return make_cell_error(
            "bytevector-copy!: 'at' index is out of bounds for 'to' bytevector",
            INDEX_ERR);
    if (from_start_idx < 0 || from_start_idx > from_bv_len)
        return make_cell_error(
            "bytevector-copy!: 'start' index is out of bounds for 'from' bytevector",
            INDEX_ERR);
    if (from_end_idx < 0 || from_end_idx > from_bv_len)
        return make_cell_error(
            "bytevector-copy!: 'end' index is out of bounds for 'from' bytevector",
            INDEX_ERR);
    if (from_start_idx > from_end_idx)
        return make_cell_error(
            "bytevector-copy!: 'start' index cannot be greater than 'end' index",
            INDEX_ERR);

    const int32_t num_bytes = from_end_idx - from_start_idx;
    for (int i = to_start_idx; i < to_start_idx + num_bytes; i++) {
        if (from_type == BV_F32 || from_type == BV_F64) {
            BV_FP_OPS[to_type].set(to_bv, i, BV_FP_OPS[from_type].get(from_bv, from_start_idx));
            from_start_idx++;
            continue;
        }
        BV_INT_OPS[to_type].set(to_bv, i, BV_INT_OPS[from_type].get(from_bv, from_start_idx));
        from_start_idx++;
    }
    return USP_Obj;
}


/* (bytevector-append bytevector ...)
* Returns a newly allocated bytevector whose elements are
the concatenation of the elements in the given bytevectors. */
Cell* builtin_bytevector_append(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_BYTEVECTOR, "bytevector-append");
    if (err) return err;

    if (a->count == 0) {
        return make_cell_bytevector(BV_U8, 0);
    }

    const bv_t type = a->cell[0]->bv->type;
    Cell* vec = make_cell_bytevector(type, 8);
    for (int i = 0; i < a->count; i++) {
        const Cell* bv = a->cell[i];
        if (bv->bv->type != type) {
            return make_cell_error(
                "bytevector-append: cannot append different type-sized bytevector",
                VALUE_ERR);
        }
        for (int j = 0; j < bv->count; j++) {
            if (type == BV_F32 || type == BV_F64) {
                const long double byte = BV_FP_OPS[type].get(bv, j);
                err = fp_byte_check(type, byte);
                if (err) return err;
                BV_FP_OPS[vec->bv->type].append(vec, byte);
                continue;
            }
            const int64_t byte =  BV_INT_OPS[type].get(bv, j);
            err = int_byte_check(type, byte);
            if (err) return err;
            BV_INT_OPS[vec->bv->type].append(vec, byte);
        }
    }
    return vec;
}


/* (utf8->string bytevector)
 * (utf8->string bytevector start)
 * (utf8->string bytevector start end)
 * The utf8->string procedure decodes the bytes of a bytevector between start and end and returns the corresponding
 * string. */
Cell* builtin_utf8_string(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3, "utf8->string");
    if (err) return err;

    const Cell* bv = a->cell[0];
    if (bv->type != CELL_BYTEVECTOR || bv->bv->type != BV_U8) {
        return make_cell_error(
            "utf8->string: arg 1 must be a u8 bytevector",
            TYPE_ERR);
    }

    /* start and end are byte offsets into the bytevector. */
    int start = 0;
    int end = bv->count;

    if (a->count > 1) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error(
                "utf8->string: arg 2 must be a non-negative integer",
                TYPE_ERR);
        }
        if (a->cell[1]->integer_v < 0) {
            return make_cell_error(
                "utf8->string: arg 2 must be non-negative",
                VALUE_ERR);
        }
        start = (int)a->cell[1]->integer_v;
    }
    if (a->count == 3) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error(
                "utf8->string: arg 3 must be a non-negative integer",
                TYPE_ERR);
        }
        if (a->cell[2]->integer_v < 0) {
            return make_cell_error(
                "utf8->string: arg 3 must be non-negative",
                VALUE_ERR);
        }
        end = (int)a->cell[2]->integer_v;
    }

    if (start > bv->count || end > bv->count) {
        return make_cell_error(
            "utf8->string: index out of range",
            INDEX_ERR);
    }
    if (start > end) {
        return make_cell_error(
            "utf8->string: start index cannot be greater than end index",
            INDEX_ERR);
    }

    const int byte_count = end - start;

    /* Validate that the byte range begins on a valid UTF-8 codepoint boundary.
     * A continuation byte (0x80-0xBF) is not a valid start byte. */
    if (byte_count > 0) {
        const uint8_t lead = (uint8_t)BV_INT_OPS[BV_U8].get(bv, start);
        if ((lead & 0xC0) == 0x80) {
            return make_cell_error(
                "utf8->string: start is not on a codepoint boundary",
                VALUE_ERR);
        }
    }

    /* Copy the raw UTF-8 bytes into a buffer and let make_cell_string
     * handle character counting and the ASCII flag. */
    char* the_str = GC_MALLOC_ATOMIC(byte_count + 1);
    for (int i = 0; i < byte_count; i++) {
        the_str[i] = (char)BV_INT_OPS[BV_U8].get(bv, start + i);
    }
    the_str[byte_count] = '\0';

    return make_cell_string(the_str);
}


/* (string->utf8 string)
 * (string->utf8 string start)
 * (string->utf8 string start end)
 *  The string->utf8 procedure encodes the characters of a string between start and end and returns the corresponding
 *  bytevector. */
Cell* builtin_string_utf8(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3, "string->utf8");
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "string->utf8: arg 1 must be a string",
            TYPE_ERR);
    }

    const Cell* str_cell = a->cell[0];
    int start_char = 0;
    int end_char = str_cell->char_count;

    if (a->count > 1) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error(
                "string->utf8: arg 2 must be a non-negative integer",
                TYPE_ERR);
        }
        if (a->cell[1]->integer_v < 0) {
            return make_cell_error(
                "string->utf8: arg 2 must be non-negative",
                VALUE_ERR);
        }
        start_char = (int)a->cell[1]->integer_v;
    }
    if (a->count == 3) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error(
                "string->utf8: arg 3 must be a non-negative integer",
                TYPE_ERR);
        }
        if (a->cell[2]->integer_v < 0) {
            return make_cell_error(
                "string->utf8: arg 3 must be non-negative",
                VALUE_ERR);
        }
        end_char = (int)a->cell[2]->integer_v;
    }

    if (start_char > str_cell->char_count || end_char > str_cell->char_count) {
        return make_cell_error(
            "string->utf8: index out of range",
            INDEX_ERR);
    }
    if (start_char > end_char) {
        return make_cell_error(
            "string->utf8: start index cannot be greater than end index",
            INDEX_ERR);
    }

    /* Convert character indices to byte offsets.
     * get_utf8_byte_offset fast-paths pure ASCII strings automatically. */
    const int32_t start_byte = get_utf8_byte_offset(str_cell, start_char);
    const int32_t end_byte   = get_utf8_byte_offset(str_cell, end_char);

    const char* the_s = str_cell->str;
    Cell* bv = make_cell_bytevector(BV_U8, end_byte - start_byte);
    for (int32_t i = start_byte; i < end_byte; i++) {
        err = int_byte_check(BV_U8, (uint8_t)the_s[i]);
        if (err) return err;
        BV_INT_OPS[bv->bv->type].append(bv, (uint8_t)the_s[i]);
    }
    return bv;
}
