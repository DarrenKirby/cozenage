
#include "test_meta.h"
#include "../src/load_library.h"
#include "../src/eval.h"
#include "../src/parser.h"
#include "../src/repr.h"
#include "../src/symbols.h"
#include "../src/transforms.h"

#include <locale.h>
#include <gc/gc.h>

/* Define the global test environment variable. */
Lex* test_env;

void setup_each_test(void) {
    setlocale(LC_ALL, "");
}

void teardown_each_test(void) {
}

void load_math_lib(void) {
    setlocale(LC_ALL, "");
}

static bool engine_prepped = false;

char* t_eval(const char* input) {
    if (!engine_prepped) {
        GC_INIT();
        symbol_table = ht_create(512);
        init_global_singletons();
        init_special_forms();
        engine_prepped = true;
    }

    init_default_ports();

    /* Initialize bootstrap environment. */
    const Lex* bootstrap = lex_initialize_bootstrap_env();
    lex_add_builtins(bootstrap);
    ht_table* core_builtins = bootstrap->working;
    /* Initialize working environment. */
    test_env = lex_initialize_working_env(core_builtins);

    TokenArray* ta = scan_all_tokens(input);
    Cell* parsed = parse_tokens(ta);
    Cell* expr = expand(parsed);
    const Cell *result = coz_eval(test_env, expr);

    return cell_to_string(result, MODE_WRITE);
}

long double t_eval_math_lib(const char* input) {
    if (!engine_prepped) {
        GC_INIT();
        symbol_table = ht_create(512);
        init_global_singletons();
        init_special_forms();
        engine_prepped = true;
    }

    init_default_ports();
    /* Initialize bootstrap environment. */
    const Lex* bootstrap = lex_initialize_bootstrap_env();
    lex_add_builtins(bootstrap);
    ht_table* core_builtins = bootstrap->working;
    /* Initialize working environment. */
    test_env = lex_initialize_working_env(core_builtins);

    /* Load the lib */
    load_library("math", test_env);

    TokenArray* ta = scan_all_tokens(input);
    Cell* parsed = parse_tokens(ta);

    Cell* result = coz_eval(test_env, parsed);

    return result->real_v;
}

