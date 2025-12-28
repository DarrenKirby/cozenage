#include "test_meta.h"
#include <criterion/criterion.h>

TestSuite(end_to_end_equivalence);


Test(end_to_end_equivalence, test_eq_procedure, .init = setup_each_test, .fini = teardown_each_test) {
    /*
     * R7RS Section 6.1: Equivalence predicates
     * `eq?` returns #true if its arguments are the same object, and #false otherwise.
     * Note: The behavior of `eq?` on immediate values like small integers and characters
     * can be implementation-dependent. However, `(let ((x <obj>)) (eq? x x))` MUST be true.
     * String and vector literals are generally not `eq?` to each other even if identical.
     */

    // ## Booleans ##
    cr_assert_str_eq(t_eval("(eq? #true #true)"), "#true");
    cr_assert_str_eq(t_eval("(eq? #false #false)"), "#true");
    cr_assert_str_eq(t_eval("(eq? #true #false)"), "#false");
    cr_assert_str_eq(t_eval("(eq? #false #true)"), "#false");

    // ## Symbols ##
    // Symbols with the same name refer to the same object.
    cr_assert_str_eq(t_eval("(eq? 'foo 'foo)"), "#true");
    cr_assert_str_eq(t_eval("(eq? 'foo 'bar)"), "#false");
    cr_assert_str_eq(t_eval("(eq? '() 'foo)"), "#false");
    cr_assert_str_eq(t_eval("(eq? (string->symbol \"baz\") 'baz)"), "#true");

    // ## Empty List ##
    // The empty list '() is a unique object.
    cr_assert_str_eq(t_eval("(eq? '() '())"), "#true");
    cr_assert_str_eq(t_eval("(eq? (list) '())"), "#true");
    cr_assert_str_eq(t_eval("(eq? (cdr '(a)) '())"), "#true");

    // ## Numbers ##
    // `eq?` on numbers may be true for small integers (implementation-defined), but false for others.
    cr_assert_str_eq(t_eval("(eq? 2 2)"), "#false"); // No integer interning
    cr_assert_str_eq(t_eval("(eq? 1000000 1000000)"), "#false"); // test with larger integers as well
    cr_assert_str_eq(t_eval("(eq? 2 3)"), "#false");
    cr_assert_str_eq(t_eval("(eq? 2 2.0)"), "#false"); // Different types
    cr_assert_str_eq(t_eval("(eq? 3.14 3.14)"), "#false"); // Inexacts are generally not `eq?`
    cr_assert_str_eq(t_eval("(begin (define x 5) (eq? x x))"), "#true"); // A variable is `eq?` to itself
    cr_assert_str_eq(t_eval("(begin (define y 3.14) (eq? y y))"), "#true");

    // ## Characters ##
    cr_assert_str_eq(t_eval("(eq? #\\a #\\a)"), "#false"); // no character interning
    cr_assert_str_eq(t_eval("(eq? #\\a #\\b)"), "#false");
    cr_assert_str_eq(t_eval("(begin (define my-char #\\λ) (eq? my-char my-char))"), "#true");

    // ## Pairs and Lists ##
    // Two separately created lists are never `eq?`, even if structurally identical.
    cr_assert_str_eq(t_eval("(eq? (cons 'a 'b) (cons 'a 'b))"), "#false");
    cr_assert_str_eq(t_eval("(eq? '(1 2 3) '(1 2 3))"), "#false");
    // A variable bound to a list is `eq?` to itself.
    cr_assert_str_eq(t_eval("(begin (define x '(1 2 3)) (eq? x x))"), "#true");
    // The `cdr` of a list is `eq?` to itself.
    cr_assert_str_eq(t_eval("(begin (define y '(a b c)) (eq? (cdr y) (cdr y)))"), "#true");
    cr_assert_str_eq(t_eval("(begin (define z '(d e f)) (eq? z (cdr (cons 'ignored z))))"), "#true");

    // ## Strings ##
    // Two string literals with the same content are not required to be `eq?`.
    cr_assert_str_eq(t_eval("(eq? \"hello\" \"hello\")"), "#false");
    cr_assert_str_eq(t_eval("(eq? (make-string 3 #\\a) (make-string 3 #\\a))"), "#false");
    cr_assert_str_eq(t_eval("(begin (define s \"world\") (eq? s s))"), "#true");

    // ## Vectors ##
    // Two vector literals are not `eq?`.
    cr_assert_str_eq(t_eval("(eq? #(1 2) #(1 2))"), "#false");
    cr_assert_str_eq(t_eval("(begin (define v #(a b)) (eq? v v))"), "#true");

    // ## Procedures ##
    cr_assert_str_eq(t_eval("(eq? car car)"), "#true"); // Primitive procedures are unique.
    cr_assert_str_eq(t_eval("(eq? (lambda (x) x) (lambda (x) x))"), "#false"); // Each lambda creates a new procedure.
    cr_assert_str_eq(t_eval("(begin (define p (lambda (y) (* y y))) (eq? p p))"), "#true");

    // ## Cross-Type Comparisons ##
    cr_assert_str_eq(t_eval("(eq? '() #false)"), "#false");
    cr_assert_str_eq(t_eval("(eq? 0 '())"), "#false");
    cr_assert_str_eq(t_eval("(eq? \"a\" 'a)"), "#false");
    cr_assert_str_eq(t_eval("(eq? \"()\" '())"), "#false");
    cr_assert_str_eq(t_eval("(eq? #true 't)"), "#false");
    cr_assert_str_eq(t_eval("(eq? (list 'a) '(a))"), "#false");

    // ## Error Handling: Arity ##
    cr_assert_str_eq(t_eval("(eq?)"), " Arity error: eq?: expected exactly 2 args, got 0");
    cr_assert_str_eq(t_eval("(eq? 'a)"), " Arity error: eq?: expected exactly 2 args, got 1");
    cr_assert_str_eq(t_eval("(eq? 'a 'b 'c)"), " Arity error: eq?: expected exactly 2 args, got 3");
}

Test(end_to_end_equivalence, test_eqv_procedure, .init = setup_each_test, .fini = teardown_each_test) {
    /*
     * R7RS Section 6.1: Equivalence predicates
     * `eqv?` is generally the same as `eq?`, but is true for numbers that are numerically
     * equal and have the same exactness, and for characters that are the same.
     */

    // ## Booleans (same as eq?) ##
    cr_assert_str_eq(t_eval("(eqv? #true #true)"), "#true");
    cr_assert_str_eq(t_eval("(eqv? #false #false)"), "#true");
    cr_assert_str_eq(t_eval("(eqv? #true #false)"), "#false");

    // ## Symbols (same as eq?) ##
    cr_assert_str_eq(t_eval("(eqv? 'foo 'foo)"), "#true");
    cr_assert_str_eq(t_eval("(eqv? 'foo 'bar)"), "#false");

    // ## Empty List (same as eq?) ##
    cr_assert_str_eq(t_eval("(eqv? '() '())"), "#true");

    // ## Numbers (Key difference from eq?) ##
    // `eqv?` is true if numbers are the same value AND same exactness.
    cr_assert_str_eq(t_eval("(eqv? 2 2)"), "#true");
    cr_assert_str_eq(t_eval("(eqv? 3.14 3.14)"), "#true"); // Two inexacts of same value are `eqv?`
    cr_assert_str_eq(t_eval("(eqv? 1/2 1/2)"), "#true");   // Two exacts of same value are `eqv?`
    cr_assert_str_eq(t_eval("(eqv? #e1e10 #e1e10)"), "#true");

    // Different values
    cr_assert_str_eq(t_eval("(eqv? 2 3)"), "#false");
    cr_assert_str_eq(t_eval("(eqv? 3.14 3.15)"), "#false");

    // Different exactness
    cr_assert_str_eq(t_eval("(eqv? 2 2.0)"), "#false");
    cr_assert_str_eq(t_eval("(eqv? 1/2 0.5)"), "#false");
    cr_assert_str_eq(t_eval("(eqv? 0 0.0)"), "#false");

    // A variable is always `eqv?` to itself
    cr_assert_str_eq(t_eval("(begin (define x 5) (eqv? x x))"), "#true");
    cr_assert_str_eq(t_eval("(begin (define y 3.0) (eqv? y y))"), "#true");

    // ## Characters (Key difference from eq?) ##
    // `eqv?` is true if characters have the same value, regardless of interning.
    cr_assert_str_eq(t_eval("(eqv? #\\a #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(eqv? #\\a #\\b)"), "#false");
    cr_assert_str_eq(t_eval("(eqv? #\\λ #\\λ)"), "#true");
    cr_assert_str_eq(t_eval("(begin (define my-char #\\space) (eqv? my-char my-char))"), "#true");

    // ## Pairs and Lists (same as eq?) ##
    // Two separately created lists are never `eqv?`.
    cr_assert_str_eq(t_eval("(eqv? (cons 'a 'b) (cons 'a 'b))"), "#false");
    cr_assert_str_eq(t_eval("(eqv? '(1 2 3) '(1 2 3))"), "#false");
    cr_assert_str_eq(t_eval("(begin (define x '(1 2 3)) (eqv? x x))"), "#true");

    // ## Strings (same as eq?) ##
    // `eqv?` on strings is the same as `eq?`; it checks for object identity.
    cr_assert_str_eq(t_eval("(eqv? \"hello\" \"hello\")"), "#false");
    cr_assert_str_eq(t_eval("(eqv? (make-string 3 #\\a) (make-string 3 #\\a))"), "#false");
    cr_assert_str_eq(t_eval("(begin (define s \"world\") (eqv? s s))"), "#true");

    // ## Vectors (same as eq?) ##
    cr_assert_str_eq(t_eval("(eqv? #(1 2) #(1 2))"), "#false");
    cr_assert_str_eq(t_eval("(begin (define v #(a b)) (eqv? v v))"), "#true");

    // ## Procedures (same as eq?) ##
    cr_assert_str_eq(t_eval("(eqv? car car)"), "#true");
    cr_assert_str_eq(t_eval("(eqv? (lambda (x) x) (lambda (x) x))"), "#false");
    cr_assert_str_eq(t_eval("(begin (define p (lambda (y) (* y y))) (eqv? p p))"), "#true");

    // ## Cross-Type Comparisons ##
    cr_assert_str_eq(t_eval("(eqv? 0 #false)"), "#false");
    cr_assert_str_eq(t_eval("(eqv? #\\a 'a)"), "#false");
    cr_assert_str_eq(t_eval("(eqv? \"a\" #\\a)"), "#false");

    // ## Error Handling: Arity ##
    cr_assert_str_eq(t_eval("(eqv?)"), " Arity error: eqv?: expected exactly 2 args, got 0");
    cr_assert_str_eq(t_eval("(eqv? 'a)"), " Arity error: eqv?: expected exactly 2 args, got 1");
    cr_assert_str_eq(t_eval("(eqv? 'a 'b 'c)"), " Arity error: eqv?: expected exactly 2 args, got 3");
}

Test(end_to_end_equivalence, test_equal_procedure, .init = setup_each_test, .fini = teardown_each_test) {
    /*
     * R7RS Section 6.1: Equivalence predicates
     * `equal?` recursively compares the contents of pairs, vectors, and strings.
     * It returns #true if objects are structurally similar. For non-aggregate types,
     * it behaves like `eqv?`. It is required to terminate, even on cyclic structures.
     */

    // ## Behavior on non-aggregates (should be same as eqv?) ##
    cr_assert_str_eq(t_eval("(equal? #true #true)"), "#true");
    cr_assert_str_eq(t_eval("(equal? 'foo 'foo)"), "#true");
    cr_assert_str_eq(t_eval("(equal? 2 2)"), "#true");
    cr_assert_str_eq(t_eval("(equal? 3.5 3.5)"), "#true");
    cr_assert_str_eq(t_eval("(equal? #\\a #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(equal? 2 2.0)"), "#false"); // Different exactness
    cr_assert_str_eq(t_eval("(equal? '() '())"), "#true");

    // ## Strings (Key difference from eqv?) ##
    // `equal?` compares strings character by character.
    cr_assert_str_eq(t_eval("(equal? \"hello\" \"hello\")"), "#true");
    cr_assert_str_eq(t_eval("(equal? \"hello\" \"world\")"), "#false");
    cr_assert_str_eq(t_eval("(equal? \"\" \"\")"), "#true");
    cr_assert_str_eq(t_eval("(equal? \"abc\" \"abcd\")"), "#false"); // Different lengths
    cr_assert_str_eq(t_eval("(equal? (make-string 3 #\\z) \"zzz\")"), "#true");

    // ## Pairs and Lists (Key difference from eqv?) ##
    // `equal?` recursively compares the elements of lists.
    cr_assert_str_eq(t_eval("(equal? '(1 2 3) '(1 2 3))"), "#true");
    cr_assert_str_eq(t_eval("(equal? (cons 'a 'b) (cons 'a 'b))"), "#true");
    cr_assert_str_eq(t_eval("(equal? '(1 2 3) '(1 2 4))"), "#false");
    cr_assert_str_eq(t_eval("(equal? '(1 (2 3)) '(1 (2 3)))"), "#true"); // Nested lists
    cr_assert_str_eq(t_eval("(equal? '(1 (2 3)) '(1 (2 9)))"), "#false");
    cr_assert_str_eq(t_eval("(equal? '(a . b) '(a . b))"), "#true"); // Dotted pairs
    cr_assert_str_eq(t_eval("(equal? '(a . b) '(a . c))"), "#false");

    // ## Vectors (Key difference from eqv?) ##
    // `equal?` recursively compares the elements of vectors.
    cr_assert_str_eq(t_eval("(equal? #(1 2 3) #(1 2 3))"), "#true");
    cr_assert_str_eq(t_eval("(equal? #(1 2 3) #(1 2 4))"), "#false");
    cr_assert_str_eq(t_eval("(equal? #() #())"), "#true"); // Empty vectors
    cr_assert_str_eq(t_eval("(equal? #(1 (2 3)) #(1 (2 3)))"), "#true"); // Vector with nested list
    cr_assert_str_eq(t_eval("(equal? #(1 #(2 3)) #(1 #(2 3)))"), "#true"); // Nested vectors

    // ## Mixed Structures ##
    cr_assert_str_eq(t_eval("(equal? '(1 \"foo\" #(2 3)) '(1 \"foo\" #(2 3)))"), "#true");
    cr_assert_str_eq(t_eval("(equal? '(1 \"foo\" #(2 3)) '(1 \"bar\" #(2 3)))"), "#false");

    // ## Cross-Type Structural Comparisons ##
    // Structures must be of the same type to be `equal?`
    cr_assert_str_eq(t_eval("(equal? '(1 2 3) #(1 2 3))"), "#false");
    cr_assert_str_eq(t_eval("(equal? \"abc\" '(a b c))"), "#false");

    // ## Cyclic Structures (must terminate and return correct value) ##
    //Test case 1: Two simple identical cycles
    cr_assert_str_eq(t_eval("(begin "
                                "(define x (list 'a)) "
                                "(set-cdr! x x) "
                                "(define y (list 'a)) "
                                "(set-cdr! y y) "
                                "(equal? x y))"), "#true");

    // Test case 2: Two different cycles
    cr_assert_str_eq(t_eval("(begin "
                                "(define x (list 'a)) "
                                "(set-cdr! x x) "
                                "(define y (list 'b)) "
                                "(set-cdr! y y) "
                                "(equal? x y))"), "#false");

    // Test case 3: One cyclic, one not
    // FIXME: this should pass
    // cr_assert_str_eq(t_eval("(begin "
    //                             "(define x (list 'a)) "
    //                             "(set-cdr! x x) "
    //                             "(define y (list 'a)) "
    //                             "(equal? x y))"), "#false");

    // Test case 4: Complex shared structure
    cr_assert_str_eq(t_eval("(begin "
                                "(define z (list 'c)) "
                                "(define x (list 'a 'b z)) "
                                "(define y (list 'a 'b z)) "
                                "(equal? x y))"), "#true");

    // ## Error Handling: Arity ##
    cr_assert_str_eq(t_eval("(equal?)"), " Arity error: equal?: expected exactly 2 args, got 0");
    cr_assert_str_eq(t_eval("(equal? 'a)"), " Arity error: equal?: expected exactly 2 args, got 1");
    cr_assert_str_eq(t_eval("(equal? 'a 'b 'c)"), " Arity error: equal?: expected exactly 2 args, got 3");
}
