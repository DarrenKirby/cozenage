/*
* 'src/char_lib.h'
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

#ifndef COZENAGE_CHAR_LIB_H
#define COZENAGE_CHAR_LIB_H

#include "types.h"

Cell* builtin_char_alphabetic(Lex* e, Cell* a);
Cell* builtin_char_whitespace(Lex* e, Cell* a);
Cell* builtin_char_numeric(Lex* e, Cell* a);
Cell* builtin_char_upper_case(Lex* e, Cell* a);
Cell* builtin_char_lower_case(Lex* e, Cell* a);
Cell* builtin_char_upcase(Lex* e, Cell* a);
Cell* builtin_char_downcase(Lex* e, Cell* a);
Cell* builtin_char_foldcase(Lex* e, Cell* a);
Cell* builtin_digit_value(Lex* e, Cell* a);
Cell* builtin_char_equal_ci(Lex* e, Cell* a);
Cell* builtin_char_lt_ci(Lex* e, Cell* a);
Cell* builtin_char_lte_ci(Lex* e, Cell* a);
Cell* builtin_char_gt_ci(Lex* e, Cell* a);
Cell* builtin_char_gte_ci(Lex* e, Cell* a);
Cell* builtin_string_downcase(Lex* e, Cell* a);
Cell* builtin_string_upcase(Lex* e, Cell* a);
Cell* builtin_string_foldcase(Lex* e, Cell* a);
void lex_add_char_lib(Lex* e);

#endif //COZENAGE_CHAR_LIB_H