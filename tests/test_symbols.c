/*
 * 'tests/test_symbols.c'
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

#include "test_meta.h"
#include <criterion/criterion.h>


TestSuite(end_to_end_symbols);

Test(end_to_end_symbols, test_symbol_interning, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. Basic Uniqueness: (eq? 'a 'a) */
    /* This tests if make_cell_symbol returns the exact same pointer */
    cr_assert_str_eq(t_eval("(if (eq? 'apple 'apple) #true #false)"), "#true");

    /* 2. Symbol vs String: Symbols are NOT strings */
    cr_assert_str_eq(t_eval("(if (eq? 'apple \"apple\") #true #false)"), "#false");

    /* 3. Case Sensitivity (R7RS) */
    cr_assert_str_eq(t_eval("(if (eq? 'Apple 'apple) #true #false)"), "#false");

    /* 4. UTF-8 Symbols */
    cr_assert_str_eq(t_eval("(if (eq? 'λ 'λ) #true #false)"), "#true");
    cr_assert_str_eq(t_eval("(if (eq? 'π 'π) #true #false)"), "#true");

    /* 5. symbol=? predicate (Your variadic version) */
    cr_assert_str_eq(t_eval("(symbol=? 'a 'a 'a)"), "#true");
    cr_assert_str_eq(t_eval("(symbol=? 'a 'a 'b)"), "#false");

    /* 6. Special Form Identity (Checking if sf_id is stable) */
    /* Even if 'if' is a special form, it's still a symbol that should intern */
    cr_assert_str_eq(t_eval("(if (eq? 'if 'if) #true #false)"), "#true");
}

Test(end_to_end_symbols, test_symbol_conversion, .init = setup_each_test, .fini = teardown_each_test) {
    /* 1. string->symbol Basic */
    cr_assert_str_eq(t_eval("(if (symbol? (string->symbol \"apple\")) #true #false)"), "#true");

    /* 2. string->symbol Interning Check */
    /* If we convert a string to a symbol, it should be eq? to a quoted symbol of the same name */
    cr_assert_str_eq(t_eval("(if (eq? (string->symbol \"banana\") 'banana) #true #false)"), "#true");

    /* 3. symbol->string Basic */
    cr_assert_str_eq(t_eval("(symbol->string 'cherry)"), "\"cherry\"");

    /* 4. Round-trip: string -> symbol -> string */
    cr_assert_str_eq(t_eval("(symbol->string (string->symbol \"date\"))"), "\"date\"");

    /* 5. Round-trip: symbol -> string -> symbol */
    cr_assert_str_eq(t_eval("(if (eq? (string->symbol (symbol->string 'elderberry)) 'elderberry) #true #false)"), "#true");

    /* 6. UTF-8 Symbols via string conversion */
    /* Testing that multi-byte strings correctly intern as symbols */
    cr_assert_str_eq(t_eval("(if (eq? (string->symbol \"π\") 'π) #true #false)"), "#true");

    /* 7. Special Characters in Symbols */
    /* Symbols can have names that are usually difficult to type directly */
    cr_assert_str_eq(t_eval("(symbol->string (string->symbol \"a b c\"))"), "\"a b c\"");
}

