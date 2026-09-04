#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <gc/gc.h>

#include "lexer.h"


static void setup(void) {
    GC_INIT();
}

Test(test_lexer, test_lex_simp, .init = setup) {
    char *src = "(+ 1 1)";
    TokenArray *ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 6, "Expected 6; got %d", ta->count);
    cr_assert_not_null(ta->tokens);
    cr_assert_eq(ta->tokens->type, T_LEFT_PAREN);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_RIGHT_PAREN);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);
}

Test(test_lexer, test_lex_list, .init = setup) {
    char *src = "'(1 2 3)";
    TokenArray *ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 7, "Expected 7; got %d", ta->count);
    cr_assert_not_null(ta->tokens);
    cr_assert_eq(ta->tokens->type, T_QUOTE, "Expected %d: got %d", T_QUOTE, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_LEFT_PAREN, "Expected %d: got %d", T_LEFT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_RIGHT_PAREN);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);
}

Test(test_lexer, test_lex_vector, .init = setup) {
    char *src = "#(1 2 3)";
    TokenArray *ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 7, "Expected 7; got %d", ta->count);
    cr_assert_not_null(ta->tokens);
    cr_assert_eq(ta->tokens->type, T_HASH, "Expected %d: got %d", T_HASH, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_LEFT_PAREN, "Expected %d: got %d", T_LEFT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_RIGHT_PAREN);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);
}

Test(test_lexer, test_lex_bytevector, .init = setup) {
    char *src = "#u8(1 2 3)";
    TokenArray *ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 8, "Expected 8; got %d", ta->count);
    cr_assert_not_null(ta->tokens);
    cr_assert_eq(ta->tokens->type, T_HASH, "Expected %d: got %d", T_HASH, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_LEFT_PAREN, "Expected %d: got %d", T_LEFT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_RIGHT_PAREN);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);
}

Test(test_lexer, test_lex_unquotesplice, .init = setup) {
    char *src = "`(a ,@xs b)";
    TokenArray *ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 8, "Expected 8; got %d", ta->count);
    cr_assert_not_null(ta->tokens);
    cr_assert_eq(ta->tokens->type, T_QUASIQUOTE, "Expected %d: got %d", T_QUASIQUOTE, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_LEFT_PAREN, "Expected %d: got %d", T_LEFT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_COMMA_AT, "Expected %d: got %d", T_COMMA_AT, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_RIGHT_PAREN);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);
}

Test(test_lexer, test_lex_string, .init = setup) {
    char *src = "\"Hello\"";
    TokenArray *ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 2, "Expected 2; got %d", ta->count);
    cr_assert_eq(ta->tokens->type, T_STRING, "Expected %d: got %d", T_STRING, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);
}

Test(test_lexer, test_lex_symbol, .init = setup) {
    char *src = "Hello";
    TokenArray *ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 2, "Expected 2; got %d", ta->count);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);

    // Test multi-word identifier - lex as single symbol
    src = "|long variable name|";
    ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 2, "Expected 2; got %d", ta->count);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);
}

Test(test_lexer, test_lex_bool, .init = setup) {
    char *src = "#f #t #false #true";
    TokenArray *ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 5, "Expected 5; got %d", ta->count);
    cr_assert_eq(ta->tokens->type, T_BOOLEAN, "Expected %d: got %d", T_BOOLEAN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_BOOLEAN, "Expected %d: got %d", T_BOOLEAN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_BOOLEAN, "Expected %d: got %d", T_BOOLEAN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_BOOLEAN, "Expected %d: got %d", T_BOOLEAN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);
}

Test(test_lexer, test_lex_char, .init = setup) {
    char *src = "#\\a #\\b #\\c";
    TokenArray *ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 4, "Expected 4; got %d", ta->count);
    cr_assert_eq(ta->tokens->type, T_CHAR, "Expected %d: got %d", T_CHAR, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_CHAR, "Expected %d: got %d", T_CHAR, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_CHAR, "Expected %d: got %d", T_CHAR, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);
}

Test(test_lexer, test_lex_set, .init = setup) {
    char *src = "#{1 2 3}";
    TokenArray *ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 6, "Expected 6; got %d", ta->count);
    cr_assert_not_null(ta->tokens);
    cr_assert_eq(ta->tokens->type, T_SET_START, "Expected %d: got %d", T_SET_START, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_RIGHT_BRACE);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);
}

Test(test_lexer, test_lex_hash, .init = setup) {
    char *src = "#[one 1 two 2 three 3]";
    TokenArray *ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 9, "Expected 9; got %d", ta->count);
    cr_assert_not_null(ta->tokens);
    cr_assert_eq(ta->tokens->type, T_HASH_START, "Expected %d: got %d", T_SET_START, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_RIGHT_BRACKET);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);
}

// Test that the lexer skips comments.
Test(test_lexer, test_lex_line_comment, .init = setup) {
    char *src = "foo ; a symbol 'foo'\nbar ;;; another symbol 'bar'";
    TokenArray *ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 3, "Expected 3; got %d", ta->count);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);
}

Test(test_lexer, test_lex_block_comment, .init = setup) {
    char *src = R"(
(define (fact n)
    #|
    This is a comment
    It should not be lexed
    |#
    (if (= n 1) 1
        (* n (fact (- n 1)))))
    )";

    TokenArray *ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 29, "Expected 29; got %d", ta->count);
    cr_assert_eq(ta->tokens->type, T_LEFT_PAREN, "Expected %d: got %d", T_LEFT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_LEFT_PAREN, "Expected %d: got %d", T_LEFT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_RIGHT_PAREN, "Expected %d: got %d", T_RIGHT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_LEFT_PAREN, "Expected %d: got %d", T_LEFT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_LEFT_PAREN, "Expected %d: got %d", T_LEFT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_RIGHT_PAREN, "Expected %d: got %d", T_RIGHT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_LEFT_PAREN, "Expected %d: got %d", T_LEFT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_LEFT_PAREN, "Expected %d: got %d", T_LEFT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_LEFT_PAREN, "Expected %d: got %d", T_LEFT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_NUMBER);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_RIGHT_PAREN, "Expected %d: got %d", T_RIGHT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_RIGHT_PAREN, "Expected %d: got %d", T_RIGHT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_RIGHT_PAREN, "Expected %d: got %d", T_RIGHT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_RIGHT_PAREN, "Expected %d: got %d", T_RIGHT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_RIGHT_PAREN, "Expected %d: got %d", T_RIGHT_PAREN, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);
}

// Test that error token is returned on bare hash
Test(test_lexer, test_lex_error, .init = setup) {
    // With no space, lexer returns T_HASH then T_EOF.
    char *src = "foo #";
    TokenArray *ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 3, "Expected 3; got %d", ta->count);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_HASH, "Expected %d: got %d", T_HASH, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);

    // With a space, return T_ERROR
    src = "foo # foo";
    ta = scan_all_tokens(src);
    cr_assert_eq(ta->count, 4, "Expected 4; got %d", ta->count);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_ERROR, "Expected %d: got %d", T_ERROR, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_SYMBOL, "Expected %d: got %d", T_SYMBOL, ta->tokens->type);
    cr_assert_not_null(ta->tokens++);
    cr_assert_eq(ta->tokens->type, T_EOF);

}


