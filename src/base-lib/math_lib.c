/*
 * 'src/base-lib/math_lib.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 - 2026 Darren Kirby <darren@dragonbyte.ca>
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


#include "types.h"
#include "numerics.h"
#include "pairs.h"

#include <math.h>


#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
/* *BSD does not provide l versions in complex.h; provide fallback ccosl / csinl / ctanl. */
static inline long double complex ccosl(long double complex z) {
    long double x = creall(z);
    long double y = cimagl(z);
    return cosl(x) * coshl(y) - (sinl(x) * sinhl(y)) * I;
}

static inline long double complex csinl(long double complex z) {
    long double x = creall(z);
    long double y = cimagl(z);
    return sinl(x) * coshl(y) + (cosl(x) * sinhl(y)) * I;
}

static inline long double complex ctanl(long double complex z) {
    return csinl(z) / ccosl(z);
}
#endif


/* (cos z)
 * Returns the cosine of arg (arg is in radians). */
static Cell* math_cos(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "cos");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "cos"))) { return err; }

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


/* (acos z)
 * Returns the arccosine of arg, in radians. */
static Cell* math_acos(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "acos");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "acos"))) { return err; }

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


/* (sin z)
 * Returns the sine of arg (arg is in radians). */
static Cell* math_sin(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "sin");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "sin"))) { return err; }

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


/* (asin z)
 * Returns the arcsine of arg, in radians. */
static Cell* math_asin(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "asin");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "asin"))) { return err; }

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


/* (tan z)
 * Returns the tangent of arg (arg is in radians). */
static Cell* math_tan(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "tan");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "tan"))) { return err; }

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


/* (atan z)
 * (atan y x)
 * With one arg: Returns the arctangent of arg as a numeric value between -PI/2 and PI/2 radians
 * With two args: Returns the angle theta from the conversion of rectangular coordinates (x, y)
 * to polar coordinates (r, theta) */
static Cell* math_atan(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "atan");
    if (err) { return err; }
    if ((err = CHECK_ARITY_RANGE(a, 1, 2, "atan"))) { return err; }

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
    /* Two args. */
    if (a->cell[0]->type == CELL_COMPLEX) {
        return make_cell_error(
            "atan: invalid complex arg. Use 'make-polar'",
            TYPE_ERR);
    }

    const long double x = cell_to_long_double(a->cell[0]);
    const long double y = cell_to_long_double(a->cell[1]);
    Cell* result = make_cell_from_double(atan2l(y, x));
    return result;
}


/* (exp z)
 * Returns the value of E raised to arg power. */
static Cell* math_exp(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "exp");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "exp"))) { return err; }

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


/* (log z)
 * (log z1 z2)
 * With one arg: Returns the natural logarithm of arg.
 * With two args (n, b): Returns log n base b. */
static Cell* math_log(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "log");
    if (err) { return err; }
    if ((err = CHECK_ARITY_RANGE(a, 1, 2, "log"))) { return err; }

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
    /* Two args - will not work with complex. */
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


/* (log2 z)
 * Equivalent to (log n 2). */
static Cell* math_log2(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL, "log2");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "log2"))) { return err; }

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(log2l(n));
    return result;
}


/* (log10 z)
 * Equivalent to (log n 10). */
static Cell* math_log10(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL, "log10");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "log10"))) { return err; }

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(log10l(n));
    return result;
}


/* (cbrt z)
 * Returns the cube root of arg.
 * */
static Cell* math_cbrt(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL, "cbrt");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "cbrt"))) { return err; }

    const long double n = cell_to_long_double(a->cell[0]);
    Cell* result = make_cell_from_double(cbrtl(n));
    return result;
}


/* These procedures implement number-theoretic (integer) division. It is an error if n2 is zero. The procedures ending
 * in / return two integers; the other procedures return an integer. All the procedures compute a quotient nq and
 * remainder nr such that n1 = n2nq + nr. For each of the division operators, there are three procedures defined as
 * follows:
 *
 * (⟨operator⟩/ n1 n2) => nq, nr
 * (⟨operator⟩-quotient n1 n2) => nq
 * (⟨operator⟩-remainder n1 n2) => nr
 */

/* (floor-quotient n1 n2) */
static Cell* math_floor_quotient(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER, "floor-quotient");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2, "floor-quotient"))) { return err; }

    const long long n1 = a->cell[0]->integer_v ;
    const long long n2 = a->cell[1]->integer_v ;

    long long q = n1 / n2;
    const long long r = n1 % n2;

    if (r != 0 && (n1 > 0) != (n2 > 0)) {
        q = q - 1;
    }

    return make_cell_integer(q);
}


/* (floor/ n1 n2 ) */
static Cell* math_floor_div(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER, "floor/");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2, "floor/"))) { return err; }

    const long long n1 = a->cell[0]->integer_v;
    const long long n2 = a->cell[1]->integer_v;

    if (n2 == 0) {
        return make_cell_error(
            "floor/: division by zero",
            VALUE_ERR);
    }

    long long q = n1 / n2; /* C's integer division truncates. */
    long long r = n1 % n2; /* C's modulo is consistent with its division. */

    /* If the remainder is non-zero and the signs of n and d differ,
     * C's division truncated towards zero, which is the wrong direction
     * for floor. We need to adjust. */
    if (r != 0 && (n1 > 0) != (n2 > 0)) {
        q = q - 1;
        r = r + n2;
    }

    return builtin_list(e, make_sexpr_len2(
        make_cell_integer(q),
        make_cell_integer(r)));
}


/* (truncate/ n1 n2 ) */
static Cell* math_truncate_div(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER, "truncate/");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2, "truncate/"))) { return err; }

    const long long n1 = a->cell[0]->integer_v ;
    const long long n2 = a->cell[1]->integer_v ;

    if (n2 == 0) {
        return make_cell_error(
            "truncate/: division by zero",
            VALUE_ERR);
    }

    const long long q = n1 / n2; /* C's integer division truncates. */
    const long long r = n1 % n2; /* C's modulo is consistent with its division. */

    return builtin_list(e, make_sexpr_len2(
        make_cell_integer(q),
        make_cell_integer(r)));
}


/* (real-part z)
 * returns the real part of a complex number. */
static Cell* math_real_part(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "real-part");
    if (err) return err;
    Cell* sub = a->cell[0];
    if (sub->type == CELL_INTEGER ||
        sub->type == CELL_BIGINT ||
        sub->type == CELL_REAL ||
        sub->type == CELL_RATIONAL) {
        return sub;
    }
    if (sub->type == CELL_COMPLEX) {
        return sub->real;
    }
    /* If we didn't return early, we have the wrong arg type. */
    return check_arg_types(make_sexpr_len1(sub),
        CELL_COMPLEX|CELL_REAL|CELL_RATIONAL|CELL_INTEGER|CELL_BIGINT,
        "real-part");
}


/* (imag-part z)
 * returns the imaginary part of a complex number. */
static Cell* math_imag_part(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "imag-part");
    if (err) return err;
    const Cell* sub = a->cell[0];
    if (sub->type == CELL_INTEGER ||
        sub->type == CELL_BIGINT ||
        sub->type == CELL_REAL ||
        sub->type == CELL_RATIONAL) {
        return make_cell_integer(0);
        }
    if (sub->type == CELL_COMPLEX) {
        return sub->imag;
    }
    /* If we didn't return early, we have the wrong arg type. */
    return check_arg_types(make_sexpr_len1(sub),
        CELL_COMPLEX|CELL_REAL|CELL_RATIONAL|CELL_INTEGER|CELL_BIGINT,
        "imag-part");
}


/* (make-rectangular x1 x2 )
 * Convert a complex number to rectangular form. */
static Cell* math_make_rectangular(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "make-rectangular");
    if (err) return err;
    err = check_arg_types(a, CELL_REAL|CELL_RATIONAL|CELL_INTEGER, "make-rectangular");
    if (err) return err;

    return make_cell_complex(a->cell[0], a->cell[1]);
}


/* (angle z)
 * Calculate angle 'θ' of complex number. */
static Cell* math_angle(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "angle");
    if (err) return err;
    err = check_arg_types(a, CELL_REAL|CELL_RATIONAL|CELL_INTEGER|CELL_COMPLEX, "angle");
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


/* (make-polar x3 x4)
 * Convert a complex number to polar form. */
static Cell* math_make_polar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "make-polar");
    if (err) return err;
    err = check_arg_types(a, CELL_REAL|CELL_RATIONAL|CELL_INTEGER, "make-polar");
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


/* Register the procedures in the global environment. */
void cozenage_library_init(const Lex* e)
{
    lex_add_builtin(e, "cos", math_cos);
    lex_add_builtin(e, "acos", math_acos);
    lex_add_builtin(e, "sin", math_sin);
    lex_add_builtin(e, "asin", math_asin);
    lex_add_builtin(e, "tan", math_tan);
    lex_add_builtin(e, "atan", math_atan);
    lex_add_builtin(e, "exp", math_exp);
    lex_add_builtin(e, "log", math_log);
    lex_add_builtin(e, "log2", math_log2);
    lex_add_builtin(e, "log10", math_log10);
    lex_add_builtin(e, "cbrt", math_cbrt);
    lex_add_builtin(e, "truncate/", math_truncate_div);
    lex_add_builtin(e, "truncate-quotient", builtin_quotient);
    lex_add_builtin(e, "truncate-remainder", builtin_remainder);
    lex_add_builtin(e, "floor/", math_floor_div);
    lex_add_builtin(e, "floor-quotient", math_floor_quotient);
    lex_add_builtin(e, "floor-remainder", builtin_modulo);
    lex_add_builtin(e, "real-part", math_real_part);
    lex_add_builtin(e, "imag-part", math_imag_part);
    lex_add_builtin(e, "make-rectangular", math_make_rectangular);
    /* 'magnitude' is identical to 'abs' for real/complex numbers -
     * so we just make an alias. */
    lex_add_builtin(e,"magnitude", builtin_abs);
    lex_add_builtin(e,"angle", math_angle);
    lex_add_builtin(e,"make-polar", math_make_polar);
}
