/*
 * 'src/transforms.c'
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

#include "transforms.h"
#include "cell.h"
#include "symbols.h"

# include <string.h>


Cell* expand_body_expressions(const Cell* body_elements, const int start_index) {
    Cell* inner_defines = make_cell_sexpr();
    int i = start_index;

    /* Collect all leading defines. */
    while (i < body_elements->count) {
        Cell* current = body_elements->cell[i];
        if (current->type == CELL_SEXPR && current->count > 0 &&
            current->cell[0]->type == CELL_SYMBOL &&
            strcmp(current->cell[0]->sym, "define") == 0) {
            cell_add(inner_defines, current);
            i++;
            } else {
                break;
            }
    }

    Cell* final_body_expr;

    /* Build the "executable" part of the body
       If there's more than one expression left, wrap in 'begin'. */
    const int remaining_count = body_elements->count - i;
    if (remaining_count > 1) {
        final_body_expr = make_cell_sexpr();
        cell_add(final_body_expr, G_begin_sym);
        for (int j = i; j < body_elements->count; j++) {
            cell_add(final_body_expr, expand(body_elements->cell[j]));
        }
    } else if (remaining_count == 1) {
        final_body_expr = expand(body_elements->cell[i]);
    } else {
        /* Handle empty body error or return unspecified. */
        return make_cell_error("Procedure body is empty", SYNTAX_ERR);
    }

    /* If there WERE defines, wrap everything in letrec. */
    if (inner_defines->count > 0) {
        Cell* letrec_expr = make_cell_sexpr();
        cell_add(letrec_expr, G_letrec_sym);
        cell_add(letrec_expr, transform_defines_to_bindings(inner_defines));
        cell_add(letrec_expr, final_body_expr);
        return letrec_expr;
    }

    Cell* begin_block = make_cell_sexpr();
    cell_add(begin_block, G_begin_sym);
    for (int j = i; j < body_elements->count; j++) {
        cell_add(begin_block, expand(body_elements->cell[j]));
    }
    return begin_block;
}


Cell* expand(Cell* c) {
    if (c->type != CELL_SEXPR || c->count == 0) return c;

    Cell* first = c->cell[0];
    if (first->type == CELL_SYMBOL) {
        /* (define (name args) body...) */
        if (strcmp(first->sym, "define") == 0 && c->count > 2 && c->cell[1]->type == CELL_SEXPR) {
            Cell* result = make_cell_sexpr();
            cell_add(result, first);        /* 'define' */
            cell_add(result, c->cell[1]);   /* '(name args)' */
            cell_add(result, expand_body_expressions(c, 2));
            return result;
        }

        /* (lambda (args) body...) */
        if (strcmp(first->sym, "lambda") == 0 && c->count > 2) {
            Cell* result = make_cell_sexpr();
            cell_add(result, first);        /* 'lambda' */
            cell_add(result, c->cell[1]);   /* '(args)' */
            cell_add(result, expand_body_expressions(c, 2));
            return result;
        }
    }

    /* Default: recursive expansion. */
    Cell* result = make_cell_sexpr();
    for (int i = 0; i < c->count; i++) {
        cell_add(result, expand(c->cell[i]));
    }
    return result;
}


Cell* transform_defines_to_bindings(const Cell* inner_defines) {
    Cell* bindings_list = make_cell_sexpr();

    for (int i = 0; i < inner_defines->count; i++) {
        Cell* def = inner_defines->cell[i];
        Cell* binding_pair = make_cell_sexpr();

        /* def->cell[0] is 'define'
           def->cell[1] is either the name OR (name args...). */
        Cell* target = def->cell[1];

        if (target->type == CELL_SYMBOL) {
            /* Case: (define name value) */
            cell_add(binding_pair, target);
            /* The value is the 3rd element: def->cell[2]
               We expand it in case it's a lambda or has its own inner defines. */
            cell_add(binding_pair, expand(def->cell[2]));
        }
        else if (target->type == CELL_SEXPR) {
            /* Case: (define (name args...) body...)
               The name is the first element of the signature S-expr. */
            cell_add(binding_pair, target->cell[0]);

            /* Wrap the rest in a lambda. */
            Cell* lambda_expr = make_cell_sexpr();
            cell_add(lambda_expr, G_lambda_sym);

            /* Construct the args list from the rest of the signature. */
            Cell* args_list = make_cell_sexpr();
            for (int j = 1; j < target->count; j++) {
                cell_add(args_list, target->cell[j]);
            }
            cell_add(lambda_expr, args_list);

            // Add the body expressions to the lambda
            // for (int j = 2; j < def->count; j++) {
            //     cell_add(lambda_expr, expand(def->cell[j]));
            // }

            /* Use our existing body-fixer to handle internal defines and implicit begins. */
            cell_add(lambda_expr, expand_body_expressions(def, 2));
            cell_add(binding_pair, lambda_expr);
        }
        cell_add(bindings_list, binding_pair);
    }
    return bindings_list;
}
