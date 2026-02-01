/*
 * 'src/eval.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 - 2026 Darren Kirby <darren@dragonbyte.ca>
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

/* This file defines the main internal-interpreter eval and apply functions.
 * eval first direct-returns all self-evaluating types except for symbols
 * which get looked up in the global environment first. It then dispatches out
 * S-expressions with special forms in the first position.
 *
 * After that, the S-expression is assumed to be a procedure call. The procedure
 * is evaluated, then the arguments are copied and evaluated, and the procedure,
 * arguments, and environment are sent to apply. Builtin procedures will
 * directly return a result, but user-defined lambda procedures will construct
 * the lambda environment, and be sent back to eval as a tail call.
 *
 * The file also defines an apply_and_get_val function which will directly
 * return a value instead of tail-calling. This allows for it to be used to
 * evaluate a result from other C-functions internally.
 */

#include "eval.h"
#include "special_forms.h"
#include "cell.h"
#include "types.h"
#include "repr.h"
#include "symbols.h"


/* Helper to extract procedure args from s-expr. */
static Cell* get_args_from_sexpr(const Cell* v)
{
    Cell* args = make_cell_sexpr();
    for (int i = 1; i < v->count; i++) {
        cell_add(args, v->cell[i]);
    }
    return args;
}


/* This is only for special forms that are manually
 * implemented in special_forms.c */
special_form_handler_t SF_DISPATCH_TABLE[] = {
    [SF_ID_DEFINE]   = &sf_define,
    [SF_ID_QUOTE]    = &sf_quote,
    [SF_ID_LAMBDA]   = &sf_lambda,
    [SF_ID_IF]       = &sf_if,
    [SF_ID_IMPORT]   = &sf_import,
    [SF_ID_LET]      = &sf_let,
    [SF_ID_LETREC]   = &sf_letrec,
    [SF_ID_SET_BANG] = &sf_set_bang,
    [SF_ID_BEGIN]    = &sf_begin,
    [SF_ID_AND]      = &sf_and,
    [SF_ID_DEFMACRO] = &sf_defmacro,
    [SF_ID_DEBUG]    = &sf_with_gc_stats
};


static Cell* coz_apply(const Cell* proc, Cell* args, Lex** env_out, Cell** expr_out);

/* Evaluate a Cell in the given environment. */
Cell* coz_eval(Lex* env, Cell* expr)
{
    while (true) {
        if (!expr) return nullptr;

        /* Quick exit for all the self-evaluating types. */
        if (expr->type & (CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|
                          CELL_BOOLEAN|CELL_CHAR|CELL_STRING|CELL_PAIR|
                          CELL_VECTOR|CELL_BYTEVECTOR|CELL_NIL|CELL_EOF|
                          CELL_PROC|CELL_PORT|CELL_ERROR|CELL_UNSPEC|
                          CELL_BIGINT|CELL_BIGFLOAT)) {
            return expr;
        }

        /* Symbols: look them up in the environment. */
        if (expr->type & CELL_SYMBOL) {

            /* Scold for using syntax dumbly */
            if (is_syntactic_keyword(expr)) {
                return make_cell_error(
                    fmt_err("Syntax keyword '%s' cannot be used as a variable", expr->sym),
                    SYNTAX_ERR);
            }

            return lex_get(env, expr);
        }

        /* S-expressions:  */
        /* Grab first element without evaluating yet. */
        if (expr->count == 0) {
            /* Unquoted "()" */
            return make_cell_error(
                "bad expression: '()'",
                SYNTAX_ERR);
        }

        Cell* first = expr->cell[0];

        /* Special forms need to be dispatched out of
         * eval_sexpr() early, so the arguments are not evaluated. */
        if (first->type == CELL_SYMBOL && first->sf_id > 0) {
            const special_form_handler_t handler = SF_DISPATCH_TABLE[first->sf_id];
            if (!handler) {
                return make_cell_error(
                    fmt_err("special form: '%s' not registered (did you forget to import?)", first->sym),
                    SYNTAX_ERR);
            }
            const HandlerResult result = handler(env, get_args_from_sexpr(expr));
            /* ACTION_RETURN final value. */
            if (result.action == ACTION_RETURN) {
                return result.value;
            }

            /* ACTION_CONTINUE from a tail call. */
            expr = result.value;
            env = result.env;
            continue;
        }

        /* It's not a special form, so it's a procedure call or macro. */
        /* First, evaluate the procedure itself. */
        Cell* f = coz_eval(env, first);
        if (f->type == CELL_ERROR) {
            return f;
        }

        if (f->type == CELL_MACRO) {
            /* Transform the macro. */
            Cell* raw_args = get_args_from_sexpr(expr);
            Cell* result = coz_apply_and_get_val(f, raw_args, env);
            /* Tail-call evaluate the result of the transformation. */
            Cell* s = make_sexpr_from_list(result, true);
            expr = s;
            continue;
        }

        /* Here, if first position is not a procedure, it is an error. */
        if (f->type != CELL_PROC) {
            return make_cell_error(
                fmt_err("bad identifier: '%s'. Expression must start with a procedure",
                    cell_to_string(f, MODE_REPL)),
                TYPE_ERR);
        }

        /* Create a new list containing the unevaluated arguments. */
        Cell* args = get_args_from_sexpr(expr);

        /* Evaluate each argument within this new list. */
        for (int i = 0; i < args->count; i++) {
            Cell* result = coz_eval(env, args->cell[i]);
            /* Ignore legitimate null. */
            if (!result) { continue; }
            args->cell[i] = result;

            if (args->cell[i]->type == CELL_ERROR) {
                /* If an argument evaluation fails, return the error. */
                return args->cell[i];
            }
        }

        Cell* result = coz_apply(f, args, &env, &expr);
        if (!result) return nullptr;

        if (result != TCS_Obj) {
            /* f was a primitive C function that returned a final value. */
            return result;
        }
        /* If here ...it was a TCS, and apply() updated
         * expr and env. Allow the control flow to begin another
         * loop, performing the tail call. */
    }
}


/* Apply that procedure on them args! */
static Cell* coz_apply(const Cell* proc, Cell* args, Lex** env_out, Cell** expr_out)
{
    if (proc->is_builtin) {
        /* Run the builtin. */
        Cell* result = proc->builtin(*env_out, args);

        /* If the builtin returned TCS_Obj (only 'apply' does this thus far),
         * it means that 'result' is an expression that needs to be tail-called... */
        if (result->type == CELL_TCS) {
            result->type = CELL_SEXPR;
            *expr_out = result;
            return TCS_Obj;
        }
        /* Otherwise, it's a final result. */
        return result;
    }

    /* It's a Scheme lambda, return TCO. */
    Lex* le = build_lambda_env(proc->lambda->env, proc->lambda->formals, args);
    if (le == nullptr) {
        /* We cannot return a specific error message from build_lambda_env(),
         * so we have to return this generic error. */
        return make_cell_error(
            "bad lambda expression",
            SYNTAX_ERR);
    }
    *env_out = le;
    *expr_out = proc->lambda->body;
    return TCS_Obj;
}


/*
 * Applies a procedure to a list of ARGUMENTS (which are already evaluated)
 * and returns the final computed value.
 * This function handles the trampoline loop internally for Scheme lambdas,
 * making it safe to call from C code like builtins.
 */
Cell* coz_apply_and_get_val(const Cell* proc, Cell* args, const Lex* env)
{
    if (proc->is_builtin) {
        return proc->builtin(env, args);
    }

    /*
     * This is a Scheme lambda. We can't just call it because it might tail-call
     * internally. We need to set it up and then kick off a self-contained
     * evaluation loop that runs until it produces a final value. */
    Lex* lambda_env  = build_lambda_env(proc->lambda->env, proc->lambda->formals, args);
    if (lambda_env == nullptr) {
        /* We cannot return a specific error message from build_lambda_env(),
         * so we have to return this generic error. */
        return make_cell_error(
            "bad lambda expression",
            SYNTAX_ERR);
    }
    Cell* body_expr = proc->lambda->body;
    return coz_eval(lambda_env, body_expr);
}
