#include "test_meta.h"
#include <criterion/criterion.h>

TestSuite(end_to_end_numeric_predicates);

Test(end_to_end_numeric_predicates, test_exact_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Exact Numbers ##
    cr_assert_str_eq(t_eval("(exact? 5)"), "#true");
    cr_assert_str_eq(t_eval("(exact? -10)"), "#true");
    cr_assert_str_eq(t_eval("(exact? 3/4)"), "#true");
    cr_assert_str_eq(t_eval("(exact? 1+2i)"), "#true");
    cr_assert_str_eq(t_eval("(exact? #e123)"), "#true");
    cr_assert_str_eq(t_eval("(exact? #e3/4)"), "#true");

    // Test with different bases
    cr_assert_str_eq(t_eval("(exact? #b101)"), "#true");
    cr_assert_str_eq(t_eval("(exact? #o77)"), "#true");
    cr_assert_str_eq(t_eval("(exact? #d123)"), "#true");
    cr_assert_str_eq(t_eval("(exact? #xAF)"), "#true");

    // ## Inexact Numbers ##
    cr_assert_str_eq(t_eval("(exact? 5.0)"), "#false");
    cr_assert_str_eq(t_eval("(exact? -10.5)"), "#false");
    cr_assert_str_eq(t_eval("(exact? 1e2)"), "#false");
    cr_assert_str_eq(t_eval("(exact? 1.0+2.0i)"), "#false");
    cr_assert_str_eq(t_eval("(exact? #i123)"), "#false");
    cr_assert_str_eq(t_eval("(exact? #i3/4)"), "#false"); // Equivalent to 0.75

    // ## Non-numeric types ##
    cr_assert_str_eq(t_eval("(exact? #true)"), " Type error: exact?: bad type at arg 1: got bool, expected integer|real|rational|complex|bigint");
    cr_assert_str_eq(t_eval("(exact? \"hello\")"), " Type error: exact?: bad type at arg 1: got string, expected integer|real|rational|complex|bigint");
    cr_assert_str_eq(t_eval("(exact? 'foo)"), " Type error: exact?: bad type at arg 1: got symbol, expected integer|real|rational|complex|bigint");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(exact?)"), " Arity error: exact?: expected exactly 1 arg, got 0");
    cr_assert_str_eq(t_eval("(exact? 1 2)"), " Arity error: exact?: expected exactly 1 arg, got 2");
}

Test(end_to_end_numeric_predicates, test_inexact_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Inexact Numbers ##
    cr_assert_str_eq(t_eval("(inexact? 5.0)"), "#true");
    cr_assert_str_eq(t_eval("(inexact? -10.5)"), "#true");
    cr_assert_str_eq(t_eval("(inexact? 1e2)"), "#true");
    cr_assert_str_eq(t_eval("(inexact? 1.0+2i)"), "#true");
    cr_assert_str_eq(t_eval("(inexact? #i123)"), "#true");
    cr_assert_str_eq(t_eval("(inexact? #i3.5)"), "#true");

    // Test with different bases (with inexact prefix)
    cr_assert_str_eq(t_eval("(inexact? #i#b101)"), "#true");
    cr_assert_str_eq(t_eval("(inexact? #i#o77)"), "#true");

    // ## Exact Numbers ##
    cr_assert_str_eq(t_eval("(inexact? 5)"), "#false");
    cr_assert_str_eq(t_eval("(inexact? 3/4)"), "#false");
    cr_assert_str_eq(t_eval("(inexact? 1+2i)"), "#false");
    cr_assert_str_eq(t_eval("(inexact? #e123)"), "#false");

    // ## Non-numeric types ##
    cr_assert_str_eq(t_eval("(inexact? #false)"), " Type error: inexact?: bad type at arg 1: got bool, expected integer|real|rational|complex|bigint");
    cr_assert_str_eq(t_eval("(inexact? #\\a)"), " Type error: inexact?: bad type at arg 1: got char, expected integer|real|rational|complex|bigint");
    cr_assert_str_eq(t_eval("(inexact? '(1 2))"), " Type error: inexact?: bad type at arg 1: got pair, expected integer|real|rational|complex|bigint");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(inexact?)"), " Arity error: inexact?: expected exactly 1 arg, got 0");
    cr_assert_str_eq(t_eval("(inexact? 1 2)"), " Arity error: inexact?: expected exactly 1 arg, got 2");
}

Test(end_to_end_numeric_predicates, test_complex_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // `complex?` is true for any number.
    cr_assert_str_eq(t_eval("(complex? 1)"), "#true");
    cr_assert_str_eq(t_eval("(complex? 1.5)"), "#true");
    cr_assert_str_eq(t_eval("(complex? 3/4)"), "#true");
    cr_assert_str_eq(t_eval("(complex? 1+2i)"), "#true");
    cr_assert_str_eq(t_eval("(complex? #e1+2i)"), "#true");
    cr_assert_str_eq(t_eval("(complex? #i1.0+2.0i)"), "#true");
    cr_assert_str_eq(t_eval("(complex? +inf.0)"), "#true");
    cr_assert_str_eq(t_eval("(complex? -nan.0)"), "#true");

    // ## Non-numeric types ##
    cr_assert_str_eq(t_eval("(complex? #true)"), "#false");
    cr_assert_str_eq(t_eval("(complex? \"1+2i\")"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(complex?)"), " Arity error: complex?: expected exactly 1 arg, got 0");
}

Test(end_to_end_numeric_predicates, test_real_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // `real?` is true for numbers with a zero imaginary part.
    cr_assert_str_eq(t_eval("(real? 1)"), "#true");
    cr_assert_str_eq(t_eval("(real? 1.5)"), "#true");
    cr_assert_str_eq(t_eval("(real? #e1.5)"), "#true");
    cr_assert_str_eq(t_eval("(real? #i1.5)"), "#true");
    cr_assert_str_eq(t_eval("(real? 3/4)"), "#true");
    cr_assert_str_eq(t_eval("(real? +inf.0)"), "#true");

    // Complex numbers with zero imaginary part are real
    cr_assert_str_eq(t_eval("(real? 1+0i)"), "#true");
    cr_assert_str_eq(t_eval("(real? #e5+0i)"), "#true");
    cr_assert_str_eq(t_eval("(real? -2.0+0.0i)"), "#true");

    // ## Non-real numbers ##
    cr_assert_str_eq(t_eval("(real? 1+2i)"), "#false");
    cr_assert_str_eq(t_eval("(real? 1-2.5i)"), "#false");

    // ## Non-numeric types ##
    cr_assert_str_eq(t_eval("(real? #false)"), "#false");
    cr_assert_str_eq(t_eval("(real? 'foo)"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(real? 1 2)"), " Arity error: real?: expected exactly 1 arg, got 2");
}

Test(end_to_end_numeric_predicates, test_rational_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // `rational?` is true for numbers that can be expressed as a ratio of integers.
    cr_assert_str_eq(t_eval("(rational? 1)"), "#true");
    cr_assert_str_eq(t_eval("(rational? 3/4)"), "#true");
    cr_assert_str_eq(t_eval("(rational? #e-5/2)"), "#true");
    cr_assert_str_eq(t_eval("(rational? 123.0)"), "#true"); // Inexact integers are rational
    cr_assert_str_eq(t_eval("(rational? 1+0i)"), "#true");  // Reals are rational

    // ## Non-rational numbers ##
    cr_assert_str_eq(t_eval("(rational? 1.5)"), "#true"); // Floats that can be represented as rationals
    cr_assert_str_eq(t_eval("(rational? +inf.0)"), "#false");
    cr_assert_str_eq(t_eval("(rational? -inf.0)"), "#false");
    cr_assert_str_eq(t_eval("(rational? +nan.0)"), "#false");
    cr_assert_str_eq(t_eval("(rational? 1+2i)"), "#false");

    // ## Non-numeric types ##
    cr_assert_str_eq(t_eval("(rational? '())"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(rational?)"), " Arity error: rational?: expected exactly 1 arg, got 0");
}

Test(end_to_end_numeric_predicates, test_integer_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // `integer?` is true for numbers with no fractional part.
    cr_assert_str_eq(t_eval("(integer? 5)"), "#true");
    cr_assert_str_eq(t_eval("(integer? #d-10)"), "#true");
    cr_assert_str_eq(t_eval("(integer? 5.0)"), "#true"); // Inexact integer
    cr_assert_str_eq(t_eval("(integer? #i123)"), "#true");
    cr_assert_str_eq(t_eval("(integer? 5.000000000000000)"), "#true");
    cr_assert_str_eq(t_eval("(integer? 10/2)"), "#true"); // A rational that simplifies to an integer
    cr_assert_str_eq(t_eval("(integer? 4+0i)"), "#true");   // A real complex number

    // ## Non-integer numbers ##
    cr_assert_str_eq(t_eval("(integer? 5.1)"), "#false");
    cr_assert_str_eq(t_eval("(integer? 3/4)"), "#false");
    cr_assert_str_eq(t_eval("(integer? +inf.0)"), "#false");
    cr_assert_str_eq(t_eval("(integer? +nan.0)"), "#false");
    cr_assert_str_eq(t_eval("(integer? 1+2i)"), "#false");

    // ## Non-numeric types ##
    cr_assert_str_eq(t_eval("(integer? #\\5)"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(integer?)"), " Arity error: integer?: expected exactly 1 arg, got 0");
}

Test(end_to_end_numeric_predicates, test_exact_integer_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // `exact-integer?` is a library procedure, equivalent to `(and (exact? x) (integer? x))`

    // ## Exact Integers ##
    cr_assert_str_eq(t_eval("(exact-integer? 5)"), "#true");
    cr_assert_str_eq(t_eval("(exact-integer? #e-10)"), "#true");
    cr_assert_str_eq(t_eval("(exact-integer? #b1101)"), "#true");

    // ## Not exact integers ##
    cr_assert_str_eq(t_eval("(exact-integer? 5.0)"), "#false"); // Inexact
    cr_assert_str_eq(t_eval("(exact-integer? #i5)"), "#false"); // Inexact
    cr_assert_str_eq(t_eval("(exact-integer? 3/4)"), "#false"); // Not an integer
    cr_assert_str_eq(t_eval("(exact-integer? 5.2)"), "#false"); // Inexact and not an integer
    cr_assert_str_eq(t_eval("(exact-integer? 1+0i)"), "#true"); // exact(real) && exact(imag) && integer?

    // ## Non-numeric types ##
    cr_assert_str_eq(t_eval("(exact-integer? \"5\")"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(exact-integer?)"), " Arity error: exact-integer?: expected exactly 1 arg, got 0");
}

Test(end_to_end_numeric_predicates, test_zero_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // `zero?` is true for any numeric zero.
    cr_assert_str_eq(t_eval("(zero? 0)"), "#true");
    cr_assert_str_eq(t_eval("(zero? 0.0)"), "#true");
    cr_assert_str_eq(t_eval("(zero? #e0.0)"), "#true");
    cr_assert_str_eq(t_eval("(zero? #i0)"), "#true");
    cr_assert_str_eq(t_eval("(zero? 0/100)"), "#true");
    cr_assert_str_eq(t_eval("(zero? 0+0i)"), "#true"); // Complex zero
    cr_assert_str_eq(t_eval("(zero? -0.0)"), "#true"); // Negative zero

    // ## Non-zero numbers ##
    cr_assert_str_eq(t_eval("(zero? 1)"), "#false");
    cr_assert_str_eq(t_eval("(zero? -1.5)"), "#false");
    cr_assert_str_eq(t_eval("(zero? 1+0i)"), "#false");
    cr_assert_str_eq(t_eval("(zero? +inf.0)"), "#false");
    cr_assert_str_eq(t_eval("(zero? +nan.0)"), "#false");

    // ## Type Errors ##
    cr_assert_str_eq(t_eval("(zero? 'a)"), " Type error: zero?: bad type at arg 1: got symbol, expected integer|real|rational|complex|bigint");
    cr_assert_str_eq(t_eval("(zero? 1+2i)"), "#false");


    // ## Arity ##
    cr_assert_str_eq(t_eval("(zero?)"), " Arity error: zero?: expected exactly 1 arg, got 0");
    cr_assert_str_eq(t_eval("(zero? 0 0)"), " Arity error: zero?: expected exactly 1 arg, got 2");
}

Test(end_to_end_numeric_predicates, test_positive_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Positive Numbers ##
    cr_assert_str_eq(t_eval("(positive? 1)"), "#true");
    cr_assert_str_eq(t_eval("(positive? 0.0001)"), "#true");
    cr_assert_str_eq(t_eval("(positive? 1/1000)"), "#true");
    cr_assert_str_eq(t_eval("(positive? +inf.0)"), "#true");

    // ## Not Positive Numbers ##
    cr_assert_str_eq(t_eval("(positive? 0)"), "#false");
    cr_assert_str_eq(t_eval("(positive? 0.0)"), "#false");
    cr_assert_str_eq(t_eval("(positive? -0.0)"), "#false");
    cr_assert_str_eq(t_eval("(positive? -1)"), "#false");
    cr_assert_str_eq(t_eval("(positive? -inf.0)"), "#false");
    cr_assert_str_eq(t_eval("(positive? +nan.0)"), "#false");

    // ## Type Errors ##
    cr_assert_str_eq(t_eval("(positive? 1+2i)"), " Value error: positive?: expected real, got complex");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(positive?)"), " Arity error: positive?: expected exactly 1 arg, got 0");
}

Test(end_to_end_numeric_predicates, test_negative_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Negative Numbers ##
    cr_assert_str_eq(t_eval("(negative? -1)"), "#true");
    cr_assert_str_eq(t_eval("(negative? -0.0001)"), "#true");
    cr_assert_str_eq(t_eval("(negative? -1/1000)"), "#true");
    cr_assert_str_eq(t_eval("(negative? -inf.0)"), "#true");

    // ## Not Negative Numbers ##
    cr_assert_str_eq(t_eval("(negative? 0)"), "#false");
    cr_assert_str_eq(t_eval("(negative? 0.0)"), "#false");
    cr_assert_str_eq(t_eval("(negative? -0.0)"), "#false");
    cr_assert_str_eq(t_eval("(negative? 1)"), "#false");
    cr_assert_str_eq(t_eval("(negative? +inf.0)"), "#false");
    cr_assert_str_eq(t_eval("(negative? +nan.0)"), "#false");

    // ## Type Errors ##
    cr_assert_str_eq(t_eval("(negative? #true)"), " Type error: negative?: bad type at arg 1: got bool, expected integer|real|rational|complex|bigint");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(negative? -1 -2)"), " Arity error: negative?: expected exactly 1 arg, got 2");
}

Test(end_to_end_numeric_predicates, test_odd_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Odd Integers ##
    cr_assert_str_eq(t_eval("(odd? 1)"), "#true");
    cr_assert_str_eq(t_eval("(odd? -1)"), "#true");
    cr_assert_str_eq(t_eval("(odd? 99)"), "#true");
    cr_assert_str_eq(t_eval("(odd? -12345)"), "#true");

    // ## Not Odd Integers ##
    cr_assert_str_eq(t_eval("(odd? 0)"), "#false");
    cr_assert_str_eq(t_eval("(odd? 2)"), "#false");
    cr_assert_str_eq(t_eval("(odd? -100)"), "#false");

    // ## Type Errors (must be an integer) ##
    cr_assert_str_eq(t_eval("(odd? 1.0)"), "#true");
    cr_assert_str_eq(t_eval("(odd? 3/2)"), " Value error: odd?: expected integer");
    cr_assert_str_eq(t_eval("(odd? +inf.0)"), " Value error: odd?: expected integer");
    cr_assert_str_eq(t_eval("(odd? 'foo)"), " Type error: odd?: bad type at arg 1: got symbol, expected integer|real|rational|complex|bigint");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(odd?)"), " Arity error: odd?: expected exactly 1 arg, got 0");
}

Test(end_to_end_numeric_predicates, test_even_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Even Integers ##
    cr_assert_str_eq(t_eval("(even? 0)"), "#true");
    cr_assert_str_eq(t_eval("(even? 2)"), "#true");
    cr_assert_str_eq(t_eval("(even? -2)"), "#true");
    cr_assert_str_eq(t_eval("(even? 100)"), "#true");
    cr_assert_str_eq(t_eval("(even? -5432)"), "#true");

    // ## Not Even Integers ##
    cr_assert_str_eq(t_eval("(even? 1)"), "#false");
    cr_assert_str_eq(t_eval("(even? -99)"), "#false");

    // ## Type Errors (must be an integer) ##
    cr_assert_str_eq(t_eval("(even? 2.0)"), "#true");
    cr_assert_str_eq(t_eval("(even? 4/2)"), "#true"); // Note: (even? 2) is #true
    cr_assert_str_eq(t_eval("(even? -inf.0)"), " Value error: even?: expected integer");
    cr_assert_str_eq(t_eval("(even? #\\a)"), " Type error: even?: bad type at arg 1: got char, expected integer|real|rational|complex|bigint");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(even?)"), " Arity error: even?: expected exactly 1 arg, got 0");
}

Test(end_to_end_numeric_predicates, test_nan_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## NaN Values ##
    // `nan?` is true only for "Not a Number" values.
    cr_assert_str_eq(t_eval("(nan? +nan.0)"), "#true");
    cr_assert_str_eq(t_eval("(nan? -nan.0)"), "#true");
    // Standard operations that produce NaN
    cr_assert_str_eq(t_eval("(nan? (/ 0.0 0.0))"), "#true");
    cr_assert_str_eq(t_eval("(nan? (- +inf.0 +inf.0))"), "#true");

    // ## Not NaN values ##
    // It should be false for all other numbers.
    cr_assert_str_eq(t_eval("(nan? 0)"), "#false");
    cr_assert_str_eq(t_eval("(nan? 123)"), "#false");
    cr_assert_str_eq(t_eval("(nan? 12.34)"), "#false");
    cr_assert_str_eq(t_eval("(nan? 3/4)"), "#false");
    cr_assert_str_eq(t_eval("(nan? +inf.0)"), "#false");
    cr_assert_str_eq(t_eval("(nan? -inf.0)"), "#false");
    cr_assert_str_eq(t_eval("(nan? 1+0i)"), "#false"); // Complex numbers with 0i are real

    // ## Type Errors ##
    // R7RS specifies the argument must be a real number.
    cr_assert_str_eq(t_eval("(nan? 1+2i)"), "#false");
    cr_assert_str_eq(t_eval("(nan? #true)"), " Type error: nan?: bad type at arg 1: got bool, expected integer|real|rational|complex|bigint");
    cr_assert_str_eq(t_eval("(nan? 'nan)"), " Type error: nan?: bad type at arg 1: got symbol, expected integer|real|rational|complex|bigint");
    cr_assert_str_eq(t_eval("(nan? \"nan\")"), " Type error: nan?: bad type at arg 1: got string, expected integer|real|rational|complex|bigint");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(nan?)"), " Arity error: nan?: expected exactly 1 arg, got 0");
    cr_assert_str_eq(t_eval("(nan? +nan.0 1)"), " Arity error: nan?: expected exactly 1 arg, got 2");
}
