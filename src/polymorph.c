/*
 * 'polymorph.c'
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
    }
    return result;
}

static Cell* string_reverse(const Cell* v)
{
    (void)v;
    return USP_Obj;
}

static Cell* list_idx(const Lex* e, const Cell* a)
{
    const Cell* v = builtin_list_to_vector(e, make_sexpr_len1(a->cell[0]));
    const int32_t start = a->cell[1]->integer_v;
    int32_t stop = v->count;
    int32_t step = 1;
    if (a->count > 2) {
        stop = a->cell[2]->integer_v;
    }
    if (a->count > 3) {
        step = a->cell[3]->integer_v;
    }

    Cell* result = make_cell_vector();
    for (int32_t i = start; i < stop; i+=step) {
        cell_add(result, v->cell[i]);
    }
    return builtin_vector_to_list(e, make_sexpr_len1(result));
}

static Cell* vector_idx(const Cell* a)
{
    const Cell* v = a->cell[0];
    const int32_t start = a->cell[1]->integer_v;
    int32_t stop = v->count;
    int32_t step = 1;
    if (a->count > 2) {
        stop = a->cell[2]->integer_v;
    }
    if (a->count > 3) {
        step = a->cell[3]->integer_v;
    }

    Cell* result = make_cell_vector();
    for (int32_t i = start; i < stop; i+=step) {
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
            fmt_err("len: no length for non-compound type %s",
                cell_type_name(a->cell[0]->type)), TYPE_ERR);
    }
}

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
        return builtin_bytevector_ref(e, a);
    case CELL_STRING:
        return builtin_string_ref(e, a);
    default:
        return make_cell_error(
        fmt_err("idx: cannot subscript non-compound type %s",
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
            fmt_err("rev: cannot reverse non-compound type %s",
                cell_type_name(a->cell[0]->type)),
                TYPE_ERR);
    }
}