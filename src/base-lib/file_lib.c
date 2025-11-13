/*
 * 'src/base-lib/file_lib.c'
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

#include "types.h"
#include "cell.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>


/* 'file-exists?' -> CELL_BOOLEAN - file exists predicate */
static Cell* file_exists(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const char* filename = a->cell[0]->str;
    if (access(filename, F_OK) == 0) {
        return make_cell_boolean(1);
    }
    return make_cell_boolean(0);
}

/* 'delete-file -> CELL_BOOLEAN - delete a file, and return a bool confirming outcome */
static Cell* delete_file(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const char* filename = a->cell[0]->str;
    if (unlink(filename) != 0) {
        Cell* f_err = make_cell_error(strerror(errno), FILE_ERR);
        return f_err;
    }
    return make_cell_boolean(1);
}

/* Register the procedures in the environment */
void cozenage_library_init(const Lex* e) {
    lex_add_builtin(e, "file-exists?", file_exists);
    lex_add_builtin(e, "delete-file", delete_file);
}
