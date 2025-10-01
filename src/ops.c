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
#include "pairs.h"
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




