/*
 * 'src/sets.h'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2026 Darren Kirby <darren@dragonbyte.ca>
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


#ifndef COZENAGE_SETS_H
#define COZENAGE_SETS_H

#include "cell.h"

Cell* copy_hash_table(const Cell* t);
Cell* builtin_set(const Lex* e, const Cell* a);
Cell* builtin_set_copy(const Lex* e, const Cell* a);
Cell* builtin_set_add(const Lex* e, const Cell* a);
Cell* builtin_set_remove(const Lex* e, const Cell* a);
Cell* builtin_set_member(const Lex* e, const Cell* a);
Cell* builtin_set_is_disjoint(const Lex* e, const Cell* a);
Cell* builtin_set_is_subset(const Lex* e, const Cell* a);
Cell* builtin_set_is_superset(const Lex* e, const Cell* a);
Cell* builtin_set_union(const Lex* e, const Cell* a);
Cell* builtin_set_union_bang(const Lex* e, const Cell* a);
Cell* builtin_set_intersection(const Lex* e, const Cell* a);
Cell* builtin_set_intersection_bang(const Lex* e, const Cell* a);
Cell* builtin_set_difference(const Lex* e, const Cell* a);
Cell* builtin_set_difference_bang(const Lex* e, const Cell* a);
Cell* builtin_set_sym_difference(const Lex* e, const Cell* a);
Cell* builtin_set_sym_difference_bang(const Lex* e, const Cell* a);
Cell* builtin_set_map(const Lex* e, const Cell* a);
Cell* builtin_set_foreach(const Lex* e, const Cell* a);
Cell* builtin_list_to_set(const Lex* e, const Cell* a);
Cell* builtin_set_to_list(const Lex* e, const Cell* a);

#endif //COZENAGE_SETS_H
