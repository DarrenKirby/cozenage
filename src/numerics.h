/*
 * 'src/numerics.h'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 - 2026 Darren Kirby <darren@dragonbyte.ca>
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

#ifndef COZENAGE_NUMERICS_H
#define COZENAGE_NUMERICS_H

#include "cell.h"


/* Basic arithmetic operators */
Cell* builtin_add(const Lex* e, const Cell* a);
Cell* builtin_sub(const Lex* e, const Cell* a);
Cell* builtin_mul(const Lex* e, const Cell* a);
Cell* builtin_div(const Lex* e, const Cell* a);
/* Generic numeric operations */
Cell* builtin_abs(const Lex* e, const Cell* a);
Cell* builtin_expt(const Lex* e, const Cell* a);
Cell* builtin_modulo(const Lex* e, const Cell* a);
Cell* builtin_quotient(const Lex* e, const Cell* a);
Cell* builtin_remainder(const Lex* e, const Cell* a);
Cell* builtin_max(const Lex* e, const Cell* a);
Cell* builtin_min(const Lex* e, const Cell* a);
Cell* builtin_floor(const Lex* e, const Cell* a);
Cell* builtin_ceiling(const Lex* e, const Cell* a);
Cell* builtin_round(const Lex* e, const Cell* a);
Cell* builtin_truncate(const Lex* e, const Cell* a);
Cell* builtin_numerator(const Lex* e, const Cell* a);
Cell* builtin_denominator(const Lex* e, const Cell* a);
Cell* builtin_rationalize(const Lex* e, const Cell* a);
Cell* builtin_square(const Lex* e, const Cell* a);
Cell* builtin_sqrt(const Lex* e, const Cell* a);
Cell* builtin_exact_integer_sqrt(const Lex* e, const Cell* a);
Cell* builtin_exact(const Lex* e, const Cell* a);
Cell* builtin_inexact(const Lex* e, const Cell* a);
Cell* builtin_infinite(const Lex* e, const Cell* a);
Cell* builtin_finite(const Lex* e, const Cell* a);
Cell* builtin_nan(const Lex* e, const Cell* a);
Cell* builtin_lcm(const Lex* e, const Cell* a);
Cell* builtin_gcd(const Lex* e, const Cell* a);

#endif //COZENAGE_NUMERICS_H
