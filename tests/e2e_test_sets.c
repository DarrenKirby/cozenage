#include "test_meta.h"
#include <criterion/criterion.h>

TestSuite(end_to_end_sets);

/* Due to unordered nature of hash, sets are best tested using unit tests. */

Test(end_to_end_sets, test_set_constructor, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert_str_eq(t_eval("(set)"), "#{}");
    cr_assert_str_eq(t_eval("(set 1)"), "#{1}");
    cr_assert_str_eq(t_eval("(set 'a)"), "#{a}");
    cr_assert_str_eq(t_eval("(set '(1 2 3 4 5))"), " Type error: set: arg type 'pair' is not a hashable");
    cr_assert_str_eq(t_eval("(set #\\c)"), "#{#\\c}");
}

