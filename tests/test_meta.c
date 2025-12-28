
#include "test_meta.h"
#include "load_library.h"
#include "../src/eval.h"
#include "../src/parser.h"
#include "../src/repr.h"
#include "../src/symbols.h"
#include "../src/transforms.h"
#include <locale.h>
#include <gc/gc.h>

/* Define the global test environment variable. */
Lex* test_env;
/* Unused -- this is to satisfy the linker, as there is no main.c
 * in the tests, and command-line in control_features.c need these
 * vars */
int g_argc;
char** g_argv;

void setup_each_test(void) {
    // setup logic
    // symbol_table = ht_create(128);
    // init_default_ports();
    // init_global_singletons();
    // test_env = lex_initialize_global_env();
    // lex_add_builtins(test_env);
    // init_special_forms();
    setlocale(LC_ALL, "");
}

// This function will run after EACH test
void teardown_each_test(void) {
    //ht_destroy(symbol_table);
}

void load_math_lib(void) {
    setlocale(LC_ALL, "");
}

static bool engine_prepped = false;

char* t_eval(const char* input) {
    if (!engine_prepped) {
        GC_INIT(); // Only call this ONCE per process
        symbol_table = ht_create(128);
        init_global_singletons();
        init_special_forms();
        engine_prepped = true;
    }

    // These ARE safe to refresh per call
    init_default_ports();
    test_env = lex_initialize_global_env();
    lex_add_builtins(test_env);

    TokenArray* ta = scan_all_tokens(input);
    Cell* parsed = parse_tokens(ta);
    Cell* expr = expand(parsed);
    const Cell *result = coz_eval(test_env, expr);

    return cell_to_string(result, MODE_WRITE);
}

char* t_eval_math_lib(const char* input) {
    if (!engine_prepped) {
        GC_INIT(); // Only call this ONCE per process
        symbol_table = ht_create(128);
        init_global_singletons();
        init_special_forms();
        engine_prepped = true;
    }

    // These ARE safe to refresh per call
    init_default_ports();
    test_env = lex_initialize_global_env();
    lex_add_builtins(test_env);

    /* Load the math lib */
    const Cell* ll = load_library("math", test_env);
    if (ll == False_Obj) {
        printf("Failed to load math library!!!!\n");
        return "";
    }

    TokenArray* ta = scan_all_tokens(input);
    Cell* parsed = parse_tokens(ta);
    Cell* expr = expand(parsed);
    const Cell *result = coz_eval(test_env, expr);

    return cell_to_string(result, MODE_WRITE);
}
