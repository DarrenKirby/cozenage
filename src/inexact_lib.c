#include "inexact_lib.h"
#include "types.h"
#include <math.h>

/*
infinite?
nan?
finite?
*/

/* Returns the cosine of arg (arg is in radians). */
Cell* builtin_cos(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    /* TODO: add complex support */

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(cosl(n));
    return result;
}

/* Returns the arccosine of arg, in radians */
Cell* builtin_acos(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    /* TODO: add complex support */

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(acosl(n));
    return result;
}

/* Returns the sine of arg (arg is in radians) */
Cell* builtin_sin(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    /* TODO: add complex support */

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(sinl(n));
    return result;
}

/* Returns the arcsine of arg, in radians */
Cell* builtin_asin(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    /* TODO: add complex support */

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(asinl(n));
    return result;
}

/* Returns the tangent of arg (arg is in radians) */
Cell* builtin_tan(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    /* TODO: add complex support */

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(tanl(n));
    return result;
}

/* With one arg: Returns the arctangent of arg as a numeric value between -PI/2 and PI/2 radians
 * With two args: Returns the angle theta from the conversion of rectangular coordinates (x, y)
 * to polar coordinates (r, theta) */
Cell* builtin_atan(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_RANGE(a, 1, 2))) { return err; }
    /* TODO: add complex support */

    if (a->count == 1) {
        const long double n = cell_to_long_double(a->cell[0]);
        Cell* result = make_cell_from_double(atanl(n));
        return result;
    }
    /* two args */
    const long double x = cell_to_long_double(a->cell[0]);
    const long double y = cell_to_long_double(a->cell[1]);
    Cell* result = make_cell_from_double(atan2l(x, y));
    return result;
}

/* Returns the value of E raised to arg power */
Cell* builtin_exp(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    /* TODO: add complex support */

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(expl(n));
    return result;
}

/* With one arg: Returns the natural logarithm of arg
 * With two args (n, b): Returns log n base b */
Cell* builtin_log(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_RANGE(a, 1, 2))) { return err; }
    /* TODO: add complex support */
    if (a->count == 1) {
        const long double n = cell_to_long_double(a->cell[0]);
        Cell* result = make_cell_from_double(logl(n));
        return result;
    }
    const long double n = cell_to_long_double(a->cell[0]);
    const long double b = cell_to_long_double(a->cell[1]);
    Cell* result = make_cell_from_double(logl(n)/logl(b));
    return result;
}

/* Equivalent to (log n 2) */
Cell* builtin_log2(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    /* TODO: add complex support */

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(log2l(n));
    return result;
}

/* Equivalent to (log n 10) */
Cell* builtin_log10(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    /* TODO: add complex support */

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(log10l(n));
    return result;
}

/* Returns the square root of arg */
Cell* builtin_sqrt(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    /* TODO: add complex support */

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(sqrtl(n));
    return result;
}

/* Returns the cube root of arg */
Cell* builtin_cbrt(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    /* TODO: add complex support */

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(cbrtl(n));
    return result;
}

void lex_add_inexact_lib(Lex* e) {
    lex_add_builtin(e, "cos", builtin_cos);
    lex_add_builtin(e, "acos", builtin_acos);
    lex_add_builtin(e, "sin", builtin_sin);
    lex_add_builtin(e, "asin", builtin_asin);
    lex_add_builtin(e, "tan", builtin_tan);
    lex_add_builtin(e, "atan", builtin_atan);
    lex_add_builtin(e, "exp", builtin_exp);
    lex_add_builtin(e, "log", builtin_log);
    lex_add_builtin(e, "log2", builtin_log2);
    lex_add_builtin(e, "log10", builtin_log10);
    lex_add_builtin(e, "sqrt", builtin_sqrt);
    lex_add_builtin(e, "cbrt", builtin_cbrt);
}
