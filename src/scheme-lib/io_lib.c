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
#include "types.h"
#include "repr.h"
#include "ports.h"
#include <string.h>


Cell* builtin_display(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2);
    if (err) return err;

    Cell* port;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error("arg1 must be a port", TYPE_ERR);
        }
        port = a->cell[1];
    }
    const Cell* val = a->cell[0];
    fprintf(port->port->fh, "%s", cell_to_string(val, MODE_DISPLAY));
    return nullptr;
}

void lex_add_write_lib(const Lex* e) {
    lex_add_builtin(e, "display", builtin_display);
}
