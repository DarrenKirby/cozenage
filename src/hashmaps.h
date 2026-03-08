/*
 * 'src/hashmaps.h'
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


#ifndef COZENAGE_HASHMAPS_H
#define COZENAGE_HASHMAPS_H

#include "cell.h"

Cell* builtin_hash(const Lex* e, const Cell* a);
Cell* builtin_hash_copy(const Lex* e, const Cell* a);
Cell* builtin_hash_clear(const Lex* e, const Cell* a);
Cell* builtin_hash_get(const Lex* e, const Cell* a);
Cell* builtin_hash_add(const Lex* e, const Cell* a);
Cell* builtin_hash_remove(const Lex* e, const Cell* a);
Cell* builtin_hash_keys(const Lex* e, const Cell* a);
Cell* builtin_hash_values(const Lex* e, const Cell* a);
Cell* builtin_hash_to_alist(const Lex* e, const Cell* a);
Cell* builtin_alist_to_hash(const Lex* e, const Cell* a);
Cell* builtin_hash_keys_map(const Lex* e, const Cell* a);
Cell* builtin_hash_keys_foreach(const Lex* e, const Cell* a);
Cell* builtin_hash_values_map(const Lex* e, const Cell* a);
Cell* builtin_hash_values_foreach(const Lex* e, const Cell* a);
Cell* builtin_hash_items_map(const Lex* e, const Cell* a);
Cell* builtin_hash_items_foreach(const Lex* e, const Cell* a);

#endif //COZENAGE_HASHMAPS_H
