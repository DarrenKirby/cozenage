#include <criterion/criterion.h>
#include <gc/gc.h>

#include "cell.h"
#include "symbols.h"
#include "types.h"


void setup(void) {
    GC_INIT();
    init_global_singletons();
    symbol_table = ht_create(128);
    init_special_forms();
}

Test(cell_constructors, boolean, .init = setup) {
    constexpr int true_int = 1;
    constexpr int false_int = 0;

    const Cell* t = make_cell_boolean(true_int);
    // Verify the pointer isn't null
    cr_assert_not_null(t, "make_cell_boolean returned a NULL pointer");

    // Verify the core type
    cr_assert_eq(t->type, CELL_BOOLEAN,
                 "Expected type %d (CELL_BOOLEAN), got %d", CELL_BOOLEAN, t->type);

    // Verify global singleton
    cr_assert_eq(t, True_Obj);

    const Cell* f = make_cell_boolean(false_int);
    // Verify the pointer isn't null
    cr_assert_not_null(f, "make_cell_boolean returned a NULL pointer");

    // Verify the core type
    cr_assert_eq(f->type, CELL_BOOLEAN,
                 "Expected type %d (CELL_BOOLEAN), got %d", CELL_BOOLEAN, f->type);

    // Verify global singleton
    cr_assert_eq(f, False_Obj);
}

Test(cell_constructors, pos_real_alloc, .init = setup) {
    const long double r = 1234.5678;

    const Cell* v = make_cell_real(r);
    cr_assert_not_null(v, "make_cell_real returned a NULL pointer");
    cr_assert_eq(v->type, CELL_REAL);
    cr_assert_eq(v->exact, false, "Expected v-exact = 0, got %d", v->exact);
    cr_assert_float_eq(v->real_v, 1234.5678, 1e-9);
}

Test(cell_constructors, neg_real_alloc, .init = setup) {
    const long double r = -1234.5678;

    const Cell* v = make_cell_real(r);
    cr_assert_not_null(v, "make_cell_real returned a NULL pointer");
    cr_assert_eq(v->type, CELL_REAL);
    cr_assert_eq(v->exact, false, "Expected v-exact = 0, got %d", v->exact);
    cr_assert_float_eq(v->real_v, -1234.5678, 1e-9);
}

Test(cell_constructors, zero_real_alloc, .init = setup) {
    const long double r = 0.0;

    const Cell* v = make_cell_real(r);
    cr_assert_not_null(v, "make_cell_real returned a NULL pointer");
    cr_assert_eq(v->type, CELL_REAL);
    cr_assert_eq(v->exact, false, "Expected v-exact = 0, got %d", v->exact);
    cr_assert_float_eq(v->real_v, 0.0, 1e-9);
}

Test(cell_constructors, pos_int_alloc, .init = setup) {
    const long long int i = 1234567;

    const Cell* v = make_cell_integer(i);
    cr_assert_not_null(v, "make_cell_int returned a NULL pointer");
    cr_assert_eq(v->type, CELL_INTEGER);
    cr_assert_eq(v->exact, true, "Expected v-exact = 1, got %d", v->exact);
    cr_assert_eq(v->integer_v, 1234567);
}

Test(cell_constructors, neg_int_alloc, .init = setup) {
    const long long int i = -1234567;

    const Cell* v = make_cell_integer(i);
    cr_assert_not_null(v, "make_cell_int returned a NULL pointer");
    cr_assert_eq(v->type, CELL_INTEGER);
    cr_assert_eq(v->exact, true, "Expected v->exact = 1, got %d", v->exact);
    cr_assert_eq(v->integer_v, -1234567);
}

Test(cell_constructors, zero_int_alloc, .init = setup) {
    const long long int i = 0;

    const Cell* v = make_cell_integer(i);
    cr_assert_not_null(v, "make_cell_int returned a NULL pointer");
    cr_assert_eq(v->type, CELL_INTEGER);
    cr_assert_eq(v->exact, true, "Expected v->exact = 1, got %d", v->exact);
    cr_assert_eq(v->integer_v, 0);
}

Test(cell_constructors, pos_rat_alloc, .init = setup) {
    const long int num = 26;
    const long int den = 50;

    // Test without simplification
    Cell* v = make_cell_rational(num, den, false);
    cr_assert_not_null(v, "make_cell_rational returned a NULL pointer");
    cr_assert_eq(v->type, CELL_RATIONAL);
    cr_assert_eq(v->exact, true, "Expected v->exact = 1, got %d", v->exact);
    cr_assert_eq(v->num, 26, "Expected num 26 got %ld", v->num);
    cr_assert_eq(v->den, 50,  "Expected den 50 got %ld", v->den);

    // Test with simplification
    v = make_cell_rational(num, den, true);
    cr_assert_not_null(v, "make_cell_rational returned a NULL pointer");
    cr_assert_eq(v->type, CELL_RATIONAL);
    cr_assert_eq(v->exact, true, "Expected v->exact = 1, got %d", v->exact);
    cr_assert_eq(v->num, 13, "Expected num 13 got %ld", v->num);
    cr_assert_eq(v->den, 25,  "Expected den 25 got %ld", v->den);
}

Test(cell_constructors, neg_rat_alloc, .init = setup) {
    const long int pos_num = 26;
    const long int neg_num = -26;
    const long int pos_den = 50;
    const long int neg_den = -50;

    // Test neg num/pos den
    Cell* v = make_cell_rational(neg_num, pos_den, true);
    cr_assert_not_null(v, "make_cell_rational returned a NULL pointer");
    cr_assert_eq(v->type, CELL_RATIONAL);
    cr_assert_eq(v->exact, true, "Expected v->exact = 1, got %d", v->exact);
    cr_assert_eq(v->num, -13, "Expected num -13 got %ld", v->num);
    cr_assert_eq(v->den, 25,  "Expected den 25 got %ld", v->den);

    // Test neg_num/neg den
    v = make_cell_rational(neg_num, neg_den, true);
    cr_assert_not_null(v, "make_cell_rational returned a NULL pointer");
    cr_assert_eq(v->type, CELL_RATIONAL);
    cr_assert_eq(v->exact, true, "Expected v->exact = 1, got %d", v->exact);
    cr_assert_eq(v->num, 13, "Expected num 13 got %ld", v->num);
    cr_assert_eq(v->den, 25,  "Expected den 25 got %ld", v->den);

    // Test pos_num/neg den
    v = make_cell_rational(pos_num, neg_den, true);
    cr_assert_not_null(v, "make_cell_rational returned a NULL pointer");
    cr_assert_eq(v->type, CELL_RATIONAL);
    cr_assert_eq(v->exact, true, "Expected v->exact = 1, got %d", v->exact);
    cr_assert_eq(v->num, -13, "Expected num -13 got %ld", v->num);
    cr_assert_eq(v->den, 25,  "Expected den 25 got %ld", v->den);
}

/* -------------------------------------------------------------------------
 * Test 1: Exactness Propagation (Integer + Integer)
 * Both parts are exact, so the resulting complex number must be exact.
 * ------------------------------------------------------------------------- */
Test(cell_constructors, complex_exact_int_int, .init = setup) {
    Cell* real_part = make_cell_integer(42);
    Cell* imag_part = make_cell_integer(-7);

    Cell* v = make_cell_complex(real_part, imag_part);

    cr_assert_not_null(v, "make_cell_complex returned a NULL pointer");
    cr_assert_eq(v->type, CELL_COMPLEX);

    // Verify exactness propagation
    cr_assert_eq(v->exact, true, "Complex(int, int) should be exact");

    // Verify pointer assignments (careful to check real vs imag correctly)
    cr_assert_eq(v->real, real_part, "Real pointer mismatch");
    cr_assert_eq(v->imag, imag_part, "Imag pointer mismatch");

    // Deep verification of the values
    cr_assert_eq(v->real->integer_v, 42);
    cr_assert_eq(v->imag->integer_v, -7);
}

/* -------------------------------------------------------------------------
 * Test 2: Exactness Propagation (Rational + Rational)
 * Both parts are exact, so the resulting complex number must be exact.
 * ------------------------------------------------------------------------- */
Test(cell_constructors, complex_exact_rat_rat, .init = setup) {
    Cell* real_part = make_cell_rational(13, 25, false);
    Cell* imag_part = make_cell_rational(-3, 4, false);

    Cell* v = make_cell_complex(real_part, imag_part);

    cr_assert_not_null(v);
    cr_assert_eq(v->type, CELL_COMPLEX);
    cr_assert_eq(v->exact, true, "Complex(rational, rational) should be exact");

    // Deep verification
    cr_assert_eq(v->real->num, 13);
    cr_assert_eq(v->real->den, 25);
    cr_assert_eq(v->imag->num, -3);
    cr_assert_eq(v->imag->den, 4);
}

/* -------------------------------------------------------------------------
 * Test 3: Inexact Propagation (Integer + Real)
 * One part is inexact (Real), so the resulting complex number must be inexact.
 * ------------------------------------------------------------------------- */
Test(cell_constructors, complex_inexact_int_real, .init = setup) {
    Cell* real_part = make_cell_integer(5);
    Cell* imag_part = make_cell_real(3.14159265);

    Cell* v = make_cell_complex(real_part, imag_part);

    cr_assert_not_null(v);
    cr_assert_eq(v->type, CELL_COMPLEX);

    // The inexact imag_part should taint the complex cell
    cr_assert_eq(v->exact, false, "Complex should be inexact because imag part is inexact");

    cr_assert_eq(v->real->integer_v, 5);
    cr_assert_float_eq(v->imag->real_v, 3.14159265, 1e-9);
}

/* -------------------------------------------------------------------------
 * Test 4: Inexact Propagation (Real + Rational)
 * One part is inexact (Real), so the resulting complex number must be inexact.
 * ------------------------------------------------------------------------- */
Test(cell_constructors, complex_inexact_real_rat, .init = setup) {
    Cell* real_part = make_cell_real(-2.71828);
    Cell* imag_part = make_cell_rational(22, 7, false);

    Cell* v = make_cell_complex(real_part, imag_part);

    cr_assert_not_null(v);
    cr_assert_eq(v->type, CELL_COMPLEX);

    // The inexact real_part should taint the complex cell
    cr_assert_eq(v->exact, false, "Complex should be inexact because real part is inexact");

    cr_assert_float_eq(v->real->real_v, -2.71828, 1e-9);
    cr_assert_eq(v->imag->num, 22);
}

/* -------------------------------------------------------------------------
 * Test 5: Error Handling (Nested Complex Numbers)
 * Passing a complex number as either real or imag should return an error cell.
 * ------------------------------------------------------------------------- */
Test(cell_constructors, complex_error_nested, .init = setup) {
    // Construct a valid complex cell to use as a faulty argument
    Cell* valid_int1 = make_cell_integer(1);
    Cell* valid_int2 = make_cell_integer(2);
    Cell* nested_complex = make_cell_complex(valid_int1, valid_int2);

    Cell* valid_real = make_cell_real(1.0);

    // Try passing the complex cell as the real part
    Cell* err_cell_1 = make_cell_complex(nested_complex, valid_real);
    cr_assert_not_null(err_cell_1);
    cr_assert_neq(err_cell_1->type, CELL_COMPLEX, "Should not successfully construct a nested complex cell");
    cr_assert_eq(err_cell_1->type, CELL_ERROR);

    // Try passing the complex cell as the imaginary part
    Cell* err_cell_2 = make_cell_complex(valid_real, nested_complex);
    cr_assert_not_null(err_cell_2);
    cr_assert_neq(err_cell_2->type, CELL_COMPLEX, "Should not successfully construct a nested complex cell");
}

Test(cell_constructors, string_ascii_allocation, .init = setup) {
    const char* input = "hello";

    // Call the internal C function directly
    Cell* v = make_cell_string(input);

    // Verify the pointer isn't null
    cr_assert_not_null(v, "make_cell_string returned a NULL pointer");

    // Verify the core type
    cr_assert_eq(v->type, CELL_STRING,
                 "Expected type %d (CELL_STRING), got %d", CELL_STRING, v->type);

    // Verify the internal lengths
    cr_assert_eq(v->count, 5, "Expected byte_len 5, got %d", v->count);
    cr_assert_eq(v->char_count, 5, "Expected char_count 5, got %d", v->char_count);

    // Verify the SWAR optimization flag triggered
    cr_assert_eq(v->ascii, 1, "Expected ascii flag to be 1 for pure ASCII string");

    // Verify the actual buffer payload
    cr_assert_not_null(v->str, "String buffer pointer is NULL");
    cr_assert_str_eq(v->str, "hello", "Buffer content mismatch");
}

Test(cell_constructors, string_utf8_allocation, .init = setup) {
    // "pi: π" - The pi symbol is 2 bytes in UTF-8
    const char* input = "pi: \xCF\x80";

    Cell* v = make_cell_string(input);

    cr_assert_not_null(v);
    cr_assert_eq(v->type, CELL_STRING);

    // Byte length is 6 ("pi: " = 4, "π" = 2)
    cr_assert_eq(v->count, 6, "Expected byte_len 6, got %d", v->count);

    // Char length is 5 visual characters
    cr_assert_eq(v->char_count, 5, "Expected char_count 5, got %d", v->char_count);

    // Verify the SWAR check correctly identified it as NON-ascii
    cr_assert_eq(v->ascii, 0, "Expected ascii flag to be 0 for UTF-8 string");
}


Test(cell_constructors, make_cell_symbol, .init = setup) {
    Cell* sym = make_cell_symbol("hello");
    cr_assert_not_null(sym);
    cr_assert_eq(sym->type, CELL_SYMBOL);
    cr_assert_eq(sym->sf_id, 0);
    cr_assert_not_null(sym->sym);
    cr_assert_str_eq(sym->sym, "hello");

    sym = make_cell_symbol("goodbye");
    cr_assert_not_null(sym);
    cr_assert_eq(sym->type, CELL_SYMBOL);
    cr_assert_eq(sym->sf_id, 0);
    cr_assert_not_null(sym->sym);
    cr_assert_str_eq(sym->sym, "goodbye");

    // Ensure SF symbol returns correct sf_id
    sym = make_cell_symbol("define");
    cr_assert_not_null(sym);
    cr_assert_eq(sym->type, CELL_SYMBOL);
    cr_assert_eq(sym->sf_id, SF_ID_DEFINE);
    cr_assert_not_null(sym->sym);
    cr_assert_str_eq(sym->sym, "define");

    sym = make_cell_symbol("import");
    cr_assert_not_null(sym);
    cr_assert_eq(sym->type, CELL_SYMBOL);
    cr_assert_eq(sym->sf_id, SF_ID_IMPORT);
    cr_assert_not_null(sym->sym);
    cr_assert_str_eq(sym->sym, "import");
}

Test(cell_constructors, make_cell_sexpr, .init = setup) {
    Cell* i = make_cell_integer(25);
    Cell* s = make_sexpr_len4(i, i, i, i);
    cr_assert_not_null(s);
    cr_assert_eq(s->type, CELL_SEXPR);
    cr_assert_eq(s->count, 4);

    Cell* j = make_cell_integer(25);
    Cell* k = make_cell_integer(25);
    Cell* l = make_cell_integer(25);
    Cell* m = make_cell_integer(25);

    s = make_sexpr_len4(j, k, l, m);
    cr_assert_not_null(s);
    cr_assert_eq(s->type, CELL_SEXPR);
    cr_assert_eq(s->count, 4);
}

Test(cell_constructors, make_cell_char, .init = setup) {
    UChar32 c = 0x0041;
    Cell* ch = make_cell_char(c);
    cr_assert_not_null(ch);
    cr_assert_eq(ch->type, CELL_CHAR);
    cr_assert_eq(ch->char_v, 0x0041);

    c = 0x0041;
    ch = make_cell_char(c);
    cr_assert_not_null(ch);
    cr_assert_eq(ch->type, CELL_CHAR);
    cr_assert_eq(ch->char_v, 0x0041);

    c = 0x00ac;
    ch = make_cell_char(c);
    cr_assert_not_null(ch);
    cr_assert_eq(ch->type, CELL_CHAR);
    cr_assert_eq(ch->char_v, 0x00ac);

    c = 0x00DF;
    ch = make_cell_char(c);
    cr_assert_not_null(ch);
    cr_assert_eq(ch->type, CELL_CHAR);
    cr_assert_eq(ch->char_v, 0x00DF);

    c = 0x03A9;
    ch = make_cell_char(c);
    cr_assert_not_null(ch);
    cr_assert_eq(ch->type, CELL_CHAR);
    cr_assert_eq(ch->char_v, 0x03A9);
}

