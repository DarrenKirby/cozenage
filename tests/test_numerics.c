#include "test_meta.h"
#include <criterion/criterion.h>

TestSuite(end_to_end_numerics, .init = suite_setup_wrapper, .fini = teardown_suite);

Test(end_to_end_numerics, test_add_integer) {
    cr_assert_str_eq(t_eval("(+)"), "0");
    cr_assert_str_eq(t_eval("(+ 1)"), "1");
    cr_assert_str_eq(t_eval("(+ 1 1)"), "2");
    cr_assert_str_eq(t_eval("(+ 1 -1)"), "0");
    cr_assert_str_eq(t_eval("(+ 12345 54321)"), "66666");
    cr_assert_str_eq(t_eval("(+ -123 500)"), "377");
    cr_assert_str_eq(t_eval("(+ -250 -250)"), "-500");
    cr_assert_str_eq(t_eval("(+ #xff #x20)"), "287");
    cr_assert_str_eq(t_eval("(+ #b010101 #b110101)"), "74");
    cr_assert_str_eq(t_eval("(+ #d123 #d888)"), "1011");
    cr_assert_str_eq(t_eval("(+ #o111 #o777)"), "584");
    cr_assert_str_eq(t_eval("(+ 1 2 3 4 5 6 7)"), "28");
    cr_assert_str_eq(t_eval("(+ 1 -2 3 -4 5 -6 7 -8)"), "-4");
    cr_assert_str_eq(t_eval("(+ #b1010 #o444 #d100 #xff)"), "657");
    cr_assert_str_eq(t_eval("(+ 1 (+ 2 (+ 3 4)))"), "10");
    cr_assert_str_eq(t_eval("(+ #xdead #xbee #xfeed #xcede #xbead)"), "227091");
}

Test(end_to_end_numerics, test_add_rational) {
    cr_assert_str_eq(t_eval("(+ 1/1 1/1)"), "2");
    cr_assert_str_eq(t_eval("(+ -1/1 -1/1)"), "-2");
    cr_assert_str_eq(t_eval("(+ 1/4 1/4 1/4 1/4)"), "1");
    cr_assert_str_eq(t_eval("(+ 1/2 1/4 1/8 1/16 1/32 1/64)"), "63/64");
    cr_assert_str_eq(t_eval("(+ 5/4 2/3 6/7)"), "233/84");
    cr_assert_str_eq(t_eval("(+ -5/4 -2/3 -6/7)"), "-233/84");
    cr_assert_str_eq(t_eval("(+ 3/4 3/4 3/4 3/4)"), "3");
    cr_assert_str_eq(t_eval("(+ 466/885 34/57)"), "18884/16815");
    cr_assert_str_eq(t_eval("(+ 22/7 5087/10752)"), "38879/10752");
}

Test(end_to_end_numerics, test_add_real) {
    cr_assert_str_eq(t_eval("(+ 1.0 1.0)"), "2.0");
    cr_assert_str_eq(t_eval("(+ 2.5 3.5)"), "6.0");
    cr_assert_str_eq(t_eval("(+ 0.5 0.25)"), "0.75");
    cr_assert_str_eq(t_eval("(+ 1.0)"), "1.0");
    cr_assert_str_eq(t_eval("(+ 1.1 2.2 3.3)"), "6.6");
    cr_assert_str_eq(t_eval("(+ 1.0 2.0 3.0 4.0 5.0)"), "15.0");
    cr_assert_str_eq(t_eval("(+)"), "0.0");
    cr_assert_str_eq(t_eval("(+ 0.0 5.5)"), "5.5");
    cr_assert_str_eq(t_eval("(+ 5.5 0.0)"), "5.5");
    cr_assert_str_eq(t_eval("(+ 0.0 0.0)"), "0.0");
    cr_assert_str_eq(t_eval("(+ -1.0 -2.0)"), "-3.0");
    cr_assert_str_eq(t_eval("(+ -5.5 2.5)"), "-3.0");
    cr_assert_str_eq(t_eval("(+ 5.5 -2.5)"), "3.0");
    cr_assert_str_eq(t_eval("(+ 0.1 0.2)"), "0.3");
    cr_assert_str_eq(t_eval("(+ 1.000001 2.000002)"), "3.000003");
}

Test(end_to_end_numerics, test_add_complex) {
    cr_assert_str_eq(t_eval("(+ 1+2i 3+4i)"), "4+6i");
    cr_assert_str_eq(t_eval("(+ 10-5i 3+2i)"), "13-3i");
    cr_assert_str_eq(t_eval("(+ -5+10i 2-3i)"), "-3+7i");
    cr_assert_str_eq(t_eval("(+ -1-1i -2-2i)"), "-3-3i");
    cr_assert_str_eq(t_eval("(+ 5i 8i)"), "0+13i");
    cr_assert_str_eq(t_eval("(+ 10+2i 3i)"), "10+5i");
    cr_assert_str_eq(t_eval("(+ -12i 5i)"), "0-7i");
    cr_assert_str_eq(t_eval("(+ 5-10i -2i)"), "5-12i");
    cr_assert_str_eq(t_eval("(+ 5+2i 3-2i)"), "8+0i");
    cr_assert_str_eq(t_eval("(+ 5+2i -5+3i)"), "0+5i");
    cr_assert_str_eq(t_eval("(+ 10+5i)"), "10+5i");
    cr_assert_str_eq(t_eval("(+ 1+1i 2+2i 3+3i)"), "6+6i");
    cr_assert_str_eq(t_eval("(+ 0+0i 5+5i)"), "5+5i");
    cr_assert_str_eq(t_eval("(+ 5+5i 0+0i)"), "5+5i");
    cr_assert_str_eq(t_eval("(+ 10-20i -10+20i)"), "0+0i");
}
Test(end_to_end_numerics, test_add_mixed) {
    cr_assert_str_eq(t_eval("(+ 5 2.5)"), "7.5");
    cr_assert_str_eq(t_eval("(+ 2.5 5)"), "7.5");
    cr_assert_str_eq(t_eval("(+ 10 1+2i)"), "11+2i");
    cr_assert_str_eq(t_eval("(+ 1+2i 10)"), "11+2i");
    cr_assert_str_eq(t_eval("(+ 1 2 3.5)"), "6.5");
    cr_assert_str_eq(t_eval("(+ 1 2.5 3)"), "6.5");
    cr_assert_str_eq(t_eval("(+ 1 2.5 3+4i)"), "6.5+4i");
    cr_assert_str_eq(t_eval("(+ 1+1i 2.5 3)"), "6.5+1i");
    cr_assert_str_eq(t_eval("(+ 5.5 2 3i)"), "7.5+3i");
    cr_assert_str_eq(t_eval("(+ 1+2i 2-2i 3.0)"), "6.0+0i");
    cr_assert_str_eq(t_eval("(+ 5 -2.5 -2.5)"), "0.0");
    cr_assert_str_eq(t_eval("(+ 10 5i -10)"), "0+5i");
}

Test(end_to_end_numerics, test_sub_integer) {
    cr_assert_str_eq(t_eval("(- 10)"), "-10");
    cr_assert_str_eq(t_eval("(- -10)"), "10");
    cr_assert_str_eq(t_eval("(- 0)"), "0");
    cr_assert_str_eq(t_eval("(- #xff)"), "-255");
    cr_assert_str_eq(t_eval("(- 8 5)"), "3");
    cr_assert_str_eq(t_eval("(- 5 8)"), "-3");
    cr_assert_str_eq(t_eval("(- 10 -2)"), "12");
    cr_assert_str_eq(t_eval("(- -10 2)"), "-12");
    cr_assert_str_eq(t_eval("(- -10 -2)"), "-8");
    cr_assert_str_eq(t_eval("(- 12345 54321)"), "-41976");
    cr_assert_str_eq(t_eval("(- 500 -123)"), "623");
    cr_assert_str_eq(t_eval("(- -250 -250)"), "0");
    cr_assert_str_eq(t_eval("(- #xff #x20)"), "223");
    cr_assert_str_eq(t_eval("(- #b110101 #b010101)"), "32");
    cr_assert_str_eq(t_eval("(- #d888 #d123)"), "765");
    cr_assert_str_eq(t_eval("(- #o777 #o111)"), "438");
    cr_assert_str_eq(t_eval("(- 10 1 2 3)"), "4");
    cr_assert_str_eq(t_eval("(- 100 10 -20 5)"), "105");
    cr_assert_str_eq(t_eval("(- 0 5 10)"), "-15");
    cr_assert_str_eq(t_eval("(- 10 1 2 3 4)"), "0");
    cr_assert_str_eq(t_eval("(- #xff #o444 #b1010)"), "-47");
    cr_assert_str_eq(t_eval("(- 100 (- 50 25))"), "75");
    cr_assert_str_eq(t_eval("(- (- 100 50) 25)"), "25");
    cr_assert_str_eq(t_eval("(- #xdead (- #xfeed #xcede))"), "44702");
}

Test(end_to_end_numerics, test_sub_rational) {
    cr_assert_str_eq(t_eval("(- 1/2)"), "-1/2");
    cr_assert_str_eq(t_eval("(- -3/4)"), "3/4");
    cr_assert_str_eq(t_eval("(- 5/10)"), "-1/2");
    cr_assert_str_eq(t_eval("(- 0/8)"), "0");
    cr_assert_str_eq(t_eval("(- 5/-8)"), "5/8");
    cr_assert_str_eq(t_eval("(- 3/5 1/5)"), "2/5");
    cr_assert_str_eq(t_eval("(- 1/2 1/3)"), "1/6");
    cr_assert_str_eq(t_eval("(- 1/4 3/4)"), "-1/2");
    cr_assert_str_eq(t_eval("(- 1/2 -1/3)"), "5/6");
    cr_assert_str_eq(t_eval("(- -5/8 1/8)"), "-3/4");
    cr_assert_str_eq(t_eval("(- 7/8 1/8)"), "3/4");
    cr_assert_str_eq(t_eval("(- 3/2 1/2)"), "1");
    cr_assert_str_eq(t_eval("(- 5/2 1/2)"), "2");
    cr_assert_str_eq(t_eval("(- 1/3 1/3)"), "0");
    cr_assert_str_eq(t_eval("(- 10/4 1/2)"), "2");
    cr_assert_str_eq(t_eval("(- 1/2 5/2)"), "-2");
    cr_assert_str_eq(t_eval("(- 1/3 5/-6)"), "7/6");
    cr_assert_str_eq(t_eval("(- 2/-5 1/5)"), "-3/5");
    cr_assert_str_eq(t_eval("(- 1/1 1/2 1/4)"), "1/4");
    cr_assert_str_eq(t_eval("(- 10/3 1/3 2/3)"), "7/3");
    cr_assert_str_eq(t_eval("(- 1/2 1/3 1/6)"), "0");
    cr_assert_str_eq(t_eval("(- 2/1 1/5 2/5 3/5)"), "4/5");
    cr_assert_str_eq(t_eval("(- 1/1 (- 1/2 1/4))"), "3/4");
    cr_assert_str_eq(t_eval("(- (- 1/1 1/2) 1/4)"), "1/4");
}

Test(end_to_end_numerics, test_sub_real) {
    cr_assert_str_eq(t_eval("(- 10.5)"), "-10.5");
    cr_assert_str_eq(t_eval("(- -2.25)"), "2.25");
    cr_assert_str_eq(t_eval("(- 0.0)"), "-0.0");
    cr_assert_str_eq(t_eval("(- 8.5 5.0)"), "3.5");
    cr_assert_str_eq(t_eval("(- 5.0 8.5)"), "-3.5");
    cr_assert_str_eq(t_eval("(- 10.0 -2.5)"), "12.5");
    cr_assert_str_eq(t_eval("(- -10.0 2.5)"), "-12.5");
    cr_assert_str_eq(t_eval("(- -10.0 -2.5)"), "-7.5");
    cr_assert_str_eq(t_eval("(- 0.75 0.25)"), "0.5");
    cr_assert_str_eq(t_eval("(- 5.5 0.0)"), "5.5");
    cr_assert_str_eq(t_eval("(- 0.0 5.5)"), "-5.5");
    cr_assert_str_eq(t_eval("(- 3.3 3.3)"), "0.0");
    cr_assert_str_eq(t_eval("(- 10.0 1.5 2.5)"), "6.0");
    cr_assert_str_eq(t_eval("(- 20.0 5.0 -2.5 1.5)"), "16.0");
    cr_assert_str_eq(t_eval("(- 0.0 5.5 4.5)"), "-10.0");
    cr_assert_str_eq(t_eval("(- 10.0 1.0 2.0 3.0 4.0)"), "0.0");
    cr_assert_str_eq(t_eval("(- 0.3 0.1)"), "0.2");
    cr_assert_str_eq(t_eval("(- 5.000008 3.000002)"), "2.000006");
    cr_assert_str_eq(t_eval("(- 100.0 (- 50.0 25.0))"), "75.0");
    cr_assert_str_eq(t_eval("(- (- 100.0 50.0) 25.0)"), "25.0");
}

Test(end_to_end_numerics, test_sub_complex) {
    cr_assert_str_eq(t_eval("(- 1+2i)"), "-1-2i");
    cr_assert_str_eq(t_eval("(- -3-4i)"), "3+4i");
    cr_assert_str_eq(t_eval("(- 10-0i)"), "-10+0i");
    cr_assert_str_eq(t_eval("(- 5i)"), "0-5i");
    cr_assert_str_eq(t_eval("(- -12i)"), "0+12i");
    cr_assert_str_eq(t_eval("(- 5+3i 2+1i)"), "3+2i");
    cr_assert_str_eq(t_eval("(- 2+0i 5+3i)"), "-3-3i");
    cr_assert_str_eq(t_eval("(- 10+5i -2-3i)"), "12+8i");
    cr_assert_str_eq(t_eval("(- -8-2i -1-0i)"), "-7-2i");
    cr_assert_str_eq(t_eval("(- 10+2i 3-4i)"), "7+6i");
    cr_assert_str_eq(t_eval("(- 12i 5i)"), "0+7i");
    cr_assert_str_eq(t_eval("(- 10+8i 3i)"), "10+5i");
    cr_assert_str_eq(t_eval("(- 10+0i 5i)"), "10-5i");
    cr_assert_str_eq(t_eval("(- 5+10i 0+10i)"), "5+0i");
    cr_assert_str_eq(t_eval("(- 5+10i 5+0i)"), "0+10i");
    cr_assert_str_eq(t_eval("(- 10+2i 10-2i)"), "0+4i");
    cr_assert_str_eq(t_eval("(- 5-3i -5-3i)"), "10+0i");
    cr_assert_str_eq(t_eval("(- 3+2i 3+2i)"), "0+0i");
    cr_assert_str_eq(t_eval("(- 10+10i 2+2i 1+1i)"), "7+7i");
    cr_assert_str_eq(t_eval("(- 20+5i 10+0i -2-3i)"), "12+8i");
    cr_assert_str_eq(t_eval("(- 0+0i 5+5i 2-3i)"), "-7-2i");
    cr_assert_str_eq(t_eval("(- 10+10i (- 5+5i 2+3i))"), "7+8i");
    cr_assert_str_eq(t_eval("(- (- 10+10i 5+5i) 2+3i)"), "3+2i");
}

Test(end_to_end_numerics, test_sub_mixed) {
    cr_assert_str_eq(t_eval("(- 5 1/2)"), "9/2");
    cr_assert_str_eq(t_eval("(- 1/2 5)"), "-9/2");
    cr_assert_str_eq(t_eval("(- 10 1/2 1/4)"), "37/4");
    cr_assert_str_eq(t_eval("(- 10 2.5)"), "7.5");
    cr_assert_str_eq(t_eval("(- 2.5 10)"), "-7.5");
    cr_assert_str_eq(t_eval("(- 5/2 1.0)"), "1.5");
    cr_assert_str_eq(t_eval("(- 1.0 1/2)"), "0.5");
    cr_assert_str_eq(t_eval("(- 10 0.5 1/4)"), "9.25");
    cr_assert_str_eq(t_eval("(- 10.0 1 1/2)"), "8.5");
    cr_assert_str_eq(t_eval("(- 5 1+2i)"), "4-2i");
    cr_assert_str_eq(t_eval("(- 1+2i 5)"), "-4+2i");
    cr_assert_str_eq(t_eval("(- 1/2 2-3i)"), "-3/2+3i");
    cr_assert_str_eq(t_eval("(- 2-3i 1/2)"), "3/2-3i");
    cr_assert_str_eq(t_eval("(- 10.5 1-0i)"), "9.5+0i");
    cr_assert_str_eq(t_eval("(- 1-0i 10.5)"), "-9.5-1i");
    cr_assert_str_eq(t_eval("(- 10 1/2 0.25)"), "9.25");
    cr_assert_str_eq(t_eval("(- 20 2.5 1+1i)"), "16.5-1i");
    cr_assert_str_eq(t_eval("(- 5/2 1.5 2+2i)"), "-1.0-2i");
    cr_assert_str_eq(t_eval("(- 1+10i 1 1/2 0.5)"), "-1.0+10i");
    cr_assert_str_eq(t_eval("(- 5.5 1/2)"), "5.0");
    cr_assert_str_eq(t_eval("(- 3/4 0.75)"), "0.0");
    cr_assert_str_eq(t_eval("(- 1+5i 1)"), "0+5i");
    cr_assert_str_eq(t_eval("(- 1.5+2i 0.5+2i)"), "1.0+0i");
    cr_assert_str_eq(t_eval("(- 10/4 0.5)"), "2.0");
    cr_assert_str_eq(t_eval("(- 10.0 (- 5 1/2))"), "5.5");
    cr_assert_str_eq(t_eval("(- (- 10.0 5) 1/2)"), "4.5");
}

Test(end_to_end_numerics, test_mul_integer) {
    cr_assert_str_eq(t_eval("(*)"), "1");
    cr_assert_str_eq(t_eval("(* 7)"), "7");
    cr_assert_str_eq(t_eval("(* -10)"), "-10");
    cr_assert_str_eq(t_eval("(* 5 5)"), "25");
    cr_assert_str_eq(t_eval("(* 5 -5)"), "-25");
    cr_assert_str_eq(t_eval("(* -5 -5)"), "25");
    cr_assert_str_eq(t_eval("(* 1 2 3 4 5)"), "120");
    cr_assert_str_eq(t_eval("(* 1 -2 3 -4 5)"), "120");
    cr_assert_str_eq(t_eval("(* 12345 0)"), "0");
    cr_assert_str_eq(t_eval("(* 1 2 3 4 5 0)"), "0");
    cr_assert_str_eq(t_eval("(* 0)"), "0");
    cr_assert_str_eq(t_eval("(* #xff 2)"), "510");
    cr_assert_str_eq(t_eval("(* #o10 #d8)"), "64");
    cr_assert_str_eq(t_eval("(* #b101 #d5)"), "25");
    cr_assert_str_eq(t_eval("(* #x10 #o10 #b10)"), "256");
    cr_assert_str_eq(t_eval("(* 2 (* 3 (* 4 5)))"), "120");
}

Test(end_to_end_numerics, test_mul_rational) {
    cr_assert_str_eq(t_eval("(* 1/2 1/2)"), "1/4");
    cr_assert_str_eq(t_eval("(* 3/4 4/3)"), "1");
    cr_assert_str_eq(t_eval("(* -1/2 3/4)"), "-3/8");
    cr_assert_str_eq(t_eval("(* -2/3 -3/4)"), "1/2");
    cr_assert_str_eq(t_eval("(* 1/2 2)"), "1");
    cr_assert_str_eq(t_eval("(* 4/5 15/2)"), "6");
    cr_assert_str_eq(t_eval("(* 1/3 1/3 1/3)"), "1/27");
    cr_assert_str_eq(t_eval("(* 123/456 0)"), "0");
    cr_assert_str_eq(t_eval("(* 1/2 1/3 0/1)"), "0");
    cr_assert_str_eq(t_eval("(* 7/8)"), "7/8");
    cr_assert_str_eq(t_eval("(* 1/1)"), "1");
}

Test(end_to_end_numerics, test_mul_real) {
    cr_assert_str_eq(t_eval("(* 2.5 2.0)"), "5.0");
    cr_assert_str_eq(t_eval("(* 0.5 0.5)"), "0.25");
    cr_assert_str_eq(t_eval("(* -1.5 3.0)"), "-4.5");
    cr_assert_str_eq(t_eval("(* -2.25 -2.0)"), "4.5");
    cr_assert_str_eq(t_eval("(* 1.5 2.0 3.0)"), "9.0");
    cr_assert_str_eq(t_eval("(* 10.0 0.1 5.0 -1.0)"), "-5.0");
    cr_assert_str_eq(t_eval("(* 99.9 0.0)"), "0.0");
    cr_assert_str_eq(t_eval("(* 0.0)"), "0.0");
    cr_assert_str_eq(t_eval("(* -123.456)"), "-123.456");
}

Test(end_to_end_numerics, test_mul_complex) {
    cr_assert_str_eq(t_eval("(* 2+3i 4+5i)"), "-7+22i");
    cr_assert_str_eq(t_eval("(* 1-0i 1+0i)"), "2+0i");
    cr_assert_str_eq(t_eval("(* 2+0i 0+2i)"), "0+4i");
    cr_assert_str_eq(t_eval("(* 3i 4i)"), "-12+0i");
    cr_assert_str_eq(t_eval("(* 1+0i 1+0i)"), "0+2i");
    cr_assert_str_eq(t_eval("(* 1-2i 3-4i)"), "-5-10i");
    cr_assert_str_eq(t_eval("(* 1+0i 2+0i 3i)"), "-6+6i");
    cr_assert_str_eq(t_eval("(* 123+456i 0+0i)"), "0+0i");
    cr_assert_str_eq(t_eval("(* 5-10i)"), "5-10i");
}

Test(end_to_end_numerics, test_mul_mixed) {
    cr_assert_str_eq(t_eval("(* 2 1/2)"), "1");
    cr_assert_str_eq(t_eval("(* 1/3 6)"), "2");
    cr_assert_str_eq(t_eval("(* 4 2.5)"), "10.0");
    cr_assert_str_eq(t_eval("(* 0.5 10)"), "5.0");
    cr_assert_str_eq(t_eval("(* 2 1+5i)"), "2+10i");
    cr_assert_str_eq(t_eval("(* 1/2 5.0)"), "2.5");
    cr_assert_str_eq(t_eval("(* 0.25 1/2)"), "0.125");
    cr_assert_str_eq(t_eval("(* 1/2 2+2i)"), "1+1i");
    cr_assert_str_eq(t_eval("(* 1.5 2+4i)"), "3.0+6.0i");
    cr_assert_str_eq(t_eval("(* 2 1/2 5.0)"), "5.0");
    cr_assert_str_eq(t_eval("(* 2 1/2 5.0 1+0i)"), "5.0+5.0i");
    cr_assert_str_eq(t_eval("(* 1/2 0.0)"), "0.0");
    cr_assert_str_eq(t_eval("(* 5.0 0+0i)"), "0.0+0.0i");
}

Test(end_to_end_numerics, test_div_integer) {
    // Unary division (reciprocal)
    cr_assert_str_eq(t_eval("(/ 10)"), "1/10");
    cr_assert_str_eq(t_eval("(/ -2)"), "-1/2");
    cr_assert_str_eq(t_eval("(/ 1)"), "1");
    cr_assert_str_eq(t_eval("(/ 10 2)"), "5");
    cr_assert_str_eq(t_eval("(/ -10 2)"), "-5");
    cr_assert_str_eq(t_eval("(/ -10 -2)"), "5");
    cr_assert_str_eq(t_eval("(/ 10 -2)"), "-5");
    cr_assert_str_eq(t_eval("(/ 10 3)"), "10/3");
    cr_assert_str_eq(t_eval("(/ 1 2)"), "1/2");
    cr_assert_str_eq(t_eval("(/ 2 4)"), "1/2");
    cr_assert_str_eq(t_eval("(/ 100 10 2)"), "5");
    cr_assert_str_eq(t_eval("(/ 81 3 3)"), "9");
    cr_assert_str_eq(t_eval("(/ 120 2 3 4)"), "5");
    cr_assert_str_eq(t_eval("(/ #x100 16)"), "16");
    cr_assert_str_eq(t_eval("(/ #o77 7)"), "9");
}

Test(end_to_end_numerics, test_div_rational) {
    cr_assert_str_eq(t_eval("(/ 5/4)"), "4/5");
    cr_assert_str_eq(t_eval("(/ -1/2)"), "-2");
    cr_assert_str_eq(t_eval("(/ 10/1)"), "1/10");
    cr_assert_str_eq(t_eval("(/ 1/2 1/4)"), "2");
    cr_assert_str_eq(t_eval("(/ 2/3 3/4)"), "8/9");
    cr_assert_str_eq(t_eval("(/ -1/2 1/2)"), "-1");
    cr_assert_str_eq(t_eval("(/ -3/4 -2/3)"), "9/8");
    cr_assert_str_eq(t_eval("(/ 1 1/2 1/2)"), "4");
    cr_assert_str_eq(t_eval("(/ 16/1 2/1 2/1)"), "4");
    cr_assert_str_eq(t_eval("(/ 1/2 2/1 4/1)"), "1/16");
}

Test(end_to_end_numerics, test_div_real) {
    cr_assert_str_eq(t_eval("(/ 2.0)"), "0.5");
    cr_assert_str_eq(t_eval("(/ -4.0)"), "-0.25");
    cr_assert_str_eq(t_eval("(/ 5.5)"), "0.18181818181818181819");
    cr_assert_str_eq(t_eval("(/ 10.0 4.0)"), "2.5");
    cr_assert_str_eq(t_eval("(/ 5.0 2.0)"), "2.5");
    cr_assert_str_eq(t_eval("(/ -10.0 4.0)"), "-2.5");
    cr_assert_str_eq(t_eval("(/ 9.9 3.3)"), "3.0");
    cr_assert_str_eq(t_eval("(/ 100.0 2.0 5.0)"), "10.0");
    cr_assert_str_eq(t_eval("(/ 25.0 -2.5 -2.0)"), "5.0");
}

Test(end_to_end_numerics, test_div_complex) {
    cr_assert_str_eq(t_eval("(/ 10+10i)"), "0.05-0.05i");
    cr_assert_str_eq(t_eval("(/ 1+0i)"), "0.5-0.5i");
    cr_assert_str_eq(t_eval("(/ 2i)"), "0-0.5i");
    cr_assert_str_eq(t_eval("(/ 8+6i 2+4i)"), "2-1i");
    cr_assert_str_eq(t_eval("(/ 2+4i 1+2i)"), "2+0i");
    cr_assert_str_eq(t_eval("(/ 10+20i 2+1i)"), "8+6i");
    cr_assert_str_eq(t_eval("(/ -16+0i 2+2i 2-2i)"), "-2+0i");
    cr_assert_str_eq(t_eval("(/ 20+10i 1+2i 2)"), "2-3i");
}

Test(end_to_end_numerics, test_div_mixed) {
    cr_assert_str_eq(t_eval("(/ 10 1/2)"), "20");
    cr_assert_str_eq(t_eval("(/ 1/2 10)"), "1/20");
    cr_assert_str_eq(t_eval("(/ 5 2.0)"), "2.5");
    cr_assert_str_eq(t_eval("(/ 5.0 2)"), "2.5");
    cr_assert_str_eq(t_eval("(/ 1/2 2.0)"), "0.25");
    cr_assert_str_eq(t_eval("(/ 2.0 1/2)"), "4.0");
    cr_assert_str_eq(t_eval("(/ 10 1+i)"), "5-5i");
    cr_assert_str_eq(t_eval("(/ 1/2 1+i)"), "0.25-0.25i");
    cr_assert_str_eq(t_eval("(/ 2.0 1+i)"), "1.0-1.0i");
    cr_assert_str_eq(t_eval("(/ 5+5i 2)"), "2.5+2.5i");
    cr_assert_str_eq(t_eval("(/ 100 2.0 5)"), "10.0");
    cr_assert_str_eq(t_eval("(/ 40+0i 2.0 5)"), "4.0+0.0i");
}

//
// Test(end_to_end_numerics), test_add_complex) {
//     /* Test integer addition. */
//     cr_assert_str_eq(t_eval(""), "");
//     cr_assert_str_eq(t_eval(""), "");
//     cr_assert_str_eq(t_eval(""), "");
//     cr_assert_str_eq(t_eval(""), "");
//     cr_assert_str_eq(t_eval(""), "");
//     cr_assert_str_eq(t_eval(""), "");
//     cr_assert_str_eq(t_eval(""), "");
//     cr_assert_str_eq(t_eval(""), "");
//     cr_assert_str_eq(t_eval(""), "");
// }




