#include "test_meta.h"
#include <criterion/criterion.h>


TestSuite(end_to_end_chars);

Test(end_to_end_chars, test_char_integer_conversions, .init = setup_each_test, .fini = teardown_each_test) {
    /* Test integer->char */
    cr_assert_str_eq(t_eval("(integer->char 65)"), "#\\A");
    cr_assert_str_eq(t_eval("(integer->char 97)"), "#\\a");
    cr_assert_str_eq(t_eval("(integer->char 48)"), "#\\0");
    cr_assert_str_eq(t_eval("(integer->char 32)"), "#\\space");
    cr_assert_str_eq(t_eval("(integer->char 10)"), "#\\newline");
    cr_assert_str_eq(t_eval("(integer->char 9)"), "#\\tab");
    cr_assert_str_eq(t_eval("(integer->char 0)"), "#\\null");
    cr_assert_str_eq(t_eval("(integer->char 127)"), "#\\delete");
    cr_assert_str_eq(t_eval("(integer->char 8364)"), "#\\€"); // Euro symbol (€)
    cr_assert_str_eq(t_eval("(integer->char 955)"), "#\\λ");  // Greek lambda (λ)

    /* Test char->integer */
    cr_assert_str_eq(t_eval("(char->integer #\\A)"), "65");
    cr_assert_str_eq(t_eval("(char->integer #\\a)"), "97");
    cr_assert_str_eq(t_eval("(char->integer #\\0)"), "48");
    cr_assert_str_eq(t_eval("(char->integer #\\space)"), "32");
    cr_assert_str_eq(t_eval("(char->integer #\\newline)"), "10");
    cr_assert_str_eq(t_eval("(char->integer #\\tab)"), "9");
    cr_assert_str_eq(t_eval("(char->integer #\\null)"), "0");
    cr_assert_str_eq(t_eval("(char->integer #\\delete)"), "127");
    cr_assert_str_eq(t_eval("(char->integer #\\€)"), "8364");
    cr_assert_str_eq(t_eval("(char->integer #\\λ)"), "955");

    /* Test nested expressions and round-trip conversions */
    cr_assert_str_eq(t_eval("(char->integer (integer->char 120))"), "120");
    cr_assert_str_eq(t_eval("(integer->char (char->integer #\\z))"), "#\\z");
    cr_assert_str_eq(t_eval("(integer->char (+ 50 15))"), "#\\A");
    cr_assert_str_eq(t_eval("(char->integer (car '(#\\b #\\c)))"), "98");

    /* The following tests are for error conditions. */
    cr_assert_str_eq(t_eval("(integer->char -1)"), " Value error: integer->char: invalid code point");
    cr_assert_str_eq(t_eval("(integer->char #x110000)"), " Value error: integer->char: invalid code point");
    cr_assert_str_eq(t_eval("(integer->char #xD800)"), " Value error: integer->char: invalid code point (surrogate)");
    cr_assert_str_eq(t_eval("(char->integer 123)"), " Type error: bad type at arg 1: got integer, expected char");
    cr_assert_str_eq(t_eval("(integer->char 1.0)"), " Type error: bad type at arg 1: got float, expected integer");
}

Test(end_to_end_chars, test_char_equal, .init = setup_each_test, .fini = teardown_each_test) {
    /* Test basic equality */
    cr_assert_str_eq(t_eval("(char=? #\\a #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(char=? #\\a #\\b)"), "#false");
    cr_assert_str_eq(t_eval("(char=? #\\A #\\a)"), "#false"); // Case-sensitive

    /* Test with multiple arguments */
    cr_assert_str_eq(t_eval("(char=? #\\z #\\z #\\z #\\z)"), "#true");
    cr_assert_str_eq(t_eval("(char=? #\\z #\\z #\\a #\\z)"), "#false");

    /* Test named characters */
    cr_assert_str_eq(t_eval("(char=? #\\space #\\space)"), "#true");
    cr_assert_str_eq(t_eval("(char=? #\\newline #\\space)"), "#false");
    cr_assert_str_eq(t_eval("(char=? #\\tab #\\tab #\\tab)"), "#true");

    /* Test Unicode characters */
    cr_assert_str_eq(t_eval("(char=? #\\λ #\\λ)"), "#true");
    cr_assert_str_eq(t_eval("(char=? #\\€ #\\λ)"), "#false");

    /*
     * R7RS is ambiguous about zero or one argument for char=?.
     * Most interpreters return #t in these cases, following the pattern
     * of numerical comparisons like =.
     */
    cr_assert_str_eq(t_eval("(char=?)"), "#true");
    cr_assert_str_eq(t_eval("(char=? #\\a)"), "#true");

    /* Test evaluation of arguments */
    cr_assert_str_eq(t_eval("(char=? (integer->char 97) #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(char=? #\\b (car '(#\\b #\\c)))"), "#true");

    /* The following tests are for error conditions. */
    cr_assert_str_eq(t_eval("(char=? #\\a 1)"), " Type error: bad type at arg 2: got integer, expected char");
    cr_assert_str_eq(t_eval("(char=? 'a #\\a)"), " Type error: bad type at arg 1: got symbol, expected char");
    cr_assert_str_eq(t_eval("(char=? #\\a \"a\")"), " Type error: bad type at arg 2: got string, expected char");
}

Test(end_to_end_chars, test_char_less_than, .init = setup_each_test, .fini = teardown_each_test) {
    /* Test basic < comparison */
    cr_assert_str_eq(t_eval("(char<? #\\a #\\b)"), "#true");
    cr_assert_str_eq(t_eval("(char<? #\\b #\\a)"), "#false");
    cr_assert_str_eq(t_eval("(char<? #\\a #\\a)"), "#false");

    /* Test case sensitivity */
    cr_assert_str_eq(t_eval("(char<? #\\A #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(char<? #\\Z #\\a)"), "#true");

    /* Test with multiple arguments (must be strictly increasing) */
    cr_assert_str_eq(t_eval("(char<? #\\a #\\b #\\c #\\d)"), "#true");
    cr_assert_str_eq(t_eval("(char<? #\\a #\\c #\\b #\\d)"), "#false");
    cr_assert_str_eq(t_eval("(char<? #\\a #\\b #\\b #\\d)"), "#false");

    /* Test named characters */
    cr_assert_str_eq(t_eval("(char<? #\\tab #\\newline #\\space)"), "#true"); // 9 < 10 < 32
    cr_assert_str_eq(t_eval("(char<? #\\null #\\delete)"), "#true"); // 0 < 127

    /* Test Unicode characters */
    cr_assert_str_eq(t_eval("(char<? #\\z #\\λ)"), "#true");   // 122 < 955
    cr_assert_str_eq(t_eval("(char<? #\\λ #\\€)"), "#true");   // 955 < 8364
    cr_assert_str_eq(t_eval("(char<? #\\€ #\\λ)"), "#false");

    /*
     * R7RS is ambiguous about zero or one argument for char<?.
     * Most interpreters return #t in these cases, following the pattern
     * of numerical comparisons like <.
     */
    cr_assert_str_eq(t_eval("(char<?)"), "#true");
    cr_assert_str_eq(t_eval("(char<? #\\a)"), "#true");

    /* Test evaluation of arguments */
    cr_assert_str_eq(t_eval("(char<? #\\a (integer->char 98))"), "#true");
    cr_assert_str_eq(t_eval("(char<? (car '(#\\a #\\b)) #\\c)"), "#true");

    /* The following tests are for error conditions. */
    cr_assert_str_eq(t_eval("(char<? #\\a 97)"), " Type error: bad type at arg 2: got integer, expected char");
    cr_assert_str_eq(t_eval("(char<? #\\a #\\b 'c)"), " Type error: bad type at arg 3: got symbol, expected char");
    cr_assert_str_eq(t_eval("(char<? \"a\" #\\b)"), " Type error: bad type at arg 1: got string, expected char");
}

Test(end_to_end_chars, test_char_less_than_or_equal, .init = setup_each_test, .fini = teardown_each_test) {
    /* Test basic <= comparison */
    cr_assert_str_eq(t_eval("(char<=? #\\a #\\b)"), "#true");
    cr_assert_str_eq(t_eval("(char<=? #\\b #\\a)"), "#false");
    cr_assert_str_eq(t_eval("(char<=? #\\a #\\a)"), "#true");

    /* Test case sensitivity */
    cr_assert_str_eq(t_eval("(char<=? #\\A #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(char<=? #\\Z #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(char<=? #\\a #\\A)"), "#false");

    /* Test with multiple arguments (must be non-decreasing) */
    cr_assert_str_eq(t_eval("(char<=? #\\a #\\b #\\c #\\d)"), "#true");
    cr_assert_str_eq(t_eval("(char<=? #\\a #\\b #\\b #\\d)"), "#true");
    cr_assert_str_eq(t_eval("(char<=? #\\a #\\c #\\b #\\d)"), "#false");

    /* Test named characters */
    cr_assert_str_eq(t_eval("(char<=? #\\tab #\\newline #\\space)"), "#true"); // 9 <= 10 <= 32
    cr_assert_str_eq(t_eval("(char<=? #\\space #\\space)"), "#true");

    /* Test Unicode characters */
    cr_assert_str_eq(t_eval("(char<=? #\\z #\\λ #\\€)"), "#true");   // 122 <= 955 <= 8364
    cr_assert_str_eq(t_eval("(char<=? #\\λ #\\λ #\\€)"), "#true");
    cr_assert_str_eq(t_eval("(char<=? #\\€ #\\λ)"), "#false");

    /*
     * R7RS is ambiguous about zero or one argument for char<=?.
     * Most interpreters return #t in these cases, following the pattern
     * of numerical comparisons like <=.
     */
    cr_assert_str_eq(t_eval("(char<=?)"), "#true");
    cr_assert_str_eq(t_eval("(char<=? #\\a)"), "#true");

    /* Test evaluation of arguments */
    cr_assert_str_eq(t_eval("(char<=? #\\a (integer->char 97))"), "#true");
    cr_assert_str_eq(t_eval("(char<=? (car '(#\\c #\\b)) #\\c)"), "#true");

    /* The following tests are for error conditions.*/
    cr_assert_str_eq(t_eval("(char<=? #\\a 98)"), " Type error: bad type at arg 2: got integer, expected char");
    cr_assert_str_eq(t_eval("(char<=? #\\a #\\b 'b)"), " Type error: bad type at arg 3: got symbol, expected char");
    cr_assert_str_eq(t_eval("(char<=? \"c\" #\\b)"), " Type error: bad type at arg 1: got string, expected char");
}

Test(end_to_end_chars, test_char_greater_than, .init = setup_each_test, .fini = teardown_each_test) {
    /* Test basic > comparison */
    cr_assert_str_eq(t_eval("(char>? #\\b #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(char>? #\\a #\\b)"), "#false");
    cr_assert_str_eq(t_eval("(char>? #\\a #\\a)"), "#false");

    /* Test case sensitivity */
    cr_assert_str_eq(t_eval("(char>? #\\a #\\A)"), "#true");
    cr_assert_str_eq(t_eval("(char>? #\\a #\\Z)"), "#true");

    /* Test with multiple arguments (must be strictly decreasing) */
    cr_assert_str_eq(t_eval("(char>? #\\d #\\c #\\b #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(char>? #\\d #\\b #\\c #\\a)"), "#false");
    cr_assert_str_eq(t_eval("(char>? #\\d #\\c #\\c #\\a)"), "#false");

    /* Test named characters */
    cr_assert_str_eq(t_eval("(char>? #\\space #\\newline #\\tab)"), "#true"); // 32 > 10 > 9
    cr_assert_str_eq(t_eval("(char>? #\\delete #\\null)"), "#true"); // 127 > 0

    /* Test Unicode characters */
    cr_assert_str_eq(t_eval("(char>? #\\€ #\\λ #\\z)"), "#true");   // 8364 > 955 > 122
    cr_assert_str_eq(t_eval("(char>? #\\λ #\\€)"), "#false");

    /*
     * R7RS is ambiguous about zero or one argument for char>?.
     * Most interpreters return #t in these cases, following the pattern
     * of numerical comparisons like >.
     */
    cr_assert_str_eq(t_eval("(char>?)"), "#true");
    cr_assert_str_eq(t_eval("(char>? #\\z)"), "#true");

    /* Test evaluation of arguments */
    cr_assert_str_eq(t_eval("(char>? (integer->char 98) #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(char>? #\\z (car '(#\\y #\\x)))"), "#true");

    /* The following tests are for error conditions. */
    cr_assert_str_eq(t_eval("(char>? #\\b 97)"), " Type error: bad type at arg 2: got integer, expected char");
    cr_assert_str_eq(t_eval("(char>? #\\c #\\b 'a)"), " Type error: bad type at arg 3: got symbol, expected char");
    cr_assert_str_eq(t_eval("(char>? \"z\" #\\y)"), " Type error: bad type at arg 1: got string, expected char");
}

Test(end_to_end_chars, test_char_greater_than_or_equal, .init = setup_each_test, .fini = teardown_each_test) {
    /* Test basic >= comparison */
    cr_assert_str_eq(t_eval("(char>=? #\\b #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(char>=? #\\a #\\b)"), "#false");
    cr_assert_str_eq(t_eval("(char>=? #\\b #\\b)"), "#true");

    /* Test case sensitivity */
    cr_assert_str_eq(t_eval("(char>=? #\\a #\\A)"), "#true");
    cr_assert_str_eq(t_eval("(char>=? #\\A #\\a)"), "#false");

    /* Test with multiple arguments (must be non-increasing) */
    cr_assert_str_eq(t_eval("(char>=? #\\d #\\c #\\b #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(char>=? #\\d #\\c #\\c #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(char>=? #\\d #\\b #\\c #\\a)"), "#false");

    /* Test named characters */
    cr_assert_str_eq(t_eval("(char>=? #\\space #\\newline #\\tab)"), "#true"); // 32 >= 10 >= 9
    cr_assert_str_eq(t_eval("(char>=? #\\alarm #\\alarm)"), "#true");

    /* Test Unicode characters */
    cr_assert_str_eq(t_eval("(char>=? #\\€ #\\λ #\\z)"), "#true");   // 8364 >= 955 >= 122
    cr_assert_str_eq(t_eval("(char>=? #\\€ #\\€ #\\z)"), "#true");
    cr_assert_str_eq(t_eval("(char>=? #\\λ #\\€)"), "#false");

    /*
     * R7RS is ambiguous about zero or one argument for char>=?.
     * Most interpreters return #t in these cases, following the pattern
     * of numerical comparisons like >=.
     */
    cr_assert_str_eq(t_eval("(char>=?)"), "#true");
    cr_assert_str_eq(t_eval("(char>=? #\\x)"), "#true");

    /* Test evaluation of arguments */
    cr_assert_str_eq(t_eval("(char>=? (integer->char 97) #\\a)"), "#true");
    cr_assert_str_eq(t_eval("(char>=? #\\c (car '(#\\c #\\b)))"), "#true");

    /* The following tests are for error conditions. */
    cr_assert_str_eq(t_eval("(char>=? #\\b 97)"), " Type error: bad type at arg 2: got integer, expected char");
    cr_assert_str_eq(t_eval("(char>=? #\\c #\\b 'b)"), " Type error: bad type at arg 3: got symbol, expected char");
    cr_assert_str_eq(t_eval("(char>=? \"b\" #\\a)"), " Type error: bad type at arg 1: got string, expected char");
}

Test(end_to_end_chars, test_extended_named_characters, .init = setup_each_test, .fini = teardown_each_test) {
    /* Test extended named characters (Greek uppercase) */
    cr_assert_str_eq(t_eval("(char->integer #\\Alpha)"), "913");
    cr_assert_str_eq(t_eval("(char->integer #\\Delta)"), "916");
    cr_assert_str_eq(t_eval("(char->integer #\\Lambda)"), "923");
    cr_assert_str_eq(t_eval("(char->integer #\\Omega)"), "937");
    cr_assert_str_eq(t_eval("(char->integer #\\Pi)"), "928");
    cr_assert_str_eq(t_eval("(char->integer #\\Sigma)"), "931");
    cr_assert_str_eq(t_eval("(char->integer #\\Xi)"), "926");

    /* Test extended named characters (Greek lowercase) */
    cr_assert_str_eq(t_eval("(char->integer #\\alpha)"), "945");
    cr_assert_str_eq(t_eval("(char->integer #\\beta)"), "946");
    cr_assert_str_eq(t_eval("(char->integer #\\delta)"), "948");
    cr_assert_str_eq(t_eval("(char->integer #\\epsilon)"), "949");
    cr_assert_str_eq(t_eval("(char->integer #\\lambda)"), "955");
    cr_assert_str_eq(t_eval("(char->integer #\\omega)"), "969");
    cr_assert_str_eq(t_eval("(char->integer #\\pi)"), "960");
    cr_assert_str_eq(t_eval("(char->integer #\\sigma)"), "963");
    cr_assert_str_eq(t_eval("(char->integer #\\zeta)"), "950");

    /* Test extended named characters (Symbols) */
    cr_assert_str_eq(t_eval("(char->integer #\\copy)"), "169");
    cr_assert_str_eq(t_eval("(char->integer #\\curren)"), "164");
    cr_assert_str_eq(t_eval("(char->integer #\\deg)"), "176");
    cr_assert_str_eq(t_eval("(char->integer #\\divide)"), "247");
    cr_assert_str_eq(t_eval("(char->integer #\\euro)"), "8364");
    cr_assert_str_eq(t_eval("(char->integer #\\iquest)"), "191");
    cr_assert_str_eq(t_eval("(char->integer #\\micro)"), "181");
    cr_assert_str_eq(t_eval("(char->integer #\\para)"), "182");
    cr_assert_str_eq(t_eval("(char->integer #\\plusnm)"), "177");
    cr_assert_str_eq(t_eval("(char->integer #\\pound)"), "163");
    cr_assert_str_eq(t_eval("(char->integer #\\reg)"), "174");
    cr_assert_str_eq(t_eval("(char->integer #\\sect)"), "167");
    cr_assert_str_eq(t_eval("(char->integer #\\times)"), "215");
    cr_assert_str_eq(t_eval("(char->integer #\\yen)"), "165");
}
