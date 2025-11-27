/*
 * 'src/numerics.c'
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

#include "numerics.h"
#include "types.h"
#include "environment.h"
#include "comparators.h"
#include "bignum.h"

#include <math.h>
#include <complex.h>
#include <stdlib.h>
#include <float.h>


/*------------------------------------*
 *     Basic arithmetic operators     *
 * -----------------------------------*/

/* '+' -> CELL_INTEGER|CELL_REAL|CELL_RATIONAL|VAL_COMP - returns the sum of its arguments */
Cell* builtin_add(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a,
        CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_BIGINT|CELL_BIGFLOAT, "+");
    if (err) { return err; }
    /* identity law logic */
    if (a->count == 0) return make_cell_integer(0);
    if (a->count == 1) return a->cell[0];

    /* deep-copy required as result gets mutated */
    Cell* result = cell_copy(a->cell[0]);

    for (register int i = 1; i < a->count; i++) {
        Cell* rhs = a->cell[i];
        numeric_promote(&result, &rhs);

        switch (result->type) {
            case CELL_INTEGER: {
                int64_t out;
                if (add_will_overflow_i64(result->integer_v, rhs->integer_v, &out)) {
                    result = bigint_add(make_cell_bigint(nullptr, result, 10),
                        make_cell_bigint(nullptr, rhs, 10));
                } else {
                    result->integer_v = out;
                }
                break;
            }
            case CELL_RATIONAL:
                /* (a/b) + (c/d) = (ad + bc)/bd */
                result->num = result->num * rhs->den + rhs->num * result->den;
                result->den = result->den * rhs->den;
                result = simplify_rational(result);
                break;
            case CELL_REAL:
                result->real_v += rhs->real_v;
                break;
            case CELL_COMPLEX:
                complex_apply(builtin_add, e, result, rhs);
                break;
            case CELL_BIGINT:
                result = bigint_add(result, rhs);
                break;
            default:
                ;
        }
        result->exact = result->exact && rhs->exact;
    }
    return result;
}

/* '-' -> CELL_INTEGER|CELL_REAL|CELL_RATIONAL|VAL_COMP - returns the difference of its arguments */
Cell* builtin_sub(const Lex* e, const Cell* a)
{
    Cell* err = check_arg_types(a,
        CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_BIGINT|CELL_BIGFLOAT, "-");
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }

    /* Handle unary minus */
    if (a->count == 1) {
        return negate_numeric(cell_copy(a->cell[0]));
    }
    /* multiple args */
    /* deep-copy required as result gets mutated */
    Cell* result = cell_copy(a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        Cell* rhs = a->cell[i];
        numeric_promote(&result, &rhs);

        switch (result->type) {
            case CELL_INTEGER: {
                int64_t out;
                if (sub_will_overflow_i64(result->integer_v, rhs->integer_v, &out)) {
                    result = bigint_sub(make_cell_bigint(nullptr, result, 10),
                        make_cell_bigint(nullptr, rhs, 10));
                } else {
                    result->integer_v = out;
                }
                break;
            }
            case CELL_RATIONAL:
                /* (a/b) - (c/d) = (ad - bc)/bd */
                /* 1/1 - 1/2 = ((1)(2) - (1)(1))/(1)(2) = (2 - 1)/2 = 1/2 */
                result->num = result->num * rhs->den - rhs->num * result->den;
                result->den = result->den * rhs->den;
                result = simplify_rational(result);
                break;
            case CELL_REAL:
                result->real_v -= rhs->real_v;
                break;
            case CELL_COMPLEX:
                complex_apply(builtin_sub, e, result, rhs);
                break;
            case CELL_BIGINT:
                result = bigint_sub(result, rhs);
                /* Check if we can demote back to int64_t */
                if (mpz_fits_int64(*result->bi)) {
                    const int64_t v = mpz_get_i64_checked(*result->bi);
                    result = make_cell_integer(v);
                }
            default:
                ;
        }
        result->exact = result->exact && rhs->exact;
    }
    return result;
}

/* '*' -> CELL_INTEGER|VAL_FLOAT|CELL_RATIONAL|VAL_COMP - returns the product of its arguments */
Cell* builtin_mul(const Lex* e, const Cell* a)
{
    Cell* err = check_arg_types(a,
        CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_BIGINT, "*");
    if (err) { return err; }
    /* identity law logic */
    if (a->count == 0) return make_cell_integer(1);
    if (a->count == 1) return a->cell[0];

    /* deep-copy required as result gets mutated */
    Cell* result = cell_copy(a->cell[0]);

    for (register int i = 1; i < a->count; i++) {
        Cell* rhs = a->cell[i];
        numeric_promote(&result, &rhs);

        switch (result->type) {
            case CELL_INTEGER: {
                int64_t out;
                if (mul_will_overflow_i64(result->integer_v, rhs->integer_v, &out)) {
                    result = bigint_mul(make_cell_bigint(nullptr, result, 10),
                        make_cell_bigint(nullptr, rhs, 10));
                } else {
                    result->integer_v = out;
                }
                break;
            }
            case CELL_RATIONAL:
                /* (a/b) * (c/d) = (a * c)/(b * d) */
                result->num = result->num * rhs->num;
                result->den = result->den * rhs->den;
                result = simplify_rational(result);
                break;
            case CELL_REAL:
                result->real_v *= rhs->real_v;
                break;
            case CELL_COMPLEX:
                complex_apply(builtin_mul, e, result, rhs);
                break;
            case CELL_BIGINT:
                result = bigint_mul(result, rhs);
                break;
            default:
                ;
        }
        result->exact = result->exact && rhs->exact;
    }
    return result;
}

/* '/' -> CELL_INTEGER|CELL_REAL|CELL_RATIONAL|VAL_COMP - returns the quotient of its arguments */
Cell* builtin_div(const Lex* e, const Cell* a)
{
    Cell* err = check_arg_types(a,
        CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_BIGINT, "/");
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }

    /* unary division reciprocal */
    if (a->count == 1) {
        if (a->cell[0]->type == CELL_INTEGER) {
            return make_cell_rational(1, a->cell[0]->integer_v, 1);
        }
        if (a->cell[0]->type == CELL_RATIONAL) {
            const long int n = a->cell[0]->num;
            const long int d = a->cell[0]->den;
            return make_cell_rational(d, n, 1);
        }
        if (a->cell[0]->type == CELL_REAL) {
            return make_cell_real(1.0L / a->cell[0]->real_v);
        }
        if (a->cell[0]->type == CELL_COMPLEX) {
            const Cell* z = a->cell[0];
            const Cell* a_part = z->real;
            const Cell* b_part = z->imag;

            /* Calculate the denominator: a^2 + b^2 */
            const Cell* a_sq_args = make_sexpr_len2(a_part, a_part);
            const Cell* a_sq = builtin_mul(e, a_sq_args);

            const Cell* b_sq_args = make_sexpr_len2(b_part, b_part);
            const Cell* b_sq = builtin_mul(e, b_sq_args);

            const Cell* denom_args = make_sexpr_len2(a_sq, b_sq);
            const Cell* denom = builtin_add(e, denom_args);

            /* Calculate the new real part: a / (a^2 + b^2) */
            const Cell* new_real_args = make_sexpr_len2(a_part, denom);
            Cell* new_real = builtin_div(e, new_real_args);

            /* Calculate the new imaginary part: -b / (a^2 + b^2) */
            const Cell* zero = make_cell_integer(0);
            const Cell* neg_b_args = make_sexpr_len2(zero, b_part);
            const Cell* neg_b = builtin_sub(e, neg_b_args);

            const Cell* new_imag_args = make_sexpr_len2(neg_b, denom);
            Cell* new_imag = builtin_div(e, new_imag_args);

            /* Create the final result */
            Cell* result = make_cell_complex(new_real, new_imag);
            return result;
        }
    }
    /* multiple args */
    /* deep-copy required as result gets mutated */
    Cell* result = cell_copy(a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        Cell* rhs = a->cell[i];
        numeric_promote(&result, &rhs);

        switch (result->type) {
        case CELL_INTEGER:
            if (rhs->integer_v == 0) {
                return make_cell_error(
                    "Division by zero.",
                    VALUE_ERR);
            }
            /* pretty hacky way to get (/ 9 3) -> 3 but (/ 10 3) -> 10/3 */
            const double r = remainder((double)result->integer_v, (double)rhs->integer_v);
            if (r == 0 || r == 0.0) {
                result->integer_v /= rhs->integer_v;
            } else {
                Cell* new_rat = make_cell_rational(result->integer_v, rhs->integer_v, 1);
                result = new_rat;
            }
            break;
        case CELL_RATIONAL:
            /* (a/b) / (c/d) = (a * d)/(b * c)   */
            result->num = result->num * rhs->den;
            result->den = result->den * rhs->num;
            result = simplify_rational(result);
            break;
        case CELL_REAL:
            if (rhs->real_v == 0) {
                return make_cell_error(
                    "Division by zero.",
                    VALUE_ERR);
            }
            result->real_v /= rhs->real_v;
            break;
        case CELL_COMPLEX:
            complex_apply(builtin_div, e, result, rhs);
            break;
        case CELL_BIGINT:
            result = bigint_div(result, rhs);
            /* Check if we can demote back to int64_t */
            if (mpz_fits_int64(*result->bi)) {
                const int64_t v = mpz_get_i64_checked(*result->bi);
                result = make_cell_integer(v);
            }
            break;
        default:
            ;
        }
        result->exact = result->exact && rhs->exact;
    }
    return result;
}

/* -----------------------------------*
 *     Generic numeric operations     *
 * -----------------------------------*/

/* 'abs' -> CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX-
 * returns the absolute value (magnitude) of its argument. */
Cell* builtin_abs(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a,
        CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_BIGINT|CELL_BIGFLOAT, "abs");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const Cell* arg = a->cell[0];

    if (arg->type == CELL_BIGINT) {
        /* Check signedness - if negative, return
         * a negated copy of the value, else, just return it directly */
        const int sign = mpz_sgn(*arg->bi);
        if (sign == -1) {
            return bigint_neg(cell_copy(arg));
        }
        return (Cell*)arg;
    }

    /* Handle all real numbers */
    if (cell_is_real(arg)) {
        /* If the number is a complex type, we operate on its real part.
         * Otherwise, we operate on the number itself. */
        const Cell* real_v = arg->type == CELL_COMPLEX? arg->real : arg;

        if (cell_is_negative(real_v)) {
            return negate_numeric(real_v);
        }
        /* The number is non-negative, so just return it. */
        return (Cell*)real_v;
    }
    /* Handle non-real complex numbers
     * At this point, we know 'arg' is a CELL_COMPLEX.
     * Convert real and imaginary parts to long doubles. */
    const long double x = cell_to_long_double(arg->real);
    const long double y = cell_to_long_double(arg->imag);

    /* Calculate the magnitude: sqrt(x² + y²) */
    const long double magnitude = sqrtl(x * x + y * y);

    /* The result of a magnitude calculation is always an inexact real number. */
    return make_cell_real(magnitude);
}

/* Helper to bridge expt with complex_apply */
static Cell* expt_complex_op(const BuiltinFn op, const Lex* e, const Cell* z1, const Cell* z2)
{
    /* Copy z1 to serve as the 'result' parameter that complex_apply will modify. */
    Cell* result = cell_copy(z1);

    /* Promote the types to be compatible, and pass a pointer to a
     * temporary Cell* for rhs so numeric_promote can modify it. */
    Cell* rhs_copy = cell_copy(z2);
    numeric_promote(&result, &rhs_copy);
    complex_apply(op, e, result, rhs_copy);
    return result;
}

/* 'expt' -> CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX- returns its first arg calculated
 * to the power of its second arg */
Cell* builtin_expt(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX, "expt");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }

    const Cell* base = a->cell[0];
    const Cell* exp = a->cell[1];

    /* Handle simple edge cases */
    if (cell_is_real_zero(exp)) { return make_cell_integer(1); }
    if (cell_is_real_zero(base)) { return make_cell_integer(0); }

    /* Base is a non-negative real number */
    if (cell_is_real(base) && !cell_is_negative(base)) {
        const long double b = cell_to_long_double(base);
        const long double x = cell_to_long_double(exp);
        return make_cell_from_double(powl(b, x));
    }

    /* Base is a negative real number */
    if (cell_is_real(base)) {
        if (cell_is_integer(exp)) {
            const long double b = cell_to_long_double(base);
            const long double x = cell_to_long_double(exp);
            return make_cell_from_double(powl(b, x));
        }
        const long double x = cell_to_long_double(base);
        const long double y = cell_to_long_double(exp);
        const long double magnitude = powl(fabsl(x), y);
        const long double angle = y * M_PI;

        Cell* real_part = make_cell_from_double(magnitude * cosl(angle));
        Cell* imag_part = make_cell_from_double(magnitude * sinl(angle));
        return make_cell_complex(real_part, imag_part);
    }

    /* Base is a complex number */
    if (base->type == CELL_COMPLEX) {
        if (cell_is_integer(exp)) {
            const long long n = (long long)cell_to_long_double(exp);
            Cell* result = make_cell_integer(1); // Multiplicative identity
            const Cell* current_power = cell_copy(base);

            long long abs_n = n > 0 ? n : -n;
            while (abs_n > 0) {
                if (abs_n & 1) { /* If exponent is odd */
                    result = expt_complex_op(builtin_mul, e, result, current_power);
                }
                current_power = expt_complex_op(builtin_mul, e, current_power, current_power);
                abs_n >>= 1; /* Halve the exponent */
            }

            /* Handle negative exponent: z^-n = 1 / z^n */
            if (n < 0) {
                const Cell* one = make_cell_integer(1);
                result = expt_complex_op(builtin_div, e, one, result);
            }
            return result;
        }
        return make_cell_error(
            "expt: complex base with non-integer exponent not implemented",
            GEN_ERR);
    }
    return make_cell_error(
        "expt: unreachable code",
        GEN_ERR);
}

/* 'modulo' -> CELL_INTEGER - returns the remainder of dividing the first argument
 * by the second, with the result having the same sign as the divisor.*/
Cell* builtin_modulo(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER, "modulo");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    long long r = a->cell[0]->integer_v % a->cell[1]->integer_v;
    if (r != 0 && ((a->cell[1]->integer_v > 0 && r < 0) || (a->cell[1]->integer_v < 0 && r > 0))) {
        r += a->cell[1]->integer_v;
    }
    return make_cell_integer(r);
}

/* 'quotient' -> CELL_INTEGER - returns the integer result of dividing
 * the first argument by the second, discarding any remainder.*/
Cell* builtin_quotient(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER, "quotient");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    return make_cell_integer(a->cell[0]->integer_v / a->cell[1]->integer_v);
}

/* 'remainder' -> CELL_INTEGER - returns the remainder of dividing the first argument
 * by the second, with the result having the same sign as the dividend.*/
Cell* builtin_remainder(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER, "remainder");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    return make_cell_integer(a->cell[0]->integer_v % a->cell[1]->integer_v);
}


/* 'max' -> CELL_INTEGER|CELL_RATIONAL|CELL_REAL - return the largest value in numeric args */
Cell* builtin_max(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "max");
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }

    /* Validate that all arguments are real numbers. */
    for (int i = 0; i < a->count; i++) {
        if (!cell_is_real(a->cell[i])) {
            return make_cell_error(
                "max: all arguments must be real numbers",
                TYPE_ERR);
        }
    }

    const Cell* largest_so_far = a->cell[0];
    for (int i = 1; i < a->count; i++) {
        const Cell* rhs = a->cell[i];

        const Cell* arg_list = make_sexpr_len2(largest_so_far, rhs);
        const Cell* result = builtin_lt_op(e, arg_list);

        if (result->boolean_v == 1) { /* if (largest_so_far < rhs) */
            largest_so_far = rhs;
        }
    }
    return cell_copy(largest_so_far);
}

/* 'min' -> CELL_INTEGER|CELL_RATIONAL|CELL_REAL - return the smallest value in numeric args */
Cell* builtin_min(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "min");
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }

    for (int i = 0; i < a->count; i++) {
        if (!cell_is_real(a->cell[i])) {
            return make_cell_error(
                "min: all arguments must be real numbers",
                TYPE_ERR);
        }
    }

    Cell* smallest_so_far = a->cell[0];
    for (int i = 1; i < a->count; i++) {
        Cell* rhs = a->cell[i];

        const Cell* arg_list = make_sexpr_len2(smallest_so_far, rhs);
        const Cell* result = builtin_gt_op(e, arg_list); /* Using > for min */

        if (result->boolean_v == 1) { /* if (smallest_so_far > rhs) */
            smallest_so_far = rhs;
        }
    }
    return smallest_so_far;
}

Cell* builtin_floor(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL, "floor");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    long double val = cell_to_long_double(a->cell[0]);
    val = floorl(val);

    return make_cell_from_double(val);
}

Cell* builtin_ceiling(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL, "ceiling");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    long double val = cell_to_long_double(a->cell[0]);
    val = ceill(val);

    return make_cell_from_double(val);
}

Cell* builtin_round(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL, "round");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    long double val = cell_to_long_double(a->cell[0]);
    val = roundl(val);

    return make_cell_from_double(val);
}

Cell* builtin_truncate(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL, "truncate");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    long double val = cell_to_long_double(a->cell[0]);
    val = truncl(val);

    return make_cell_from_double(val);
}

Cell* builtin_numerator(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL, "numerator");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    /* Return ints unchanged. */
    if (a->cell[0]->type == CELL_INTEGER) {
        return a->cell[0];
    }
    return make_cell_integer(a->cell[0]->num);
}

Cell* builtin_denominator(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL, "denominator");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    /* Denominator of int is always 1. */
    if (a->cell[0]->type == CELL_INTEGER) {
        return make_cell_integer(1);
    }
    return make_cell_integer(a->cell[0]->den);
}

/* (rationalize x y)
 * The rationalize procedure returns the simplest rational number differing from x by no more than y. A rational
 * number r1 is simpler than another rational number r2 if r1 = p1/q1 and r2 = p2/q2 (in lowest terms) and
 * |p1| ≤ |p2|and |q1| ≤ |q2|. Thus, 3/5 is simpler than 4/7. Although not all rationals are comparable in this ordering
 * (consider 2/7 and 3/5), any interval contains a rational number that is simpler than every other rational number in
 * that interval(the simpler 2/5 lies between 2/7 and 3/5). Note that 0 = 0/1 is the simplest rational of all. */
Cell* builtin_rationalize(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL, "rationalize");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }

    /* A practical limit based on ~16-17 digits of precision. */
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const long long MAX_DENOMINATOR = 10000000000000000LL; /* 10^16 */

    const long double x_ld = cell_to_long_double(a->cell[0]);
    const long double y_ld = cell_to_long_double(a->cell[1]);

    const long double lower = x_ld - y_ld;
    const long double upper = x_ld + y_ld;

    if (lower <= 0.0 && upper >= 0.0) {
        /* Return the rational 0/1. */
        return make_cell_rational(0, 1, false);
    }

    for (long long q = 1; q < MAX_DENOMINATOR; ++q) {
        // We need to find an integer p in the range [q * lower, q * upper]
        long double p_lower_bound = q * lower;
        long double p_upper_bound = q * upper;

        /* Find the smallest integer p that is >= p_lower_bound.
         * That's just ceil(p_lower_bound). */
        long long p = (long long)ceill(p_lower_bound);

        /* Now, check if this p is within the upper bound. */
        if (p <= p_upper_bound) {
            /* SUCCESS! We found the simplest p/q. */
            return make_cell_rational(p, q, true);
        }
    }

    /* If we cannot converge on a simpler rational,
     * return the original x as a rat. */
    int exponent;
    /* frexpl breaks x into a mantissa in [0.5, 1.0) and an exponent. */
    const long double mantissa = frexpl(x_ld, &exponent);

    /* We need to turn the mantissa into an integer.
     * LDBL_MANT_DIG is the number of bits in the mantissa (e.g., 64). */
    const long long mantissa_as_int = (long long)ldexpl(mantissa, LDBL_MANT_DIG);
    const long long denominator = (long long)1ULL << (LDBL_MANT_DIG - exponent);

    return make_cell_rational(mantissa_as_int, denominator, true);
}

Cell* builtin_square(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "square");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const Cell* args = make_sexpr_len2(a->cell[0], a->cell[0]);
    return builtin_mul(e, args);
}

/* Returns the square root of arg */
Cell* builtin_sqrt(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "sqrt");
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

/* Implements exact-integer-sqrt for unsigned 64-bit integers.
 * Returns s, the integer square root. */
static unsigned long long integer_sqrt(const unsigned long long k)
{
    if (k == 0) return 0;

    /* Initial guess is a good, but low, approximation. */
    unsigned long long s = 1ULL << ((63 - __builtin_clzll(k)) / 2);

    /* Perform one iteration of Newton's method unconditionally.
     * This jumps from the initial underestimate to an overestimate,
     * which allows the main loop to converge correctly. */
    s = (s + k / s) >> 1;

    while (1) {
        const unsigned long long s_next = (s + k / s) >> 1;

        /* The sequence now decreases until it converges.
         * The loop terminates when the next guess is no longer smaller. */
        if (s_next >= s) {
            return s; /* We've found the solution. */
        }
        s = s_next;
    }
}

/* (exact-integer-sqrt k)
 * Returns two values s and r such that k = s² + r. */
Cell* builtin_exact_integer_sqrt(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER, "exact-integer-sqrt");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long long k = a->cell[0]->integer_v;
    if (k < 1) {
        make_cell_error(
            "exact-integer-sqrt: arg1 must be an exact positive integer",
            VALUE_ERR);
    }

    const unsigned long long s = integer_sqrt(k);
    const unsigned long long r = k - s * s;

    Cell* result = make_cell_mrv();
    cell_add(result, make_cell_integer((long long)s));
    cell_add(result, make_cell_integer((long long)r));
    return result;
}

Cell* builtin_exact(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "exact");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    /* Just return if it's already exact */
    if (a->cell[0]->exact) {
        return a->cell[0];
    }
    if (a->cell[0]->type == CELL_COMPLEX) {
        a->cell[0]->real->exact = 1;
        a->cell[0]->imag->exact = 1;
    }
    a->cell[0]->exact = 1;
    return a->cell[0];
}

Cell* builtin_inexact(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "inexact");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    /* Just return if it's already inexact. */
    if (a->cell[0]->exact == 0) {
        return a->cell[0];
    }
    if (a->cell[0]->type == CELL_COMPLEX) {
        a->cell[0]->real->exact = 0;
        a->cell[0]->imag->exact = 0;
    }
    a->cell[0]->exact = 0;
    return a->cell[0];
}

/* Predicate to test if val is infinite */
Cell* builtin_infinite(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "infinite?");
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
Cell* builtin_finite(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX, "finite?");
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
Cell* builtin_nan(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX,"nan?");
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
