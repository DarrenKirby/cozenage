
#include "test_meta.h"
#include "../src/eval.h"
#include "../src/parser.h"
#include "../src/repr.h"
#include "../src/symbols.h"
#include <locale.h>

/* Define the global test environment variable. */
Lex* test_env;

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

/* I moved all the setup logic here because there
 * was some weird Criterion - libgc interaction
 * that was causing bizarre memory corruption,
 * and causing tests to fail when they should not.
 *
 * Yes - the tests will run slower, but tests aren't
 * supposed tobe fast,they're supposed to test if
 * the code works!
 */
char* t_eval(const char* input) {
    symbol_table = ht_create(128);
    init_default_ports();
    init_global_singletons();
    test_env = lex_initialize_global_env();
    lex_add_builtins(test_env);
    init_special_forms();

    TokenArray* ta = scan_all_tokens(input);
    Cell* parsed = parse_tokens_new(ta);
    const Cell *result = coz_eval(test_env, parsed);

    return cell_to_string(result, MODE_WRITE);
}
