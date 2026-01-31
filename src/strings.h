/*
 * 'src/strings.h'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 Darren Kirby <darren@dragonbyte.ca>
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

#include "cell.h"


/* String constructors, selectors, and procedures */
Cell* builtin_string(const Lex* e, const Cell* a);
Cell* builtin_string_length(const Lex* e, const Cell* a);
Cell* builtin_string_eq_pred(const Lex* e, const Cell* a);
Cell* builtin_string_lt_pred(const Lex* e, const Cell* a);
Cell* builtin_string_lte_pred(const Lex* e, const Cell* a);
Cell* builtin_string_gt_pred(const Lex* e, const Cell* a);
Cell* builtin_string_gte_pred(const Lex* e, const Cell* a);
Cell* builtin_string_append(const Lex* e, const Cell* a);
Cell* builtin_string_ref(const Lex* e, const Cell* a);
Cell* builtin_make_string(const Lex* e, const Cell* a);
Cell* builtin_string_list(const Lex* e, const Cell* a);
Cell* builtin_list_string(const Lex* e, const Cell* a);
Cell* builtin_substring(const Lex* e, const Cell* a);
Cell* builtin_string_set_bang(const Lex* e, const Cell* a);
Cell* builtin_string_copy(const Lex* e, const Cell* a);
Cell* builtin_string_copy_bang(const Lex* e, const Cell* a);
Cell* builtin_string_fill_bang(const Lex* e, const Cell* a);
Cell* builtin_string_number(const Lex* e, const Cell* a);
Cell* builtin_number_string(const Lex* e, const Cell* a);
Cell* builtin_string_downcase(const Lex* e, const Cell* a);
Cell* builtin_string_upcase(const Lex* e, const Cell* a);
Cell* builtin_string_foldcase(const Lex* e, const Cell* a);
Cell* builtin_string_equal_ci(const Lex* e, const Cell* a);
Cell* builtin_string_lt_ci(const Lex* e, const Cell* a);
Cell* builtin_string_lte_ci(const Lex* e, const Cell* a);
Cell* builtin_string_gt_ci(const Lex* e, const Cell* a);
Cell* builtin_string_gte_ci(const Lex* e, const Cell* a);
Cell* builtin_string_split(const Lex* e, const Cell* a);

#endif //COZENAGE_STRINGS_H
