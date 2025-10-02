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
        case VAL_SYM: {
            if (is_syntactic_keyword(v->sym)) {
                char err_buf[128];
                snprintf(err_buf, sizeof(err_buf),
                         "Syntax keyword '%s' cannot be used as a variable", v->sym);
                return make_val_err(err_buf, GEN_ERR);
            }
            if (v->exact) {
                Cell* x = lex_get(e, v);
                return x;
            }
            return v;
        }
        /* S-expressions: recursively evaluate */
        case VAL_SEXPR:
            return eval_sexpr(e, v);
        /* All literals evaluate to themselves */
        case VAL_INT:
        case VAL_REAL:
        case VAL_RAT:
        case VAL_COMPLEX:
        case VAL_BOOL:
        case VAL_CHAR:
        case VAL_STR:
        case VAL_PAIR:
        case VAL_VEC:
        case VAL_BYTEVEC:
        case VAL_NIL:
        /* Functions, ports, continuations, and errors are returned as-is */
        case VAL_PROC:
        case VAL_PORT:
        case VAL_CONT:
        case VAL_ERR:
            return v;
        default:
            return make_val_err("Unknown val type in eval()", GEN_ERR);
    }
}

/* Evaluate an S-expression. */
Cell* eval_sexpr(Lex* e, Cell* v) {
    if (v->count == 0) return v;

    /* Grab first element without evaluating yet */
    Cell* first = cell_pop(v, 0);

    /* NOTE: These special forms need to be dispatched out of
     * eval_sexpr() early, so that their arguments are not evaluated */

    /* special form: define */
    if (first->type == VAL_SYM && strcmp(first->sym, "define") == 0) {
        return sf_define(e, v);
    }
    /* Special form: quote */
    if (first->type == VAL_SYM && strcmp(first->sym, "quote") == 0) {
        return sf_quote(e, v);
    }
    /* Special form: lambda */
    if (first->type == VAL_SYM && strcmp(first->sym, "lambda") == 0) {
        return sf_lambda(e, v);
    }
    /* special form - if */
    if (first->type == VAL_SYM && strcmp(first->sym, "if") == 0) {
        return sf_if(e, v);
    }
    /* special form - when */
    if (first->type == VAL_SYM && strcmp(first->sym, "when") == 0) {
        return sf_when(e, v);
    }
    /* special form - unless */
    if (first->type == VAL_SYM && strcmp(first->sym, "unless") == 0) {
        return sf_unless(e, v);
    }
    /* special form - cond */
    if (first->type == VAL_SYM && strcmp(first->sym, "cond") == 0) {
        return sf_cond(e, v);
    }
    /* special form - import */
    if (first->type == VAL_SYM && strcmp(first->sym, "import") == 0) {
        return sf_import(e, v);
    }

    /* Otherwise, evaluate first element normally (should become a function) */
    Cell* f = coz_eval(e, first);
    if (f->type != VAL_PROC) {
        printf("Bad token: ");
        printf(ANSI_RED_B);
        print_cell(f);
        printf("%s: ", ANSI_RESET);
        return make_val_err("S-expression does not start with a procedure", GEN_ERR);
    }

    /* Evaluate arguments */
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = coz_eval(e, v->cell[i]);
        if (v->cell[i]->type == VAL_ERR) {
            return cell_take(v, i);
        }
    }

    /* Apply function */
    Cell* result;
    if (f->builtin) {
        result = f->builtin(e, v);
    } else {
        result = apply_lambda(f, v);
    }
    return result;
}
