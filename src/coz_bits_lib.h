/*
 * 'coz_bits_lib.h'
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

#ifndef COZENAGE_COZ_BITS_LIB_H
#define COZENAGE_COZ_BITS_LIB_H

#include "types.h"

Cell* bits_left_shift(const Lex* e, const Cell* a);
Cell* bits_right_shift(const Lex* e, const Cell* a);
Cell* bits_bitwise_and(const Lex* e, const Cell* a);
Cell* bits_bitwise_or(const Lex* e, const Cell* a);
Cell* bits_bitwise_xor(const Lex* e, const Cell* a);
Cell* bits_bitwise_not(const Lex* e, const Cell* a);
Cell* bits_bitstring_to_int(const Lex* e, const Cell* a);
Cell* bits_int_to_bitstring(const Lex* e, const Cell* a);
void lex_add_coz_bits_lib(Lex* e);

#endif //COZENAGE_COZ_BITS_LIB_H