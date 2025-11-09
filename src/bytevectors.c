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
#include "types.h"


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

    Cell *vec = make_cell_bytevector();
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type != CELL_INTEGER || a->cell[i]->integer_v < 0 || a->cell[i]->integer_v > 255) {
            return make_cell_error(
                "bytevector: args must be integers 0 to 255 inclusive",
                VALUE_ERR);
        }
        cell_add(vec, a->cell[i]);
    }
    return vec;
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
    const int i = (int)a->cell[1]->integer_v;

    if (i >= a->cell[0]->count) {
        return make_cell_error(
            "bytevector-ref: index out of bounds",
            INDEX_ERR);
    }
    return a->cell[0]->cell[i];
}

/* (bytevector-u8-set! bytevector k obj)
 * It is an error if k is not a valid index of vector. This procedure stores obj in the kth position
 * of vector */
Cell* builtin_bytevector_set_bang(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_BYTEVECTOR) {
        return make_cell_error(
            "vector->set!: arg must be a vector",
            TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error(
            "vector->set!: arg must be an integer",
            TYPE_ERR);
    }

    const long long idx = a->cell[1]->integer_v;
    const Cell* vec = a->cell[0];
    Cell* obj = a->cell[2];

    if (idx < 0 || idx >= a->cell[0]->count) {
        return make_cell_error(
            "vector->set!: index out of range",
            INDEX_ERR);
    }

    vec->cell[idx] = obj;
    return nullptr;
}


/* (make-bytevector k)
 * (make-bytevector k byte)
 * The make-bytevector procedure returns a newly allocated bytevector of length k. If byte is given,
 * then all elements of the bytevector are initialized to byte, otherwise the contents of each
 * element are set to 0. */
Cell* builtin_make_bytevector(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2);
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
    Cell* fill;
    if (a->count == 2) {
        fill = a->cell[1];
        if (fill->integer_v < 0 || fill->integer_v > 255) {
            return make_cell_error(
                "make-bytevector: arg 2 must be between 0 and 255 inclusive",
                VALUE_ERR);
        }
    } else {
        fill = make_cell_integer(0);
    }
    Cell *vec = make_cell_bytevector();
    for (int i = 0; i < n; i++) {
        cell_add(vec, fill);
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
    if (a->count == 3) {
        start = (int)a->cell[1]->integer_v;
        end = (int)a->cell[2]->integer_v;
    }
    Cell* vec = make_cell_vector();
    for (int i = start; i < end; i++) {
        cell_add(vec, a->cell[0]->cell[i]);
    }
    return vec;
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
        return make_cell_bytevector();
    }

    Cell* result = make_cell_bytevector();
    for (int i = 0; i < a->count; i++) {
        const Cell* bv = a->cell[i];
        for (int j = 0; j < bv->count; j++) {
            cell_add(result, bv->cell[j]);
        }
    }
    return result;
}
