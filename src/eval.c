/*
 * 'src/eval.c'
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

#include "eval.h"
#include "special_forms.h"
#include "printer.h"
#include "main.h"
#include "cell.h"
#include "symbols.h"
#include <stdio.h>


/* Helper to extract procedure args from s-expr */
static Cell* get_args_from_sexpr(const Cell* v) {
    Cell* args = make_cell_sexpr();
    for (int i = 1; i < v->count; i++) {
        cell_add(args, v->cell[i]);
    }
    return args;
}

special_form_handler_t SF_DISPATCH_TABLE[] = {
    nullptr,
    &sf_define,
    &sf_quote,
    &sf_lambda,
    &sf_if,
    &sf_when,
    &sf_unless,
    &sf_cond,
    &sf_import,
    &sf_let,
    &sf_let_star,
    &sf_letrec,
    &sf_set_bang,
    &sf_begin,
    &sf_and,
    &sf_or,
};

/* Evaluate a Cell in the given environment. */
Cell* coz_eval(Lex* env, Cell* expr) {
    while (true) {
        if (!expr) return nullptr;

        /* Turf all the self-evaluating types. */
        if (expr->type & (CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|
                          CELL_BOOLEAN|CELL_CHAR|CELL_STRING|CELL_PAIR|
                          CELL_VECTOR|CELL_BYTEVECTOR|CELL_NIL|CELL_EOF|
                          CELL_PROC|CELL_PORT|CELL_CONT|CELL_ERROR)) {
            return expr;
                          }

        /* Symbols: look them up in the environment unless quoted */
        if (expr->type & CELL_SYMBOL) {

            /* Scold for using syntax dumbly */
            if (is_syntactic_keyword(expr->sym)) {
                char err_buf[128];
                snprintf(err_buf, sizeof(err_buf),
                         "Syntax keyword '%s' cannot be used as a variable", expr->sym);
                return make_cell_error(err_buf, SYNTAX_ERR);
            }
            if (expr->quoted) {
                return expr;
            }
            return lex_get(env, expr);
        }

        /* S-expressions:  */
        /* Grab first element without evaluating yet */
        Cell* first = expr->cell[0];

        /* NOTE: These special forms need to be dispatched out of
         * eval_sexpr() early, so the arguments are not evaluated. */
        if (first->type == CELL_SYMBOL && first->sf_id > 0) {
            /* It's a special form! Dispatch using the array. */
            const special_form_handler_t handler = SF_DISPATCH_TABLE[first->sf_id];
            const HandlerResult result = handler(env, get_args_from_sexpr(expr));
            /* Straight return */
            if (result.action == ACTION_RETURN) {
                return result.value;
            }
            /* ACTION_CONTINUE from a tail call */
            expr = result.value;
            continue;
        }

        /* It's not a special form, so it's a procedure call. */
        /* First, evaluate the procedure itself. */
        const Cell* f = coz_eval(env, first);
        if (f->type != CELL_PROC) {
            printf("Bad token: ");
            printf(ANSI_RED_B);
            print_cell(f);
            printf("%s: ", ANSI_RESET);
            return make_cell_error("S-expression does not start with a procedure", TYPE_ERR);
        }

        /* Create a new list containing the unevaluated arguments. */
        Cell* args = get_args_from_sexpr(expr);

        /* Now, evaluate each argument within this new list. */
        for (int i = 0; i < args->count; i++) {
            args->cell[i] = coz_eval(env, args->cell[i]);
            if (args->cell[i]->type == CELL_ERROR) {
                /* If an argument evaluation fails, return the error. */
                return cell_take(args, i);
            }
        }

        /* Apply the function to the list of evaluated arguments. */
        /* Dispatch builtin */
        if (f->is_builtin) {
            return f->builtin(env, args);
        }
        /* Tail-call evaluate the lambda */
        const Cell* body = f->body;
        if (body->count == 0) {
            return make_cell_error("lambda has no body!", VALUE_ERR);
        }
        if (f->formals->count != args->count) {
            return make_cell_error("Lambda: wrong number of arguments", ARITY_ERR);
        }
        env = build_lambda_env(f->env, f->formals, args);
        if (body->count == 1) {
            expr = body->cell[0];
        } else {
            expr = sequence_sf_body(body);
        }
    }
}
