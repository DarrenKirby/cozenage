#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <gc/gc.h>

#define CRITERION_TEST_BUILD 1
#include "parser.h"

static void setup(void) {
    GC_INIT();
}

TestSuite(test_parser);

Test(test_lexer, test_parse_float_checked, .init = setup) {
    // Test expected successful conversions
    int err = 0;
    char err_buf[128] = {0};
    char* str = "123.456";
    long double res = parse_float_checked(str, err_buf, &err);
    cr_assert_float_eq(res, 123.456, 1e-9);
    cr_assert_str_eq(err_buf, "");
    cr_assert_eq(err, 1, "Expected 1, got %d", err);

    err = 0;
    err_buf[0] = '\0';
    str = "0.123456";
    res = parse_float_checked(str, err_buf, &err);
    cr_assert_float_eq(res, 0.123456, 1e-9);
    cr_assert_str_eq(err_buf, "");
    cr_assert_eq(err, 1, "Expected 1, got %d", err);

    /*
    err = 0;
    err_buf[0] = '\0';
    str = "99999999.9999999";
    res = parse_float_checked(str, err_buf, &err);
    cr_assert_float_eq(res, 99999999.9999999, 1e-9, "Expected 99999999.9999999, got %Lf", res);
    cr_assert_str_eq(err_buf, "");
    cr_assert_eq(err, 1, "Expected 1, got %d", err);
*/

    // Test expected failures
    err = 0;
    err_buf[0] = '\0';
    str = "eat beef";
    res = parse_float_checked(str, err_buf, &err);
    cr_assert_float_eq(res, 0, 1e-9);
    cr_assert_str_eq(err_buf, "Invalid numeric: '\x1b[31;1meat beef\x1b[0m'");
    cr_assert_eq(err, 0, "Expected 1, got %d", err);

}

