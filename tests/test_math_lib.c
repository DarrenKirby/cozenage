/*
 * 'tests/test_math_lib.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 Darren Kirby <darren@dragonbyte.ca>
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
#include <criterion/criterion.h>


TestSuite(end_to_end_math_lib);

Test(end_to_end_math_lib, test_cos, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert_str_eq(t_eval_lib("math","(cos 1.0)"), "0.54030230586814");
    cr_assert_str_eq(t_eval_lib("math","(cos 0.2)"), "0.980066577841242");
    cr_assert_str_eq(t_eval_lib("math","(cos 0)"), "1");
    cr_assert_str_eq(t_eval_lib("math","(cos 90)"), "-0.44807361612917");
    cr_assert_str_eq(t_eval_lib("math","(cos 180)"), "-0.598460069057858");
    cr_assert_str_eq(t_eval_lib("math","(cos 270)"), "0.984381950632505");
}
