/*
 * 'pairs.h'
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

#ifndef COZENAGE_PAIRS_H
#define COZENAGE_PAIRS_H

#include "cell.h"


/* Helpers */
Cell* car__(const Cell* list);
Cell* cdr__(const Cell* list);
/* pair/list constructors, selectors, and procedures */
Cell* builtin_cons(const Lex* e, const Cell* a);
Cell* builtin_car(const Lex* e, const Cell* a);
Cell* builtin_cdr(const Lex* e, const Cell* a);
Cell* builtin_caar(const Lex* e, const Cell* a);
Cell* builtin_cadr(const Lex* e, const Cell* a);
Cell* builtin_cdar(const Lex* e, const Cell* a);
Cell* builtin_cddr(const Lex* e, const Cell* a);
Cell* builtin_list(const Lex* e, const Cell* a);
Cell* builtin_set_car(const Lex* e, const Cell* a);
Cell* builtin_set_cdr(const Lex* e, const Cell* a);
Cell* builtin_list_length(const Lex* e, const Cell* a);
Cell* builtin_list_ref(const Lex* e, const Cell* a);
Cell* builtin_list_append(const Lex* e, const Cell* a);
Cell* builtin_list_reverse(const Lex* e, const Cell* a);
Cell* builtin_list_tail(const Lex* e, const Cell* a);
Cell* builtin_make_list(const Lex* e, const Cell* a);
Cell* builtin_list_set(const Lex* e, const Cell* a);
Cell* builtin_memq(const Lex* e, const Cell* a);
Cell* builtin_memv(const Lex* e, const Cell* a);
Cell* builtin_filter(const Lex* e, const Cell* a);
Cell* builtin_foldl(const Lex* e, const Cell* a);

#endif //COZENAGE_PAIRS_H
