/*
 * 'chars.h'
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

#ifndef COZENAGE_CHARS_H
#define COZENAGE_CHARS_H

#include "cell.h"


/* Char constructors, selectors, and procedures */
Cell* builtin_char_to_int(const Lex* e, const Cell* a);
Cell* builtin_int_to_char(const Lex* e, const Cell* a);
Cell* builtin_char_equal_pred(const Lex* e, const Cell* a);
Cell* builtin_char_lt_pred(const Lex* e, const Cell* a);
Cell* builtin_char_lte_pred(const Lex* e, const Cell* a);
Cell* builtin_char_gt_pred(const Lex* e, const Cell* a);
Cell* builtin_char_gte_pred(const Lex* e, const Cell* a);
Cell* builtin_char_alphabetic(const Lex* e, const Cell* a);
Cell* builtin_char_whitespace(const Lex* e, const Cell* a);
Cell* builtin_char_numeric(const Lex* e, const Cell* a);
Cell* builtin_char_upper_case(const Lex* e, const Cell* a);
Cell* builtin_char_lower_case(const Lex* e, const Cell* a);
Cell* builtin_char_upcase(const Lex* e, const Cell* a);
Cell* builtin_char_downcase(const Lex* e, const Cell* a);
Cell* builtin_char_foldcase(const Lex* e, const Cell* a);
Cell* builtin_digit_value(const Lex* e, const Cell* a);
Cell* builtin_char_equal_ci(const Lex* e, const Cell* a);
Cell* builtin_char_lt_ci(const Lex* e, const Cell* a);
Cell* builtin_char_lte_ci(const Lex* e, const Cell* a);
Cell* builtin_char_gt_ci(const Lex* e, const Cell* a);
Cell* builtin_char_gte_ci(const Lex* e, const Cell* a);

#endif //COZENAGE_CHARS_H
