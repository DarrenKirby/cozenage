/*
 * 'src/coz_ext_lib.c'
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

#include "coz_ext_lib.h"
#include "environment.h"
#include "main.h"
#include "numerics.h"
#include "printer.h"


/* These are non-R7RS procedures that don't fit well elsewhere
 * FIXME: Probably just delete the two aliases, put print-env
 * somewhere appropriate, and delete this file and header */


Cell* builtin_print_env(const Lex* e, const Cell* a) {
    (void)a;
    for (int i = 0; i < e->count; i++) {
        printf("%s%s%s -> ", ANSI_WHITE_B, e->syms[i], ANSI_RESET);
        print_cell(e->vals[i]);
        printf("\n");
    }
    /* for side effects - no useful return value */
    return nullptr;
}

void lex_add_coz_ext(Lex* e) {
    lex_add_builtin(e, "print-env", builtin_print_env);
    lex_add_builtin(e, "^", builtin_expt); /* non-standard alias for expt */
    lex_add_builtin(e, "%", builtin_modulo); /* non-standard alias for modulo */
}
