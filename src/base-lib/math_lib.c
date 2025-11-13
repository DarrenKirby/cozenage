/*
 * 'math_lib.c'
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

#include "math_lib.h"
#include "types.h"
#include "numerics.h"
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

/* Helper for the core GCD algorithm (Euclidean) for two integers. */
long long gcd_helper(long long x, long long y) {
    x = llabs(x);
    y = llabs(y);
    while (x != 0) {
        const long long tmp = x;
        x = y % x;
        y = tmp;
    }
    return y;
}

/* Helper for the core LCM logic (using the overflow-safe formula). */
long long lcm_helper(long long x, long long y) {
    if (x == 0 || y == 0) return 0;
    x = llabs(x);
    y = llabs(y);
    return (x / gcd_helper(x, y)) * y;
}

Cell* builtin_gcd(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER);
    if (err) return err;

    if (a->count == 0) {
        return make_cell_integer(0); /* Identity for GCD */
    }

    long long result = a->cell[0]->integer_v;
    for (int i = 1; i < a->count; i++) {
        result = gcd_helper(result, a->cell[i]->integer_v);
    }

    return make_cell_integer(llabs(result)); /* Final result must be non-negative */
}

Cell* builtin_lcm(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER);
    if (err) { return err; }

    if (a->count == 0) {
        return make_cell_integer(1); /* Identity for LCM */
    }

    long long result = a->cell[0]->integer_v;
    for (int i = 1; i < a->count; i++) {
        result = lcm_helper(result, a->cell[i]->integer_v);
    }

    return make_cell_integer(llabs(result)); /* Final result must be non-negative */
}

/* (floor-quotient n1 n2) */
Cell* builtin_floor_quotient(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }

    const long long n1 = a->cell[0]->integer_v ;
    const long long n2 = a->cell[1]->integer_v ;

    long long q = n1 / n2;
    const long long r = n1 % n2;

    if (r != 0 && n1 > 0 != n2 > 0) {
        q = q - 1;
    }

    return make_cell_integer(q);
}

/* (floor/ n1 n2 ) */
Cell* builtin_floor_div(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }

    const long long n1 = a->cell[0]->integer_v;
    const long long n2 = a->cell[1]->integer_v;

    if (n2 == 0) {
        return make_cell_error(
            "truncate/: division by zero",
            VALUE_ERR);
    }

    long long q = n1 / n2; /* C's integer division truncates. */
    long long r = n1 % n2; /* C's modulo is consistent with its division. */

    /* If the remainder is non-zero and the signs of n and d differ,
     * C's division truncated towards zero, which is the wrong direction
     * for floor. We need to adjust. */
    if (r != 0 && n1 > 0 != n2 > 0) {
        q = q - 1;
        r = r + n2;
    }

    Cell* result = make_cell_mrv();
    cell_add(result, make_cell_integer(q));
    cell_add(result, make_cell_integer(r));
    return result;
}

/* (truncate/ n1 n2 ) */
Cell* builtin_truncate_div(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }

    const long long n1 = a->cell[0]->integer_v ;
    const long long n2 = a->cell[1]->integer_v ;

    if (n2 == 0) {
        return make_cell_error(
            "truncate/: division by zero",
            VALUE_ERR);
    }

    const long long q = n1 / n2; /* C's integer division truncates. */
    const long long r = n1 % n2; /* C's modulo is consistent with its division. */

    Cell* result = make_cell_mrv();
    cell_add(result, make_cell_integer(q));
    cell_add(result, make_cell_integer(r));
    return result;
}

/* 'real-part' -> CELL_REAL|CELL_RATIONAL|CELL_INTEGER - returns the real part of a
 * complex number */
Cell* builtin_real_part(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    Cell* sub = a->cell[0];
    if (sub->type == CELL_INTEGER ||
        sub->type == CELL_REAL ||
        sub->type == CELL_RATIONAL) {
        return sub;
    }
    if (sub->type == CELL_COMPLEX) {
        return sub->real;
    }
    /* If we didn't return early, we have the wrong arg type */
    return check_arg_types(make_sexpr_len1(sub), CELL_COMPLEX|CELL_REAL|CELL_RATIONAL|CELL_INTEGER);
}

/* 'imag-part' -> CELL_REAL|CELL_RATIONAL|CELL_INTEGER - returns the imaginary part of a
 * complex number */
Cell* builtin_imag_part(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    const Cell* sub = a->cell[0];
    if (sub->type == CELL_INTEGER ||
        sub->type == CELL_REAL ||
        sub->type == CELL_RATIONAL) {
        return make_cell_integer(0);
        }
    if (sub->type == CELL_COMPLEX) {
        return sub->imag;
    }
    /* If we didn't return early, we have the wrong arg type */
    return check_arg_types(make_sexpr_len1(sub), CELL_COMPLEX|CELL_REAL|CELL_RATIONAL|CELL_INTEGER);
}

/* 'make-rectangular' -> CELL_COMPLEX- convert a complex number to rectangular form */
Cell* builtin_make_rectangular(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    err = check_arg_types(a, CELL_REAL|CELL_RATIONAL|CELL_INTEGER);
    if (err) return err;

    return make_cell_complex(a->cell[0], a->cell[1]);
}

/* 'angle' -> CELL_REAL- calculate angle 'θ' of complex number */
Cell* builtin_angle(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_REAL|CELL_RATIONAL|CELL_INTEGER|CELL_COMPLEX);
    if (err) return err;

    const Cell* arg = a->cell[0];
    switch (arg->type) {
        case CELL_COMPLEX: {
            const long double r = cell_to_long_double(arg->real);
            const long double i = cell_to_long_double(arg->imag);
            return make_cell_from_double(atan2l(i, r));
        }
        default:
            return make_cell_from_double(atan2l(0, cell_to_long_double(arg)));
    }
}

/* 'make-polar' -> CELL_COMPLEX- convert a complex number to polar form */
Cell* builtin_make_polar(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    err = check_arg_types(a, CELL_REAL|CELL_RATIONAL|CELL_INTEGER);
    if (err) return err;

    const long double M = cell_to_long_double(a->cell[0]);
    const long double A = cell_to_long_double(a->cell[1]);

    const long double real_part = M * cosl(A);
    const long double imag_part = M * sinl(A);

    return make_cell_complex(
        make_cell_from_double(real_part),
        make_cell_from_double(imag_part)
        );
}


void lex_add_math_lib(const Lex* e) {
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
    lex_add_builtin(e, "cbrt", builtin_cbrt);
    lex_add_builtin(e, "truncate/", builtin_truncate_div);
    lex_add_builtin(e, "truncate-quotient", builtin_quotient);
    lex_add_builtin(e, "truncate-remainder", builtin_remainder);
    lex_add_builtin(e, "floor/", builtin_floor_div);
    lex_add_builtin(e, "floor-quotient", builtin_floor_quotient);
    lex_add_builtin(e, "floor-remainder", builtin_modulo);
    lex_add_builtin(e, "lcm", builtin_lcm);
    lex_add_builtin(e, "gcd", builtin_gcd);
    lex_add_builtin(e, "real-part", builtin_real_part);
    lex_add_builtin(e, "imag-part", builtin_imag_part);
    lex_add_builtin(e,"make-rectangular", builtin_make_rectangular);
    /* 'magnitude' is identical to 'abs' for real/complex numbers -
     * so we just make an alias */
    lex_add_builtin(e,"magnitude", builtin_abs);
    lex_add_builtin(e,"angle", builtin_angle);
    lex_add_builtin(e,"make-polar", builtin_make_polar);
}
