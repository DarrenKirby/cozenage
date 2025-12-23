#include <criterion/criterion.h>

#include "test_meta.h"

TestSuite(end_to_end_sf);

Test(end_to_end_sf, test_define, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert_str_eq(t_eval("(begin (define x 123) x)"), "123");
    cr_assert_str_eq(t_eval("(begin (define s \"hello\") s)"), "\"hello\"");
    cr_assert_str_eq(t_eval("(begin (define a (list 1)) a)"), "(1)");
    cr_assert_str_eq(t_eval("(begin (define v (vector 1 2)) v)"), "#(1 2)");
}

Test(end_to_end_sf, test_lambda, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert_str_eq(t_eval("((lambda (x) x) 23)"), "23");
    cr_assert_str_eq(t_eval("((lambda (x y) (+ x y)) 2 3)"), "5");
}

Test(end_to_end_sf, test_boolean_logic, .init = setup_each_test, .fini = teardown_each_test) {
    // and/or short-circuiting and truthiness
    cr_assert_str_eq(t_eval("(and 1 2 3)"), "3"); // returns last value
    cr_assert_str_eq(t_eval("(and #t #f (define x 1))"), "#false");
    cr_assert_str_eq(t_eval("(or #f 5 #t)"), "5"); // returns first truthy value

    // when/unless
    cr_assert_str_eq(t_eval("(when #t 1 2 3)"), "3");
    cr_assert_str_eq(t_eval("(unless #f 1 2 3)"), "3");
    cr_assert_str_eq(t_eval("(when #f 1)"), "");
}

Test(end_to_end_sf, test_bindings, .init = setup_each_test, .fini = teardown_each_test) {
    // let*: variables can see previous variables in same block
    cr_assert_str_eq(t_eval("(let* ((x 1) (y (+ x 1))) y)"), "2");

    // letrec: mutual recursion
    cr_assert_str_eq(t_eval(
        "(letrec ((is-even? (lambda (n) (if (= n 0) #t (is-odd? (- n 1))))) "
        "         (is-odd?  (lambda (n) (if (= n 0) #f (is-even? (- n 1)))))) "
        "(is-even? 4))"), "#true");

    // Named let (your new transform!)
    cr_assert_str_eq(t_eval("(let loop ((i 3)) (if (= i 0) 'done (loop (- i 1))))"), "done");
}

Test(end_to_end_sf, test_conditionals, .init = setup_each_test, .fini = teardown_each_test) {
    // cond
    cr_assert_str_eq(t_eval("(cond (#f 1) ((= 1 2) 2) (else 3))"), "3");
    cr_assert_str_eq(t_eval("(cond ((+ 1 1) 'truthy))"), "truthy");

    // case (your transform!)
    cr_assert_str_eq(t_eval("(case (* 2 3) ((2 3 5) 'prime) ((6 8 10) 'even) (else 'idk))"), "even");
    cr_assert_str_eq(t_eval("(case 'apple ((banana) 1) ((orange) 2) (else 3))"), "3");
}

Test(end_to_end_sf, test_state, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert_str_eq(t_eval("(begin (define x 10) (set! x 20) x)"), "20");
    // Ensure set! works on closure variables
    cr_assert_str_eq(t_eval(
        "(begin "
        "  (define counter ((lambda (n) (lambda () (set! n (+ n 1)) n)) 0)) "
        "  (counter) (counter))"), "2");
}

Test(end_to_end_sf, test_iteration, .init = setup_each_test, .fini = teardown_each_test) {
    // Sum numbers 1 to 5
    cr_assert_str_eq(t_eval(
        "(do ((i 1 (+ i 1)) (sum 0 (+ sum i))) "
        "    ((> i 5) sum))"), "15");
}

//Test(end_to_end_sf, test_map_procedure, .init = setup_each_test, .fini = teardown_each_test) {}

//Test(end_to_end_sf, test_map_procedure, .init = setup_each_test, .fini = teardown_each_test) {}

//Test(end_to_end_sf, test_map_procedure, .init = setup_each_test, .fini = teardown_each_test) {}