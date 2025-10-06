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

/* Evaluate an S-expression. */
Cell* eval_sexpr(Lex* e, Cell* v) {
    if (v->count == 0) return v;

    /* Grab first element without evaluating yet */
    Cell* first = cell_pop(v, 0);

    /* NOTE: These special forms need to be dispatched out of
     * eval_sexpr() early, so that their arguments are not evaluated */
    if (first->type == CELL_SYMBOL) {
        /* special form: define */
        if (strcmp(first->sym, "define") == 0) {
            return sf_define(e, v);
        }
        /* Special form: quote */
        if (strcmp(first->sym, "quote") == 0) {
            return sf_quote(e, v);
        }
        /* Special form: lambda */
        if (strcmp(first->sym, "lambda") == 0) {
            return sf_lambda(e, v);
        }
        /* special form - if */
        if (strcmp(first->sym, "if") == 0) {
            return sf_if(e, v);
        }
        /* special form - when */
        if (strcmp(first->sym, "when") == 0) {
            return sf_when(e, v);
        }
        /* special form - unless */
        if (strcmp(first->sym, "unless") == 0) {
            return sf_unless(e, v);
        }
        /* special form - cond */
        if (strcmp(first->sym, "cond") == 0) {
            return sf_cond(e, v);
        }
        /* special form - import */
        if (strcmp(first->sym, "import") == 0) {
            return sf_import(e, v);
        }
        /* special form - let */
        if (strcmp(first->sym, "let") == 0) {
            return sf_let(e, v);
        }
        /* special form - let* */
        if (strcmp(first->sym, "let*") == 0) {
            return sf_let_star(e, v);
        }
        /* special form - set! */
        if (strcmp(first->sym, "set!") == 0) {
            return sf_set_bang(e, v);
        }
        /* special form - begin */
        if (strcmp(first->sym, "begin") == 0) {
            return sf_begin(e, v);
        }
        /* special form - and */
        if (strcmp(first->sym, "and") == 0) {
            return sf_and(e, v);
        }
        /* special form - or */
        if (strcmp(first->sym, "or") == 0) {
            return sf_or(e, v);
        }
    }
    /* Otherwise, evaluate first element normally (should become a function) */
    Cell* f = coz_eval(e, first);
    if (f->type != CELL_PROC) {
        printf("Bad token: ");
        printf(ANSI_RED_B);
        print_cell(f);
        printf("%s: ", ANSI_RESET);
        return make_cell_error("S-expression does not start with a procedure", TYPE_ERR);
    }

    /* Evaluate arguments */
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = coz_eval(e, v->cell[i]);
        if (v->cell[i]->type == CELL_ERROR) {
            return cell_take(v, i);
        }
    }

    /* Apply function */
    Cell* result;
    if (f->is_builtin) {
        result = f->builtin(e, v);
    } else {
        result = apply_lambda(f, v);
    }
    return result;
}
