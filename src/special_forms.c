/*
 * 'special_forms.c'
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

#include "special_forms.h"
#include "eval.h"
#include "load_library.h"
#include <string.h>


/* -----------------------------*
 *         Special forms        *
 * -----------------------------*/

/* 'define' -> binds a value (or proc) to a symbol, and places it
 * into the environment */
Cell* builtin_define(Lex* e, Cell* a) {
    if (a->count < 2) {
        return make_val_err("define requires at least 2 arguments", GEN_ERR);
    }
    const Cell* target = a->cell[0];

    /* (define <symbol> <expr>) */
    if (target->type == VAL_SYM) {
        Cell* val = coz_eval(e, a->cell[1]);
        /* Bail out if error encountered during evaluation */
        if (val->type == VAL_ERR) {
            return val;
        }
        /* Grab the name for the un-sugared define lambda */
        if (val->type == VAL_PROC) {
            if (!val->name) {
                val->name = GC_strdup(target->name);
            }
        }
        lex_put(e, target, val);
        return val;
    }

    /* (define (<f-name> <args>) <body>) */
    if (target->type == VAL_SEXPR && target->count > 0 &&
        target->cell[0]->type == VAL_SYM) {

        /* first element is function name */
        const Cell* fname = target->cell[0];

        /* rest are formal args */
        Cell* formals = make_val_sexpr();
        for (int i = 1; i < target->count; i++) {
            if (target->cell[i]->type != VAL_SYM) {
                return make_val_err("lambda formals must be symbols", GEN_ERR);
            }
            cell_add(formals, cell_copy(target->cell[i]));
        }

        /* build lambda with args + body */
        Cell* body = make_val_sexpr();
        for (int i = 1; i < a->count; i++) {
            cell_add(body, cell_copy(a->cell[i]));
        }

        Cell* lam = lex_make_named_lambda(fname->sym, formals, body, e);
        lex_put(e, fname, lam);
        return lam;
        }

    return make_val_err("invalid define syntax", GEN_ERR);
}

Cell* builtin_if(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3);
    if (err) return err;

    const Cell* test = coz_eval(e, a->cell[0]);
    if (test->type != VAL_BOOL) {
        return make_val_err("'if' test must be a predicate", GEN_ERR);
    }
    Cell* result;
    if (test->b_val == 1) {
        result = coz_eval(e, a->cell[1]);
    } else {
        if (a->count == 2) {
            result = NULL;
        } else {
            result = coz_eval(e, a->cell[2]);
        }
    }
    return result;
}

Cell* builtin_when(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;

    const Cell* test = coz_eval(e, a->cell[0]);
    if (test->type != VAL_BOOL) {
        return make_val_err("'when' test must be a predicate", GEN_ERR);
    }
    Cell* result = NULL;
    if (test->b_val == 1) {
        for (int i = 1; i < a->count; i++) {
            result = coz_eval(e, a->cell[i]);
        }
    }
    return result;
}

Cell* builtin_unless(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;

    const Cell* test = coz_eval(e, a->cell[0]);
    if (test->type != VAL_BOOL) {
        return make_val_err("'unless' test must be a predicate", GEN_ERR);
    }
    Cell* result = NULL;
    if (test->b_val == 0) {
        for (int i = 1; i < a->count; i++) {
            result = coz_eval(e, a->cell[i]);
        }
    }
    return result;
}

Cell* builtin_cond(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;

    Cell* result = NULL;
    const Cell* clause = NULL;
    const Cell* test = NULL;
    for (int i = 0; i < a->count; i++) {
        clause = a->cell[i];
        test = coz_eval(e, clause->cell[0]);
        if (test->type == VAL_PROC && strcmp(test->name, "else") == 0) {
            result = coz_eval(e, clause->cell[1]);
            break;
        }
        if (test->type != VAL_BOOL) {
            result = make_val_err("'cond' test must be a predicate", GEN_ERR);
        }
        if (test->b_val == 1) {
            for (int j = 1; j < clause->count; j++) {
                result = coz_eval(e, clause->cell[j]);
            }
            break;
        }

    }
    return result;
}

/* dummy function */
Cell* builtin_else(Lex* e, Cell* a) {
    (void)e;
    (void)a;
    return make_val_bool(1);
}

Cell* builtin_import(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;
    err = check_arg_types(a, VAL_PAIR);
    if (err) return err;

    Cell* result = NULL;
    for (int i = 0; i < a->count; i++) {
        const char* library_type = a->cell[i]->car->str;
        const char* library_name = a->cell[i]->cdr->str;

        if (strcmp(library_type, "scheme") == 0) {
            /* Load the Library */
            result = load_scheme_library(library_name, e);
        // } else if (strcmp(library_type, "cozenage") == 0){
        //     result = load_scheme_library(library_name, e);
        } else {
            /* TODO: Handle User Libraries Here
             * For example, (import (my-libs utils)). */
            return make_val_err("import: user-defined libraries not yet supported", GEN_ERR);
        }
    }
    return result;
}
