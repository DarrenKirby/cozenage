/*
 * 'strings.h'
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

#ifndef COZENAGE_STRINGS_H
#define COZENAGE_STRINGS_H

#include "types.h"


/* String constructors, selectors, and procedures */
Cell* builtin_string_to_symbol(Lex* e, Cell* a);
Cell* builtin_symbol_to_string(Lex* e, Cell* a);
Cell* builtin_string(Lex* e, Cell* a);
Cell* builtin_string_length(Lex* e, Cell* a);
Cell* builtin_string_eq_pred(Lex* e, Cell* a);
Cell* builtin_string_lt_pred(Lex* e, Cell* a);
Cell* builtin_string_lte_pred(Lex* e, Cell* a);
Cell* builtin_string_gt_pred(Lex* e, Cell* a);
Cell* builtin_string_gte_pred(Lex* e, Cell* a);
Cell* builtin_string_append(Lex* e, Cell* a);

#endif //COZENAGE_STRINGS_H
