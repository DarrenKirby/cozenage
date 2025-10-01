
#include "test_meta.h"
#include <criterion/criterion.h>
#include <criterion/redirect.h>
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
    //lex_delete(test_env);
}

void suite_setup_wrapper(void) {
    cr_redirect_stdout();
    setup_suite();
}

char* t_eval(const char* input) {
    /* Parse the input string into an expression. */
    Parser *p = parse_str(input);
    Cell *v = parse_tokens(p);

    const Cell *result = coz_eval(test_env, v);

    /* Print the result to stdout, and push stdout into the capture pipe */
    print_cell(result);
    fflush(stdout);

    /* Capture the result from stdout */
    static char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    FILE* stdout_pipe = cr_get_redirected_stdout();
    fread(buffer, 1, sizeof(buffer) - 1, stdout_pipe);

    return buffer;
}
