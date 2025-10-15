/*
 * 'control_features.c'
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

#include "control_features.h"
#include "types.h"
#include "eval.h"
#include "pairs.h"


/*-------------------------------------------------------*
 *    Control features and list iteration procedures     *
 * ------------------------------------------------------*/

Cell* builtin_apply(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_PROC) {
        return make_cell_error("apply: arg 1 must be a procedure", ARITY_ERR);
    }
    Cell* final_sexpr = make_cell_sexpr();
    /* Add the proc */
    cell_add(final_sexpr, a->cell[0]);
    /* Collect individual args, if any */
    const int last_arg_index = a->count - 1;
    for (int i = 1; i < last_arg_index; i++) {
        cell_add(final_sexpr, a->cell[i]);
    }
    const Cell* final_list = a->cell[last_arg_index];
    /* Ensure last arg is a list */
    if (final_list->type != CELL_PAIR || final_list->len == -1) {
        return make_cell_error("apply: last arg must be a proper list", TYPE_ERR);
    }
    const Cell* current_item = final_list;
    while (current_item->type != CELL_NIL) {
        cell_add(final_sexpr, current_item->car);
        current_item = current_item->cdr;
    }
    /* Give the s-expr a gentle kiss on the forehead,
     * and make it a CELL_TRAMPOLINE... */
    final_sexpr->type = CELL_TRAMPOLINE;
    return final_sexpr;
}

Cell* builtin_map(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_PROC) {
        return make_cell_error("map: arg 1 must be a procedure", TYPE_ERR);
    }
    int shortest_list_length = INT32_MAX;
    for (int i = 1; i < a->count; i++) {
        /* If list arg is empty, return empty list */
        if (a->cell[i]->type == CELL_NIL) {
            return make_cell_nil();
        }
        char buf[100];
        if (a->cell[i]->type != CELL_PAIR || a->cell[i]->len == -1) {
            snprintf(buf, 100, "map: arg %d must be a proper list", i+1);
            return make_cell_error(buf, TYPE_ERR);
        }
        if (a->cell[i]->len < shortest_list_length) {
            shortest_list_length = a->cell[i]->len;
        }
    }

    const int shortest_len = shortest_list_length;
    const int num_lists = a->count - 1;
    Cell* proc = a->cell[0];

    Cell* final_result = make_cell_nil();

    for (int i = 0; i < shortest_len; i++) {
        /* Build a (reversed) list of the i-th arguments */
        Cell* arg_list = make_cell_nil();
        for (int j = 0; j < num_lists; j++) {
            const Cell* current_list = a->cell[j + 1];
            Cell* nth_item = list_get_nth_cell_ptr(current_list, i);
            arg_list = make_cell_pair(nth_item, arg_list);
            arg_list->len = j + 1;
        }

        /* Correct the argument order */
        Cell* reversed_arg_list = builtin_list_reverse(e, make_sexpr_len1(arg_list));

        Cell* tmp_result;
        if (proc->is_builtin) {
            Cell* (*func)(const Lex *, const Cell *) = proc->builtin;
            tmp_result = func(e, make_sexpr_from_list(reversed_arg_list));
        } else {
            /* Prepend the procedure to create the application form */
            Cell* application_list = make_cell_pair(proc, reversed_arg_list);
            application_list->len = arg_list->len + 1;

            /* Convert the Scheme list to an S-expression for eval */
            Cell* application_sexpr = make_sexpr_from_list(application_list);

            /* Evaluate it */
            tmp_result = coz_eval((Lex*)e, application_sexpr);
        }
        if (tmp_result->type == CELL_ERROR) {
            /* Propagate any evaluation errors */
            return tmp_result;
        }

        /* Cons the result onto our (reversed) final list */
        final_result = make_cell_pair(tmp_result, final_result);
        final_result->len = i + 1;
    }

    /* Reverse the final list to get the correct order and return */
    return builtin_list_reverse(e, make_sexpr_len1(final_result));
}
