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
#include <stdio.h>
#include <string.h>


/* Evaluate a Cell in the given environment. */
Cell* coz_eval(Lex* e, Cell* v) {
    if (!v) return nullptr;

    switch (v->type) {
        /* Symbols: look them up in the environment unless quoted */
        case CELL_SYMBOL: {
            if (is_syntactic_keyword(v->sym)) {
                char err_buf[128];
                snprintf(err_buf, sizeof(err_buf),
                         "Syntax keyword '%s' cannot be used as a variable", v->sym);
                return make_cell_error(err_buf, SYNTAX_ERR);
            }
            if (v->quoted) {
                return v;
            }
            Cell* x = lex_get(e, v);
            return x;
        }
        /* S-expressions: recursively evaluate */
        case CELL_SEXPR:
            return eval_sexpr(e, v);
        /* All literals evaluate to themselves */
        case CELL_INTEGER:
        case CELL_REAL:
        case CELL_RATIONAL:
        case CELL_COMPLEX:
        case CELL_BOOLEAN:
        case CELL_CHAR:
        case CELL_STRING:
        case CELL_PAIR:
        case CELL_VECTOR:
        case CELL_BYTEVECTOR:
        case CELL_NIL:
        /* Functions, ports, continuations, and errors are returned as-is */
        case CELL_PROC:
        case CELL_PORT:
        case CELL_CONT:
        case CELL_ERROR:
            return v;
        default:
            return make_cell_error("Unknown val type in eval()", GEN_ERR);
    }
}

/* Helper to extract procedure args from s-expr */
static Cell* get_args_from_sexpr(const Cell* v) {
    Cell* args = make_cell_sexpr();
    for (int i = 1; i < v->count; i++) {
        cell_add(args, v->cell[i]);
    }
    return args;
}

/* Evaluate an S-expression. */
Cell* eval_sexpr(Lex* e, Cell* v) {
    if (v->count == 0) return v;

    /* Grab first element without evaluating yet */
    Cell* first = v->cell[0];

    /* NOTE: These special forms need to be dispatched out of
     * eval_sexpr() early, so that their arguments are not evaluated */
    if (first->type == CELL_SYMBOL) {
        /* Create the argument list once for all special forms. */
        Cell* sf_args = get_args_from_sexpr(v);
        /* special form: define */
        if (strcmp(first->sym, "define") == 0) {
            return sf_define(e, sf_args);
        }
        /* Special form: quote */
        if (strcmp(first->sym, "quote") == 0) {
            return sf_quote(e, sf_args);
        }
        /* Special form: lambda */
        if (strcmp(first->sym, "lambda") == 0) {
            return sf_lambda(e, sf_args);
        }
        /* special form - if */
        if (strcmp(first->sym, "if") == 0) {
            return sf_if(e, sf_args);
        }
        /* special form - when */
        if (strcmp(first->sym, "when") == 0) {
            return sf_when(e, sf_args);
        }
        /* special form - unless */
        if (strcmp(first->sym, "unless") == 0) {
            return sf_unless(e, sf_args);
        }
        /* special form - cond */
        if (strcmp(first->sym, "cond") == 0) {
            return sf_cond(e, sf_args);
        }
        /* special form - import */
        if (strcmp(first->sym, "import") == 0) {
            return sf_import(e, sf_args);
        }
        /* special form - let */
        if (strcmp(first->sym, "let") == 0) {
            return sf_let(e, sf_args);
        }
        /* special form - let* */
        if (strcmp(first->sym, "let*") == 0) {
            return sf_let_star(e, sf_args);
        }
        /* special form - letrec */
        if (strcmp(first->sym, "letrec") == 0) {
            return sf_letrec(e, sf_args);
        }
        /* special form - set! */
        if (strcmp(first->sym, "set!") == 0) {
            return sf_set_bang(e, sf_args);
        }
        /* special form - begin */
        if (strcmp(first->sym, "begin") == 0) {
            return sf_begin(e, sf_args);
        }
        /* special form - and */
        if (strcmp(first->sym, "and") == 0) {
            return sf_and(e, sf_args);
        }
        /* special form - or */
        if (strcmp(first->sym, "or") == 0) {
            return sf_or(e, sf_args);
        }
    }

    /* It's not a special form, so it's a procedure call. */
    /* First, evaluate the procedure itself. */
    Cell* f = coz_eval(e, first);
    if (f->type != CELL_PROC) {
        printf("Bad token: ");
        printf(ANSI_RED_B);
        print_cell(f);
        printf("%s: ", ANSI_RESET);
        return make_cell_error("S-expression does not start with a procedure", TYPE_ERR);
    }

    /* Create a new list containing the unevaluated arguments. */
    Cell* args = get_args_from_sexpr(v);

    /* Now, evaluate each argument within this new list. */
    for (int i = 0; i < args->count; i++) {
        args->cell[i] = coz_eval(e, args->cell[i]);
        if (args->cell[i]->type == CELL_ERROR) {
            /* If an argument evaluation fails, return the error. */
            return cell_take(args, i);
        }
    }

    /* Apply the function to the list of evaluated arguments. */
    Cell* result;
    if (f->is_builtin) {
        result = f->builtin(e, args);
    } else {
        result = apply_lambda(f, args);
    }
    return result;
}
