/*
 * 'bytevectors.c'
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

#include "bytevectors.h"
#include "strings.h"
#include "types.h"
#include <string.h>
#include <gc/gc.h>
#include <inttypes.h>


DEFINE_BV_TYPE(u8,  uint8_t,  "%u")
DEFINE_BV_TYPE(s8,  int8_t,   "%d")
DEFINE_BV_TYPE(u16, uint16_t, "%u")
DEFINE_BV_TYPE(s16, int16_t,  "%d")
DEFINE_BV_TYPE(u32, uint32_t, "%u")
DEFINE_BV_TYPE(s32, int32_t,  "%d")


const bv_ops_t BV_OPS[] = {
    [BV_U8]  = { get_u8, set_u8, repr_u8, append_u8,  sizeof(uint8_t)  },
    [BV_S8]  = { get_s8, set_s8, repr_s8, append_s8,  sizeof(int8_t)   },
    [BV_U16] = { get_u16, set_u16, repr_u16, append_u16, sizeof(uint16_t) },
    [BV_S16] = { get_s16, set_s16, repr_s16, append_s16, sizeof(int16_t)  },
    [BV_U32] = { get_u32, set_u32, repr_u32, append_u32, sizeof(uint32_t) },
    [BV_S32] = { get_s32, set_s32, repr_s32, append_s32, sizeof(int32_t)  },
};

static Cell* byte_fits(const bv_t type, const int64_t byte) {
    int64_t min;
    int64_t max;
    char* t_s;
    switch (type) {
    case BV_U8: { min = 0; max = UINT8_MAX; t_s = "u8"; break; }
    case BV_S8: { min = INT8_MIN; max = INT8_MAX; t_s = "s8"; break;  }
    case BV_U16: { min = 0; max = UINT16_MAX; t_s = "u16"; break; }
    case BV_S16: { min = INT16_MIN; max = INT16_MAX; t_s = "s16"; break;  }
    case BV_U32: { min = 0; max = UINT32_MAX; t_s = "u32"; break;}
    case BV_S32: { min = INT32_MIN; max = INT32_MAX; t_s = "s32"; break; }
    default: { min = 0; max = UINT8_MAX; t_s = "u8"; break; }
    }

    if (byte < min || byte > max) {
        char buf[256];
        snprintf(buf, sizeof(buf), "byte value %" PRId64 " invalid for %s bytevector", byte, t_s);
        return make_cell_error(buf, VALUE_ERR);
    }
    return True_Obj;
}

static bv_t get_type(const Cell* t_sym)
{
    bv_t type;
    if (t_sym == make_cell_symbol("u8")) { type = BV_U8; }
    else if (t_sym == make_cell_symbol("s8")) { type = BV_S8; }
    else if (t_sym == make_cell_symbol("u16")) { type = BV_U16; }
    else if (t_sym == make_cell_symbol("s16")) { type = BV_S16; }
    else if (t_sym == make_cell_symbol("u32")) { type = BV_U32; }
    else if (t_sym == make_cell_symbol("s32")) { type = BV_S32; }
    else { type = 255; }
    return type;
}

/*------------------------------------------------------------*
 *     Byte vector constructors, selectors, and procedures    *
 * -----------------------------------------------------------*/


/* (bytevector byte ... )
 * Returns a newly allocated bytevector containing its arguments. */
Cell* builtin_bytevector(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;

    /* See if there's a type argument. */
    int32_t num_bytes = a->count;
    bv_t type = BV_U8;
    if (a->cell[num_bytes - 1]->type == CELL_SYMBOL)
    {
        type = get_type(a->cell[num_bytes - 1]);
        if (type == 255) {
            /* u8 by default. */
            type = BV_U8;
        }
        /* If it's a legit bytevector type arg,
         * don't add it to the bytevector */
        num_bytes--;
    }

    Cell* bv = make_cell_bytevector(type);
    for (int i = 0; i < num_bytes; i++) {
        if (a->cell[i]->type != CELL_INTEGER) {
            return make_cell_error(
                "bytevector: args must be integers",
                VALUE_ERR);
        }
        const int64_t byte = a->cell[i]->integer_v;
        Cell* check_if = byte_fits(type, byte);
        if (check_if->type == CELL_ERROR) { return check_if; }
        byte_add(bv, byte);
    }
    return bv;
}


/* (bytevector-length bytevector)
 * Returns the length in bytes of bytevector as an exact integer*/
Cell* builtin_bytevector_length(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_BYTEVECTOR);
    if (err) return err;

    return make_cell_integer(a->cell[0]->count);
}


/* (bytevector-u8-ref bytevector k)
 * Returns the kth byte of bytevector. */
Cell* builtin_bytevector_ref(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
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

    if (i >= a->cell[0]->count) {
        return make_cell_error(
            "bytevector-ref: index out of bounds",
            INDEX_ERR);
    }
    return make_cell_integer(BV_OPS[bv->bv->type].get(bv, i));
}


/* (bytevector-set! bytevector k byte)
 * It is an error if k is not a valid index of the bytevector.
 * This procedure stores byte in the kth position of bytevector */
Cell* builtin_bytevector_set_bang(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_BYTEVECTOR) {
        return make_cell_error(
            "vector->set!: arg 1 must be a vector",
            TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error(
            "vector->set!: arg 2 must be an exact non-negative integer",
            TYPE_ERR);
    }

    const int idx = (int)a->cell[1]->integer_v;
    Cell* bv = a->cell[0];
    const uint8_t type = bv->bv->type;
    const int byte = (int)a->cell[2]->integer_v;
    /* Check the range. */
    Cell* check_if = byte_fits(type, byte);
    if (check_if->type == CELL_ERROR) {
        return check_if;
    }

    if (idx < 0 || idx >= bv->count) {
        return make_cell_error(
            "vector->set!: index out of range",
            INDEX_ERR);
    }

    BV_OPS[type].set(bv, idx, byte);
    return nullptr;
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
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_INTEGER) {
        return make_cell_error(
            "make-bytevector: arg 1 must be an integer",
            TYPE_ERR);
    }
    const long long n = a->cell[0]->integer_v;
    if (n < 1) {
        return make_cell_error(
            "make-bytevector: arg 1 must be non-negative",
            VALUE_ERR);
    }
    /* Check for bv type */
    bv_t type;
    if (a->count == 3) {
        const Cell* t_sym = a->cell[2];
        if (t_sym->type != CELL_SYMBOL) {
            return make_cell_error(
                "make-bytevector: arg 3 must be a symbol",
                TYPE_ERR);
        }
        type = get_type(t_sym);
        if (type == 255) {
            return make_cell_error(
                "arg 3 must be one of 'u8, 's8, 'u16, 's16, 'u32, or 's32 ",
                VALUE_ERR);
        }
    } else {
        type = BV_U8;
    }

    int64_t fill;
    if (a->count > 1) {
        fill = a->cell[1]->integer_v;
        /* Check the range. */
        Cell* check_if = byte_fits(type, fill);
        if (check_if->type == CELL_ERROR) {
            return check_if;
        }
    } else {
        fill = 0;
    }
    Cell *vec = make_cell_bytevector(type);
    for (int i = 0; i < n; i++) {
        byte_add(vec, fill);
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
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_BYTEVECTOR) {
        return make_cell_error(
            "bytevector->copy: arg 1 must be a bytevector",
            TYPE_ERR);
    }

    int start = 0;
    int end = a->cell[0]->count;
    if (a->count == 2) {
        start = (int)a->cell[1]->integer_v;
    }

    const Cell* bv = a->cell[0];
    const bv_t type = bv->bv->type;
    if (a->count == 3) {
        start = (int)a->cell[1]->integer_v;
        end = (int)a->cell[2]->integer_v;
    }

    Cell* vec = make_cell_bytevector(type);
    for (int i = start; i < end; i++) {
        const int64_t byte = BV_OPS[type].get(bv, i);
        byte_add(vec, byte);
    }
    return vec;
}

/* TODO - finish this*/
Cell* builtin_bytevector_copy_bang(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    return nullptr;
}

/* (bytevector-append bytevector ...)
* Returns a newly allocated bytevector whose elements are
the concatenation of the elements in the given bytevectors. */
Cell* builtin_bytevector_append(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_BYTEVECTOR);
    if (err) return err;

    if (a->count == 0) {
        return make_cell_bytevector(BV_U8);
    }

    const bv_t type = a->cell[0]->bv->type;
    Cell* result = make_cell_bytevector(type);
    for (int i = 0; i < a->count; i++) {
        const Cell* bv = a->cell[i];
        if (bv->bv->type != type) {
            return make_cell_error(
                "bytevector->append: cannot append different bytevector types",
                VALUE_ERR);
        }
        for (int j = 0; j < bv->count; j++) {
            const int64_t byte =  BV_OPS[type].get(bv, j);
            byte_add(result, byte);
        }
    }
    return result;
}

Cell* builtin_utf8_string(const Lex* e, const Cell* a)
{
    (void)e;
    const Cell* bv = a->cell[0];
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (bv->type != CELL_BYTEVECTOR || bv->bv->type != BV_U8) {
        return make_cell_error(
            "string->utf8: arg 1 must be a u8 bytevector",
            TYPE_ERR);
    }

    int start = 0;
    int end = a->cell[0]->count;
    if (a->count > 1) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error(
                "string->utf8: arg 2 must be an exact positive integer",
                TYPE_ERR);
        }
        if (a->cell[1]->integer_v < 0) {
            return make_cell_error(
                "string->utf8: arg 2 must be non-negative",
                VALUE_ERR);
        }
        start = (int)a->cell[1]->integer_v;
    }
    if (a->count == 3) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error(
                "string->utf8: arg 3 must be an exact positive integer",
                TYPE_ERR);
        }
        if (a->cell[2]->integer_v < 0) {
            return make_cell_error(
                "string->utf8: arg 3 must be non-negative",
                VALUE_ERR);
        }
        end = (int)a->cell[2]->integer_v;
    }

    char* the_str = GC_MALLOC_ATOMIC(bv->count + 1);

    int j = 0;
    for (int i = start; i < end; i++) {
        the_str[j] = (char)BV_OPS[bv->bv->type].get(bv, i);
        j++;
    }

    the_str[j] = '\0';
    return make_cell_string(the_str);
}

Cell* builtin_string_utf8(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "string->utf8: arg 1 must be a string",
            TYPE_ERR);
    }

    int start = 0;
    size_t end = strlen(a->cell[0]->str);
    if (a->count > 1) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error(
                "string->utf8: arg 2 must be an exact positive integer",
                TYPE_ERR);
        }
        if (a->cell[1]->integer_v < 0) {
            return make_cell_error(
                "string->utf8: arg 2 must be non-negative",
                VALUE_ERR);
        }
        start = (int)a->cell[1]->integer_v;
    }
    if (a->count == 3) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error(
                "string->utf8: arg 3 must be an exact positive integer",
                TYPE_ERR);
        }
        if (a->cell[2]->integer_v < 0) {
            return make_cell_error(
                "string->utf8: arg 3 must be non-negative",
                VALUE_ERR);
        }
        end = (int)a->cell[2]->integer_v;
    }

    const char* the_s = a->cell[0]->str;
    Cell* bv = make_cell_bytevector(BV_U8);
    for (size_t i = start; i < end; i++)
    {
        const uint8_t the_char = (int)the_s[i];
        byte_add(bv, the_char);
    }
    return bv;
}
