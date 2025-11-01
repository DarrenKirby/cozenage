/*
 * 'load_lib.c'
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

#include "load_lib.h"
#include "types.h"
#include "runner.h"
#include "lexer.h"
#include "repr.h"


Cell* builtin_load(const Lex* e, const Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error("load: arg must be a string", TYPE_ERR);
    }
    const char* file = a->cell[0]->str;
    const char* input = read_file_to_string(file);
    TokenArray* ta = scan_all_tokens(input);
    const Cell* result = parse_all_expressions((Lex*)e, ta, false);

    if (result && result->type == CELL_ERROR) {
        fprintf(stderr, "%s\n", cell_to_string(result, MODE_REPL));
        return make_cell_boolean(0);
    }
    return make_cell_boolean(1);
}

void lex_add_load_lib(const Lex* e) {
    lex_add_builtin(e, "load", builtin_load);
}
