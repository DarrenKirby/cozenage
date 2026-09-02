#include <criterion/criterion.h>
#include <gc/gc.h>

#define CRITERION_TEST_BUILD 1
#include "types.h"

static void setup(void) {
    GC_INIT();
    init_global_singletons();
}

Test(type_utils, test_cell_type_name, .init = setup) {
    cr_assert_str_eq(cell_type_name(CELL_INTEGER), "integer");
    cr_assert_str_eq(cell_type_name(CELL_REAL), "real");
    cr_assert_str_eq(cell_type_name(CELL_RATIONAL), "rational");
    cr_assert_str_eq(cell_type_name(CELL_COMPLEX), "complex");
    cr_assert_str_eq(cell_type_name(CELL_BOOLEAN), "bool");
    cr_assert_str_eq(cell_type_name(CELL_SYMBOL), "symbol");
    cr_assert_str_eq(cell_type_name(CELL_STRING), "string");
    cr_assert_str_eq(cell_type_name(CELL_SEXPR), "sexpr");
    cr_assert_str_eq(cell_type_name(CELL_NIL), "nil");
    cr_assert_str_eq(cell_type_name(CELL_PROC), "procedure");
    cr_assert_str_eq(cell_type_name(CELL_ERROR), "error");
    cr_assert_str_eq(cell_type_name(CELL_PAIR), "pair");
    cr_assert_str_eq(cell_type_name(CELL_VECTOR), "vector");
    cr_assert_str_eq(cell_type_name(CELL_CHAR), "char");
    cr_assert_str_eq(cell_type_name(CELL_BYTEVECTOR), "byte vector");
    cr_assert_str_eq(cell_type_name(CELL_EOF), "eof");
    cr_assert_str_eq(cell_type_name(CELL_BIGINT), "bigint");
    cr_assert_str_eq(cell_type_name(CELL_BIGRAT), "bigrat");
    cr_assert_str_eq(cell_type_name(CELL_BIGFLOAT), "bigfloat");
    cr_assert_str_eq(cell_type_name(CELL_PROMISE), "promise");
    cr_assert_str_eq(cell_type_name(CELL_STREAM), "stream");
    cr_assert_str_eq(cell_type_name(CELL_MACRO), "macro");
    // Anything other than the enum vals should return "unknown"
    cr_assert_str_eq(cell_type_name(666), "unknown");
}

Test(type_utils, test_cell_mask_types, .init = setup) {
    cr_assert_str_eq(cell_mask_types(CELL_INTEGER|CELL_REAL|CELL_RATIONAL), "integer|real|rational");
    cr_assert_str_eq(cell_mask_types(CELL_COMPLEX|CELL_BOOLEAN|CELL_SYMBOL), "complex|bool|symbol");
    cr_assert_str_eq(cell_mask_types(CELL_STRING|CELL_SEXPR|CELL_NIL), "string|sexpr|nil");
    cr_assert_str_eq(cell_mask_types(CELL_PROC|CELL_ERROR|CELL_PAIR), "procedure|error|pair");
    cr_assert_str_eq(cell_mask_types(CELL_VECTOR|CELL_CHAR|CELL_BYTEVECTOR), "vector|char|byte vector");
    cr_assert_str_eq(cell_mask_types(CELL_EOF|CELL_BIGINT|CELL_BIGRAT), "eof|bigint|bigrat");
    cr_assert_str_eq(cell_mask_types(CELL_BIGFLOAT|CELL_PROMISE|CELL_STREAM), "bigfloat|promise|stream");
    // The string is built according to position in the enum
    cr_assert_str_eq(cell_mask_types(CELL_MACRO|CELL_VECTOR|CELL_REAL), "real|vector|macro");
}

Test(type_utils, test_check_arg_types, .init = setup) {
    Cell* i = make_cell_integer(25);
    Cell* j = make_cell_real(25.45);
    Cell* k = make_cell_rational(23, 4, true);
    Cell* l = make_cell_complex(i, i);

    Cell* arg = make_sexpr_len4(i, j, k, l);

    Cell* res = check_arg_types(arg, CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX, "foofunc");
    // This func is null on success
    cr_assert_null(res, "Expected null return value");

    res = check_arg_types(arg, CELL_STRING|CELL_SEXPR, "foofunc");
    // Should return CELL_ERROR with incorrect type args
    cr_assert_not_null(res, "Expected non-null return value");
    cr_assert_eq(res->type, CELL_ERROR);
    cr_assert_eq(res->err_t, TYPE_ERR);
}

Test(type_utils, test_check_arg_arity, .init = setup) {
    Cell* i = make_cell_integer(25);
    Cell* j = make_cell_real(25.45);
    Cell* k = make_cell_rational(23, 4, true);
    Cell* l = make_cell_complex(i, i);

    Cell* arg = make_sexpr_len4(i, j, k, l);

    // Easier to test the macros

    // Check for success
    Cell* res = CHECK_ARITY_EXACT(arg, 4, "foofunc");
    cr_assert_null(res, "Expected null return value");
    res = CHECK_ARITY_MIN(arg, 1, "foofunc");
    cr_assert_null(res, "Expected null return value");
    res = CHECK_ARITY_MAX(arg, 4, "foofunc");
    cr_assert_null(res, "Expected null return value");
    res = CHECK_ARITY_RANGE(arg, 2, 5, "foofunc");
    cr_assert_null(res, "Expected null return value");

    // Check for error
    res = CHECK_ARITY_EXACT(arg, 1, "foofunc");
    cr_assert_eq(res->type, CELL_ERROR);
    cr_assert_eq(res->err_t, ARITY_ERR);
    res = CHECK_ARITY_MIN(arg, 5, "foofunc");
    cr_assert_eq(res->type, CELL_ERROR);
    cr_assert_eq(res->err_t, ARITY_ERR);
    res = CHECK_ARITY_MAX(arg, 2, "foofunc");
    cr_assert_eq(res->type, CELL_ERROR);
    cr_assert_eq(res->err_t, ARITY_ERR);
    res = CHECK_ARITY_RANGE(arg, 1, 3, "foofunc");
    cr_assert_eq(res->type, CELL_ERROR);
    cr_assert_eq(res->err_t, ARITY_ERR);
}

Test(type_utils, test_int_to_rat, .init = setup) {
    Cell* i = make_cell_integer(25);
    Cell* j = make_cell_integer(-25);

    Cell* res = int_to_rat(i);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_RATIONAL);
    cr_assert_eq(res->num, 25);
    cr_assert_eq(res->den, 1);

    res = int_to_rat(j);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_RATIONAL);
    cr_assert_eq(res->num, -25);
    cr_assert_eq(res->den, 1);
}

Test(type_utils, test_int_to_real, .init = setup) {
    Cell* i = make_cell_integer(25);
    Cell* j = make_cell_integer(-25);

    Cell* res = int_to_real(i);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_REAL);
    cr_assert_float_eq(res->real_v, 25.0, 1e-9);

    res = int_to_real(j);
    cr_assert_not_null(res);
    cr_assert_eq(res->type, CELL_REAL);
    cr_assert_float_eq(res->real_v, -25.0, 1e-9);
}

//Test(type_utils, test_cell_type_masks, .init = setup) {}
