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
#include <math.h>
#include <stdlib.h>


/*------------------------------------*
 *     Basic arithmetic operators     *
 * -----------------------------------*/

/* '+' -> VAL_INT|VAL_REAL|VAL_RAT|VAL_COMP - returns the sum of its arguments */
Cell* builtin_add(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    /* identity law logic */
    if (a->count == 0) return make_val_int(0);
    if (a->count == 1) return a->cell[0];

    /* deep-copy required as result gets mutated */
    Cell* result = cell_copy(a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        Cell* rhs = a->cell[i];
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
Cell* builtin_sub(const Lex* e, const Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }

    /* Handle unary minus */
    if (a->count == 1) {
        return negate_numeric(a->cell[0]);
    }
    /* multiple args */
    /* deep-copy required as result gets mutated */
    Cell* result = cell_copy(a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        Cell* rhs = a->cell[i];
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
Cell* builtin_mul(const Lex* e, const Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    /* identity law logic */
    if (a->count == 0) return make_val_int(1);
    if (a->count == 1) return a->cell[0];

    /* deep-copy required as result gets mutated */
    Cell* result = cell_copy(a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        Cell* rhs = a->cell[i];
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
Cell* builtin_div(const Lex* e, const Cell* a) {
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
            const Cell* zero = make_val_int(0);
            const Cell* neg_b_args = make_sexpr_len2(zero, b_part);
            const Cell* neg_b = builtin_sub(e, neg_b_args);

            const Cell* new_imag_args = make_sexpr_len2(neg_b, denom);
            Cell* new_imag = builtin_div(e, new_imag_args);

            /* Create the final result */
            Cell* result = make_val_complex(new_real, new_imag);
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
        case VAL_INT:
            if (rhs->i_val == 0) {
                return make_val_err("Division by zero.", VALUE_ERR);
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
                return make_val_err("Division by zero.", VALUE_ERR);
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

/* 'abs' -> VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX -
 * returns the absolute value (magnitude) of its argument. */
Cell* builtin_abs(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const Cell* arg = a->cell[0];

    /* Handle all real numbers */
    if (cell_is_real(arg)) {
        /* If the number is a complex type, we operate on its real part.
         * Otherwise, we operate on the number itself. */
        const Cell* real_val = arg->type == VAL_COMPLEX ? arg->real : arg;

        if (cell_is_negative(real_val)) {
            return negate_numeric(real_val);
        }
        /* The number is non-negative, so just return it. */
        return (Cell*)real_val;
    }
    /* Handle non-real complex numbers
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
static Cell* expt_complex_op(const BuiltinFn op, const Lex* e, const Cell* z1, const Cell* z2) {
    /* Copy z1 to serve as the 'result' parameter that complex_apply will modify. */
    Cell* result = cell_copy(z1);

    /* Promote the types to be compatible, and pass a pointer to a
     * temporary Cell* for rhs so numeric_promote can modify it. */
    Cell* rhs_copy = cell_copy(z2);
    numeric_promote(&result, &rhs_copy);
    complex_apply(op, e, result, rhs_copy);
    return result;
}

/* 'expt' -> VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX - returns its first arg calculated
 * to the power of its second arg */
Cell* builtin_expt(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
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
Cell* builtin_modulo(const Lex* e, const Cell* a) {
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
Cell* builtin_quotient(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    return make_val_int(a->cell[0]->i_val / a->cell[1]->i_val);
}

/* 'remainder' -> VAL_INT - returns the remainder of dividing the first argument
 * by the second, with the result having the same sign as the dividend.*/
Cell* builtin_remainder(const Lex* e, const Cell* a) {
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

Cell* builtin_gcd(const Lex* e, const Cell* a) {
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

Cell* builtin_lcm(const Lex* e, const Cell* a) {
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
Cell* builtin_max(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }

    /* Validate that all arguments are real numbers. */
    for (int i = 0; i < a->count; i++) {
        if (!cell_is_real(a->cell[i])) {
            return make_val_err("max: all arguments must be real numbers", TYPE_ERR);
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
Cell* builtin_min(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }

    for (int i = 0; i < a->count; i++) {
        if (!cell_is_real(a->cell[i])) {
            return make_val_err("min: all arguments must be real numbers", TYPE_ERR);
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

Cell* builtin_floor(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    long double val = cell_to_long_double(a->cell[0]);
    val = floorl(val);

    return make_cell_from_double(val);
}

Cell* builtin_ceiling(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    long double val = cell_to_long_double(a->cell[0]);
    val = ceill(val);

    return make_cell_from_double(val);
}

Cell* builtin_round(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    long double val = cell_to_long_double(a->cell[0]);
    val = roundl(val);

    return make_cell_from_double(val);
}

Cell* builtin_truncate(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    long double val = cell_to_long_double(a->cell[0]);
    val = truncl(val);

    return make_cell_from_double(val);
}

Cell* builtin_numerator(const Lex* e, const Cell* a) {
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

Cell* builtin_denominator(const Lex* e, const Cell* a) {
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

Cell* builtin_square(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    Cell* args = make_sexpr_len2(a->cell[0], a->cell[0]);
    return builtin_mul(e, args);
}

Cell* builtin_exact(const Lex* e, const Cell* a) {
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

Cell* builtin_inexact(const Lex* e, const Cell* a) {
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
