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
    eval_and_check("(cons 1 2)", "(1 . 2)");
    eval_and_check("(cons 1 (cons 2 (cons 3 (cons 4 5))))", "(1 2 3 4 . 5)");
    eval_and_check("(list 1 \"two\" 3/4 #t 'sym)", "(1 \"two\" 3/4 #true sym)");
    eval_and_check("(list 1 () 3)", "(1 () 3)");
    eval_and_check("(list (list 1 2) (list 3 4))", "((1 2) (3 4))");
    eval_and_check("(list (+ 1 1) (* 2 3) (- 10 4))", "(2 6 6)");
    eval_and_check("(list (cons 1 2) 3)", "((1 . 2) 3)");
    eval_and_check("(cons 'a '(b c d))", "(a b c d)");
    eval_and_check("(cons 'a '())", "(a)");
    eval_and_check("(cons 1 (cons 2 (cons 3 '())))", "(1 2 3)");
    eval_and_check("(cons '(a b) '(c d))", "((a b) c d)");
    eval_and_check("(cons (cons 1 2) 3)", "((1 . 2) . 3)");
    eval_and_check("(cons (cons 1 2) (cons 3 4))", "((1 . 2) 3 . 4)");
    eval_and_check("(cons #t #f)", "(#true . #false)");
    eval_and_check("(cons \"hello\" 123)", "(\"hello\" . 123)");
}

Test(end_to_end_lists, test_list_length) {
    /* Test length */
    eval_and_check("(length (list 1 2 3 4))", "4");
    eval_and_check("(length '())", "0");
    eval_and_check("(length '(a))", "1");
    eval_and_check("(length (list 1 \"two\" #t 'sym))", "4");
    eval_and_check("(length '(1 (2 3) 4))", "3");
    eval_and_check("(length '((a b) (c d)))", "2");
    eval_and_check("(length '(1 () 3))", "3");
    eval_and_check("(length (cons 1 (cons 2 (cons 3 '()))))", "3");
    eval_and_check("(length '(\"hello\" #\\a 1/2 (1 . 2)))", "4");
    eval_and_check("(length (list (* 2 3) (cons 1 2) '()))", "3");
}

Test(end_to_end_lists, test_list_and_pair_selectors) {
    /* Test list-ref */
    eval_and_check("(list-ref (list 1 2 3 4) 2)", "3");
    eval_and_check("(list-ref '(a b c d) 0)", "a");
    eval_and_check("(list-ref '(a b c d) 3)", "d");
    eval_and_check("(list-ref (list #t \"hello\" 3/4 'sym) 1)", "\"hello\"");
    eval_and_check("(list-ref '(1 (2 3) 4) 1)", "(2 3)");
    eval_and_check("(list-ref '(a (b . c) d) 1)", "(b . c)");
    eval_and_check("(list-ref (cons 1 (cons 2 '())) 1)", "2");
    eval_and_check("(list-ref '(10 20 30 40) (+ 1 1))", "30");
}

Test(end_to_end_lists, test_car_cdr) {
    eval_and_check("(car '(a b c d))", "a");
    eval_and_check("(car (list 1/2 \"str\" #f))", "1/2");
    eval_and_check("(car '((1 2) 3 4))", "(1 2)");
    eval_and_check("(car '((1 . 2) 3 4))", "(1 . 2)");
    eval_and_check("(car '(a . b))", "a");
    eval_and_check("(cdr '(a b c d))", "(b c d)");
    eval_and_check("(cdr '(a b))", "(b)");
    eval_and_check("(cdr '(lonely))", "()");
    eval_and_check("(cdr (list #t 1/2 \"str\"))", "(1/2 \"str\")");
    eval_and_check("(cdr '(a (b c) d))", "((b c) d)");
    eval_and_check("(cdr '(a . b))", "b");
    eval_and_check("(cdr '(a b . c))", "(b . c)");
    eval_and_check("(car (cdr '(a b c)))", "b");
    eval_and_check("(car (cdr (cdr '(a b c))))", "c");
    eval_and_check("(cdr (cdr '(a b c d e)))", "(c d e)");
    eval_and_check("(car (car '((a b) (c d))))", "a");
    eval_and_check("(cdr (car '((a b) (c d))))", "(b)");
    eval_and_check("(car (cdr '(a (b . c) d)))", "(b . c)");
    eval_and_check("(car (car (cdr '(a (b . c) d))))", "b");
    eval_and_check("(cdr (car (cdr '(a (b . c) d))))", "c");
}


