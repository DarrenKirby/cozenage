/*
 * 'eval_lib.c'
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

#include "eval_lib.h"
#include "eval.h"


Cell* builtin_eval(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;
    Cell* args = NULL;
    /* Convert list to s-expr if we are handed a quote */
    if (a->cell[0]->type == VAL_PAIR) {
        args = make_sexpr_from_list(a->cell[0]);
        for (int i = 0; i < args->count; i++ ) {
            if (args->cell[i]->type == VAL_PAIR && args->cell[i]->len != -1) {
                Cell* tmp = cell_copy(args->cell[i]);
                args->cell[i] = make_sexpr_from_list(tmp);
            }
        }
        /* Otherwise just send straight to eval */
    } else {
        args = a->cell[0];
    }
    return coz_eval(e, args);
}

void lex_add_eval_lib(Lex* e) {
    lex_add_builtin(e, "eval", builtin_eval);
}
