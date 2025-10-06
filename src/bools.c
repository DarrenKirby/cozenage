/*
 * 'bools.c'
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

#include "bools.h"


/* ---------------------------------------*
 *     Boolean and logical procedures     *
 * ---------------------------------------*/

/* 'not' -> CELL_BOOLEAN - returns #t if obj is false, and returns #f otherwise */
Cell* builtin_not(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err;
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const int is_false = a->cell[0]->type == CELL_BOOLEAN && a->cell[0]->boolean_v == 0;
    return make_cell_boolean(is_false);
}

/* 'boolean' -> CELL_BOOLEAN - converts any value to a strict boolean */
Cell* builtin_boolean(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    const int result = a->cell[0]->type == CELL_BOOLEAN
                 ? a->cell[0]->boolean_v
                 : 1; /* everything except #f is true */
    return make_cell_boolean(result);
}

/* TODO: 'boolean=?' */
