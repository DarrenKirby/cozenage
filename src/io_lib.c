/*
 * 'src/io_lib.c'
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

#include "io_lib.h"
#include "printer.h"
#include "ports.h"
#include <string.h>
#include <errno.h>


Cell* builtin_display(const Lex* e, const Cell* a) {
    /* FIXME: only works with strings
    Will have to change/copy/edit printing functions to write to a port */
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2);
    if (err) return err;

    Cell* port = NULL;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        if (a->cell[1]->type != VAL_PORT) {
            return make_val_err("arg1 must be a port", GEN_ERR);
        }
        port = a->cell[1];
    }
    if (fputs(a->cell[0]->str, port->fh) == EOF) {
        return make_val_err(strerror(errno), FILE_ERR);
    }
    return NULL;
}

void lex_add_write_lib(Lex* e) {
    lex_add_builtin(e, "display", builtin_display);
}

