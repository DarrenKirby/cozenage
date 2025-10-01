/*
 * 'src/ops.c'
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

#include "ops.h"
#include "types.h"
#include "environment.h"
#include "eval.h"
#include "comparators.h"
#include "load_library.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unicode/utf8.h>
#include <unicode/ustring.h>
#include <gc/gc.h>
#include "printer.h"


/* Note: cell_to_long_double() and make_cell_from_long_double() moved to types.c */

/* Helper to check if a non-complex numeric cell has a value of zero. */
static bool cell_is_real_zero(const Cell* c) {
    if (!c) return false;
    switch (c->type) {
        case VAL_INT:
            return c->i_val == 0;
        case VAL_RAT:
            /* Assumes simplified rational where numerator would be 0. */
            return c->num == 0;
        case VAL_REAL:
            return c->r_val == 0.0L;
        default:
            return false;
    }
}

/* Helper to check if a cell represents an integer value, per R7RS tower. */
static bool cell_is_integer(const Cell* c) {
    if (!c) return false;
    switch (c->type) {
        case VAL_INT:
            return true;
        case VAL_RAT:
            /* A simplified rational is an integer if its denominator is 1. */
            return c->den == 1;
        case VAL_REAL:
            /* A real is an integer if it has no fractional part. */
            return c->r_val == floorl(c->r_val);
        case VAL_COMPLEX:
            /* A complex is an integer if its imaginary part is zero
             * and its real part is an integer. */
            return cell_is_real_zero(c->imag) && cell_is_integer(c->real);
        default:
            return false;
    }
}

/* Checks if a number is real-valued (i.e., has a zero imaginary part). */
static bool cell_is_real(const Cell* c) {
    if (!c) return false;
    switch (c->type) {
        case VAL_INT:
        case VAL_RAT:
        case VAL_REAL:
            return true;
        case VAL_COMPLEX:
            /* A complex number is real if its imaginary part is zero. */
            return cell_is_real_zero(c->imag);
        default:
            return false;
    }
}

/* Helper for positive? (> 0)
 * Note: R7RS 'positive?' is strictly greater than 0. */
static bool cell_is_positive(const Cell* c) {
    if (!c) return false;

    const Cell* val_to_check = c;
    if (c->type == VAL_COMPLEX) {
        /* Must be a real number to be positive */
        if (!cell_is_real_zero(c->imag)) return false;
        val_to_check = c->real;
    }

    switch (val_to_check->type) {
        case VAL_INT:  return val_to_check->i_val > 0;
        case VAL_REAL: return val_to_check->r_val > 0.0L;
        case VAL_RAT:  return val_to_check->num > 0; /* Assumes den is always positive */
        default:       return false;
    }
}

/* Helper for negative? (< 0) */
static bool cell_is_negative(const Cell* c) {
    if (!c) return false;

    const Cell* val_to_check = c;
    if (c->type == VAL_COMPLEX) {
        /* Must be a real number to be negative */
        if (!cell_is_real_zero(c->imag)) return false;
        val_to_check = c->real;
    }

    switch (val_to_check->type) {
        case VAL_INT:  return val_to_check->i_val < 0;
        case VAL_REAL: return val_to_check->r_val < 0.0L;
        case VAL_RAT:  return val_to_check->num < 0; /* Assumes den is always positive */
        default:       return false;
    }
}

/* Helper for odd? */
static bool cell_is_odd(const Cell* c) {
    /* Must be an integer to be odd or even. */
    if (!cell_is_integer(c)) {
        return false;
    }

    const Cell* int_cell = (c->type == VAL_COMPLEX) ? c->real : c;

    long long val;
    switch (int_cell->type) {
        case VAL_INT:  val = int_cell->i_val; break;
        case VAL_REAL: val = (long long)int_cell->r_val; break;
        case VAL_RAT:  val = int_cell->num; break; /* den is 1 if it's an integer */
        default: return false; /* Unreachable */
    }
    return (val % 2 != 0);
}

/* Helper for even? */
static bool cell_is_even(const Cell* c) {
    /* Must be an integer to be odd or even. */
    if (!cell_is_integer(c)) {
        return false;
    }

    const Cell* int_cell = (c->type == VAL_COMPLEX) ? c->real : c;

    long long val;
    switch (int_cell->type) {
        case VAL_INT:  val = int_cell->i_val; break;
        case VAL_REAL: val = (long long)int_cell->r_val; break;
        case VAL_RAT:  val = int_cell->num; break; /* den is 1 if it's an integer */
        default: return false; /* Unreachable */
    }
    return (val % 2 == 0);
}

/*------------------------------------*
 *     Basic arithmetic operators     *
 * -----------------------------------*/

/* '+' -> VAL_INT|VAL_REAL|VAL_RAT|VAL_COMP - returns the sum of its arguments */
Cell* builtin_add(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    /* identity law logic */
    if (a->count == 0) return make_val_int(0);
    if (a->count == 1) return cell_copy(a->cell[0]);

    Cell* result = cell_copy(a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        Cell* rhs = cell_copy(a->cell[i]);
        numeric_promote(&result, &rhs);

        switch (result->type) {
            case VAL_INT:
                result->i_val += rhs->i_val;
                break;
            case VAL_RAT:
                /* (a/b) + (c/d) = (ad + bc)/bd */
                result->num = result->num * rhs->den + rhs->num * result->den;
                result->den = result->den * rhs->den;
                result = simplify_rational(result);
                break;
            case VAL_REAL:
                result->r_val += rhs->r_val;
                break;
            case VAL_COMPLEX:
                complex_apply(builtin_add, e, result, rhs);
                break;
            default:
                return make_val_err("<builtin '+'> Oops, this shouldn't have happened.", GEN_ERR);
        }
        result->exact = result->exact && rhs->exact;
    }
    return result;
}

/* '-' -> VAL_INT|VAL_REAL|VAL_RAT|VAL_COMP - returns the difference of its arguments */
Cell* builtin_sub(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }

    /* Handle unary minus */
    if (a->count == 1) {
        return negate_numeric(a->cell[0]);
    }
    /* multiple args */
    Cell* result = cell_copy(a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        Cell* rhs = cell_copy(a->cell[i]);
        numeric_promote(&result, &rhs);

        switch (result->type) {
            case VAL_INT:
                result->i_val -= rhs->i_val;
                break;
            case VAL_RAT:
                /* (a/b) - (c/d) = (ad - bc)/bd */
                /* 1/1 - 1/2 = ((1)(2) - (1)(1))/(1)(2) = (2 - 1)/2 = 1/2 */
                result->num = result->num * rhs->den - rhs->num * result->den;
                result->den = result->den * rhs->den;
                result = simplify_rational(result);
                break;
            case VAL_REAL:
                result->r_val -= rhs->r_val;
                break;
            case VAL_COMPLEX:
                complex_apply(builtin_sub, e, result, rhs);
                break;
            default:
                return make_val_err("<builtin '-'> Oops, this shouldn't have happened.", GEN_ERR);
        }
        result->exact = result->exact && rhs->exact;
    }
    return result;
}

/* '*' -> VAL_INT|VAL_FLOAT|VAL_RAT|VAL_COMP - returns the product of its arguments */
Cell* builtin_mul(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    /* identity law logic */
    if (a->count == 0) return make_val_int(1);
    if (a->count == 1) return cell_copy(a->cell[0]);

    Cell* result = cell_copy(a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        Cell* rhs = cell_copy(a->cell[i]);
        numeric_promote(&result, &rhs);

        switch (result->type) {
            case VAL_INT:
                result->i_val *= rhs->i_val;
                break;
            case VAL_RAT:
                /* (a/b) * (c/d) = (a * c)/(b * d) */
                result->num = result->num * rhs->num;
                result->den = result->den * rhs->den;
                result = simplify_rational(result);
                break;
            case VAL_REAL:
                result->r_val *= rhs->r_val;
                break;
            case VAL_COMPLEX:
                complex_apply(builtin_mul, e, result, rhs);
                break;
            default:
                return make_val_err("<builtin '*'> Oops, this shouldn't have happened.", GEN_ERR);
        }
        result->exact = result->exact && rhs->exact;
    }
    return result;
}

/* '+' -> VAL_INT|VAL_REAL|VAL_RAT|VAL_COMP - returns the quotient of its arguments */
Cell* builtin_div(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }

    /* unary division reciprocal */
    if (a->count == 1) {
        if (a->cell[0]->type == VAL_INT) {
            return make_val_rat(1, a->cell[0]->i_val, 1);
        }
        if (a->cell[0]->type == VAL_RAT) {
            const long int n = a->cell[0]->num;
            const long int d = a->cell[0]->den;
            return make_val_rat(d, n, 1);
        }
        if (a->cell[0]->type == VAL_REAL) {
            return make_val_real(1.0L / a->cell[0]->r_val);
        }
        if (a->cell[0]->type == VAL_COMPLEX) {
            const Cell* z = a->cell[0];
            const Cell* a_part = z->real;
            const Cell* b_part = z->imag;

            /* Calculate the denominator: a^2 + b^2 */
            Cell* a_sq_args = make_sexpr_len2(a_part, a_part);
            const Cell* a_sq = builtin_mul(e, a_sq_args);

            Cell* b_sq_args = make_sexpr_len2(b_part, b_part);
            const Cell* b_sq = builtin_mul(e, b_sq_args);

            Cell* denom_args = make_sexpr_len2(a_sq, b_sq);
            const Cell* denom = builtin_add(e, denom_args);

            /* Calculate the new real part: a / (a^2 + b^2) */
            Cell* new_real_args = make_sexpr_len2(a_part, denom);
            Cell* new_real = builtin_div(e, new_real_args);

            /* Calculate the new imaginary part: -b / (a^2 + b^2) */
            const Cell* zero = make_val_int(0);
            Cell* neg_b_args = make_sexpr_len2(zero, b_part);
            const Cell* neg_b = builtin_sub(e, neg_b_args);

            Cell* new_imag_args = make_sexpr_len2(neg_b, denom);
            Cell* new_imag = builtin_div(e, new_imag_args);

            /* Create the final result */
            Cell* result = make_val_complex(new_real, new_imag);
            return result;
        }
    }
    /* multiple args */
    Cell* result = cell_copy(a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        Cell* rhs = cell_copy(a->cell[i]);
        numeric_promote(&result, &rhs);

        switch (result->type) {
        case VAL_INT:
            if (rhs->i_val == 0) {
                return make_val_err("Division by zero.", GEN_ERR);
            }
            /* pretty hacky way to get (/ 9 3) -> 3 but (/ 10 3) -> 10/3 */
            const double r = remainder((double)result->i_val, (double)rhs->i_val);
            if (r == 0 || r == 0.0) {
                result->i_val /= rhs->i_val;
            } else {
                Cell* new_rat = make_val_rat(result->i_val, rhs->i_val, 1);
                result = new_rat;
            }
            break;
        case VAL_RAT:
            /* (a/b) / (c/d) = (a * d)/(b * c)   */
            result->num = result->num * rhs->den;
            result->den = result->den * rhs->num;
            result = simplify_rational(result);
            break;
        case VAL_REAL:
            if (rhs->r_val == 0) {
                return make_val_err("Division by zero.", GEN_ERR);
            }
            result->r_val /= rhs->r_val;
            break;
        case VAL_COMPLEX:
            complex_apply(builtin_div, e, result, rhs);
            break;
        default:
            return make_val_err("<builtin '/'> Oops, this shouldn't have happened.", GEN_ERR);
        }
        result->exact = result->exact && rhs->exact;
    }
    return result;
}


/* ---------------------------------------*
 *    Generic unary numeric procedures    *
 * ---------------------------------------*/

/* 'zero?' -> VAL_BOOL - returns #t if arg is == 0 else #f */
Cell* builtin_zero(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const Cell* arg = a->cell[0];
    bool is_zero;

    if (arg->type == VAL_COMPLEX) {
        is_zero = cell_is_real_zero(arg->real) && cell_is_real_zero(arg->imag);
    } else {
        is_zero = cell_is_real_zero(arg);
    }
    return make_val_bool(is_zero);
}

/* 'positive?' -> VAL_BOOL - returns #t if arg is > 0 else #f */
Cell* builtin_positive(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    return make_val_bool(cell_is_positive(a->cell[0]));
}

/* 'negative?' -> VAL_BOOL - returns #t if arg is < 0 else #f */
Cell* builtin_negative(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    return make_val_bool(cell_is_negative(a->cell[0]));
}

/* 'odd?' -> VAL_BOOL - returns #t if arg is an odd integer else #f */
Cell* builtin_odd(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    return make_val_bool(cell_is_odd(a->cell[0]));
}

/* 'even?' -> VAL_BOOL - returns #t if arg is an even integer else #f */
Cell* builtin_even(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    return make_val_bool(cell_is_even(a->cell[0]));
}



/* -----------------------------------*
 *     Generic numeric operations     *
 * -----------------------------------*/

/* 'abs' -> VAL_INT|VAL_REAL|VAL_RAT - returns the absolute value (magnitude) of its argument. */
Cell* builtin_abs(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    Cell* arg = a->cell[0];

    /* Path 1: Handle all real numbers */
    if (cell_is_real(arg)) {
        /* If the number is a complex type, we operate on its real part.
         * Otherwise, we operate on the number itself. */
        Cell* real_val = (arg->type == VAL_COMPLEX) ? arg->real : arg;

        if (cell_is_negative(real_val)) {
            return negate_numeric(real_val);
        }
        /* The number is non-negative, so just return a copy. */
        return cell_copy(real_val);
    }
    /* Path 2: Handle non-real complex numbers
     * At this point, we know 'arg' is a VAL_COMPLEX.
     * Convert real and imaginary parts to long doubles. */
    const long double x = cell_to_long_double(arg->real);
    const long double y = cell_to_long_double(arg->imag);

    /* Calculate the magnitude: sqrt(x² + y²) */
    const long double magnitude = sqrtl(x * x + y * y);

    /* The result of a magnitude calculation is always an inexact real number. */
    return make_val_real(magnitude);
}

/* Helper to bridge expt with complex_apply */
Cell* expt_complex_op(BuiltinFn op, Lex* e, const Cell* z1, const Cell* z2) {
    /* Copy z1 to serve as the 'result' parameter that complex_apply will modify. */
    Cell* result = cell_copy(z1);

    /* Promote the types to be compatible, and pass a pointer to a
     * temporary Cell* for rhs so numeric_promote can modify it. */
    Cell* rhs_copy = cell_copy(z2);
    numeric_promote(&result, &rhs_copy);
    complex_apply(op, e, result, rhs_copy);
    return result;
}

/* 'expt' -> VAL_INT|VAL_REAL - returns its first arg calculated
 * to the power of its second arg */
Cell* builtin_expt(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT | VAL_REAL | VAL_RAT | VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }

    const Cell* base = a->cell[0];
    const Cell* exp = a->cell[1];

    /* Handle simple edge cases */
    if (cell_is_real_zero(exp)) { return make_val_int(1); }
    if (cell_is_real_zero(base)) { return make_val_int(0); }

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
        return make_val_complex(real_part, imag_part);
    }

    /* Base is a complex number */
    if (base->type == VAL_COMPLEX) {
        if (cell_is_integer(exp)) {
            const long long n = (long long)cell_to_long_double(exp);
            Cell* result = make_val_int(1); // Multiplicative identity
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
                const Cell* one = make_val_int(1);
                result = expt_complex_op(builtin_div, e, one, result);
            }
            return result;
        }
        return make_val_err("expt: complex base with non-integer exponent not implemented", GEN_ERR);
    }
    return make_val_err("expt: unreachable code", GEN_ERR);
}

/* 'modulo' -> VAL_INT - returns the remainder of dividing the first argument
 * by the second, with the result having the same sign as the divisor.*/
Cell* builtin_modulo(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    long long r = a->cell[0]->i_val % a->cell[1]->i_val;
    if (r != 0 && ((a->cell[1]->i_val > 0 && r < 0) || (a->cell[1]->i_val < 0 && r > 0))) {
        r += a->cell[1]->i_val;
    }
    return make_val_int(r);
}

/* 'quotient' -> VAL_INT - returns the integer result of dividing
 * the first argument by the second, discarding any remainder.*/
Cell* builtin_quotient(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    return make_val_int(a->cell[0]->i_val / a->cell[1]->i_val);
}

/* 'remainder' -> VAL_INT - returns the remainder of dividing the first argument
 * by the second, with the result having the same sign as the dividend.*/
Cell* builtin_remainder(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    return make_val_int(a->cell[0]->i_val % a->cell[1]->i_val);
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

Cell* builtin_gcd(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) return err;

    if (a->count == 0) {
        return make_val_int(0); /* Identity for GCD */
    }

    long long result = a->cell[0]->i_val;
    for (int i = 1; i < a->count; i++) {
        result = gcd_helper(result, a->cell[i]->i_val);
    }

    return make_val_int(llabs(result)); /* Final result must be non-negative */
}

Cell* builtin_lcm(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }

    if (a->count == 0) {
        return make_val_int(1); /* Identity for LCM */
    }

    long long result = a->cell[0]->i_val;
    for (int i = 1; i < a->count; i++) {
        result = lcm_helper(result, a->cell[i]->i_val);
    }

    return make_val_int(llabs(result)); /* Final result must be non-negative */
}

/* 'max' -> VAL_INT|VAL_RAT|VAL_REAL - return the largest value in numeric args */
Cell* builtin_max(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }

    /* Validate that all arguments are real numbers. */
    for (int i = 0; i < a->count; i++) {
        if (!cell_is_real(a->cell[i])) {
            return make_val_err("max: all arguments must be real numbers", GEN_ERR);
        }
    }

    const Cell* largest_so_far = a->cell[0];
    for (int i = 1; i < a->count; i++) {
        const Cell* rhs = a->cell[i];

        Cell* arg_list = make_sexpr_len2(largest_so_far, rhs);
        const Cell* result = builtin_lt_op(e, arg_list);

        if (result->b_val == 1) { /* if (largest_so_far < rhs) */
            largest_so_far = rhs;
        }
    }
    return cell_copy(largest_so_far);
}

/* 'min' -> VAL_INT|VAL_RAT|VAL_REAL - return the smallest value in numeric args */
Cell* builtin_min(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }

    for (int i = 0; i < a->count; i++) {
        if (!cell_is_real(a->cell[i])) {
            return make_val_err("min: all arguments must be real numbers", GEN_ERR);
        }
    }

    Cell* smallest_so_far = a->cell[0];
    for (int i = 1; i < a->count; i++) {
        Cell* rhs = a->cell[i];

        Cell* arg_list = make_sexpr_len2(smallest_so_far, rhs);
        const Cell* result = builtin_gt_op(e, arg_list); /* Using > for min */

        if (result->b_val == 1) { /* if (smallest_so_far > rhs) */
            smallest_so_far = rhs;
        }
    }
    return smallest_so_far;
}

Cell* builtin_floor(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    long double val = cell_to_long_double(a->cell[0]);
    val = floorl(val);
    //printf("val: %Lg\n", val);

    return make_cell_from_double(val);
}

Cell* builtin_ceiling(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    long double val = cell_to_long_double(a->cell[0]);
    val = ceill(val);

    return make_cell_from_double(val);
}

Cell* builtin_round(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    long double val = cell_to_long_double(a->cell[0]);
    val = roundl(val);

    return make_cell_from_double(val);
}

Cell* builtin_truncate(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    long double val = cell_to_long_double(a->cell[0]);
    val = truncl(val);

    return make_cell_from_double(val);
}

Cell* builtin_numerator(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    /* return ints unchanged */
    if (a->cell[0]->type == VAL_INT) {
        return a->cell[0];
    }
    return make_val_int(a->cell[0]->num);
}

Cell* builtin_denominator(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    /* denominator of int always 1 */
    if (a->cell[0]->type == VAL_INT) {
        return make_val_int(1);
    }
    return make_val_int(a->cell[0]->den);
}

Cell* builtin_square(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    Cell* args = make_sexpr_len2(a->cell[0], a->cell[0]);
    return builtin_mul(e, args);
}

Cell* builtin_exact(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    /* Just return if it's already exact */
    if (a->cell[0]->exact) {
        return a->cell[0];
    }
    if (a->cell[0]->type == VAL_COMPLEX) {
        a->cell[0]->real->exact = 1;
        a->cell[0]->imag->exact = 1;
    }
    a->cell[0]->exact = 1;
    return a->cell[0];
}

Cell* builtin_inexact(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    /* Just return if it's already inexact */
    if (a->cell[0]->exact == 0) {
        return a->cell[0];
    }
    if (a->cell[0]->type == VAL_COMPLEX) {
        a->cell[0]->real->exact = 0;
        a->cell[0]->imag->exact = 0;
    }
    a->cell[0]->exact = 0;
    return a->cell[0];
}
/* -------------------------------------------------*
 *       Type identity predicate procedures         *
 * -------------------------------------------------*/

/* 'number?' -> VAL_BOOL - returns #t if obj is numeric, else #f  */
Cell* builtin_number_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX;
    if (a->cell[0]->type & mask) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

/* 'boolean?' -> VAL_BOOL  - returns #t if obj is either #t or #f
    and returns #f otherwise. */
Cell* builtin_boolean_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_BOOL);
}

/* 'null?' -> VAL_BOOL - return #t if obj is null, else #f */
Cell* builtin_null_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_NIL);
}

/* 'pair?' -> VAL_BOOL - return #t if obj is a pair, else #f */
Cell* builtin_pair_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_PAIR);
}

/* 'procedure?' -> VAL_BOOL - return #t if obj is a procedure, else #f */
Cell* builtin_proc_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_PROC);
}

/* 'symbol?' -> VAL_BOOL - return #t if obj is a symbol, else #f */
Cell* builtin_sym_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_SYM);
}

/* 'string?' -> VAL_BOOL - return #t if obj is a string, else #f */
Cell* builtin_string_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_STR);
}

/* 'char?' -> VAL_BOOL - return #t if obj is a char, else #f */
Cell* builtin_char_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_CHAR);
}

/* 'vector?' -> VAL_BOOL - return #t if obj is a vector, else #f */
Cell* builtin_vector_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_VEC);
}

/* 'bytevector?' -> VAL_BOOL - return #t if obj is a byte vector, else #f */
Cell* builtin_byte_vector_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_BYTEVEC);
}

/* 'port?' -> VAL_BOOL - return #t if obj is a port, else #f */
Cell* builtin_port_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_PORT);
}

/* 'eof-object?' -> VAL_BOOL - return #t if obj is an eof, else #f */
Cell* builtin_eof_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_EOF);
}

/* ---------------------------------------*
 *      Numeric identity procedures       *
 * ---------------------------------------*/

/* 'exact?' -> VAL_BOOL -  */
Cell* builtin_exact_pred(Lex *e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == VAL_COMPLEX) {
        if (a->cell[0]->exact && a->cell[1]->exact) {
            return make_val_bool(1);
        }
        return make_val_bool(0);
    }
    return make_val_bool(a->cell[0]->exact);
}

/* 'inexact?' -> VAL_BOOL -  */
Cell* builtin_inexact_pred(Lex *e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == VAL_COMPLEX) {
        if (a->cell[0]->exact && a->cell[1]->exact) {
            return make_val_bool(0);
        }
        return make_val_bool(1);
    }
     if (a->cell[0]->exact) {
         return make_val_bool(0);
     }
    return make_val_bool(1);
}

/* 'complex?' -> VAL_BOOL - R7RS compliant */
Cell* builtin_complex(Lex *e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    /* All numbers are complex numbers. */
    const int mask = VAL_INT|VAL_RAT|VAL_REAL|VAL_COMPLEX;
    if (a->cell[0]->type & mask) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

/* 'real?' -> VAL_BOOL - R7RS compliant */
Cell* builtin_real(Lex *e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const Cell* arg = a->cell[0];
    switch (arg->type) {
        case VAL_INT:
        case VAL_RAT:
        case VAL_REAL:
            return make_val_bool(1);
        case VAL_COMPLEX:
            /* A complex number is real if its imaginary part is zero. */
            return make_val_bool(cell_is_real_zero(arg->imag));
        default:
            return make_val_bool(0);
    }
}

/* 'rational?' -> VAL_BOOL - R7RS compliant */
Cell* builtin_rational(Lex *e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const Cell* arg = a->cell[0];
    switch (arg->type) {
        case VAL_INT:
        case VAL_RAT:
        case VAL_REAL:
            /* all reals are inherently rational. */
            return make_val_bool(1);
        case VAL_COMPLEX:
            /* A complex number is rational if its imaginary part is zero. */
            return make_val_bool(cell_is_real_zero(arg->imag));
        default:
            return make_val_bool(0);
    }
}

/* 'integer?' -> VAL_BOOL - R7RS compliant */
Cell* builtin_integer(Lex *e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(cell_is_integer(a->cell[0]));
}

/* 'exact-integer?' -> VAL_BOOL - R7RS compliant */
Cell* builtin_exact_integer(Lex *e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const Cell* arg = a->cell[0];
    /* The value must be an integer AND the number must be exact. */
    return make_val_bool(cell_is_integer(arg) && arg->exact);
}

/* ---------------------------------------*
 *     Boolean and logical procedures     *
 * ---------------------------------------*/

/* 'not' -> VAL_BOOL - returns #t if obj is false, and returns #f otherwise */
Cell* builtin_not(Lex* e, Cell* a) {
    (void)e;
    Cell* err;
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const int is_false = (a->cell[0]->type == VAL_BOOL && a->cell[0]->b_val == 0);
    return make_val_bool(is_false);
}

/* 'boolean' -> VAL_BOOL - converts any value to a strict boolean */
Cell* builtin_boolean(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    const int result = (a->cell[0]->type == VAL_BOOL)
                 ? a->cell[0]->b_val
                 : 1; /* everything except #f is true */
    return make_val_bool(result);
}

/* 'and' -> VAL_BOOL|ANY - if any expression evaluates to #f, then #f is
 * returned. Any remaining expressions are not evaluated. If all the expressions
 * evaluate to true values, the values of the last expression are returned.
 * If there are no expressions, then #t is returned.*/
Cell* builtin_and(Lex* e, Cell* a) {
    (void)e;
    if (a->count == 0) {
        return make_val_bool(1);
    }
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type == VAL_BOOL && a->cell[i]->b_val == 0) {
            /* first #f encountered → return a copy of it */
            return cell_copy(a->cell[i]);
        }
    }
    /* all truthy → return copy of last element */
    return cell_copy(a->cell[a->count - 1]);
}

/* 'or' -> VAL_BOOL|ANY - the value of the first expression that evaluates
 * to true is returned. Any remaining expressions are not evaluated. If all
 * expressions evaluate to #f or if there are no expressions, #f is returned */
Cell* builtin_or(Lex* e, Cell* a) {
    (void)e;
    if (a->count == 0) {
        return make_val_bool(0);
    }
    for (int i = 0; i < a->count; i++) {
        if (!(a->cell[i]->type == VAL_BOOL && a->cell[i]->b_val == 0)) {
            /* first truthy value → return a copy */
            return cell_copy(a->cell[i]);
        }
    }
    /* all false → return copy of last element (#f) */
    return cell_copy(a->cell[a->count - 1]);
}

/* ----------------------------------------------------------*
 *     pair/list constructors, selectors, and procedures     *
 * ----------------------------------------------------------*/

/* 'cons' -> VAL_PAIR - returns a pair made from two arguments */
Cell* builtin_cons(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    return make_val_pair(cell_copy(a->cell[0]), cell_copy(a->cell[1]));
}

/* 'car' -> ANY - returns the first member of a pair */
Cell* builtin_car(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_PAIR|VAL_SEXPR);
    if (err) { return err; }

    if (a->cell[0]->type == VAL_PAIR) {
        return cell_copy(a->cell[0]->car);
    }
    /* This is for the case where the argument list was quoted.
     * It needs to be transformed into a list before taking the car */
    Cell* list = builtin_list(e, a->cell[0]);
    Cell* result = cell_copy(list->car);
    return result;
}

/* 'cdr' -> ANY - returns the second member of a pair */
Cell* builtin_cdr(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_PAIR|VAL_SEXPR);
    if (err) { return err; }

    if (a->cell[0]->type == VAL_PAIR) {
        return cell_copy(a->cell[0]->cdr);
    }
    /* This is for the case where the argument list was quoted.
     * It needs to be transformed into a list before taking the cdr */
    const Cell* list = builtin_list(e, a->cell[0]);
    Cell* result = cell_copy(list->cdr);
    return result;
}

/* 'list' -> VAL_PAIR - returns a nil-terminated proper list */
Cell* builtin_list(Lex* e, Cell* a) {
    (void)e;
    /* start with nil */
    Cell* result = make_val_nil();

    const int len = a->count;
    /* build backwards so it comes out in the right order */
    for (int i = len - 1; i >= 0; i--) {
        result = make_val_pair(cell_copy(a->cell[i]), result);
        result->len = len - i;
    }
    return result;
}

/* 'length' -> VAL_INT - returns the member count of a proper list */
Cell* builtin_list_length(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, VAL_PAIR|VAL_NIL);
    if (err) { return err; }

    const Cell* list = a->cell[0];

    if (list->type == VAL_NIL) {
        return make_val_int(0);
    }

    /* If len is not -1, we can trust the cached value. */
    if (list->len != -1) {
        return make_val_int(list->len);
    }

    /* If len is -1, this could be an improper list or a proper list
     * built with `cons`. We need to traverse it to find out. */
    int count = 0;
    const Cell* p = list;
    while (p->type == VAL_PAIR) {
        count++;
        p = p->cdr;
    }
    /* The R7RS standard for `length` requires a proper list.
     * If the list doesn't end in `nil`, it's an error. */
    if (p->type != VAL_NIL) {
        return make_val_err("length: proper list required", GEN_ERR);
    }

    /* It's a proper list. */
    return make_val_int(count);
}

/* 'list-ref' -> ANY - returns the list member at the zero-indexed
 * integer specified in the second arg. First arg is the list to act on*/
Cell* builtin_list_ref(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    if (a->cell[0]->type != VAL_PAIR) {
        return make_val_err("list-ref: arg 1 must be a list", GEN_ERR);
    }
    if (a->cell[1]->type != VAL_INT) {
        return make_val_err("list-ref: arg 2 must be an integer", GEN_ERR);
    }
    int i = (int)a->cell[1]->i_val;
    const int len = a->cell[0]->len;

    if (i >= len && len != -1) {
        return make_val_err("list-ref: arg 2 out of range", GEN_ERR);
    }

    Cell* p = a->cell[0];
    while (i > 0) {
        p = p->cdr;
        i--;
    }
    return cell_copy(p->car);
}

/* 'list-append' -> VAL_PAIR - returns a proper list of all
 * args appended to the result, in order */
Cell* builtin_list_append(Lex* e, Cell* a) {
    (void)e;
    /* Base case: (append) -> '() */
    if (a->count == 0) {
        return make_val_nil();
    }
    /* Base case: (append x) -> x */
    if (a->count == 1) {
        return cell_copy(a->cell[0]);
    }

    /* Validate args and calculate total length of copied lists */
    long long total_copied_len = 0;
    for (int i = 0; i < a->count - 1; i++) {
        const Cell* current_list = a->cell[i];
        if (current_list->type == VAL_NIL) {
            continue; /* This is a proper, empty list. */
        }
        /* All but the last argument must be a list */
        if (current_list->type != VAL_PAIR) {
            return make_val_err("append: argument is not a list", GEN_ERR);
        }

        /* Now, walk the list to ensure it's a *proper* list */
        const Cell* p = current_list;
        while (p->type == VAL_PAIR) {
            p = p->cdr;
        }
        if (p->type != VAL_NIL) {
            return make_val_err("append: middle argument is not a proper list", GEN_ERR);
        }

        /* If we get here, the list is proper. Add its length. */
        total_copied_len += current_list->len;
    }

    /* Determine the final list's properties based on the last argument */
    const Cell* last_arg = a->cell[a->count - 1];
    long long final_total_len = -1; /* Use -1 to signify an improper list. */

    if (last_arg->type == VAL_NIL) {
        final_total_len = total_copied_len;
    } else if (last_arg->type == VAL_PAIR) {
        /* If the last arg has a valid length, the result will be a proper list. */
        if (last_arg->len != -1) {
             final_total_len = total_copied_len + last_arg->len;
        }
    }

    /* Build the new list structure */
    Cell* result_head = make_val_nil();
    Cell* result_tail = NULL;
    long long len_countdown = final_total_len;

    for (int i = 0; i < a->count - 1; i++) {
        const Cell* p = a->cell[i];
        while (p->type == VAL_PAIR) {
            /* Create a new pair with a copy of the element. */
            Cell* new_pair = make_val_pair(cell_copy(p->car), make_val_nil());

            /* Assign the correct length based on our pre-calculated total. */
            if (final_total_len != -1) {
                new_pair->len = (int)len_countdown--;
            } else {
                new_pair->len = -1;
            }

            /* Append the new pair to our result list. */
            if (result_tail == NULL) {
                result_head = result_tail = new_pair;
            } else {
                result_tail->cdr = new_pair;
                result_tail = new_pair;
            }
            p = p->cdr;
        }
    }

    /* Finalize: Link the last argument and return */
    if (result_tail == NULL) {
        /* This happens if all arguments before the last were '(). */
        return cell_copy(last_arg);
    }
    /* Splice the last argument onto the end of our newly created list. */
    result_tail->cdr = (Cell*)last_arg;
    return result_head;
}

/* 'reverse' -> VAL_PAIR - returns a proper list with members of
 * arg reversed */
Cell* builtin_list_reverse(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, VAL_PAIR|VAL_NIL);
    if (err) { return err; }

    const Cell* original_list = a->cell[0];
    if (original_list->type == VAL_NIL) {
        return make_val_nil();
    }

    Cell* reversed_list = make_val_nil();
    const Cell* current = original_list;
    int length = 0;

    while (current->type == VAL_PAIR) {
        reversed_list = make_val_pair(cell_copy(current->car), reversed_list);
        length++;
        current = current->cdr;
    }

    if (current->type != VAL_NIL) {
        return make_val_err("reverse: cannot reverse improper list", GEN_ERR);
    }

    /* Set the length on the final result. */
    if (reversed_list->type != VAL_NIL) {
        reversed_list->len = length;
    }
    return reversed_list;
}


/* 'list-tail' -> VAL_PAIR - returns a proper list of the last nth
 * members of arg */
Cell* builtin_list_tail(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    if (a->cell[0]->type != VAL_PAIR &&
        a->cell[0]->type != VAL_SEXPR &&
        a->cell[0]->type != VAL_NIL) {
        return make_val_err("list-tail: arg 1 must be a list", GEN_ERR);
    }
    if (a->cell[1]->type != VAL_INT) {
        return make_val_err("list-tail: arg 2 must be an integer", GEN_ERR);
    }

    Cell* lst = a->cell[0];
    const long k = a->cell[1]->i_val;

    if (k < 0) {
        return make_val_err("list-tail: arg 2 must be non-negative", GEN_ERR);
    }

    Cell* p = lst;
    for (long i = 0; i < k; i++) {
        if (p->type != VAL_PAIR) {
            return make_val_err("list-tail: arg 2 out of range", GEN_ERR);
        }
        /* Move to the next element in the list */
        p = p->cdr;
    }
    /* After the loop, p is pointing at the k-th cdr of the original list. */
    return p;
}

/*-------------------------------------------------------*
 *     Vector constructors, selectors, and procedures    *
 * ------------------------------------------------------*/

/* 'vector' -> VAL_VECT - returns a vector of all arg objects */
Cell* builtin_vector(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;

    Cell *vec = make_val_vect();
    for (int i = 0; i < a->count; i++) {
        cell_add(vec, cell_copy(a->cell[i]));
    }
    return vec;
}

/* 'vector-length' -> VAL_INT- returns the number of members of arg */
Cell* builtin_vector_length(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, VAL_VEC);
    if (err) return err;

    return make_val_int(a->cell[0]->count);
}

Cell* builtin_vector_ref(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    if (a->cell[0]->type != VAL_VEC) {
        return make_val_err("vector-ref: arg 1 must be a vector", GEN_ERR);
    }
    if (a->cell[1]->type != VAL_INT) {
        return make_val_err("vector-ref: arg 2 must be an integer", GEN_ERR);
    }
    const int i = (int)a->cell[1]->i_val;

    if (i >= a->cell[0]->count) {
        return make_val_err("vector-ref: index out of bounds", GEN_ERR);
    }
    return cell_copy(a->cell[0]->cell[i]);
}

Cell* builtin_make_vector(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2);
    if (err) return err;
    if (a->cell[0]->type != VAL_INT) {
        return make_val_err("make-vector: arg 1 must be an integer", GEN_ERR);
    }
    long long n = a->cell[0]->i_val;
    if (n < 1) {
        return make_val_err("make-vector: arg 1 must be non-negative", GEN_ERR);
    }
    Cell* fill = NULL;
    if (a->count == 2) {
        fill = a->cell[1];
    } else {
        fill = make_val_int(0);
    }
    Cell *vec = make_val_vect();
    for (int i = 0; i < n; i++) {
        cell_add(vec, cell_copy(fill));
    }
    return vec;
}

Cell* builtin_list_to_vector(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_PAIR) {
        return make_val_err("list->vector: arg 1 must be a list", GEN_ERR);
    }
    const int list_len = a->cell[0]->len;
    if (list_len == -1) {
        return make_val_err("list->vector: arg 1 must be a proper list", GEN_ERR);
    }
    const Cell* lst = a->cell[0];
    Cell *vec = make_val_vect();
    for (int i = 0; i < list_len; i++) {
        cell_add(vec, cell_copy(lst->car));
        lst = lst->cdr;
    }
    return vec;
}

Cell* builtin_vector_to_list(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != VAL_VEC) {
        return make_val_err("vector->list: arg 1 must be a vector", GEN_ERR);
    }
    int start = 0;
    int end = a->cell[0]->count;

    if (a->count == 2) {
        if (a->cell[1]->type != VAL_INT) {
            return make_val_err("vector->list: arg 2 must be an integer", GEN_ERR);
        }
        start = (int)a->cell[1]->i_val;
        if (start < 0) {
            return make_val_err("vector->list: arg 2 must be non-negative", GEN_ERR);
        }
    }

    if (a->count == 3) {
        if (a->cell[2]->type != VAL_INT) {
            return make_val_err("vector->list: arg 2 must be an integer", GEN_ERR);
        }
        start = (int)a->cell[1]->i_val;
        end = (int)a->cell[2]->i_val + 1;
        if (end < 0) {
            return make_val_err("vector->list: arg 2 must be non-negative", GEN_ERR);
        }
        if (end > a->cell[0]->count) {
            return make_val_err("vector->list: index out of bounds", GEN_ERR);
        }
    }

    Cell* result = make_val_nil();
    const Cell* vec = a->cell[0];
    for (int i = end - 1; i >= start; i--) {
        result = make_val_pair(cell_copy(vec->cell[i]), result);
        result->len = end - i;
    }
    return result;
}

/* 'vector-copy' -> VAL_VECT - returns a newly allocated copy of arg vector */
Cell* builtin_vector_copy(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != VAL_VEC) {
        return make_val_err("vector->copy: arg 1 must be a vector", GEN_ERR);
    }
    if (a->count == 1) {
        return cell_copy(a->cell[0]);
    }
    const long long start = a->cell[1]->i_val;
    if (a->count == 2) {
        return cell_copy(a->cell[0]->cell[start]);
    }
    const long long end = a->cell[2]->i_val;
    Cell* vec = make_val_vect();
    for (long long i = start; i < end; i++) {
        cell_add(vec, cell_copy(a->cell[0]->cell[i]));
    }
    return vec;
}

/* 'vector->string' -> VAL_STR - returns a str containing all char members
 * of arg */
Cell* builtin_vector_to_string(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != VAL_VEC) {
        return make_val_err("vector->string: arg must be a vector", GEN_ERR);
    }
    int start = 0;
    int end = a->cell[0]->count;
    char* the_string = GC_MALLOC_ATOMIC(end * 4 + 1);
    if (a->count == 2) {
        if (a->cell[1]->type != VAL_INT) {
            return make_val_err("vector->string: arg2 must be an integer", GEN_ERR);
        }
        start = (int)a->cell[1]->i_val;
    }
    if (a->count == 3) {
        if (a->cell[1]->type != VAL_INT) {
            return make_val_err("vector->string: arg3 must be an integer", GEN_ERR);
        }
        end = (int)a->cell[2]->i_val;
    }

    int32_t j = 0;
    for (int i = start; i < end; i++) {
        const Cell* char_cell = a->cell[0]->cell[i];
        if (char_cell->type != VAL_CHAR) {
            return make_val_err("vector->string: vector must have only chars as members", GEN_ERR);
        }
        const UChar32 code_point = char_cell->c_val;
        U8_APPEND_UNSAFE(the_string, j, code_point);
    }
    the_string[j] = '\0';
    return make_val_str(the_string);
}

/* 'string->vector' -> VAL_VECT - returns a vector of all chars in arg */
Cell* builtin_string_to_vector(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != VAL_STR) {
        return make_val_err("string->vector: arg1 must be a string", GEN_ERR);
    }

    const char* the_string = a->cell[0]->str;
    const size_t string_byte_len = strlen(the_string);

    /* Get optional start/end character indices from args */
    int start_char_idx = 0;
    int end_char_idx = -1; /* Use -1 to signify 'to the end' */

    if (a->count >= 2) {
        if (a->cell[1]->type != VAL_INT) {
            return make_val_err("string->vector: arg2 must be an integer", GEN_ERR);
        }
        start_char_idx = (int)a->cell[1]->i_val;
    }
    if (a->count == 3) {
        if (a->cell[2]->type != VAL_INT) {
            return make_val_err("string->vector: arg3 must be an integer", GEN_ERR);
        }
        end_char_idx = (int)a->cell[2]->i_val;
    }

    Cell* vec = make_val_vect();
    int32_t byte_idx = 0;
    int32_t char_idx = 0;
    UChar32 code_point;

    /* Advance to the starting character position. */
    while (char_idx < start_char_idx && byte_idx < (int)string_byte_len) {
        U8_NEXT_UNSAFE(the_string, byte_idx, code_point);
        char_idx++;
    }

    /* Grab characters until we reach the end. */
    while (byte_idx < (int)string_byte_len) {
        /* Stop if we've reached the user-specified end character index */
        if (end_char_idx != -1 && char_idx >= end_char_idx) {
            break;
        }

        U8_NEXT_UNSAFE(the_string, byte_idx, code_point);
        cell_add(vec, make_val_char(code_point));
        char_idx++;
    }
    return vec;
}



/*-------------------------------------------------------*
 *     String constructors, selectors, and procedures    *
 * ------------------------------------------------------*/


/*-------------------------------------------------------*
 *    Control features and list iteration procedures     *
 * ------------------------------------------------------*/

Cell* builtin_apply(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    if (a->cell[0]->type != VAL_PROC) {
        return make_val_err("apply: arg 1 must be a procedure", GEN_ERR);
    }
    if (a->cell[1]->type != VAL_PAIR && a->cell[1]->len == -1) {
        return make_val_err("apply: arg 2 must be a proper list", GEN_ERR);
    }

    const Cell* composition = make_sexpr_len2(a->cell[0], make_sexpr_from_list(a->cell[1]));
    return coz_eval(e, flatten_sexpr(composition));
}

Cell* builtin_eval(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;
    Cell* args = NULL;
    /* Convert list to s-expr if we are handed a quote */
    if (a->cell[0]->type == VAL_PAIR) {
        args = make_sexpr_from_list(a->cell[0]);
        for (int i = 0; i < args->count; i++ ) {
            if (args->cell[i]->type == VAL_PAIR && args->cell[i]->len != -1) {
                Cell* tmp = cell_copy(args->cell[i]);
                args->cell[i] = make_sexpr_from_list(tmp);
            }
        }
    /* Otherwise just send straight to eval */
    } else {
        args = a->cell[0];
    }
    return coz_eval(e, args);
}

Cell* builtin_map(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;
    if (a->cell[0]->type != VAL_PROC) {
        return make_val_err("map: arg 1 must be a procedure", GEN_ERR);
    }
    int shortest_list_length = INT32_MAX;
    for (int i = 1; i < a->count; i++) {
        char buf[100];
        if (a->cell[i]->type != VAL_PAIR && a->cell[i]->len == -1) {
            snprintf(buf, 100, "map: arg %d must be a proper list", i);
            return make_val_err(buf, GEN_ERR);
        }
        if (a->cell[i]->len < shortest_list_length) {
            shortest_list_length = a->cell[i]->len;
        }
    }

    const int shortest_len = shortest_list_length;
    const int num_lists = a->count - 1;
    Cell* proc = a->cell[0];

    Cell* final_result = make_val_nil();

    for (int i = 0; i < shortest_len; i++) {
        /* Build a (reversed) list of the i-th arguments */
        Cell* arg_list = make_val_nil();
        for (int j = 0; j < num_lists; j++) {
            Cell* current_list = a->cell[j + 1];
            Cell* nth_item = list_get_nth_cell_ptr(current_list, i);
            arg_list = make_val_pair(nth_item, arg_list);
            arg_list->len = j + 1;
        }

        /* Correct the argument order */
        Cell* reversed_arg_list = builtin_list_reverse(e, make_sexpr_len1(arg_list));

        Cell* tmp_result = NULL;
        if (proc->builtin) {
            Cell* (*func)(Lex *, Cell *) = proc->builtin;
            tmp_result = func(e, make_sexpr_from_list(arg_list));
        } else {
            /* Prepend the procedure to create the application form */
            Cell* application_list = make_val_pair(proc, reversed_arg_list);
            application_list->len = arg_list->len + 1;

            /* Convert the Scheme list to an S-expression for eval */
            Cell* application_sexpr = make_sexpr_from_list(application_list);

            /* Evaluate it */
            tmp_result = coz_eval(e, application_sexpr);
        }
        if (tmp_result->type == VAL_ERR) {
            /* Propagate any evaluation errors */
            return tmp_result;
        }

        /* Cons the result onto our (reversed) final list */
        final_result = make_val_pair(tmp_result, final_result);
        final_result->len = i + 1;
    }

    /* Reverse the final list to get the correct order and return */
    return builtin_list_reverse(e, make_sexpr_len1(final_result));
}

Cell* builtin_filter(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    if (a->cell[0]->type != VAL_PROC) {
        return make_val_err("filter: arg 1 must be a procedure", GEN_ERR);
    }
    if (a->cell[1]->type != VAL_PAIR && a->cell[1]->len == -1) {
        return make_val_err("filter: arg 2 must be a proper list", GEN_ERR);
    }

    const Cell* proc = a->cell[0];
    Cell* result = make_val_nil();
    const Cell* val = a->cell[1];
    for (int i = 0; i < a->cell[1]->len; i++) {
        Cell* pred_outcome = coz_eval(e, make_sexpr_len2(proc, val->car));
        if (pred_outcome->type == VAL_ERR) {
            return pred_outcome;
        }
        /* Copy val to result list if pred is true */
        if (pred_outcome->b_val == 1) {
            result = make_val_pair(cell_copy(val->car), result);
        }
        val = val->cdr;
    }
    return builtin_list_reverse(e, make_sexpr_len1(result));
}

Cell* builtin_foldl(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 3);
    if (err) return err;
    if (a->cell[0]->type != VAL_PROC) {
        return make_val_err("foldl: arg 1 must be a procedure", GEN_ERR);
    }
    int shortest_list_length = INT32_MAX;

    for (int i = 2; i < a->count; i++) {
        char buf[128];
        if (a->cell[i]->type != VAL_PAIR || a->cell[i]->len == -1) {
            snprintf(buf, 128, "foldl: arg %d must be a proper list", i+1);
            return make_val_err(buf, GEN_ERR);
        }
        if (a->cell[i]->len < shortest_list_length) {
            shortest_list_length = a->cell[i]->len;
        }
    }

    const int shortest_len = shortest_list_length;
    const int num_lists = a->count - 2;
    Cell* proc = a->cell[0];
    Cell* init = a->cell[1];

    for (int i = 0; i < shortest_len; i++) {
        /* Build a list of the i-th arguments */
        Cell* arg_list = make_val_nil();
        /* cons the initial/accumulator */
        arg_list = make_val_pair(init, arg_list);
        /* Grab vals starting from the last list, so that after the
         * 'reversed' list is constructed, order is correct */
        for (int j = num_lists + 1; j >= 2; j--) {
            Cell* current_list = a->cell[j];
            Cell* nth_item = list_get_nth_cell_ptr(current_list, i);
            arg_list = make_val_pair(nth_item, arg_list);
            /* len is the number of lists plus the accumulator */
            arg_list->len = num_lists + 1;
        }

        Cell* tmp_result = NULL;
        /* If the procedure is a builtin - grab a pointer to it and call it directly
         * otherwise - it is a lambda and needs to be evaluated */
        if (proc->builtin) {
            Cell* (*func)(Lex *, Cell *) = proc->builtin;
            tmp_result = func(e, make_sexpr_from_list(arg_list));
        } else {
            /* Prepend the procedure to create the application form */
            Cell* application_list = make_val_pair(proc, arg_list);
            application_list->len = arg_list->len + 1;

            /* Convert the Scheme list to an S-expression for eval */
            Cell* application_sexpr = make_sexpr_from_list(application_list);

            /* Evaluate it */
            tmp_result = coz_eval(e, application_sexpr);
        }
        if (tmp_result->type == VAL_ERR) {
            /* Propagate any evaluation errors */
            return tmp_result;
        }
        /* assign the result to the accumulator */
        init = tmp_result;
    }
    /* Return the accumulator after all list args are evaluated */
    return init;
}


/*-------------------------------------------------------*
 *                Input/output and ports                 *
 * ------------------------------------------------------*/

