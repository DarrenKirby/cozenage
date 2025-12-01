/*
 * 'polymorph.h'
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

#ifndef COZENAGE_POLYMORPH_H
#define COZENAGE_POLYMORPH_H

#include "types.h"

/* Macro to reverse bytevector array by type */
#define REVERSE_CASE(C_TYPE) \
do { \
C_TYPE* arr = (C_TYPE*)v->bv->data; \
for (int i = len - 1; i >= 0; i--) \
byte_add(result, arr[i]); \
} while (0)

Cell* builtin_len(const Lex* e, const Cell* a);
Cell* builtin_idx(const Lex* e, const Cell* a);
Cell* builtin_rev(const Lex* e, const Cell* a);

#endif //COZENAGE_POLYMORPH_H
