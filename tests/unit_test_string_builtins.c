#include <criterion/criterion.h>
#include <gc/gc.h>

#include "types.h"


static void setup(void) {
    GC_INIT();
    init_global_singletons();
}

Test(string_builtins, string_split_basic, .init = setup) {
    // Manually construct the argument list: '("A,B" ",")
    Cell* src_str = make_cell_string("A,B");
    Cell* delim_str = make_cell_string(",");

    // Build the arg Sexpr
    Cell* args = make_sexpr_len2(src_str, delim_str);

    // Call the C function
    // (Assuming Lex* isn't needed for this specific builtin, pass NULL)
    Cell* result = builtin_string_split(NULL, args);

    // Assert the result is a list (Pair)
    cr_assert_not_null(result);
    cr_assert_eq(result->type, CELL_PAIR);

    // Extract and verify the first element ("A")
    Cell* first_tok = result->car;
    cr_assert_eq(first_tok->type, CELL_STRING);
    cr_assert_str_eq(first_tok->str, "A");

    // Extract and verify the second element ("B")
    Cell* next_node = result->cdr;
    cr_assert_not_null(next_node);
    cr_assert_eq(next_node->type, CELL_PAIR);

    Cell* second_tok = next_node->car;
    cr_assert_eq(second_tok->type, CELL_STRING);
    cr_assert_str_eq(second_tok->str, "B");
}
