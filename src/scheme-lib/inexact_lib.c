/*
 * 'src/inexact_lib.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025  Darren Kirby <darren@dragonbyte.ca>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "inexact_lib.h"
#include "types.h"
#include <math.h>
#include <complex.h>


/* Helper to make C complex from Cell complex */
static long double complex cell_to_c_complex(const Cell* c)
{
    long double a = cell_to_long_double(c->real);
    long double b = cell_to_long_double(c->imag);

    return CMPLXL(a, b);
}

/* Returns the cosine of arg (arg is in radians). */
Cell* builtin_cos(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == CELL_COMPLEX)
    {
        const long double complex z = cell_to_c_complex(a->cell[0]);
        const long double complex z_result = ccosl(z);
        Cell* real = make_cell_from_double(creall(z_result));
        Cell* imag = make_cell_from_double(cimagl(z_result));
        return make_cell_complex(real, imag);
    }

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(cosl(n));
    return result;
}

/* Returns the arccosine of arg, in radians */
Cell* builtin_acos(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == CELL_COMPLEX)
    {
        const long double complex z = cell_to_c_complex(a->cell[0]);
        const long double complex z_result = cacosl(z);
        Cell* real = make_cell_from_double(creall(z_result));
        Cell* imag = make_cell_from_double(cimagl(z_result));
        return make_cell_complex(real, imag);
    }

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(acosl(n));
    return result;
}

/* Returns the sine of arg (arg is in radians) */
Cell* builtin_sin(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == CELL_COMPLEX)
    {
        const long double complex z = cell_to_c_complex(a->cell[0]);
        const long double complex z_result = csinl(z);
        Cell* real = make_cell_from_double(creall(z_result));
        Cell* imag = make_cell_from_double(cimagl(z_result));
        return make_cell_complex(real, imag);
    }

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(sinl(n));
    return result;
}

/* Returns the arcsine of arg, in radians */
Cell* builtin_asin(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == CELL_COMPLEX)
    {
        const long double complex z = cell_to_c_complex(a->cell[0]);
        const long double complex z_result = casinl(z);
        Cell* real = make_cell_from_double(creall(z_result));
        Cell* imag = make_cell_from_double(cimagl(z_result));
        return make_cell_complex(real, imag);
    }

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(asinl(n));
    return result;
}

/* Returns the tangent of arg (arg is in radians) */
Cell* builtin_tan(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == CELL_COMPLEX)
    {
        const long double complex z = cell_to_c_complex(a->cell[0]);
        const long double complex z_result = ctanl(z);
        Cell* real = make_cell_from_double(creall(z_result));
        Cell* imag = make_cell_from_double(cimagl(z_result));
        return make_cell_complex(real, imag);
    }

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(tanl(n));
    return result;
}

/* With one arg: Returns the arctangent of arg as a numeric value between -PI/2 and PI/2 radians
 * With two args: Returns the angle theta from the conversion of rectangular coordinates (x, y)
 * to polar coordinates (r, theta) */
Cell* builtin_atan(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_RANGE(a, 1, 2))) { return err; }

    if (a->count == 1) {

        if (a->cell[0]->type == CELL_COMPLEX)
        {
            const long double complex z = cell_to_c_complex(a->cell[0]);
            const long double complex z_result = catanl(z);
            Cell* real = make_cell_from_double(creall(z_result));
            Cell* imag = make_cell_from_double(cimagl(z_result));
            return make_cell_complex(real, imag);
        }

        const long double n = cell_to_long_double(a->cell[0]);
        Cell* result = make_cell_from_double(atanl(n));
        return result;
    }
    /* two args */
    if (a->cell[0]->type == CELL_COMPLEX) {
        return make_cell_error(
            "atan: invalid complex arg. Use 'make-polar' from (scheme complex)",
            TYPE_ERR);
    }

    const long double x = cell_to_long_double(a->cell[0]);
    const long double y = cell_to_long_double(a->cell[1]);
    Cell* result = make_cell_from_double(atan2l(x, y));
    return result;
}

/* Returns the value of E raised to arg power */
Cell* builtin_exp(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == CELL_COMPLEX)
    {
        const long double complex z = cell_to_c_complex(a->cell[0]);
        const long double complex z_result = cexpl(z);
        Cell* real = make_cell_from_double(creall(z_result));
        Cell* imag = make_cell_from_double(cimagl(z_result));
        return make_cell_complex(real, imag);
    }

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(expl(n));
    return result;
}

/* With one arg: Returns the natural logarithm of arg
 * With two args (n, b): Returns log n base b */
Cell* builtin_log(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_RANGE(a, 1, 2))) { return err; }

    if (a->count == 1) {

        if (a->cell[0]->type == CELL_COMPLEX)
        {
            const long double complex z = cell_to_c_complex(a->cell[0]);
            const long double complex z_result = clogl(z);
            Cell* real = make_cell_from_double(creall(z_result));
            Cell* imag = make_cell_from_double(cimagl(z_result));
            return make_cell_complex(real, imag);
        }

        const long double n = cell_to_long_double(a->cell[0]);
        Cell* result = make_cell_from_double(logl(n));
        return result;
    }
    /* Two args - will not work with complex */
    if (a->cell[0]->type == CELL_COMPLEX) {
        return make_cell_error(
            "Specifying log base not valid with complex",
            TYPE_ERR);
    }

    const long double n = cell_to_long_double(a->cell[0]);
    const long double b = cell_to_long_double(a->cell[1]);
    Cell* result = make_cell_from_double(logl(n)/logl(b));
    return result;
}

/* Equivalent to (log n 2) */
Cell* builtin_log2(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(log2l(n));
    return result;
}

/* Equivalent to (log n 10) */
Cell* builtin_log10(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(log10l(n));
    return result;
}

/* Returns the square root of arg */
Cell* builtin_sqrt(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == CELL_COMPLEX)
    {
        const long double complex z = cell_to_c_complex(a->cell[0]);
        const long double complex z_result = csqrtl(z);
        Cell* real = make_cell_from_double(creall(z_result));
        Cell* imag = make_cell_from_double(cimagl(z_result));
        return make_cell_complex(real, imag);
    }

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(sqrtl(n));
    return result;
}

/* Returns the cube root of arg */
Cell* builtin_cbrt(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(cbrtl(n));
    return result;
}

/* Predicate to test if val is infinite */
Cell* builtin_infinite(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const Cell* arg = a->cell[0];
    if (arg->type == CELL_COMPLEX) {
        const long double r = cell_to_long_double(arg->real);
        const long double i = cell_to_long_double(arg->imag);
        return make_cell_boolean(isinf(r) || isinf(i));
    }

    const long double n = cell_to_long_double(arg);
    return make_cell_boolean(isinf(n));
}

/* Predicate to test if val is finite */
Cell* builtin_finite(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const Cell* arg = a->cell[0];
    if (arg->type == CELL_COMPLEX) {
        const long double r = cell_to_long_double(arg->real);
        const long double i = cell_to_long_double(arg->imag);
        return make_cell_boolean(isfinite(r) && isfinite(i));
    }

    const long double n = cell_to_long_double(arg);
    return make_cell_boolean(isfinite(n));
}

/* Predicate to test if val is nan */
Cell* builtin_nan(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX);
    if (err) { return err; }

    const Cell* arg = a->cell[0];
    if (arg->type == CELL_COMPLEX) {
        const long double r = cell_to_long_double(arg->real);
        const long double i = cell_to_long_double(arg->imag);
        return make_cell_boolean(isnan(r) || isnan(i));
    }

    const long double n = cell_to_long_double(arg);
    return make_cell_boolean(isnan(n));
}

void lex_add_inexact_lib(const Lex* e) {
    lex_add_builtin(e, "cos", builtin_cos);
    lex_add_builtin(e, "acos", builtin_acos);
    lex_add_builtin(e, "sin", builtin_sin);
    lex_add_builtin(e, "asin", builtin_asin);
    lex_add_builtin(e, "tan", builtin_tan);
    lex_add_builtin(e, "atan", builtin_atan);
    lex_add_builtin(e, "exp", builtin_exp);
    lex_add_builtin(e, "log", builtin_log);
    lex_add_builtin(e, "log2", builtin_log2); /* Non-standard. Move elsewhere? */
    lex_add_builtin(e, "log10", builtin_log10); /* Non-standard. Move elsewhere? */
    lex_add_builtin(e, "sqrt", builtin_sqrt);
    lex_add_builtin(e, "cbrt", builtin_cbrt); /* Non-standard. Move elsewhere? */
    lex_add_builtin(e, "infinite?", builtin_infinite);
    lex_add_builtin(e, "finite?", builtin_finite);
    lex_add_builtin(e, "nan?", builtin_nan);
}
