/*
 * 'src/bignum.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 - 2026  Darren Kirby <darren@dragonbyte.ca>
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

/* This file defines the bigint operators, and helper functions for
 * promotion and demotion.
 */

#include "bignum.h"
#include "cell.h"

#include <gmp.h>
#include <stdint.h>


/* bigint promotion check helpers. */
bool add_will_overflow_i64(const int64_t a, const int64_t b, int64_t *out) {
    return __builtin_add_overflow(a, b, out);
}


bool sub_will_overflow_i64(const int64_t a, const int64_t b, int64_t *out) {
    return __builtin_sub_overflow(a, b, out);
}


bool mul_will_overflow_i64(const int64_t a, const int64_t b, int64_t *out) {
    return __builtin_mul_overflow(a, b, out);
}


bool div_will_overflow_i64(const int64_t a, const int64_t b, int64_t *out) {
    if (b == 0) { return true; } /* division by zero. */
    if (a == INT64_MIN && b == -1) return true;
    *out = a / b;
    return false;
}


/* bigint demotion check helpers. */
static bool mpz_fits_int64(const mpz_t z) {
    mpz_t tmp_max, tmp_min;
    mpz_init_set_si(tmp_max, 0);
    mpz_set_ui(tmp_max, INT64_MAX);
    mpz_init(tmp_min);
    mpz_set_si(tmp_min, INT64_MIN);

    const int cmp_low = mpz_cmp(z, tmp_min);
    const int cmp_high = mpz_cmp(z, tmp_max);

    mpz_clear(tmp_max);
    mpz_clear(tmp_min);
    return cmp_low >= 0 && cmp_high <= 0;
}


static int64_t mpz_get_i64_checked(const mpz_t z) {
    /* If mpz_fits_int64 returns true; then calling this is safe. */
    return mpz_get_si(z);
}


/* bigint arithmetic procedures. */
Cell* bigint_add(Cell* a, const Cell* b)
{
    mpz_add(*a->bi, *a->bi, *b->bi);
    return a;
}


Cell* bigint_sub(Cell* a, const Cell* b)
{
    mpz_sub(*a->bi, *a->bi, *b->bi);
    /* Check if we can demote back to int64_t. */
    if (mpz_fits_int64(*a->bi)) {
        const int64_t v = mpz_get_i64_checked(*a->bi);
        return make_cell_integer(v);
    }
    return a;
}


Cell* bigint_mul(Cell* a, const Cell* b)
{
    mpz_mul(*a->bi, *a->bi, *b->bi);
    return a;
}


Cell* bigint_div(Cell* a, const Cell* b) {
    mpz_div(*a->bi, *a->bi, *b->bi);
    /* Check if we can demote back to int64_t. */
    if (mpz_fits_int64(*a->bi)) {
        const int64_t v = mpz_get_i64_checked(*a->bi);
        return make_cell_integer(v);
    }
    return a;
}


Cell* bigint_quo_rem(Cell* a, Cell* b, const qr_t op) {
    Cell* result = cell_copy(a);
    Cell* d;
    if (b->type == CELL_INTEGER) {
        d = make_cell_bigint(nullptr, b, 10);
    } else {
        d = b;
    }
    if (op == QR_QUOTIENT) {
        mpz_tdiv_q(*result->bi, *a->bi, *d->bi);
    } else {
        mpz_tdiv_r(*result->bi, *a->bi, *d->bi);
    }

    if (mpz_fits_int64(*result->bi)) {
        return make_cell_integer(mpz_get_i64_checked(*result->bi));
    }
    return result;
}


Cell* bigint_exact_int_sqrt(Cell* a) {
    Cell* rop1 = make_cell_bigint("0", nullptr, 10);
    Cell* rop2 = make_cell_bigint("0", nullptr, 10);
    mpz_sqrtrem(*rop1->bi, *rop2->bi, *a->bi);

    if (mpz_fits_int64(*rop1->bi)) {
        rop1 = make_cell_integer(mpz_get_i64_checked(*rop1->bi));
    }
    if (mpz_fits_int64(*rop2->bi)) {
        rop2 = make_cell_integer(mpz_get_i64_checked(*rop2->bi));
    }

    return make_list_from_sexpr(make_sexpr_len2(rop1, rop2));
}


Cell* bigint_mod(Cell* a, Cell* b) {
    Cell* result = cell_copy(a);
    if (b->type == CELL_INTEGER) {
        mpz_mod_ui(*result->bi, *a->bi, b->integer_v);
    } else {
        mpz_mod(*result->bi, *a->bi, *b->bi);
    }
    if (mpz_fits_int64(*result->bi)) {
        return make_cell_integer(mpz_get_i64_checked(*result->bi));
    }
    return result;
}


Cell* bigint_neg(Cell* a)
{
    mpz_neg(*a->bi, *a->bi);
    return a;
}


Cell* bigint_expt(Cell* a, const int exp)
{
    mpz_pow_ui(*a->bi, *a->bi, exp);
    return a;
}
