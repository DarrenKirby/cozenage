#include "test_meta.h"
#include <criterion/criterion.h>


TestSuite(end_to_end_numerics, .init = suite_setup_wrapper, .fini = teardown_suite);

Test(end_to_end_numerics, test_add_integer) {
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

Test(end_to_end_numerics, test_add_real) {
    eval_and_check("(+ 1.0 1.0)", "2.0");
    eval_and_check("(+ 2.5 3.5)", "6.0");
    eval_and_check("(+ 0.5 0.25)", "0.75");
    eval_and_check("(+ 1.0)", "1.0");
    eval_and_check("(+ 1.1 2.2 3.3)", "6.6");
    eval_and_check("(+ 1.0 2.0 3.0 4.0 5.0)", "15.0");
    eval_and_check("(+)", "0.0");
    eval_and_check("(+ 0.0 5.5)", "5.5");
    eval_and_check("(+ 5.5 0.0)", "5.5");
    eval_and_check("(+ 0.0 0.0)", "0.0");
    eval_and_check("(+ -1.0 -2.0)", "-3.0");
    eval_and_check("(+ -5.5 2.5)", "-3.0");
    eval_and_check("(+ 5.5 -2.5)", "3.0");
    eval_and_check("(+ 0.1 0.2)", "0.3");
    eval_and_check("(+ 1.000001 2.000002)", "3.000003");
}

Test(end_to_end_numerics, test_add_complex) {
    eval_and_check("(+ 1+2i 3+4i)", "4+6i");
    eval_and_check("(+ 10-5i 3+2i)", "13-3i");
    eval_and_check("(+ -5+10i 2-3i)", "-3+7i");
    eval_and_check("(+ -1-1i -2-2i)", "-3-3i");
    eval_and_check("(+ 5i 8i)", "0+13i");
    eval_and_check("(+ 10+2i 3i)", "10+5i");
    eval_and_check("(+ -12i 5i)", "0-7i");
    eval_and_check("(+ 5-10i -2i)", "5-12i");
    eval_and_check("(+ 5+2i 3-2i)", "8+0i");
    eval_and_check("(+ 5+2i -5+3i)", "0+5i");
    eval_and_check("(+ 10+5i)", "10+5i");
    eval_and_check("(+ 1+1i 2+2i 3+3i)", "6+6i");
    eval_and_check("(+ 0+0i 5+5i)", "5+5i");
    eval_and_check("(+ 5+5i 0+0i)", "5+5i");
    eval_and_check("(+ 10-20i -10+20i)", "0+0i");
}
Test(end_to_end_numerics, test_add_mixed) {
    eval_and_check("(+ 5 2.5)", "7.5");
    eval_and_check("(+ 2.5 5)", "7.5");
    eval_and_check("(+ 10 1+2i)", "11+2i");
    eval_and_check("(+ 1+2i 10)", "11+2i");
    eval_and_check("(+ 1 2 3.5)", "6.5");
    eval_and_check("(+ 1 2.5 3)", "6.5");
    eval_and_check("(+ 1 2.5 3+4i)", "6.5+4i");
    eval_and_check("(+ 1+1i 2.5 3)", "6.5+1i");
    eval_and_check("(+ 5.5 2 3i)", "7.5+3i");
    eval_and_check("(+ 1+2i 2-2i 3.0)", "6.0+0i");
    eval_and_check("(+ 5 -2.5 -2.5)", "0.0");
    eval_and_check("(+ 10 5i -10)", "0+5i");
}

Test(end_to_end_numerics, test_sub_integer) {
    eval_and_check("(- 10)", "-10");
    eval_and_check("(- -10)", "10");
    eval_and_check("(- 0)", "0");
    eval_and_check("(- #xff)", "-255");
    eval_and_check("(- 8 5)", "3");
    eval_and_check("(- 5 8)", "-3");
    eval_and_check("(- 10 -2)", "12");
    eval_and_check("(- -10 2)", "-12");
    eval_and_check("(- -10 -2)", "-8");
    eval_and_check("(- 12345 54321)", "-41976");
    eval_and_check("(- 500 -123)", "623");
    eval_and_check("(- -250 -250)", "0");
    eval_and_check("(- #xff #x20)", "223");
    eval_and_check("(- #b110101 #b010101)", "32");
    eval_and_check("(- #d888 #d123)", "765");
    eval_and_check("(- #o777 #o111)", "438");
    eval_and_check("(- 10 1 2 3)", "4");
    eval_and_check("(- 100 10 -20 5)", "105");
    eval_and_check("(- 0 5 10)", "-15");
    eval_and_check("(- 10 1 2 3 4)", "0");
    eval_and_check("(- #xff #o444 #b1010)", "-47");
    eval_and_check("(- 100 (- 50 25))", "75");
    eval_and_check("(- (- 100 50) 25)", "25");
    eval_and_check("(- #xdead (- #xfeed #xcede))", "44702");
}

Test(end_to_end_numerics, test_sub_rational) {
    eval_and_check("(- 1/2)", "-1/2");
    eval_and_check("(- -3/4)", "3/4");
    eval_and_check("(- 5/10)", "-1/2");
    eval_and_check("(- 0/8)", "0");
    eval_and_check("(- 5/-8)", "5/8");
    eval_and_check("(- 3/5 1/5)", "2/5");
    eval_and_check("(- 1/2 1/3)", "1/6");
    eval_and_check("(- 1/4 3/4)", "-1/2");
    eval_and_check("(- 1/2 -1/3)", "5/6");
    eval_and_check("(- -5/8 1/8)", "-3/4");
    eval_and_check("(- 7/8 1/8)", "3/4");
    eval_and_check("(- 3/2 1/2)", "1");
    eval_and_check("(- 5/2 1/2)", "2");
    eval_and_check("(- 1/3 1/3)", "0");
    eval_and_check("(- 10/4 1/2)", "2");
    eval_and_check("(- 1/2 5/2)", "-2");
    eval_and_check("(- 1/3 5/-6)", "7/6");
    eval_and_check("(- 2/-5 1/5)", "-3/5");
    eval_and_check("(- 1/1 1/2 1/4)", "1/4");
    eval_and_check("(- 10/3 1/3 2/3)", "7/3");
    eval_and_check("(- 1/2 1/3 1/6)", "0");
    eval_and_check("(- 2/1 1/5 2/5 3/5)", "4/5");
    eval_and_check("(- 1/1 (- 1/2 1/4))", "3/4");
    eval_and_check("(- (- 1/1 1/2) 1/4)", "1/4");
}

Test(end_to_end_numerics, test_sub_real) {
    eval_and_check("(- 10.5)", "-10.5");
    eval_and_check("(- -2.25)", "2.25");
    eval_and_check("(- 0.0)", "-0.0");
    eval_and_check("(- 8.5 5.0)", "3.5");
    eval_and_check("(- 5.0 8.5)", "-3.5");
    eval_and_check("(- 10.0 -2.5)", "12.5");
    eval_and_check("(- -10.0 2.5)", "-12.5");
    eval_and_check("(- -10.0 -2.5)", "-7.5");
    eval_and_check("(- 0.75 0.25)", "0.5");
    eval_and_check("(- 5.5 0.0)", "5.5");
    eval_and_check("(- 0.0 5.5)", "-5.5");
    eval_and_check("(- 3.3 3.3)", "0.0");
    eval_and_check("(- 10.0 1.5 2.5)", "6.0");
    eval_and_check("(- 20.0 5.0 -2.5 1.5)", "16.0");
    eval_and_check("(- 0.0 5.5 4.5)", "-10.0");
    eval_and_check("(- 10.0 1.0 2.0 3.0 4.0)", "0.0");
    eval_and_check("(- 0.3 0.1)", "0.2");
    eval_and_check("(- 5.000008 3.000002)", "2.000006");
    eval_and_check("(- 100.0 (- 50.0 25.0))", "75.0");
    eval_and_check("(- (- 100.0 50.0) 25.0)", "25.0");
}

Test(end_to_end_numerics, test_sub_complex) {
    eval_and_check("(- 1+2i)", "-1-2i");
    eval_and_check("(- -3-4i)", "3+4i");
    eval_and_check("(- 10-0i)", "-10+0i");
    eval_and_check("(- 5i)", "0-5i");
    eval_and_check("(- -12i)", "0+12i");
    eval_and_check("(- 5+3i 2+1i)", "3+2i");
    eval_and_check("(- 2+0i 5+3i)", "-3-3i");
    eval_and_check("(- 10+5i -2-3i)", "12+8i");
    eval_and_check("(- -8-2i -1-0i)", "-7-2i");
    eval_and_check("(- 10+2i 3-4i)", "7+6i");
    eval_and_check("(- 12i 5i)", "0+7i");
    eval_and_check("(- 10+8i 3i)", "10+5i");
    eval_and_check("(- 10+0i 5i)", "10-5i");
    eval_and_check("(- 5+10i 0+10i)", "5+0i");
    eval_and_check("(- 5+10i 5+0i)", "0+10i");
    eval_and_check("(- 10+2i 10-2i)", "0+4i");
    eval_and_check("(- 5-3i -5-3i)", "10+0i");
    eval_and_check("(- 3+2i 3+2i)", "0+0i");
    eval_and_check("(- 10+10i 2+2i 1+1i)", "7+7i");
    eval_and_check("(- 20+5i 10+0i -2-3i)", "12+8i");
    eval_and_check("(- 0+0i 5+5i 2-3i)", "-7-2i");
    eval_and_check("(- 10+10i (- 5+5i 2+3i))", "7+8i");
    eval_and_check("(- (- 10+10i 5+5i) 2+3i)", "3+2i");
}

Test(end_to_end_numerics, test_sub_mixed) {
    eval_and_check("(- 5 1/2)", "9/2");
    eval_and_check("(- 1/2 5)", "-9/2");
    eval_and_check("(- 10 1/2 1/4)", "37/4");
    eval_and_check("(- 10 2.5)", "7.5");
    eval_and_check("(- 2.5 10)", "-7.5");
    eval_and_check("(- 5/2 1.0)", "1.5");
    eval_and_check("(- 1.0 1/2)", "0.5");
    eval_and_check("(- 10 0.5 1/4)", "9.25");
    eval_and_check("(- 10.0 1 1/2)", "8.5");
    eval_and_check("(- 5 1+2i)", "4-2i");
    eval_and_check("(- 1+2i 5)", "-4+2i");
    eval_and_check("(- 1/2 2-3i)", "-3/2+3i");
    eval_and_check("(- 2-3i 1/2)", "3/2-3i");
    eval_and_check("(- 10.5 1-0i)", "9.5+0i");
    eval_and_check("(- 1-0i 10.5)", "-9.5-1i");
    eval_and_check("(- 10 1/2 0.25)", "9.25");
    eval_and_check("(- 20 2.5 1+1i)", "16.5-1i");
    eval_and_check("(- 5/2 1.5 2+2i)", "-1.0-2i");
    eval_and_check("(- 1+10i 1 1/2 0.5)", "-1.0+10i");
    eval_and_check("(- 5.5 1/2)", "5.0");
    eval_and_check("(- 3/4 0.75)", "0.0");
    eval_and_check("(- 1+5i 1)", "0+5i");
    eval_and_check("(- 1.5+2i 0.5+2i)", "1.0+0i");
    eval_and_check("(- 10/4 0.5)", "2.0");
    eval_and_check("(- 10.0 (- 5 1/2))", "5.5");
    eval_and_check("(- (- 10.0 5) 1/2)", "4.5");
}

Test(end_to_end_numerics, test_mul_integer) {
    eval_and_check("(*)", "1");
    eval_and_check("(* 7)", "7");
    eval_and_check("(* -10)", "-10");
    eval_and_check("(* 5 5)", "25");
    eval_and_check("(* 5 -5)", "-25");
    eval_and_check("(* -5 -5)", "25");
    eval_and_check("(* 1 2 3 4 5)", "120");
    eval_and_check("(* 1 -2 3 -4 5)", "120");
    eval_and_check("(* 12345 0)", "0");
    eval_and_check("(* 1 2 3 4 5 0)", "0");
    eval_and_check("(* 0)", "0");
    eval_and_check("(* #xff 2)", "510");
    eval_and_check("(* #o10 #d8)", "64");
    eval_and_check("(* #b101 #d5)", "25");
    eval_and_check("(* #x10 #o10 #b10)", "256");
    eval_and_check("(* 2 (* 3 (* 4 5)))", "120");
}

Test(end_to_end_numerics, test_mul_rational) {
    eval_and_check("(* 1/2 1/2)", "1/4");
    eval_and_check("(* 3/4 4/3)", "1");
    eval_and_check("(* -1/2 3/4)", "-3/8");
    eval_and_check("(* -2/3 -3/4)", "1/2");
    eval_and_check("(* 1/2 2)", "1");
    eval_and_check("(* 4/5 15/2)", "6");
    eval_and_check("(* 1/3 1/3 1/3)", "1/27");
    eval_and_check("(* 123/456 0)", "0");
    eval_and_check("(* 1/2 1/3 0/1)", "0");
    eval_and_check("(* 7/8)", "7/8");
    eval_and_check("(* 1/1)", "1");
}

Test(end_to_end_numerics, test_mul_real) {
    eval_and_check("(* 2.5 2.0)", "5.0");
    eval_and_check("(* 0.5 0.5)", "0.25");
    eval_and_check("(* -1.5 3.0)", "-4.5");
    eval_and_check("(* -2.25 -2.0)", "4.5");
    eval_and_check("(* 1.5 2.0 3.0)", "9.0");
    eval_and_check("(* 10.0 0.1 5.0 -1.0)", "-5.0");
    eval_and_check("(* 99.9 0.0)", "0.0");
    eval_and_check("(* 0.0)", "0.0");
    eval_and_check("(* -123.456)", "-123.456");
}

Test(end_to_end_numerics, test_mul_complex) {
    eval_and_check("(* 2+3i 4+5i)", "-7+22i");
    eval_and_check("(* 1-0i 1+0i)", "2+0i");
    eval_and_check("(* 2+0i 0+2i)", "0+4i");
    eval_and_check("(* 3i 4i)", "-12+0i");
    eval_and_check("(* 1+0i 1+0i)", "0+2i");
    eval_and_check("(* 1-2i 3-4i)", "-5-10i");
    eval_and_check("(* 1+0i 2+0i 3i)", "-6+6i");
    eval_and_check("(* 123+456i 0+0i)", "0+0i");
    eval_and_check("(* 5-10i)", "5-10i");
}

Test(end_to_end_numerics, test_mul_mixed) {
    eval_and_check("(* 2 1/2)", "1");
    eval_and_check("(* 1/3 6)", "2");
    eval_and_check("(* 4 2.5)", "10.0");
    eval_and_check("(* 0.5 10)", "5.0");
    eval_and_check("(* 2 1+5i)", "2+10i");
    eval_and_check("(* 1/2 5.0)", "2.5");
    eval_and_check("(* 0.25 1/2)", "0.125");
    eval_and_check("(* 1/2 2+2i)", "1+1i");
    eval_and_check("(* 1.5 2+4i)", "3.0+6.0i");
    eval_and_check("(* 2 1/2 5.0)", "5.0");
    eval_and_check("(* 2 1/2 5.0 1+0i)", "5.0+5.0i");
    eval_and_check("(* 1/2 0.0)", "0.0");
    eval_and_check("(* 5.0 0+0i)", "0.0+0.0i");
}

Test(end_to_end_numerics, test_div_integer) {
    // Unary division (reciprocal)
    eval_and_check("(/ 10)", "1/10");
    eval_and_check("(/ -2)", "-1/2");
    eval_and_check("(/ 1)", "1");
    eval_and_check("(/ 10 2)", "5");
    eval_and_check("(/ -10 2)", "-5");
    eval_and_check("(/ -10 -2)", "5");
    eval_and_check("(/ 10 -2)", "-5");
    eval_and_check("(/ 10 3)", "10/3");
    eval_and_check("(/ 1 2)", "1/2");
    eval_and_check("(/ 2 4)", "1/2");
    eval_and_check("(/ 100 10 2)", "5");
    eval_and_check("(/ 81 3 3)", "9");
    eval_and_check("(/ 120 2 3 4)", "5");
    eval_and_check("(/ #x100 16)", "16");
    eval_and_check("(/ #o77 7)", "9");
}

Test(end_to_end_numerics, test_div_rational) {
    eval_and_check("(/ 5/4)", "4/5");
    eval_and_check("(/ -1/2)", "-2");
    eval_and_check("(/ 10/1)", "1/10");
    eval_and_check("(/ 1/2 1/4)", "2");
    eval_and_check("(/ 2/3 3/4)", "8/9");
    eval_and_check("(/ -1/2 1/2)", "-1");
    eval_and_check("(/ -3/4 -2/3)", "9/8");
    eval_and_check("(/ 1 1/2 1/2)", "4");
    eval_and_check("(/ 16/1 2/1 2/1)", "4");
    eval_and_check("(/ 1/2 2/1 4/1)", "1/16");
}

Test(end_to_end_numerics, test_div_real) {
    eval_and_check("(/ 2.0)", "0.5");
    eval_and_check("(/ -4.0)", "-0.25");
    eval_and_check("(/ 5.5)", "0.18181818181818181819");
    eval_and_check("(/ 10.0 4.0)", "2.5");
    eval_and_check("(/ 5.0 2.0)", "2.5");
    eval_and_check("(/ -10.0 4.0)", "-2.5");
    eval_and_check("(/ 9.9 3.3)", "3.0");
    eval_and_check("(/ 100.0 2.0 5.0)", "10.0");
    eval_and_check("(/ 25.0 -2.5 -2.0)", "5.0");
}

Test(end_to_end_numerics, test_div_complex) {
    eval_and_check("(/ 10+10i)", "0.05-0.05i");
    eval_and_check("(/ 1+0i)", "0.5-0.5i");
    eval_and_check("(/ 2i)", "0-0.5i");
    eval_and_check("(/ 8+6i 2+4i)", "2-1i");
    eval_and_check("(/ 2+4i 1+2i)", "2+0i");
    eval_and_check("(/ 10+20i 2+1i)", "8+6i");
    eval_and_check("(/ -16+0i 2+2i 2-2i)", "-2+0i");
    eval_and_check("(/ 20+10i 1+2i 2)", "2-3i");
}

Test(end_to_end_numerics, test_div_mixed) {
    eval_and_check("(/ 10 1/2)", "20");
    eval_and_check("(/ 1/2 10)", "1/20");
    eval_and_check("(/ 5 2.0)", "2.5");
    eval_and_check("(/ 5.0 2)", "2.5");
    eval_and_check("(/ 1/2 2.0)", "0.25");
    eval_and_check("(/ 2.0 1/2)", "4.0");
    eval_and_check("(/ 10 1+i)", "5-5i");
    eval_and_check("(/ 1/2 1+i)", "0.25-0.25i");
    eval_and_check("(/ 2.0 1+i)", "1.0-1.0i");
    eval_and_check("(/ 5+5i 2)", "2.5+2.5i");
    eval_and_check("(/ 100 2.0 5)", "10.0");
    eval_and_check("(/ 40+0i 2.0 5)", "4.0+0.0i");
}

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




