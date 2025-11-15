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
#include "types.h"


/* ---------------------------------------*
 *     Boolean and logical procedures     *
 * ---------------------------------------*/

/* (not obj )
 * The not procedure returns #t if obj is false, and returns #f otherwise. */
Cell* builtin_not(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err;
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    if (a->cell[0]->type == CELL_BOOLEAN && a->cell[0]->boolean_v == 0) {
        return True_Obj;
    }
    return False_Obj;
}


/* (boolean=? boolean1 boolean2 boolean3 ... )
 * Returns #t if all the arguments are booleans and all are #t or all are #f. */
Cell* builtin_boolean(const Lex* e, const Cell* a)
{
    (void)e;
    /* Return #t if no args */
    if (a->count == 0) {
        return True_Obj;
    }
    /* If not all args are CELL_BOOLEAN, return #f */
    if (check_arg_types(a, CELL_BOOLEAN)) {
        return False_Obj;
    }
    /* Return #t for single boolean argument */
    if (a->count == 1) {
        return True_Obj;
    }
    /* Value of first arg */
    const bool v = a->cell[0]->boolean_v;
    /* Compare with subsequent values */
    for (int i = 1; i < a->count; i++) {
        if (v != a->cell[i]->boolean_v) {
            return False_Obj;
        }
    }
    return True_Obj;
}
