#include <criterion/criterion.h>

#include "test_meta.h"
#include <gc/gc.h>

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

    // Named let
    cr_assert_str_eq(t_eval("(let loop ((i 3)) (if (= i 0) 'done (loop (- i 1))))"), "done");
}

Test(end_to_end_sf, test_conditionals, .init = setup_each_test, .fini = teardown_each_test) {
    // cond
    cr_assert_str_eq(t_eval("(cond (#f 1) ((= 1 2) 2) (else 3))"), "3");
    cr_assert_str_eq(t_eval("(cond ((+ 1 1) 'truthy))"), "truthy");

    // case
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

Test(end_to_end_sf, test_conditionals_exhaustive, .init = setup_each_test, .fini = teardown_each_test) {
    // cond: arrow form
    cr_assert_str_eq(t_eval("(cond ((assoc 'b '((a . 1) (b . 2))) => cdr))"), "2");

    // cond: test-only (should return the result of the test)
    cr_assert_str_eq(t_eval("(cond ((member 2 '(1 2 3))))"), "(2 3)");

    // cond: implicit begin in clauses
    cr_assert_str_eq(t_eval("(cond (#t (define a 1) (define b 2) (+ a b)))"), "3");

    // case: atom vs list of keys
    cr_assert_str_eq(t_eval("(case 'a ((b c) 1) ((a) 2))"), "2");

    // case: with expressions as the key
    cr_assert_str_eq(t_eval("(case (+ 1 1) ((2) 'yes) (else 'no))"), "yes");
}

Test(end_to_end_sf, test_logic_exhaustive, .init = setup_each_test, .fini = teardown_each_test) {
    // and: returns last value if all true
    cr_assert_str_eq(t_eval("(and 1 'a \"ok\")"), "\"ok\"");
    // and: short-circuit (should not reach the error)
    cr_assert_str_eq(t_eval("(and #f (/ 1 0))"), "#false");
    // and: empty
    cr_assert_str_eq(t_eval("(and)"), "#true");

    // or: returns first truthy value
    cr_assert_str_eq(t_eval("(or #f 0 #f)"), "0"); // 0 is truthy in Scheme!
    // or: short-circuit
    cr_assert_str_eq(t_eval("(or 1 (/ 1 0))"), "1");
    // or: empty
    cr_assert_str_eq(t_eval("(or)"), "#false");
}

Test(end_to_end_sf, test_bindings_exhaustive, .init = setup_each_test, .fini = teardown_each_test) {
    // letrec*: sequential recursive initialization
    // In letrec, this might fail. In letrec*, 'y' must see 'x' as 10.
    cr_assert_str_eq(t_eval("(letrec* ((x 10) (y (+ x 5))) y)"), "15");

    // Nested Named Let (Shadowing check)
    cr_assert_str_eq(t_eval(
        "(let loop ((x 5)) "
        "  (if (= x 0) "
        "      'done "
        "      (let ((x 100)) (loop 0))))"), "done");

    // let: check that bindings are simultaneous (x should refer to outer x)
    cr_assert_str_eq(t_eval("(begin (define x 1) (let ((x 2) (y x)) y))"), "1");
}

Test(end_to_end_sf, test_internal_defines, .init = setup_each_test, .fini = teardown_each_test) {
    // Internal define in when
    cr_assert_str_eq(t_eval("(when #t (define a 10) (define b 20) (+ a b))"), "30");

    // Internal define in lambda
    cr_assert_str_eq(t_eval("((lambda (x) (define y 10) (+ x y)) 5)"), "15");

    // Internal define in unless
    cr_assert_str_eq(t_eval("(unless #f (define z \"ok\") z)"), "\"ok\"");
}

Test(end_to_end_sf, test_gc_stress, .init = setup_each_test, .fini = teardown_each_test) {
    GC_gcollect(); // Force a collection before we start
    const size_t heap_before = GC_get_heap_size();

    // 1 Million iterations of a do-loop transform
    const char* result = t_eval(
        "(do ((i 0 (+ i 1)) (sum 0 (+ sum i))) "
        "    ((>= i 1000000) \"Done\"))");

    GC_gcollect(); // Force a collection after we finish
    const size_t heap_after = GC_get_heap_size();

    printf("\n[GC Stats] Heap Before: %zu bytes\n", heap_before);
    printf("[GC Stats] Heap After:  %zu bytes\n", heap_after);
    printf("[GC Stats] Growth:      %zd bytes\n", (ssize_t)heap_after - (ssize_t)heap_before);

    cr_assert_str_eq(result, "\"Done\"");
}

//Test(end_to_end_sf, test_map_procedure, .init = setup_each_test, .fini = teardown_each_test) {}

//Test(end_to_end_sf, test_map_procedure, .init = setup_each_test, .fini = teardown_each_test) {}

//Test(end_to_end_sf, test_map_procedure, .init = setup_each_test, .fini = teardown_each_test) {}