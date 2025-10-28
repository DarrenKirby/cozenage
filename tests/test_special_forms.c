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

//Test(end_to_end_sf, test_map_procedure, .init = setup_each_test, .fini = teardown_each_test) {}

//Test(end_to_end_sf, test_map_procedure, .init = setup_each_test, .fini = teardown_each_test) {}

//Test(end_to_end_sf, test_map_procedure, .init = setup_each_test, .fini = teardown_each_test) {}