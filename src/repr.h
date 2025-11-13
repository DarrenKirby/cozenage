/*
 * 'src/repr.h'
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

#ifndef COZENAGE_REPR_H
#define COZENAGE_REPR_H

#include "cell.h"

typedef enum {
    /** R7RS `write` mode.
        Strings are quoted, chars are #\, etc. */
    MODE_WRITE,

    /** R7RS `display` mode.
        Strings and chars are printed literally. */
    MODE_DISPLAY,

    /** For the REPL.
        Like MODE_WRITE, but with ANSI color codes. */
    MODE_REPL
} print_mode_t;

char* cell_to_string(const Cell* cell, print_mode_t mode);
void debug_print_cell(const Cell* v);
void print_env(const Lex* e, const Cell* a);

#endif //COZENAGE_REPR_H
