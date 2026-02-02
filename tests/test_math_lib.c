/*
 * 'tests/test_math_lib.c'
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

#include "test_meta.h"
#include <math.h>
#include <criterion/criterion.h>
#include <criterion/new/assert.h>


TestSuite(end_to_end_math_lib);

Test(end_to_end_math_lib, test_cos, .init = setup_each_test, .fini = teardown_each_test) {

    long double v = t_eval_math_lib("(cos 0.0)");

    cr_assert(eq(int, !isnan(v), 1));
    cr_assert(eq(int, isfinite(v), 1));

    fprintf(stderr, "%Lg\n", v);

    // cr_assert(epsilon_eq(ldbl,
    //     t_eval_math_lib("(cos 0.0)"),
    //     1.0L,
    //     1e-14L));

    long double x = 0.7L;
    cr_assert(ieee_ulp_eq(ldbl,
        t_eval_math_lib("(cos 0.7)"),
        cosl(x),
        4));

    long double y = t_eval_math_lib("(cos -0.7)");
    cr_assert(y >= -1.0L && y <= 1.0L);
}

Test(end_to_end_math_lib, test_sin, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert(epsilon_eq(ldbl,
        t_eval_math_lib("(sin 0.0)"),
        0.0L,
        1e-18L));

    long double x = 0.7L;
    cr_assert(ieee_ulp_eq(ldbl,
        t_eval_math_lib("(sin 0.7)"),
        sinl(x),
        4));

    cr_assert(ieee_ulp_eq(ldbl,
        t_eval_math_lib("(sin -0.7)"),
       -sinl(x),
        4));
}

Test(end_to_end_math_lib, test_tan, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert(epsilon_eq(ldbl,
        t_eval_math_lib("(tan 0.0)"),
        0.0L,
        1e-18L));

    long double x = 0.5L;
    cr_assert(ieee_ulp_eq(ldbl,
        t_eval_math_lib("(tan 0.5)"),
        tanl(x),
        4));
}

Test(end_to_end_math_lib, test_acos, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert(epsilon_eq(ldbl,
        t_eval_math_lib("(acos 1.0)"),
        0.0L,
        1e-18L));

    long double x = 0.3L;
    long double v = t_eval_math_lib("(acos 0.3)");

    cr_assert(ieee_ulp_eq(ldbl, cosl(v), x, 4));
}

Test(end_to_end_math_lib, test_asin, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert(epsilon_eq(ldbl,
        t_eval_math_lib("(asin 0.0)"),
        0.0L,
        1e-18L));

    long double x = 0.3L;
    long double v = t_eval_math_lib("(asin 0.3)");

    cr_assert(ieee_ulp_eq(ldbl, sinl(v), x, 4));
}

Test(end_to_end_math_lib, test_atan, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert(epsilon_eq(ldbl,
        t_eval_math_lib("(atan 0.0)"),
        0.0L,
        1e-18L));

    long double x = 0.7L;
    long double v = t_eval_math_lib("(atan 0.7)");

    cr_assert(ieee_ulp_eq(ldbl, tanl(v), x, 4));
}

Test(end_to_end_math_lib, test_exp, .init = setup_each_test, .fini = teardown_each_test) {
    // cr_assert(epsilon_eq(ldbl,
    //     t_eval_math_lib("(exp 0.0)"),
    //     1.0L,
    //     1e-18L));

    long double x = 0.5L;
    long double v = t_eval_math_lib("(exp 0.5)");

    cr_assert(ieee_ulp_eq(ldbl, logl(v), x, 4));
}

Test(end_to_end_math_lib, test_log, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert(epsilon_eq(ldbl,
        t_eval_math_lib("(log 1.0)"),
        0.0L,
        1e-18L));

    long double x = 1.7L;
    long double v = t_eval_math_lib("(log 1.7)");

    cr_assert(ieee_ulp_eq(ldbl, expl(v), x, 4));
}

// Test(end_to_end_math_lib, test_log2, .init = setup_each_test, .fini = teardown_each_test) {
//     cr_assert(epsilon_eq(ldbl,
//         t_eval_math_lib("(log2 8.0)"),
//         3.0L,
//         1e-18L));
// }

// Test(end_to_end_math_lib, test_log10, .init = setup_each_test, .fini = teardown_each_test) {
//     cr_assert(epsilon_eq(ldbl,
//         t_eval_math_lib("(log10 1000.0)"),
//         3.0L,
//         1e-18L));
// }

Test(end_to_end_math_lib, test_cbrt, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert(epsilon_eq(ldbl,
        t_eval_math_lib("(cbrt 0.0)"),
        0.0L,
        1e-18L));

    // long double x = 2.0L;
    // cr_assert(ieee_ulp_eq(ldbl,
    //     t_eval_math_lib("(cbrt 8.0)"),
    //     x,
    //     4));
}

