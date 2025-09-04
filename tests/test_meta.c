
#include "test_meta.h"
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include "../src/ops.h"
#include "../src/eval.h"
#include "../src/parser.h"
#include "../src/printer.h"

/* Define the global test environment variable. */
Lex* test_env;

void setup_suite(void) {
    test_env = lex_initialize();
    lex_add_builtins(test_env);
}

void teardown_suite(void) {
    lex_delete(test_env);
}

void suite_setup_wrapper(void) {
    cr_redirect_stdout();
    setup_suite();
}

void eval_and_check(const char* input, const char* expected_output) {
    /* Parse the input string into an expression. */
    Parser *p = parse_str(input);
    Cell *v = parse_tokens(p);
    free_tokens(p->array, p->size);
    free(p);
    Cell *result = coz_eval(test_env, v);

    /* Print the result to stdout, and push stdout into the capture pipe */
    print_cell(result);
    fflush(stdout);

    /* Capture the result and compare it to the expected string */
    char buffer[4096] = {0};
    FILE* stdout_pipe = cr_get_redirected_stdout();
    fread(buffer, 1, sizeof(buffer) - 1, stdout_pipe);

    //cr_assert_str_eq(buffer, expected_output,
    //    "Test failed for input: \"%s\"", input);
    cr_assert_str_eq(buffer, expected_output,
        "Expected '%s' but got '%s' for input '%s'",
        expected_output, buffer, input);

    cell_delete(result);
}
