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


/*------------------------------------------------------------*
 *     Byte vector constructors, selectors, and procedures    *
 * -----------------------------------------------------------*/

Cell* builtin_bytevector(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;

    Cell *vec = make_val_bytevec();
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type != VAL_INT || a->cell[i]->i_val < 0 || a->cell[i]->i_val > 255) {
            return make_val_err("bytevector: args must be integers 0 to 255 inclusive", VALUE_ERR);
        }
        cell_add(vec, a->cell[i]);
    }
    return vec;
}

Cell* builtin_bytevector_length(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, VAL_BYTEVEC);
    if (err) return err;

    return make_val_int(a->cell[0]->count);
}

Cell* builtin_bytevector_ref(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    if (a->cell[0]->type != VAL_BYTEVEC) {
        return make_val_err("bytevector-ref: arg 1 must be a vector", TYPE_ERR);
    }
    if (a->cell[1]->type != VAL_INT) {
        return make_val_err("bytevector-ref: arg 2 must be an integer", TYPE_ERR);
    }
    const int i = (int)a->cell[1]->i_val;

    if (i >= a->cell[0]->count) {
        return make_val_err("bytevector-ref: index out of bounds", INDEX_ERR);
    }
    return a->cell[0]->cell[i];
}

Cell* builtin_make_bytevector(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2);
    if (err) return err;
    if (a->cell[0]->type != VAL_INT) {
        return make_val_err("make-bytevector: arg 1 must be an integer", TYPE_ERR);
    }
    const long long n = a->cell[0]->i_val;
    if (n < 1) {
        return make_val_err("make-bytevector: arg 1 must be non-negative", VALUE_ERR);
    }
    Cell* fill;
    if (a->count == 2) {
        fill = a->cell[1];
        if (fill->i_val < 0 || fill->i_val > 255) {
            return make_val_err("make-bytevector: arg 2 must be between 0 and 255 inclusive", VALUE_ERR);
        }
    } else {
        fill = make_val_int(0);
    }
    Cell *vec = make_val_bytevec();
    for (int i = 0; i < n; i++) {
        cell_add(vec, fill);
    }
    return vec;
}

Cell* builtin_bytevector_copy(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != VAL_BYTEVEC) {
        return make_val_err("bytevector->copy: arg 1 must be a vector", TYPE_ERR);
    }
    int start = 0;
    int end = a->cell[0]->count;

    if (a->count == 2) {
        start = (int)a->cell[1]->i_val;
    }
    if (a->count == 3) {
        start = (int)a->cell[1]->i_val;
        end = (int)a->cell[2]->i_val;
    }
    Cell* vec = make_val_vect();
    for (int i = start; i < end; i++) {
        cell_add(vec, a->cell[0]->cell[i]);
    }
    return vec;
}
