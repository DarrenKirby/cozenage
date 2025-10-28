#include "test_meta.h"
#include <criterion/criterion.h>


TestSuite(end_to_end_lists);

// Test(end_to_end_lists, test_list_and_pair_constructors, .init = setup_each_test, .fini = teardown_each_test) {
//     /* Test list/pair constructors */
//     cr_assert_str_eq(t_eval("(list 1 2 3 4)"), "(1 2 3 4)");
//     cr_assert_str_eq(t_eval("(list '(1 2 3 4))"), "((1 2 3 4))");
//     cr_assert_str_eq(t_eval("(list '1 '2 '3 '4)"), "(1 2 3 4)");
//     cr_assert_str_eq(t_eval("(list)"), "()");
//     cr_assert_str_eq(t_eval("(list \"1\" \"2\" \"3\" \"4\")"), "(\"1\" \"2\" \"3\" \"4\")");
//     cr_assert_str_eq(t_eval("(list #\\c #\\d #\\g)"), "(#\\c #\\d #\\g)");
//     cr_assert_str_eq(t_eval("(cons 1 2)"), "(1 . 2)");
//     cr_assert_str_eq(t_eval("(cons 1 (cons 2 (cons 3 (cons 4 5))))"), "(1 2 3 4 . 5)");
//     cr_assert_str_eq(t_eval("(list 1 \"two\" 3/4 #t 'sym)"), "(1 \"two\" 3/4 #true sym)");
//     cr_assert_str_eq(t_eval("(list 1 '() 3)"), "(1 () 3)");
//     cr_assert_str_eq(t_eval("(list (list 1 2) (list 3 4))"), "((1 2) (3 4))");
//     cr_assert_str_eq(t_eval("(list (+ 1 1) (* 2 3) (- 10 4))"), "(2 6 6)");
//     cr_assert_str_eq(t_eval("(list (cons 1 2) 3)"), "((1 . 2) 3)");
//     cr_assert_str_eq(t_eval("(cons 'a '(b c d))"), "(a b c d)");
//     cr_assert_str_eq(t_eval("(cons 'a '())"), "(a)");
//     cr_assert_str_eq(t_eval("(cons 1 (cons 2 (cons 3 '())))"), "(1 2 3)");
//     cr_assert_str_eq(t_eval("(cons '(a b) '(c d))"), "((a b) c d)");
//     cr_assert_str_eq(t_eval("(cons (cons 1 2) 3)"), "((1 . 2) . 3)");
//     cr_assert_str_eq(t_eval("(cons (cons 1 2) (cons 3 4))"), "((1 . 2) 3 . 4)");
//     cr_assert_str_eq(t_eval("(cons #t #f)"), "(#true . #false)");
//     cr_assert_str_eq(t_eval("(cons \"hello\" 123)"), "(\"hello\" . 123)");
// }
//
// Test(end_to_end_lists, test_list_length, .init = setup_each_test, .fini = teardown_each_test) {
//     /* Test length */
//     cr_assert_str_eq(t_eval("(length (list 1 2 3 4))"), "4");
//     cr_assert_str_eq(t_eval("(length '())"), "0");
//     cr_assert_str_eq(t_eval("(length '(a))"), "1");
//     cr_assert_str_eq(t_eval("(length (list 1 \"two\" #t 'sym))"), "4");
//     cr_assert_str_eq(t_eval("(length '(1 (2 3) 4))"), "3");
//     cr_assert_str_eq(t_eval("(length '((a b) (c d)))"), "2");
//     cr_assert_str_eq(t_eval("(length '(1 () 3))"), "3");
//     cr_assert_str_eq(t_eval("(length (cons 1 (cons 2 (cons 3 '()))))"), "3");
//     cr_assert_str_eq(t_eval("(length '(\"hello\" #\\a 1/2 (1 . 2)))"), "4");
//     cr_assert_str_eq(t_eval("(length (list (* 2 3) (cons 1 2) '()))"), "3");
// }
//
// Test(end_to_end_lists, test_list_ref, .init = setup_each_test, .fini = teardown_each_test) {
//     /* Test list-ref */
//     cr_assert_str_eq(t_eval("(list-ref (list 1 2 3 4) 2)"), "3");
//     cr_assert_str_eq(t_eval("(list-ref '(a b c d) 0)"), "a");
//     cr_assert_str_eq(t_eval("(list-ref '(a b c d) 3)"), "d");
//     cr_assert_str_eq(t_eval("(list-ref (list #t \"hello\" 3/4 'sym) 1)"), "\"hello\"");
//     cr_assert_str_eq(t_eval("(list-ref '(1 (2 3) 4) 1)"), "(2 3)");
//     cr_assert_str_eq(t_eval("(list-ref '(a (b . c) d) 1)"), "(b . c)");
//     cr_assert_str_eq(t_eval("(list-ref (cons 1 (cons 2 '())) 1)"), "2");
//     cr_assert_str_eq(t_eval("(list-ref '(10 20 30 40) (+ 1 1))"), "30");
// }
//
// Test(end_to_end_lists, test_car_cdr, .init = setup_each_test, .fini = teardown_each_test) {
//     cr_assert_str_eq(t_eval("(car '(a b c d))"), "a");
//     cr_assert_str_eq(t_eval("(car (list 1/2 \"str\" #f))"), "1/2");
//     cr_assert_str_eq(t_eval("(car '((1 2) 3 4))"), "(1 2)");
//     cr_assert_str_eq(t_eval("(car '((1 . 2) 3 4))"), "(1 . 2)");
//     cr_assert_str_eq(t_eval("(car '(a . b))"), "a");
//     cr_assert_str_eq(t_eval("(cdr '(a b c d))"), "(b c d)");
//     cr_assert_str_eq(t_eval("(cdr '(a b))"), "(b)");
//     cr_assert_str_eq(t_eval("(cdr '(lonely))"), "()");
//     cr_assert_str_eq(t_eval("(cdr (list #t 1/2 \"str\"))"), "(1/2 \"str\")");
//     cr_assert_str_eq(t_eval("(cdr '(a (b c) d))"), "((b c) d)");
//     cr_assert_str_eq(t_eval("(cdr '(a . b))"), "b");
//     cr_assert_str_eq(t_eval("(cdr '(a b . c))"), "(b . c)");
//     cr_assert_str_eq(t_eval("(car (cdr '(a b c)))"), "b");
//     cr_assert_str_eq(t_eval("(car (cdr (cdr '(a b c))))"), "c");
//     cr_assert_str_eq(t_eval("(cdr (cdr '(a b c d e)))"), "(c d e)");
//     cr_assert_str_eq(t_eval("(car (car '((a b) (c d))))"), "a");
//     cr_assert_str_eq(t_eval("(cdr (car '((a b) (c d))))"), "(b)");
//     cr_assert_str_eq(t_eval("(car (cdr '(a (b . c) d)))"), "(b . c)");
//     cr_assert_str_eq(t_eval("(car (car (cdr '(a (b . c) d))))"), "b");
//     cr_assert_str_eq(t_eval("(cdr (car (cdr '(a (b . c) d))))"), "c");
// }
//
// Test(end_to_end_lists, test_append, .init = setup_each_test, .fini = teardown_each_test) {
//     /* Test append */
//     cr_assert_str_eq(t_eval("(append '(1 2 3) '(4 5 6))"), "(1 2 3 4 5 6)");
//     cr_assert_str_eq(t_eval("(append '(a b c) '(d e))"), "(a b c d e)");
//     cr_assert_str_eq(t_eval("(append '() '(1 2 3))"), "(1 2 3)");
//     cr_assert_str_eq(t_eval("(append '(1 2 3) '())"), "(1 2 3)");
//     cr_assert_str_eq(t_eval("(append '() '())"), "()");
//     cr_assert_str_eq(t_eval("(append)"), "()");
//     cr_assert_str_eq(t_eval("(append '(1 2 3))"), "(1 2 3)");
//     cr_assert_str_eq(t_eval("(append '(a) '(b c d) '(e f))"), "(a b c d e f)");
//
//     /* Per R7RS, the last argument to append can be any object */
//     cr_assert_str_eq(t_eval("(append '(1 2 3) 4)"), "(1 2 3 . 4)");
//     cr_assert_str_eq(t_eval("(append '(a b) 'c)"), "(a b . c)");
//
//     /* Nested lists */
//     cr_assert_str_eq(t_eval("(append '((1 2)) '((3 4)))"), "((1 2) (3 4))");
// }
//
// Test(end_to_end_lists, test_reverse, .init = setup_each_test, .fini = teardown_each_test) {
//     /* Test reverse */
//     cr_assert_str_eq(t_eval("(reverse '(1 2 3 4))"), "(4 3 2 1)");
//     cr_assert_str_eq(t_eval("(reverse '(a b c))"), "(c b a)");
//     cr_assert_str_eq(t_eval("(reverse '(a (b c) d (e (f))))"), "((e (f)) d (b c) a)");
//     cr_assert_str_eq(t_eval("(reverse '())"), "()");
//     cr_assert_str_eq(t_eval("(reverse '(1))"), "(1)");
//     cr_assert_str_eq(t_eval("(reverse (list 1 \"hello\" #t))"), "(#true \"hello\" 1)");
//     cr_assert_str_eq(t_eval("(reverse (reverse '(1 2 3)))"), "(1 2 3)");
//     cr_assert_str_eq(t_eval("(reverse (cons 1 (cons 2 '())))"), "(2 1)");
// }
//
// Test(end_to_end_lists, test_list_tail, .init = setup_each_test, .fini = teardown_each_test) {
//     /* Test list-tail */
//     cr_assert_str_eq(t_eval("(list-tail '(a b c d) 0)"), "(a b c d)");
//     cr_assert_str_eq(t_eval("(list-tail '(a b c d) 2)"), "(c d)");
//     cr_assert_str_eq(t_eval("(list-tail '(a b c d) 4)"), "()");
//     cr_assert_str_eq(t_eval("(list-tail '() 0)"), "()");
//     cr_assert_str_eq(t_eval("(list-tail '(1) 1)"), "()");
//     cr_assert_str_eq(t_eval("(list-tail '(a (b c) d) 1)"), "((b c) d)");
//     cr_assert_str_eq(t_eval("(list-tail '(10 20 30) (+ 1 1))"), "(30)");
// }
//
// Test(end_to_end_lists, test_filter_procedure, .init = setup_each_test, .fini = teardown_each_test) {
//     /* Test filter with basic predicates */
//     cr_assert_str_eq(t_eval("(filter odd? '(1 2 3 4 5 6))"), "(1 3 5)");
//     cr_assert_str_eq(t_eval("(filter even? '(1 2 3 4 5 6))"), "(2 4 6)");
//     cr_assert_str_eq(t_eval("(filter symbol? '(a 1 \"b\" #\\c d))"), "(a d)");
//     cr_assert_str_eq(t_eval("(filter string? '(\"a\" 1 \"b\" #\\c d))"), "(\"a\" \"b\")");
//     cr_assert_str_eq(t_eval("(filter number? '(a 1.5 2 #f))"), "(1.5 2)");
//     cr_assert_str_eq(t_eval("(filter pair? '((1 . 2) 3 (4) ()))"), "((1 . 2) (4))");
//
//     /* Test with lambda procedures */
//     cr_assert_str_eq(t_eval("(filter (lambda (x) (> x 5)) '(1 10 3 8 5 0))"), "(10 8)");
//     cr_assert_str_eq(t_eval("(filter (lambda (x) (<= (string-length x) 2)) '(\"a\" \"bb\" \"ccc\" \"\"))"), "(\"a\" \"bb\" \"\")");
//     cr_assert_str_eq(t_eval("(filter (lambda (lst) (memq 'a lst)) '((x y) (a b) (c d) (e a)))"), "((a b) (e a))");
//
//     /* Test edge cases */
//     cr_assert_str_eq(t_eval("(filter odd? '())"), "()");
//     cr_assert_str_eq(t_eval("(filter even? '(1 3 5))"), "()");
//     cr_assert_str_eq(t_eval("(filter integer? '(1 2 3))"), "(1 2 3)");
//
//     /* Test truthiness: any value other than #f is true */
//     cr_assert_str_eq(t_eval("(filter (lambda (x) x) '(1 #f \"hello\" () 0))"), "(1 \"hello\" () 0)");
//     cr_assert_str_eq(t_eval("(filter (lambda (x) #t) '(a b c))"), "(a b c)");
//     cr_assert_str_eq(t_eval("(filter (lambda (x) #f) '(a b c))"), "()");
//
//     /* Ensure filter does not modify the original list */
//     cr_assert_str_eq(t_eval("(begin (define my-list '(1 2 3 4)) (filter even? my-list) my-list)"), "(1 2 3 4)");
//
//     /* The following tests are for error conditions. */
//     cr_assert_str_eq(t_eval("(filter)"), " Arity error: expected exactly 2 args, got 0");
//     cr_assert_str_eq(t_eval("(filter odd?)"), " Arity error: expected exactly 2 args, got 1");
//     cr_assert_str_eq(t_eval("(filter 1 '(1 2 3))"), " Type error: filter: arg 1 must be a procedure");
//     cr_assert_str_eq(t_eval("(filter odd? 123)"), " Type error: filter: arg 2 must be a proper list");
//     cr_assert_str_eq(t_eval("(filter odd? '(1 . 2))"), " Type error: filter: arg 2 must be a proper list");
// }
//
// Test(end_to_end_lists, test_foldl_procedure, .init = setup_each_test, .fini = teardown_each_test) {
//     /* Test foldl with a single list */
//     cr_assert_str_eq(t_eval("(foldl + 0 '(1 2 3 4 5))"), "15");
//     cr_assert_str_eq(t_eval("(foldl * 1 '(2 3 4 5))"), "120");
//     /* Use a non-commutative operation to verify left-to-right order */
//     cr_assert_str_eq(t_eval("(foldl - 10 '(1 2 3))"), "-8");
//     /* A classic use case: reversing a list */
//     cr_assert_str_eq(t_eval("(foldl (lambda (acc elem) (cons acc elem)) '() '(a b c d))"), "(d c b a)");
//
//     /* Test with different initial values */
//     cr_assert_str_eq(t_eval("(foldl + 100 '(1 2 3))"), "106");
//     cr_assert_str_eq(t_eval("(foldl cons '(z) '(a b c))"), "(c b a z)");
//
//     /* Test foldl with multiple lists */
//     cr_assert_str_eq(t_eval("(foldl + 0 '(1 2 3) '(10 20 30))"), "66"); /* (+ (+ (+ 0 1 10) 2 20) 3 30) */
//     cr_assert_str_eq(t_eval("(foldl list '() '(a b c) '(1 2 3))"), "(c 3 (b 2 (a 1 ())))");
//     cr_assert_str_eq(t_eval("(foldl + 0 '(1 2 3) '(4 5) '(6))"), "11"); /* Stops when the shortest list is exhausted */
//
//     /* Test with mixed types */
//     cr_assert_str_eq(t_eval("(foldl (lambda (acc x) (if (number? x) (+ acc x) acc)) 0 '(a 1 b 2 c 3))"), " Type error: bad type at arg 1: got symbol, expected integer|real|rational|complex");
//
//     /* Edge cases */
//     cr_assert_str_eq(t_eval("(foldl + 42 '())"), "42");
//     cr_assert_str_eq(t_eval("(foldl + 0 '(10))"), "10");
//     cr_assert_str_eq(t_eval("(foldl + 0 '() '(1 2 3))"), "0"); /* Returns init immediately if any list is empty */
//
//     /* The following tests are for error conditions. */
//     cr_assert_str_eq(t_eval("(foldl)"), " Arity error: expected at least 3 args, got 0");
//     cr_assert_str_eq(t_eval("(foldl +)"), " Arity error: expected at least 3 args, got 1");
//     cr_assert_str_eq(t_eval("(foldl + 0)"), " Arity error: expected at least 3 args, got 2");
//     cr_assert_str_eq(t_eval("(foldl 1 0 '(1 2 3))"), " Type error: foldl: arg 1 must be a procedure");
//     cr_assert_str_eq(t_eval("(foldl + 0 '(a . b))"), " Type error: foldl: arg 3 must be a proper list");
//     cr_assert_str_eq(t_eval("(foldl + 0 '(1 2) '(a . b))"), " Type error: foldl: arg 4 must be a proper list");
// }

Test(end_to_end_lists, test_map_procedure, .init = setup_each_test, .fini = teardown_each_test) {
    /* Test map with a single list argument */
    cr_assert_str_eq(t_eval("(map (lambda (x) (* x 2)) '(1 2 3 4))"), "(2 4 6 8)");
    cr_assert_str_eq(t_eval("(map car '((a 1) (b 2) (c 3)))"), "(a b c)");
    cr_assert_str_eq(t_eval("(map cdr '((a 1) (b 2) (c 3)))"), "((1) (2) (3))");
    cr_assert_str_eq(t_eval("(map (lambda (x) (cons x '())) '(a b c))"), "((a) (b) (c))");

    /* Test map with multiple list arguments */
    cr_assert_str_eq(t_eval("(map + '(1 2 3) '(10 20 30))"), "(11 22 33)");
    cr_assert_str_eq(t_eval("(map list '(a b c) '(1 2 3))"), "((a 1) (b 2) (c 3))");
    cr_assert_str_eq(t_eval("(map + '(1 2 3) '(4 5) '(6 7))"), "(11 14)"); /* Stops when shortest list is exhausted */
    cr_assert_str_eq(t_eval("(map * '(1 2 3 4) '(10 20))"), "(10 40)");
    cr_assert_str_eq(t_eval("(map + '(1) '(2) '(3) '(4))"), "(10)");

    /* Edge cases */
    cr_assert_str_eq(t_eval("(map (lambda (x) x) '())"), "()");
    cr_assert_str_eq(t_eval("(map + '() '(1 2 3))"), "()");
    cr_assert_str_eq(t_eval("(map + '(1 2 3) '())"), "()");

    /* Test that map returns a newly allocated list */
    cr_assert_str_eq(t_eval("(begin (define a (list 1 2 3)) a)"), "(1 2 3)");
    cr_assert_str_eq(t_eval("(begin (define b (list 1 2 3)) (define c (map (lambda (x) x) b)) (eq? b c))"), "#false");

    /* The following tests are for error conditions. */
    cr_assert_str_eq(t_eval("(map)"), " Arity error: expected at least 2 args, got 0");
    cr_assert_str_eq(t_eval("(map +)"), " Arity error: expected at least 2 args, got 1");
    cr_assert_str_eq(t_eval("(map 1 '(1 2 3))"), " Type error: map: arg 1 must be a procedure");
    cr_assert_str_eq(t_eval("(map + '(1 2) 3)"), " Type error: map: arg 3 must be a proper list");
    cr_assert_str_eq(t_eval("(map + '(1 . 2))"), " Type error: map: arg 2 must be a proper list");
    cr_assert_str_eq(t_eval("(map + '(1 2) '(3 . 4))"), " Type error: map: arg 3 must be a proper list");
}
