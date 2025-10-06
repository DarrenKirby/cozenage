/*
 * 'comparators.h'
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

#ifndef COZENAGE_COMPARATORS_H
#define COZENAGE_COMPARATORS_H

#include "cell.h"


/* Comparison operators */
Cell* builtin_eq_op(const Lex* e, const Cell* a);
Cell* builtin_gt_op(const Lex* e, const Cell* a);
Cell* builtin_lt_op(const Lex* e, const Cell* a);
Cell* builtin_gte_op(const Lex* e, const Cell* a);
Cell* builtin_lte_op(const Lex* e, const Cell* a);
/* Equality and equivalence comparators */
Cell* builtin_equal(const Lex* e, const Cell* a);
Cell* builtin_eqv(const Lex* e, const Cell* a);
Cell* builtin_eq(const Lex* e, const Cell* a);

#endif //COZENAGE_COMPARATORS_H
