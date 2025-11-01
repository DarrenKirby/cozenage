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

#include <stdlib.h>

#include "eval.h"
#include "repr.h"
#include "types.h"
#include "runner.h"
#include "lexer.h"
#include "parser.h"


Cell* builtin_load(const Lex* e, const Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error("load: arg must be a string", TYPE_ERR);
    }
    FILE *script_file = fopen(a->cell[0]->str, "r");
    if (script_file == NULL) {
        perror("Error opening Scheme file");
        /* Cannot open the file, return failure */
        return nullptr;
    }

    char *expression_str;
    while ((expression_str = collect_one_expression_from_file(script_file)) != NULL) {
        TokenArray* ta = scan_all_tokens(expression_str);
        Cell* expression = parse_tokens_new(ta);

        /* Free the expression string */
        free(expression_str);

        /* Check if the parser failed */
        if (expression == NULL) {
            fprintf(stderr, "Fatal Syntax Error in script.\n");
            return nullptr;
        }

        /* Evaluate */
        coz_eval((Lex*)e, expression);
    }
    return make_cell_boolean(1);
}

Cell* builtin_eval(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;
    Cell* args;
    /* Convert list to s-expr if we are handed a quote */
    if (a->cell[0]->type == CELL_PAIR) {
        args = make_sexpr_from_list(a->cell[0]);
        for (int i = 0; i < args->count; i++ ) {
            if (args->cell[i]->type == CELL_PAIR && args->cell[i]->len != -1) {
                Cell* tmp = args->cell[i];
                args->cell[i] = make_sexpr_from_list(tmp);
            }
            /* 'Unquote' the symbols */
            if (args->cell[i]->type == CELL_SYMBOL) {
                args->cell[i]->quoted = false;
            }
        }
        /* Otherwise just send straight to eval */
    } else {
        args = a->cell[0];
    }
    return coz_eval((Lex*)e, args);
}

void lex_add_eval_lib(const Lex* e) {
    lex_add_builtin(e, "eval", builtin_eval);
    lex_add_builtin(e, "load", builtin_load);
}
