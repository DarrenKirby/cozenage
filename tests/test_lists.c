#include "test_meta.h"
#include <criterion/criterion.h>

TestSuite(end_to_end_lists, .init = suite_setup_wrapper, .fini = teardown_suite);

Test(end_to_end_lists, test_list_and_pair_constructors) {
    /* Test list/pair constructors */
    cr_assert_str_eq(t_eval("(list 1 2 3 4)"), "(1 2 3 4)");
    cr_assert_str_eq(t_eval("(list '(1 2 3 4))"), "((1 2 3 4))");
    cr_assert_str_eq(t_eval("(list '1 '2 '3 '4)"), "(1 2 3 4)");
    cr_assert_str_eq(t_eval("(list)"), "()");
    cr_assert_str_eq(t_eval("(list \"1\" \"2\" \"3\" \"4\")"), "(\"1\" \"2\" \"3\" \"4\")");
    cr_assert_str_eq(t_eval("(list #\\c #\\d #\\g)"), "(#\\c #\\d #\\g)");
    cr_assert_str_eq(t_eval("(cons 1 2)"), "(1 . 2)");
    cr_assert_str_eq(t_eval("(cons 1 (cons 2 (cons 3 (cons 4 5))))"), "(1 2 3 4 . 5)");
    cr_assert_str_eq(t_eval("(list 1 \"two\" 3/4 #t 'sym)"), "(1 \"two\" 3/4 #true sym)");
    cr_assert_str_eq(t_eval("(list 1 () 3)"), "(1 () 3)");
    cr_assert_str_eq(t_eval("(list (list 1 2) (list 3 4))"), "((1 2) (3 4))");
    cr_assert_str_eq(t_eval("(list (+ 1 1) (* 2 3) (- 10 4))"), "(2 6 6)");
    cr_assert_str_eq(t_eval("(list (cons 1 2) 3)"), "((1 . 2) 3)");
    cr_assert_str_eq(t_eval("(cons 'a '(b c d))"), "(a b c d)");
    cr_assert_str_eq(t_eval("(cons 'a '())"), "(a)");
    cr_assert_str_eq(t_eval("(cons 1 (cons 2 (cons 3 '())))"), "(1 2 3)");
    cr_assert_str_eq(t_eval("(cons '(a b) '(c d))"), "((a b) c d)");
    cr_assert_str_eq(t_eval("(cons (cons 1 2) 3)"), "((1 . 2) . 3)");
    cr_assert_str_eq(t_eval("(cons (cons 1 2) (cons 3 4))"), "((1 . 2) 3 . 4)");
    cr_assert_str_eq(t_eval("(cons #t #f)"), "(#true . #false)");
    cr_assert_str_eq(t_eval("(cons \"hello\" 123)"), "(\"hello\" . 123)");
}

Test(end_to_end_lists, test_list_length) {
    /* Test length */
    cr_assert_str_eq(t_eval("(length (list 1 2 3 4))"), "4");
    cr_assert_str_eq(t_eval("(length '())"), "0");
    cr_assert_str_eq(t_eval("(length '(a))"), "1");
    cr_assert_str_eq(t_eval("(length (list 1 \"two\" #t 'sym))"), "4");
    cr_assert_str_eq(t_eval("(length '(1 (2 3) 4))"), "3");
    cr_assert_str_eq(t_eval("(length '((a b) (c d)))"), "2");
    cr_assert_str_eq(t_eval("(length '(1 () 3))"), "3");
    cr_assert_str_eq(t_eval("(length (cons 1 (cons 2 (cons 3 '()))))"), "3");
    cr_assert_str_eq(t_eval("(length '(\"hello\" #\\a 1/2 (1 . 2)))"), "4");
    cr_assert_str_eq(t_eval("(length (list (* 2 3) (cons 1 2) '()))"), "3");
}

Test(end_to_end_lists, test_list_ref) {
    /* Test list-ref */
    cr_assert_str_eq(t_eval("(list-ref (list 1 2 3 4) 2)"), "3");
    cr_assert_str_eq(t_eval("(list-ref '(a b c d) 0)"), "a");
    cr_assert_str_eq(t_eval("(list-ref '(a b c d) 3)"), "d");
    cr_assert_str_eq(t_eval("(list-ref (list #t \"hello\" 3/4 'sym) 1)"), "\"hello\"");
    cr_assert_str_eq(t_eval("(list-ref '(1 (2 3) 4) 1)"), "(2 3)");
    cr_assert_str_eq(t_eval("(list-ref '(a (b . c) d) 1)"), "(b . c)");
    cr_assert_str_eq(t_eval("(list-ref (cons 1 (cons 2 '())) 1)"), "2");
    cr_assert_str_eq(t_eval("(list-ref '(10 20 30 40) (+ 1 1))"), "30");
}

Test(end_to_end_lists, test_car_cdr) {
    cr_assert_str_eq(t_eval("(car '(a b c d))"), "a");
    cr_assert_str_eq(t_eval("(car (list 1/2 \"str\" #f))"), "1/2");
    cr_assert_str_eq(t_eval("(car '((1 2) 3 4))"), "(1 2)");
    cr_assert_str_eq(t_eval("(car '((1 . 2) 3 4))"), "(1 . 2)");
    cr_assert_str_eq(t_eval("(car '(a . b))"), "a");
    cr_assert_str_eq(t_eval("(cdr '(a b c d))"), "(b c d)");
    cr_assert_str_eq(t_eval("(cdr '(a b))"), "(b)");
    cr_assert_str_eq(t_eval("(cdr '(lonely))"), "()");
    cr_assert_str_eq(t_eval("(cdr (list #t 1/2 \"str\"))"), "(1/2 \"str\")");
    cr_assert_str_eq(t_eval("(cdr '(a (b c) d))"), "((b c) d)");
    cr_assert_str_eq(t_eval("(cdr '(a . b))"), "b");
    cr_assert_str_eq(t_eval("(cdr '(a b . c))"), "(b . c)");
    cr_assert_str_eq(t_eval("(car (cdr '(a b c)))"), "b");
    cr_assert_str_eq(t_eval("(car (cdr (cdr '(a b c))))"), "c");
    cr_assert_str_eq(t_eval("(cdr (cdr '(a b c d e)))"), "(c d e)");
    cr_assert_str_eq(t_eval("(car (car '((a b) (c d))))"), "a");
    cr_assert_str_eq(t_eval("(cdr (car '((a b) (c d))))"), "(b)");
    cr_assert_str_eq(t_eval("(car (cdr '(a (b . c) d)))"), "(b . c)");
    cr_assert_str_eq(t_eval("(car (car (cdr '(a (b . c) d))))"), "b");
    cr_assert_str_eq(t_eval("(cdr (car (cdr '(a (b . c) d))))"), "c");
}

Test(end_to_end_lists, test_append) {
    /* Test append */
    cr_assert_str_eq(t_eval("(append '(1 2 3) '(4 5 6))"), "(1 2 3 4 5 6)");
    cr_assert_str_eq(t_eval("(append '(a b c) '(d e))"), "(a b c d e)");
    cr_assert_str_eq(t_eval("(append '() '(1 2 3))"), "(1 2 3)");
    cr_assert_str_eq(t_eval("(append '(1 2 3) '())"), "(1 2 3)");
    cr_assert_str_eq(t_eval("(append '() '())"), "()");
    cr_assert_str_eq(t_eval("(append)"), "()");
    cr_assert_str_eq(t_eval("(append '(1 2 3))"), "(1 2 3)");
    cr_assert_str_eq(t_eval("(append '(a) '(b c d) '(e f))"), "(a b c d e f)");

    /* Per R7RS, the last argument to append can be any object */
    cr_assert_str_eq(t_eval("(append '(1 2 3) 4)"), "(1 2 3 . 4)");
    cr_assert_str_eq(t_eval("(append '(a b) 'c)"), "(a b . c)");

    /* Nested lists */
    cr_assert_str_eq(t_eval("(append '((1 2)) '((3 4)))"), "((1 2) (3 4))");
}

Test(end_to_end_lists, test_reverse) {
    /* Test reverse */
    cr_assert_str_eq(t_eval("(reverse '(1 2 3 4))"), "(4 3 2 1)");
    cr_assert_str_eq(t_eval("(reverse '(a b c))"), "(c b a)");
    cr_assert_str_eq(t_eval("(reverse '(a (b c) d (e (f))))"), "((e (f)) d (b c) a)");
    cr_assert_str_eq(t_eval("(reverse '())"), "()");
    cr_assert_str_eq(t_eval("(reverse '(1))"), "(1)");
    cr_assert_str_eq(t_eval("(reverse (list 1 \"hello\" #t))"), "(#true \"hello\" 1)");
    cr_assert_str_eq(t_eval("(reverse (reverse '(1 2 3)))"), "(1 2 3)");
    cr_assert_str_eq(t_eval("(reverse (cons 1 (cons 2 '())))"), "(2 1)");
}

Test(end_to_end_lists, test_list_tail) {
    /* Test list-tail */
    cr_assert_str_eq(t_eval("(list-tail '(a b c d) 0)"), "(a b c d)");
    cr_assert_str_eq(t_eval("(list-tail '(a b c d) 2)"), "(c d)");
    cr_assert_str_eq(t_eval("(list-tail '(a b c d) 4)"), "()");
    cr_assert_str_eq(t_eval("(list-tail '() 0)"), "()");
    cr_assert_str_eq(t_eval("(list-tail '(1) 1)"), "()");
    cr_assert_str_eq(t_eval("(list-tail '(a (b c) d) 1)"), "((b c) d)");
    cr_assert_str_eq(t_eval("(list-tail '(10 20 30) (+ 1 1))"), "(30)");
}
