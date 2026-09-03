#include <criterion/criterion.h>
#include <gc/gc.h>

#include "bools.h"
#include "cell.h"
#include "types.h"


static void setup(void) {
    GC_INIT();
    init_global_singletons();
}

Test(bool_builtins, builtin_not, .init = setup) {
    // Random cell value == false
    Cell* i = make_cell_integer(25);
    Cell* arg = make_sexpr_len1(i);
    Cell* res = builtin_not(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_BOOLEAN);
    cr_assert_eq(res->boolean_v, 0);
    cr_assert_eq(res, False_Obj);

    // Actual #t == false
    Cell* b = True_Obj;
    arg = make_sexpr_len1(b);
    res = builtin_not(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_BOOLEAN);
    cr_assert_eq(res->boolean_v, 0);
    cr_assert_eq(res, False_Obj);

    // Actual #f == true
    b = False_Obj;
    arg = make_sexpr_len1(b);
    res = builtin_not(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_BOOLEAN);
    cr_assert_eq(res->boolean_v, 1);
    cr_assert_eq(res, True_Obj);
}


Test(bool_builtins, builtin_boolean, .init = setup) {
    // Empty arg == true
    Cell* arg = make_cell_sexpr();
    Cell* res = builtin_boolean(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_BOOLEAN);
    cr_assert_eq(res->boolean_v, 1);
    cr_assert_eq(res, True_Obj);

    // All true == true
    arg = make_sexpr_len4(True_Obj, True_Obj, True_Obj, True_Obj);
    res = builtin_boolean(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_BOOLEAN);
    cr_assert_eq(res->boolean_v, 1);
    cr_assert_eq(res, True_Obj);

    // All false == true
    arg = make_sexpr_len4(False_Obj, False_Obj, False_Obj, False_Obj);
    res = builtin_boolean(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_BOOLEAN);
    cr_assert_eq(res->boolean_v, 1);
    cr_assert_eq(res, True_Obj);

    // single #t or single #f == true
    arg = make_sexpr_len1(True_Obj);
    res = builtin_boolean(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_BOOLEAN);
    cr_assert_eq(res->boolean_v, 1);
    cr_assert_eq(res, True_Obj);

    arg = make_sexpr_len1(False_Obj);
    res = builtin_boolean(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_BOOLEAN);
    cr_assert_eq(res->boolean_v, 1);
    cr_assert_eq(res, True_Obj);

    // Bools with random non-bool == false
    Cell* i = make_cell_integer(25);
    arg = make_sexpr_len4(True_Obj, True_Obj, i, True_Obj);
    res = builtin_boolean(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_BOOLEAN);
    cr_assert_eq(res->boolean_v, 0);
    cr_assert_eq(res, False_Obj);

    // Mixed #t/#f == false
    arg = make_sexpr_len4(True_Obj, False_Obj, True_Obj, False_Obj);
    res = builtin_boolean(nullptr, arg);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_BOOLEAN);
    cr_assert_eq(res->boolean_v, 0);
    cr_assert_eq(res, False_Obj);
}

