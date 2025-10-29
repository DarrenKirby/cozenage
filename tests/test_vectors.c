#include "test_meta.h"
#include <criterion/criterion.h>

TestSuite(end_to_end_vectors);

Test(end_to_end_vectors, test_vector_constructor, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Basic construction ##
    cr_assert_str_eq(t_eval("(vector? (vector 1 2 3))"), "#true");
    cr_assert_str_eq(t_eval("(vector->list (vector 1 #\\a \"foo\" #true 's))"), "(1 #\\a \"foo\" #true s)");

    // ## Empty vector ##
    cr_assert_str_eq(t_eval("(vector-length (vector))"), "0");
    cr_assert_str_eq(t_eval("(equal? (vector) #())"), "#true");

    // ## Arity ##
    // `vector` can take any number of arguments, so it doesn't have arity errors in the typical sense.
    // A call with no args is valid.
    cr_assert_str_eq(t_eval("(vector)"), "#()");
}

Test(end_to_end_vectors, test_vector_length, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Length of various vectors ##
    cr_assert_str_eq(t_eval("(vector-length #())"), "0");
    cr_assert_str_eq(t_eval("(vector-length #(1))"), "1");
    cr_assert_str_eq(t_eval("(vector-length #(a b c d e))"), "5");
    cr_assert_str_eq(t_eval("(vector-length (vector 1 2 3))"), "3");

    // ## Type Errors ##
    cr_assert_str_eq(t_eval("(vector-length '(1 2 3))"), " Type error: bad type at arg 1: got pair, expected vector");
    cr_assert_str_eq(t_eval("(vector-length \"abc\")"), " Type error: bad type at arg 1: got string, expected vector");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(vector-length)"), " Arity error: expected exactly 1 arg, got 0");
    cr_assert_str_eq(t_eval("(vector-length #(1) #(2))"), " Arity error: expected exactly 1 arg, got 2");
}

Test(end_to_end_vectors, test_make_vector, .init = setup_each_test, .fini = teardown_each_test) {
    // ## One-argument form (no fill) ##
    cr_assert_str_eq(t_eval("(vector-length (make-vector 5))"), "5");

    // ## Two-argument form (with fill) ##
    cr_assert_str_eq(t_eval("(make-vector 3 'a)"), "#(a a a)");
    cr_assert_str_eq(t_eval("(make-vector 4 1.5)"), "#(1.5 1.5 1.5 1.5)");
    cr_assert_str_eq(t_eval("(make-vector 2 #true)"), "#(#true #true)");
    cr_assert_str_eq(t_eval("(make-vector 0 99)"), "#()");

    // ## Type Errors ##
    cr_assert_str_eq(t_eval("(make-vector -1)"), " Value error: make-vector: arg 1 must be non-negative");
    cr_assert_str_eq(t_eval("(make-vector 1.5)"), " Type error: make-vector: arg 1 must be an integer");
    cr_assert_str_eq(t_eval("(make-vector 'a)"), " Type error: make-vector: arg 1 must be an integer");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(make-vector)"), " Arity error: expected at least 1 arg, got 0");
    cr_assert_str_eq(t_eval("(make-vector 1 2 3)"), " Arity error: expected at most 2 args, got 3");
}

Test(end_to_end_vectors, test_vector_ref, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Basic access ##
    cr_assert_str_eq(t_eval("(vector-ref #(a b c) 0)"), "a");
    cr_assert_str_eq(t_eval("(vector-ref #(a b c) 2)"), "c");
    cr_assert_str_eq(t_eval("(vector-ref #(1 \"s\" #true) 1)"), "\"s\"");

    // ## Index Errors (out of bounds) ##
    cr_assert_str_eq(t_eval("(vector-ref #(a b) -1)"), " Index error: vector-ref: index out of bounds");
    cr_assert_str_eq(t_eval("(vector-ref #(a b) 2)"), " Index error: vector-ref: index out of bounds");
    cr_assert_str_eq(t_eval("(vector-ref #() 0)"), " Index error: vector-ref: index out of bounds");

    // ## Type Errors ##
    cr_assert_str_eq(t_eval("(vector-ref '(1 2) 0)"), " Type error: vector-ref: arg 1 must be a vector");
    cr_assert_str_eq(t_eval("(vector-ref #(1 2) 'a)"), " Type error: vector-ref: arg 2 must be an exact integer");
    cr_assert_str_eq(t_eval("(vector-ref #(1 2) 1.0)"), " Type error: vector-ref: arg 2 must be an exact integer");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(vector-ref)"), " Arity error: expected exactly 2 args, got 0");
    cr_assert_str_eq(t_eval("(vector-ref #(1))"), " Arity error: expected exactly 2 args, got 1");
}

Test(end_to_end_vectors, test_vector_set, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Basic mutation ##
    cr_assert_str_eq(t_eval("(begin (define v #(1 2 3)) (vector-set! v 1 'x) v)"), "#(1 x 3)");
    cr_assert_str_eq(t_eval("(begin (define v #(a b c)) (vector-set! v 0 99) v)"), "#(99 b c)");
    cr_assert_str_eq(t_eval("(begin (define v #(a b c)) (vector-set! v 2 '(x)) v)"), "#(a b (x))");

    // ## Index Errors (out of bounds) ##
    cr_assert_str_eq(t_eval("(vector-set! #(a b) 2 'x)"), " Index error: vector->set!: index out of range");
    cr_assert_str_eq(t_eval("(vector-set! #() 0 'x)"), " Index error: vector->set!: index out of range");

    // ## Type Errors ##
    cr_assert_str_eq(t_eval("(vector-set! '(1 2) 0 9)"), " Type error: vector->set!: arg must be a vector");
    cr_assert_str_eq(t_eval("(vector-set! #(1 2) 'a 9)"), " Type error: vector->set!: arg must be an integer");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(vector-set!)"), " Arity error: expected exactly 3 args, got 0");
    cr_assert_str_eq(t_eval("(vector-set! #(1) 0)"), " Arity error: expected exactly 3 args, got 2");
}

Test(end_to_end_vectors, test_vector_fill, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Basic fill ##
    cr_assert_str_eq(t_eval("(begin (define v #(1 2 3 4)) (vector-fill! v 'x) v)"), "#(x x x x)");
    cr_assert_str_eq(t_eval("(begin (define v (make-vector 5)) (vector-fill! v 0) v)"), "#(0 0 0 0 0)");

    // ## Fill empty vector (should be a no-op) ##
    cr_assert_str_eq(t_eval("(begin (define v #()) (vector-fill! v 'x) v)"), "#()");

    // ## Type Errors ##
    cr_assert_str_eq(t_eval("(vector-fill! '(1 2) 0)"), " Type error: vector-fill!: arg 1 must be a vector");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(vector-fill!)"), " Arity error: expected at least 2 args, got 0");
    cr_assert_str_eq(t_eval("(vector-fill! #(1))"), " Arity error: expected at least 2 args, got 1");
}

Test(end_to_end_vectors, test_vector_copy, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Basic copy ##
    cr_assert_str_eq(t_eval("(equal? #(1 2 3) (vector-copy #(1 2 3)))"), "#true");
    cr_assert_str_eq(t_eval("(eq? #(1 2 3) (vector-copy #(1 2 3)))"), "#false");

    // ## Shallow copy behavior ##
    cr_assert_str_eq(t_eval("(begin "
                                "(define v1 #(1 2 3)) "
                                "(define v2 (vector-copy v1)) "
                                "(vector-set! v1 0 99) "
                                "v2)"), "#(1 2 3)"); // v2 is unchanged

    cr_assert_str_eq(t_eval("(begin "
                                "(define lst '(a)) "
                                "(define v1 (vector lst)) "
                                "(define v2 (vector-copy v1)) "
                                "(set-car! (vector-ref v1 0) 'b) "
                                "v2)"), "#((b))"); // v2 reflects change because it's a shallow copy

    // ## Arity ##
    cr_assert_str_eq(t_eval("(vector-copy)"), " Arity error: expected at least 1 arg, got 0");
}

Test(end_to_end_vectors, test_vector_append, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Appending vectors ##
    cr_assert_str_eq(t_eval("(vector-append #(1 2) #(3 4))"), "#(1 2 3 4)");
    cr_assert_str_eq(t_eval("(vector-append #() #(a b))"), "#(a b)");
    cr_assert_str_eq(t_eval("(vector-append #(a b) #())"), "#(a b)");
    cr_assert_str_eq(t_eval("(vector-append #() #())"), "#()");
    cr_assert_str_eq(t_eval("(vector-append #(1) #(2) #(3) #(4))"), "#(1 2 3 4)");

    // ## No-op ##
    cr_assert_str_eq(t_eval("(vector-append)"), "#()");
    cr_assert_str_eq(t_eval("(vector-append #(a b))"), "#(a b)");

    // ## Type Errors ##
    cr_assert_str_eq(t_eval("(vector-append #(1) '(2))"), " Type error: bad type at arg 2: got pair, expected vector");
}

Test(end_to_end_vectors, test_list_to_vector, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert_str_eq(t_eval("(list->vector '(1 2 3))"), "#(1 2 3)");
    cr_assert_str_eq(t_eval("(list->vector '())"), "#()");
    cr_assert_str_eq(t_eval("(list->vector (list 1 #true \"s\"))"), "#(1 #true \"s\")");

    // ## Type Errors ##
    cr_assert_str_eq(t_eval("(list->vector '(a . b))"), " Type error: list->vector: arg 1 must be a proper list");
    cr_assert_str_eq(t_eval("(list->vector 'a)"), " Type error: list->vector: arg 1 must be a list");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(list->vector)"), " Arity error: expected exactly 1 arg, got 0");
}

Test(end_to_end_vectors, test_vector_to_list, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert_str_eq(t_eval("(vector->list #(1 2 3))"), "(1 2 3)");
    cr_assert_str_eq(t_eval("(vector->list #())"), "()");
    cr_assert_str_eq(t_eval("(vector->list #(a #false \"b\"))"), "(a #false \"b\")");

    // ## Type Errors ##
    cr_assert_str_eq(t_eval("(vector->list '(1 2))"), " Type error: vector->list: arg 1 must be a vector");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(vector->list)"), " Arity error: expected at least 1 arg, got 0");
}

Test(end_to_end_vectors, test_string_to_vector, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert_str_eq(t_eval("(string->vector \"abc\")"), "#(#\\a #\\b #\\c)");
    cr_assert_str_eq(t_eval("(string->vector \"\")"), "#()");
    /* FIXME: this test returns an empty string. It should not */
    //cr_assert_str_eq(t_eval("(string->vector \"λ\")"), "#(#\\λ)");

    // ## Type Errors ##
    cr_assert_str_eq(t_eval("(string->vector 'abc)"), " Type error: string->vector: arg1 must be a string");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(string->vector)"), " Arity error: expected at least 1 arg, got 0");
}

Test(end_to_end_vectors, test_vector_to_string, .init = setup_each_test, .fini = teardown_each_test) {
    cr_assert_str_eq(t_eval("(vector->string #(#\\a #\\b #\\c))"), "\"abc\"");
    cr_assert_str_eq(t_eval("(vector->string #())"), "\"\"");
    cr_assert_str_eq(t_eval("(vector->string #(#\\S #\\p #\\a #\\c #\\e))"), "\"Space\"");

    // ## Type Errors ##
    cr_assert_str_eq(t_eval("(vector->string #(#\\a 1 #\\c))"), " Type error: vector->string: vector must have only chars as members");
    cr_assert_str_eq(t_eval("(vector->string '(#\\a))"), " Type error: vector->string: arg must be a vector");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(vector->string)"), " Arity error: expected at least 1 arg, got 0");
}
