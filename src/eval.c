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
#include "cell.h"
#include "repr.h"
#include "symbols.h"


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

static Cell* coz_apply(const Cell* proc, const Cell* args, Lex** env_out, Cell** expr_out);

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
        if (expr->count == 0) {
            /* Unquoted "()" */
            return make_cell_error("bad expression: '()'", SYNTAX_ERR);
        }

        Cell* first = expr->cell[0];

        /* These special forms need to be dispatched out of
         * eval_sexpr() early, so the arguments are not evaluated. */
        if (first->type == CELL_SYMBOL && first->sf_id > 0) {
            const special_form_handler_t handler = SF_DISPATCH_TABLE[first->sf_id];
            const HandlerResult result = handler(env, get_args_from_sexpr(expr));
            /* A final value. */
            if (result.action == ACTION_RETURN) {
                return result.value;
            }
            /* ACTION_CONTINUE from a tail call */
            expr = result.value;
            continue;
        }

        /* It's not a special form, so it's a procedure call. */
        /* First, evaluate the procedure itself. */
        Cell* f = coz_eval(env, first);
        if (f->type == CELL_ERROR) {
            return f;
        }
        if (f->type != CELL_PROC) {
            char buf[512];
            snprintf(buf, sizeof(buf), "bad identifier: '%s'. Expression must start with a procedure",
                cell_to_string(f, MODE_REPL));
            return make_cell_error(buf, TYPE_ERR);
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

        Cell* result = coz_apply(f, args, &env, &expr);
        if (!result) return nullptr;
        if (result->type == CELL_TRAMPOLINE) {
            expr = result;
            continue;
        }
        if (result != TCS_Obj) {
            /* f was a primitive C function, it returned a final value. */
            return result;
        }
        /* If here ... f was a Scheme lambda, and apply() updated
         * our expr and env. Allow the control flow to begin another
         * loop, performing the tail call. */
    }
}

/* Apply that procedure on them args! */
static Cell* coz_apply(const Cell* proc, const Cell* args, Lex** env_out, Cell** expr_out) {
    if (proc->is_builtin) {
        return proc->builtin(*env_out, args); /* Return final value */
    }
    /* It's a Scheme lambda, return TCO */
    *env_out = build_lambda_env(proc->lambda->env, proc->lambda->formals, args);
    *expr_out = sequence_sf_body(proc->lambda->body);
    return TCS_Obj;
}
