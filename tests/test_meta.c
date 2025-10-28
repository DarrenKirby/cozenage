
#include "test_meta.h"
#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <gc/gc.h>
#include "../src/eval.h"
#include "../src/parser.h"
#include "../src/repr.h"
#include "../src/symbols.h"

/* Define the global test environment variable. */
Lex* test_env;

// This function will run before EACH test
void setup_each_test(void) {
    GC_INIT();
    cr_redirect_stdout(); // Good to keep this here

    // setup logic
    symbol_table = ht_create(128);
    //fprintf(stderr, "SETUP address of symbol_table: %p\n", (void*)symbol_table);
    init_default_ports();
    init_global_singletons();
    test_env = lex_initialize_global_env();
    lex_add_builtins(test_env);
    init_special_forms();
}

// This function will run after EACH test
void teardown_each_test(void) {
    ht_destroy(symbol_table);
}

// void setup_suite(void) {
//     /* Initialize symbol table with initial size of 128 */
//     //symbol_table = ht_create(128);
//     /* Initialize default ports */
//     //init_default_ports();
//     /* Initialize global singleton objects, nil, #t, #f, and EOF */
//     //init_global_singletons();
//     /* Initialize global environment */
//     //test_env = lex_initialize_global_env();
//     /* Load (scheme base) procedures into the environment*/
//     //lex_add_builtins(test_env);
//     /* Initialize special form lookup table */
//     //init_special_forms();
// }
//
// void teardown_suite(void) {
//     /* Don't need to manually destroy with GC */
// }
//
// void suite_setup_wrapper(void) {
//     cr_redirect_stdout();
//     setup_suite();
// }

// No longer needs to return a static buffer
char* t_eval(const char* input) {
    // These steps are correct
    TokenArray* ta = scan_all_tokens(input);
    Cell* parsed = parse_tokens_new(ta);
    const Cell *result = coz_eval(test_env, parsed);

    // The new, safer way: return the string directly.
    // This assumes cell_to_string returns a GC-allocated string.
    return cell_to_string(result, MODE_WRITE);
}

// char* t_eval(const char* input) {
//     /* Parse the input string into an expression. */
//     TokenArray* ta = scan_all_tokens(input);
//     Cell* parsed = parse_tokens_new(ta);
//
//     const Cell *result = coz_eval(test_env, parsed);
//
//     /* Print the result to stdout, and push stdout into the capture pipe */
//     printf("%s", cell_to_string(result, MODE_WRITE));
//     fflush(stdout);
//
//     /* Capture the result from stdout */
//     static char buffer[4096];
//     memset(buffer, 0, sizeof(buffer));
//     FILE* stdout_pipe = cr_get_redirected_stdout();
//     fread(buffer, 1, sizeof(buffer) - 1, stdout_pipe);
//
//     return buffer;
// }
