/*
 * 'bignum.h'
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

#ifndef COZENAGE_BIGNUM_H
#define COZENAGE_BIGNUM_H

#include "types.h"

#include <gmp.h>


bool add_will_overflow_i64(int64_t a, int64_t b, int64_t *out);
bool sub_will_overflow_i64(int64_t a, int64_t b, int64_t *out);
bool mul_will_overflow_i64(int64_t a, int64_t b, int64_t *out);
bool div_will_overflow_i64(int64_t a, int64_t b, int64_t *out);

bool mpz_fits_int64(const mpz_t z);
int64_t mpz_get_i64_checked(const mpz_t z);

Cell* bigint_add(Cell* a, const Cell* b);
Cell* bigint_sub(Cell* a, const Cell* b);
Cell* bigint_mul(Cell* a, const Cell* b);
Cell* bigint_div(Cell* a, const Cell* b);
Cell* bigint_mod(Cell* a, const Cell* b);
Cell* bigint_expt(Cell* a, int exp);

Cell* bigint_neg(Cell* a);

#endif //COZENAGE_BIGNUM_H
