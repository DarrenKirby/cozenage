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

/* 'not' -> VAL_BOOL - returns #t if obj is false, and returns #f otherwise */
Cell* builtin_not(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err;
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const int is_false = (a->cell[0]->type == VAL_BOOL && a->cell[0]->b_val == 0);
    return make_val_bool(is_false);
}

/* 'boolean' -> VAL_BOOL - converts any value to a strict boolean */
Cell* builtin_boolean(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    const int result = (a->cell[0]->type == VAL_BOOL)
                 ? a->cell[0]->b_val
                 : 1; /* everything except #f is true */
    return make_val_bool(result);
}

/* TODO: 'boolean=?' */

/* FIXME: 'and' and 'or' are syntax/special forms. They need to be pulled
 * out of here (and out of the env) and placed in special_forms.c */

/* 'and' -> VAL_BOOL|ANY - if any expression evaluates to #f, then #f is
 * returned. Any remaining expressions are not evaluated. If all the expressions
 * evaluate to true values, the values of the last expression are returned.
 * If there are no expressions, then #t is returned.*/
Cell* builtin_and(const Lex* e, const Cell* a) {
    (void)e;
    if (a->count == 0) {
        return make_val_bool(1);
    }
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type == VAL_BOOL && a->cell[i]->b_val == 0) {
            /* first #f encountered → return a copy of it */
            return cell_copy(a->cell[i]);
        }
    }
    /* all truthy → return copy of last element */
    return cell_copy(a->cell[a->count - 1]);
}

/* 'or' -> VAL_BOOL|ANY - the value of the first expression that evaluates
 * to true is returned. Any remaining expressions are not evaluated. If all
 * expressions evaluate to #f or if there are no expressions, #f is returned */
Cell* builtin_or(const Lex* e, const Cell* a) {
    (void)e;
    if (a->count == 0) {
        return make_val_bool(0);
    }
    for (int i = 0; i < a->count; i++) {
        if (!(a->cell[i]->type == VAL_BOOL && a->cell[i]->b_val == 0)) {
            /* first truthy value → return a copy */
            return cell_copy(a->cell[i]);
        }
    }
    /* all false → return copy of last element (#f) */
    return cell_copy(a->cell[a->count - 1]);
}
