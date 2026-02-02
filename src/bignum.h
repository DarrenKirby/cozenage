/*
 * 'src/bignum.h'
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

#ifndef COZENAGE_BIGNUM_H
#define COZENAGE_BIGNUM_H

#include "types.h"


/* Flag for bigint_quo_rem(). */
typedef enum : uint8_t {
    QR_QUOTIENT,
    QR_REMAINDER
} qr_t;


/* Helpers to check if operation will overflow. */
bool add_will_overflow_i64(int64_t a, int64_t b, int64_t *out);
bool sub_will_overflow_i64(int64_t a, int64_t b, int64_t *out);
bool mul_will_overflow_i64(int64_t a, int64_t b, int64_t *out);
bool div_will_overflow_i64(int64_t a, int64_t b, int64_t *out);


Cell* bigint_add(Cell* a, const Cell* b);
Cell* bigint_sub(Cell* a, const Cell* b);
Cell* bigint_mul(Cell* a, const Cell* b);
Cell* bigint_div(Cell* a, const Cell* b);
Cell* bigint_quo_rem(Cell* a, Cell* b, qr_t op);
Cell* bigint_exact_int_sqrt(Cell* a);
Cell* bigint_mod(Cell* a, Cell* b);
Cell* bigint_expt(Cell* a, int exp);
Cell* bigint_neg(Cell* a);

#endif //COZENAGE_BIGNUM_H
