#include "test_meta.h"
#include <criterion/criterion.h>


TestSuite(end_to_end_lists, .init = suite_setup_wrapper, .fini = teardown_suite);

Test(end_to_end_lists, test_list_and_pair_constructors) {
    /* Test list/pair constructors */
    eval_and_check("(list 1 2 3 4)", "(1 2 3 4)");
    eval_and_check("(list '(1 2 3 4))", "((1 2 3 4))");
    eval_and_check("(list '1 '2 '3 '4)", "(1 2 3 4)");
    eval_and_check("(list)", "()");
    eval_and_check("(list \"1\" \"2\" \"3\" \"4\")", "(\"1\" \"2\" \"3\" \"4\")");
    eval_and_check("(list #\\c #\\d #\\g)", "(#\\c #\\d #\\g)");
    eval_and_check("(list 1 2 3 4)", "(1 2 3 4)");
    eval_and_check("(cons 1 2)", "(1 . 2)");
}

Test(end_to_end_lists, test_list_length) {
    /* Test length */
    eval_and_check("(length (list 1 2 3 4))", "4");
    eval_and_check("(length '())", "0");
}

Test(end_to_end_lists, test_list_and_pair_selectors) {
    /* Test list-ref */
    eval_and_check("(list-ref (list 1 2 3 4) 2)", "3");
    //eval_and_check("(list-ref (list 1 2 3 4) \"hello\")", " \x1b[31;1mError:\x1b[0m list-ref: arg 2 must be an integer");
    //eval_and_check("(list-ref (list 1 2 3 4) 5)", "list-ref: index out of bounds");
}

