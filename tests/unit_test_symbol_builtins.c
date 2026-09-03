#include <criterion/criterion.h>
#include <gc/gc.h>

#include "symbols.h"
#include "cell.h"
#include "types.h"


static void setup(void) {
    GC_INIT();
    init_global_singletons();
    symbol_table = ht_create(128);
    init_special_forms();
}

Test(symbol_builtins, builtin_symbol_equal_pred, .init = setup) {
    Cell* a = make_cell_symbol("a");
    Cell* b = make_cell_symbol("b");
    Cell* c = make_cell_symbol("c");
    Cell* d = make_cell_symbol("d");

    // Return #t with 0 or 1 arg
    Cell* arg = make_sexpr_len1(a);
    Cell* res = builtin_symbol_equal_pred(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->boolean_v, 1);
    cr_assert_eq(res, True_Obj);

    arg = make_cell_sexpr();
    res = builtin_symbol_equal_pred(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->boolean_v, 1);
    cr_assert_eq(res, True_Obj);

    // Return #f with different symbols
    arg = make_sexpr_len4(a, b, c, d);
    res = builtin_symbol_equal_pred(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->boolean_v, 0);
    cr_assert_eq(res, False_Obj);

    // return #t with same symbols
    arg = make_sexpr_len4(a, a, a, a);
    res = builtin_symbol_equal_pred(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->boolean_v, 1);
    cr_assert_eq(res, True_Obj);
}

Test(symbol_builtins, builtin_string_to_symbol, .init = setup) {
    // Test existing interned symbol
    Cell* s = make_cell_string("define");
    Cell* arg = make_sexpr_len1(s);
    Cell* res = builtin_string_to_symbol(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_SYMBOL);
    cr_assert_str_eq(res->sym, "define");
    cr_assert_eq(res, G_define_sym);

    // Test uninterned symbol
    s = make_cell_string("zzzzzz");
    arg = make_sexpr_len1(s);
    res = builtin_string_to_symbol(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_SYMBOL);
    cr_assert_str_eq(res->sym, "zzzzzz");
}

Test(symbol_builtins, builtin_symbol_to_string, .init = setup) {
    // Test interned symbol
    Cell* arg = make_sexpr_len1(G_define_sym);
    Cell* res = builtin_symbol_to_string(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_STRING);
    cr_assert_str_eq(res->str, "define");
    cr_assert_eq(res->ascii, 1);
    cr_assert_eq(res->char_count, 6);

    // Test uninterned symbol
    Cell* s = make_cell_symbol("zzzzzz");
    cr_assert_eq(s->type, CELL_SYMBOL);
    arg = make_sexpr_len1(s);
    res = builtin_symbol_to_string(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_STRING);
    cr_assert_str_eq(res->str, "zzzzzz");
    cr_assert_eq(res->ascii, 1);
    cr_assert_eq(res->char_count, 6);

    // Test UTF-8 symbol
    s = make_cell_symbol("Σσμπ");
    cr_assert_eq(s->type, CELL_SYMBOL);
    arg = make_sexpr_len1(s);
    res = builtin_symbol_to_string(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_STRING);
    cr_assert_str_eq(res->str, "Σσμπ");
    cr_assert_eq(res->ascii, 0);
    cr_assert_eq(res->char_count, 4);
}

