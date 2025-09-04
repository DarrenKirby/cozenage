#include "test_meta.h"
#include <criterion/criterion.h>


TestSuite(end_to_end_numerics, .init = suite_setup_wrapper, .fini = teardown_suite);

Test(end_to_end_numerics, test_add_integer) {
    /* Test integer addition. */
    eval_and_check("(+)", "0");
    eval_and_check("(+ 1)", "1");
    eval_and_check("(+ 1 1)", "2");
    eval_and_check("(+ 1 -1)", "0");
    eval_and_check("(+ 12345 54321)", "66666");
    eval_and_check("(+ -123 500)", "377");
    eval_and_check("(+ -250 -250)", "-500");
    eval_and_check("(+ #xff #x20)", "287");
    eval_and_check("(+ #b010101 #b110101)", "74");
    eval_and_check("(+ #d123 #d888)", "1011");
    eval_and_check("(+ #o111 #o777)", "584");
    eval_and_check("(+ 1 2 3 4 5 6 7)", "28");
    eval_and_check("(+ 1 -2 3 -4 5 -6 7 -8)", "-4");
    eval_and_check("(+ #b1010 #o444 #d100 #xff)", "657");
    eval_and_check("(+ 1 (+ 2 (+ 3 4)))", "10");
    eval_and_check("(+ #xdead #xbee #xfeed #xcede #xbead)", "227091");

}

Test(end_to_end_numerics, test_add_rational) {
    /* Test rational number addition. */
    eval_and_check("(+ 1/1 1/1)", "2");
    eval_and_check("(+ -1/1 -1/1)", "-2");
    eval_and_check("(+ 1/4 1/4 1/4 1/4)", "1");
    eval_and_check("(+ 1/2 1/4 1/8 1/16 1/32 1/64)", "63/64");
    eval_and_check("(+ 5/4 2/3 6/7)", "233/84");
    eval_and_check("(+ -5/4 -2/3 -6/7)", "-233/84");
    eval_and_check("(+ 3/4 3/4 3/4 3/4)", "3");
    eval_and_check("(+ 466/885 34/57)", "18884/16815");
    eval_and_check("(+ 22/7 5087/10752)", "38879/10752");
}

// Test(end_to_end_numerics, test_add_real) {
//     /* Test integer addition. */
//     eval_and_check("", "");
//     eval_and_check("", "");
//     eval_and_check("", "");
//     eval_and_check("", "");
//     eval_and_check("", "");
//     eval_and_check("", "");
//     eval_and_check("", "");
//     eval_and_check("", "");
//     eval_and_check("", "");
// }
//
// Test(end_to_end_numerics, test_add_complex) {
//     /* Test integer addition. */
//     eval_and_check("", "");
//     eval_and_check("", "");
//     eval_and_check("", "");
//     eval_and_check("", "");
//     eval_and_check("", "");
//     eval_and_check("", "");
//     eval_and_check("", "");
//     eval_and_check("", "");
//     eval_and_check("", "");
// }




