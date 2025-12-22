#include "test_meta.h"
#include <criterion/criterion.h>

TestSuite(end_to_end_type_predicates);

Test(end_to_end_type_predicates, test_number_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Numbers ##
    cr_assert_str_eq(t_eval("(number? 1)"), "#true");
    cr_assert_str_eq(t_eval("(number? -1.5)"), "#true");
    cr_assert_str_eq(t_eval("(number? 3/4)"), "#true");
    cr_assert_str_eq(t_eval("(number? 1+2i)"), "#true");
    cr_assert_str_eq(t_eval("(number? +inf.0)"), "#true");

    // ## Non-numbers ##
    cr_assert_str_eq(t_eval("(number? #true)"), "#false");
    cr_assert_str_eq(t_eval("(number? \"1\")"), "#false");
    cr_assert_str_eq(t_eval("(number? 'a)"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(number?)"), " Arity error: number?: expected exactly 1 arg, got 0");
    cr_assert_str_eq(t_eval("(number? 1 2)"), " Arity error: number?: expected exactly 1 arg, got 2");
}

Test(end_to_end_type_predicates, test_boolean_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Booleans ##
    cr_assert_str_eq(t_eval("(boolean? #true)"), "#true");
    cr_assert_str_eq(t_eval("(boolean? #false)"), "#true");

    // ## Non-booleans ##
    cr_assert_str_eq(t_eval("(boolean? '())"), "#false");
    cr_assert_str_eq(t_eval("(boolean? 0)"), "#false");
    cr_assert_str_eq(t_eval("(boolean? \"#false\")"), "#false");
    cr_assert_str_eq(t_eval("(boolean? 'true)"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(boolean?)"), " Arity error: boolean?: expected exactly 1 arg, got 0");
}

Test(end_to_end_type_predicates, test_null_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## The Empty List ##
    cr_assert_str_eq(t_eval("(null? '())"), "#true");

    // ## Non-null values ##
    cr_assert_str_eq(t_eval("(null? #false)"), "#false");
    cr_assert_str_eq(t_eval("(null? 0)"), "#false");
    cr_assert_str_eq(t_eval("(null? \"\")"), "#false");
    cr_assert_str_eq(t_eval("(null? #())"), "#false");
    cr_assert_str_eq(t_eval("(null? '(a))"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(null?)"), " Arity error: null?: expected exactly 1 arg, got 0");
}

Test(end_to_end_type_predicates, test_pair_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Pairs ##
    cr_assert_str_eq(t_eval("(pair? '(a . b))"), "#true");
    cr_assert_str_eq(t_eval("(pair? '(1 2 3))"), "#true");
    cr_assert_str_eq(t_eval("(pair? (cons 1 '()))"), "#true");

    // ## Not pairs ##
    cr_assert_str_eq(t_eval("(pair? '())"), "#false");
    cr_assert_str_eq(t_eval("(pair? 'a)"), "#false");
    cr_assert_str_eq(t_eval("(pair? #(1 2))"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(pair?)"), " Arity error: pair?: expected exactly 1 arg, got 0");
}

Test(end_to_end_type_predicates, test_list_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // `list?` returns true for proper lists.
    // ## Proper Lists ##
    cr_assert_str_eq(t_eval("(list? '())"), "#true");
    cr_assert_str_eq(t_eval("(list? '(1 2 3))"), "#true");
    // FIXME: this should pass
    cr_assert_str_eq(t_eval("(list? (cons 1 (cons 2 '())))"), "#true");

    // ## Not proper lists ##
    cr_assert_str_eq(t_eval("(list? '(a . b))"), "#false");
    cr_assert_str_eq(t_eval("(list? 'a)"), "#false");
    cr_assert_str_eq(t_eval("(list? #(1 2))"), "#false");
    // FIXME: need to implement cycle detection
    //cr_assert_str_eq(t_eval("(begin (define x (list 'a 'b)) (set-cdr! (cdr x) x) (list? x))"), "#false"); // cyclic list

    // ## Arity ##
    cr_assert_str_eq(t_eval("(list?)"), " Arity error: list?: expected exactly 1 arg, got 0");
}

Test(end_to_end_type_predicates, test_procedure_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Procedures ##
    cr_assert_str_eq(t_eval("(procedure? +)"), "#true");
    cr_assert_str_eq(t_eval("(procedure? car)"), "#true");
    cr_assert_str_eq(t_eval("(procedure? (lambda (x) (* x x)))"), "#true");

    // ## Not procedures ##
    cr_assert_str_eq(t_eval("(procedure? 1)"), "#false");
    cr_assert_str_eq(t_eval("(procedure? '+)"), "#false");
    cr_assert_str_eq(t_eval("(procedure? '(lambda (x) x))"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(procedure?)"), " Arity error: procedure?: expected exactly 1 arg, got 0");
}

Test(end_to_end_type_predicates, test_symbol_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Symbols ##
    cr_assert_str_eq(t_eval("(symbol? 'foo)"), "#true");
    cr_assert_str_eq(t_eval("(symbol? (string->symbol \"bar\"))"), "#true");
    cr_assert_str_eq(t_eval("(symbol? '())"), "#false");

    // ## Not symbols ##
    cr_assert_str_eq(t_eval("(symbol? \"foo\")"), "#false");
    cr_assert_str_eq(t_eval("(symbol? 123)"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(symbol?)"), " Arity error: symbol?: expected exactly 1 arg, got 0");
}

Test(end_to_end_type_predicates, test_string_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Strings ##
    cr_assert_str_eq(t_eval("(string? \"hello\")"), "#true");
    cr_assert_str_eq(t_eval("(string? \"\")"), "#true");
    cr_assert_str_eq(t_eval("(string? (make-string 3 #\\a))"), "#true");

    // ## Not strings ##
    cr_assert_str_eq(t_eval("(string? 'hello)"), "#false");
    cr_assert_str_eq(t_eval("(string? #\\a)"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(string?)"), " Arity error: string?: expected exactly 1 arg, got 0");
}

Test(end_to_end_type_predicates, test_char_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Chars ##
    cr_assert_str_eq(t_eval("(char? #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(char? #\\space)"), "#true");
    cr_assert_str_eq(t_eval("(char? #\\λ)"), "#true");

    // ## Not chars ##
    cr_assert_str_eq(t_eval("(char? \"a\")"), "#false");
    cr_assert_str_eq(t_eval("(char? 'a)"), "#false");
    cr_assert_str_eq(t_eval("(char? 97)"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(char?)"), " Arity error: char?: expected exactly 1 arg, got 0");
}

Test(end_to_end_type_predicates, test_vector_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Vectors ##
    cr_assert_str_eq(t_eval("(vector? #(1 2 3))"), "#true");
    cr_assert_str_eq(t_eval("(vector? #())"), "#true");
    cr_assert_str_eq(t_eval("(vector? (make-vector 5))"), "#true");

    // ## Not vectors ##
    cr_assert_str_eq(t_eval("(vector? '(1 2 3))"), "#false");
    cr_assert_str_eq(t_eval("(vector? #u8(1 2 3))"), "#false"); // bytevector is not a vector
    cr_assert_str_eq(t_eval("(vector? \"abc\")"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(vector?)"), " Arity error: vector?: expected exactly 1 arg, got 0");
}

Test(end_to_end_type_predicates, test_bytevector_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Bytevectors ##
    cr_assert_str_eq(t_eval("(bytevector? #u8(1 2 3))"), "#true");
    cr_assert_str_eq(t_eval("(bytevector? #u8())"), "#true");
    cr_assert_str_eq(t_eval("(bytevector? (make-bytevector 5))"), "#true");

    // ## Not bytevectors ##
    cr_assert_str_eq(t_eval("(bytevector? #(1 2 3))"), "#false");
    cr_assert_str_eq(t_eval("(bytevector? '(1 2 3))"), "#false");
    cr_assert_str_eq(t_eval("(bytevector? \"abc\")"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(bytevector?)"), " Arity error: bytevector?: expected exactly 1 arg, got 0");
}

Test(end_to_end_type_predicates, test_port_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## Ports ##
    cr_assert_str_eq(t_eval("(port? (current-input-port))"), "#true");
    cr_assert_str_eq(t_eval("(port? (current-output-port))"), "#true");
    cr_assert_str_eq(t_eval("(port? (current-error-port))"), "#true");

    // ## Not ports ##
    cr_assert_str_eq(t_eval("(port? 'stdin)"), "#false");
    cr_assert_str_eq(t_eval("(port? 0)"), "#false");
    cr_assert_str_eq(t_eval("(port? #false)"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(port?)"), " Arity error: port?: expected exactly 1 arg, got 0");
}

Test(end_to_end_type_predicates, test_eof_object_predicate, .init = setup_each_test, .fini = teardown_each_test) {
    // ## EOF Object ##
    cr_assert_str_eq(t_eval("(eof-object? (eof-object))"), "#true");

    // ## Not EOF Object ##
    cr_assert_str_eq(t_eval("(eof-object? 'eof)"), "#false");
    cr_assert_str_eq(t_eval("(eof-object? -1)"), "#false");
    cr_assert_str_eq(t_eval("(eof-object? #false)"), "#false");
    cr_assert_str_eq(t_eval("(eof-object? '())"), "#false");

    // ## Arity ##
    cr_assert_str_eq(t_eval("(eof-object?)"), " Arity error: eof-object?: expected exactly 1 arg, got 0");
}
