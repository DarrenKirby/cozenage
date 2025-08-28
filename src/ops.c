/* ops.c - definitions of built-in functions */

#include "ops.h"
#include "types.h"
#include "environment.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "printer.h"


/* Return 1 if l_val is a number (int or float) */
int lval_is_num(const Cell* v) {
    return v->type == VAL_INT || v->type == VAL_REAL;
}

/* Convert any l_val number to long double for calculation */
long double lval_to_ld(const Cell* v) {
    return (v->type == VAL_INT) ? (long double)v->int_v : v->real_v;
}

/* Helper which determines if there is a meaningful fractional portion
 * in the result, and returns LVAL_INT or LVAL_FLOAT accordingly */
Cell* make_lval_from_double(const long double x) {
    /* epsilon: what counts as “effectively an integer” */
    const long double EPS = 1e-12L;

    const long double rounded = roundl(x);
    if (fabsl(x - rounded) < EPS) {
        /* treat as integer */
        return make_val_int((long long)rounded);
    }
    /* treat as float */
    return make_val_real(x);
}

/*------------------------------------*
 *     Basic arithmetic operators     *
 * -----------------------------------*/

/* '+' -> LVAL_INT|LVAL_FLOAT|LVAL_RAT|LVAL_COMP - returns the sum of its arguments */
Cell* builtin_add(Lex* e, Cell* a) {
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
                result->int_v += rhs->int_v;
                break;
            case VAL_RAT:
                /* (a/b) + (c/d) = (ad + bc)/bd */
                result->num = result->num * rhs->den + rhs->num * result->den;
                result->den = result->den * rhs->den;
                result = simplify_rational(result);
                break;
            case VAL_REAL:
                result->real_v += rhs->real_v;
                break;
            case VAL_COMPLEX:
                result->real = builtin_add(e, make_sexpr_len2(result->real, rhs->real));
                result->imag = builtin_add(e, make_sexpr_len2(result->imag, rhs->imag));
                break;
            default:
                return make_val_err("<builtin '+'> Oops, this shouldn't have happened.");
        }
        result->exact = result->exact && rhs->exact;
        cell_delete(rhs); /* free temp */
    }
    return result;
}

/* '-' -> LVAL_INT|LVAL_FLOAT|LVAL_RAT|LVAL_COMP - returns the difference of its arguments */
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
                result->int_v -= rhs->int_v;
                break;
            case VAL_RAT:
                /* (a/b) - (c/d) = (ad - bc)/bd */
                result->num = result->num * rhs->den - rhs->num * result->den;
                result->den = result->den * rhs->den;
                result = simplify_rational(result);
                break;
            case VAL_REAL:
                result->real_v -= rhs->real_v;
                break;
            case VAL_COMPLEX:
                result->real = builtin_sub(e, make_sexpr_len2(result->real, rhs->real));
                result->imag = builtin_sub(e, make_sexpr_len2(result->imag, rhs->imag));
                break;
            default:
                return make_val_err("<builtin '-'> Oops, this shouldn't have happened.");
        }
        result->exact = result->exact && rhs->exact;
        cell_delete(rhs); /* free temp */
    }
    return result;
}

/* '*' -> LVAL_INT|LVAL_FLOAT|LVAL_RAT|LVAL_COMP - returns the product of its arguments */
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
                result->int_v *= rhs->int_v;
                break;
            case VAL_RAT:
                /* (a/b) * (c/d) = (a * c)/(b * d) */
                result->num = result->num * rhs->num;
                result->den = result->den * rhs->den;
                result = simplify_rational(result);
                break;
            case VAL_REAL:
                result->real_v *= rhs->real_v;
                break;
            case VAL_COMPLEX:
                result->real = builtin_mul(e, make_sexpr_len2(result->real, rhs->real));
                result->imag = builtin_mul(e, make_sexpr_len2(result->imag, rhs->imag));
                break;
            default:
                return make_val_err("<builtin '*'> Oops, this shouldn't have happened.");
        }
        result->exact = result->exact && rhs->exact;
        cell_delete(rhs); /* free temp */
    }
    return result;
}

/* '+' -> LVAL_INT|LVAL_FLOAT|LVAL_RAT|LVAL_COMP - returns the quotient of its arguments */
Cell* builtin_div(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }

    /* unary division reciprocal/inverse */
    if (a->count == 1) {
        if (a->cell[0]->type == VAL_INT) {
            return make_val_rat(1, a->cell[0]->int_v);
        }
        if (a->cell[0]->type == VAL_RAT) {
            const long int n = a->cell[0]->num;
            const long int d = a->cell[0]->den;
            return make_val_rat(d, n);
        }
        if (a->cell[0]->type == VAL_REAL) {
            return make_val_real(1.0L / a->cell[0]->real_v);
        }
        if (a->cell[0]->type == VAL_COMPLEX) {
            const Cell* real = a->cell[0]->real;
            const Cell* imag = a->cell[0]->imag;

            /* denominator = real^2 + imag^2 */
            const Cell* real_sq = builtin_mul(e, make_sexpr_len2(cell_copy(real), cell_copy(real)));
            const Cell* imag_sq = builtin_mul(e, make_sexpr_len2(cell_copy(imag), cell_copy(imag)));
            const Cell* denominator = builtin_add(e, make_sexpr_len2(real_sq, imag_sq));

            /* new real = real / denominator */
            Cell* new_real = builtin_div(e, make_sexpr_len2(cell_copy(real), cell_copy(denominator)));
            /* new imag = -imag / denominator */
            const Cell* neg_imag = negate_numeric(cell_copy(imag));
            Cell* new_imag = builtin_div(e, make_sexpr_len2(neg_imag, denominator));

            return make_val_complex(new_real, new_imag);
        }
    }
    /* multiple args */
    Cell* result = cell_copy(a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        Cell* rhs = cell_copy(a->cell[i]);
        numeric_promote(&result, &rhs);

        switch (result->type) {
        case VAL_INT:
            if (rhs->int_v == 0) {
                cell_delete(rhs);
                return make_val_err("Division by zero.");
            }
            result->int_v /= rhs->int_v;
            break;
        case VAL_RAT:
            /* (a/b) / (c/d) = (a * d)/(b * c)   */
            result->num = result->num * rhs->den;
            result->den = result->den * rhs->num;
            result = simplify_rational(result);
            break;
        case VAL_REAL:
            if (rhs->real_v == 0) {
                cell_delete(rhs);
                return make_val_err("Division by zero.");
            }
            result->real_v /= rhs->real_v;
            break;
        case VAL_COMPLEX: {
            /* result = (a + bi), rhs = (c + di) */
            const Cell* a_real = result->real;
            const Cell* a_imag = result->imag;
            const Cell* b_real = rhs->real;
            const Cell* b_imag = rhs->imag;

            /* denominator = c^2 + d^2 */
            const Cell* b_real_sq = builtin_mul(e, make_sexpr_len2(cell_copy(b_real), cell_copy(b_real)));
            const Cell* b_imag_sq = builtin_mul(e, make_sexpr_len2(cell_copy(b_imag), cell_copy(b_imag)));
            const Cell* denominator = builtin_add(e, make_sexpr_len2(b_real_sq, b_imag_sq));

            /* numerator real = (ac + bd) */
            const Cell* ac = builtin_mul(e, make_sexpr_len2(cell_copy(a_real), cell_copy(b_real)));
            const Cell* bd = builtin_mul(e, make_sexpr_len2(cell_copy(a_imag), cell_copy(b_imag)));
            const Cell* num_real = builtin_add(e, make_sexpr_len2(ac, bd));

            /* numerator imag = (bc - ad) */
            const Cell* bc = builtin_mul(e, make_sexpr_len2(cell_copy(a_imag), cell_copy(b_real)));
            const Cell* ad = builtin_mul(e, make_sexpr_len2(cell_copy(a_real), cell_copy(b_imag)));
            const Cell* num_imag = builtin_sub(e, make_sexpr_len2(bc, ad));

            /* divide both by denominator */
            result->real = builtin_div(e, make_sexpr_len2(num_real, cell_copy(denominator)));
            result->imag = builtin_div(e, make_sexpr_len2(num_imag, denominator));
            break;
        }
        default:
            return make_val_err("<builtin '/'> Oops, this shouldn't have happened.");
        }
        result->exact = result->exact && rhs->exact;
        cell_delete(rhs); /* free temp */
    }
    return result;
}

/* -----------------------------*
 *     Comparison operators     *
 * -----------------------------*/

/* '==' -> LVAL_BOOL - returns true if all arguments are equal. */
Cell* builtin_eq_op(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT | VAL_REAL);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        if (!(LVAL_AS_NUM(a->cell[i]) == LVAL_AS_NUM(a->cell[i+1]))) {
            return make_val_bool(0);  /* false */
        }
    }
    return make_val_bool(1);
}

/* '>' -> LVAL_BOOL - returns true if each argument is greater than the one that follows. */
Cell* builtin_gt_op(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT | VAL_REAL);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        if (!(LVAL_AS_NUM(a->cell[i]) > LVAL_AS_NUM(a->cell[i+1]))) {
            return make_val_bool(0);  /* false */
        }
    }
    return make_val_bool(1);
}

/* '<' -> LVAL_BOOL - returns true if each argument is less than the one that follows. */
Cell* builtin_lt_op(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT | VAL_REAL);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        if (!(LVAL_AS_NUM(a->cell[i]) < LVAL_AS_NUM(a->cell[i+1]))) {
            return make_val_bool(0);  /* false */
        }
    }
    return make_val_bool(1);
}

/* '>=' -> LVAL_BOOL - */
Cell* builtin_gte_op(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT | VAL_REAL);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        if (!(LVAL_AS_NUM(a->cell[i]) >= LVAL_AS_NUM(a->cell[i+1]))) {
            return make_val_bool(0);  /* false */
        }
    }
    return make_val_bool(1);
}

/* '<=' -> LVAL_BOOL - */
Cell* builtin_lte_op(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT | VAL_REAL);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        if (!(LVAL_AS_NUM(a->cell[i]) <= LVAL_AS_NUM(a->cell[i+1]))) {
            return make_val_bool(0);  /* false */
        }
    }
    return make_val_bool(1);
}

/* ---------------------------------------*
 *    Generic unary numeric procedures    *
 * ---------------------------------------*/

/* 'zero?' -> LVAL - returns #t if arg is == 0 else #f */
Cell* builtin_zero(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT | VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long double result = lval_to_ld(a->cell[0]);
    if (result == 0 || result == 0.0) { return make_val_bool(1); }
    return make_val_bool(0);
}

/* 'positive?' -> LVAL_BOOL - returns #t if arg is >= 0 else #f */
Cell* builtin_positive(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT | VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long double result = lval_to_ld(a->cell[0]);
    if (result >= 0) { return make_val_bool(1); }
    return make_val_bool(0);
}

/* 'negative?' -> LVAL_BOOL - returns #t if arg is < 0 else #f */
Cell* builtin_negative(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT | VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long double result = lval_to_ld(a->cell[0]);
    if (result < 0) { return make_val_bool(1); }
    return make_val_bool(0);
}

/* 'odd?' -> LVAL_BOOL - returns #t if arg is odd else #f */
Cell* builtin_odd(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long long n = a->cell[0]->int_v;
    if (n % 2 == 0) { return make_val_bool(0); }
    return make_val_bool(1);
}

/* 'even?' -> LVAL_BOOL - returns #t if arg is even else #f */
Cell* builtin_even(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long long n = a->cell[0]->int_v;
    if (n % 2 == 0) { return make_val_bool(1); }
    return make_val_bool(0);
}

/* -----------------------------*
 *         Special forms        *
 * -----------------------------*/

/* 'quote' -> LVAL_SEXPR -  returns the sole argument unevaluated */
Cell* builtin_quote(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    /* Take the first argument and do NOT evaluate it */
    return cell_take(a, 0);
}

/* ------------------------------------------*
*    Equality and equivalence comparators    *
* -------------------------------------------*/

/* 'eq?' -> LVAL_BOOL - Tests whether its two arguments are the exact same object
 * (pointer equality). Typically used for symbols and other non-numeric atoms.
 * May not give meaningful results for numbers or characters, since distinct but
 * equal values aren’t guaranteed to be the same object. */
Cell* builtin_eq(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    const Cell* x = a->cell[0];
    const Cell* y = a->cell[1];

    /* Strict pointer equality */
    return make_val_bool(x == y);
}

/* 'eqv?' -> LVAL_BOOL - Like 'eq?', but also considers numbers and characters
 * with the same value as equivalent. (eqv? 2 2) is true, even if those 2s are
 * not the same object. Use when: you want a general-purpose equality predicate
 * that works for numbers, characters, and symbols, but you don’t need deep
 * structural comparison. */
Cell* builtin_eqv(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    const Cell* x = a->cell[0];
    const Cell* y = a->cell[1];

    if (x->type != y->type) return make_val_bool(0);

    switch (x->type) {
        case VAL_INT:  return make_val_bool(x->int_v == y->int_v);
        case VAL_REAL: return make_val_bool(x->real_v == y->real_v);
        case VAL_CHAR: return make_val_bool(x->char_val == y->char_val);
        default:       return make_val_bool(x == y); /* fall back to identity */
    }
}

/* Helper for equal? */
Cell* lval_equal(const Cell* x, const Cell* y) {
    if (x->type != y->type) return make_val_bool(0);

    switch (x->type) {
        case VAL_INT:  return make_val_bool(x->int_v == y->int_v);
        case VAL_REAL: return make_val_bool(x->real_v == y->real_v);
        case VAL_CHAR: return make_val_bool(x->char_val == y->char_val);
        case VAL_SYM:  return make_val_bool(strcmp(x->sym, y->sym) == 0);
        case VAL_STR:  return make_val_bool(strcmp(x->str, y->str) == 0);

        case VAL_SEXPR:
        case VAL_VEC:
            if (x->count != y->count) return make_val_bool(0);
            for (int i = 0; i < x->count; i++) {
                Cell* eq = lval_equal(x->cell[i], y->cell[i]);
                if (!eq->boolean) { cell_delete(eq); return make_val_bool(0); }
                cell_delete(eq);
            }
            return make_val_bool(1);

        default:
            return make_val_bool(0);
    }
}

/* 'equal?' -> LVAL_BOOL - Tests structural (deep) equality, comparing contents
 * recursively in lists, vectors, and strings. (equal? '(1 2 3) '(1 2 3)) → true,
 * even though the two lists are distinct objects.
 * Use when: you want to compare data structures by content, not identity.*/
Cell* builtin_equal(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    return lval_equal(a->cell[0], a->cell[1]);
}

/* -----------------------------------*
 *     Generic numeric operations     *
 * -----------------------------------*/

/* 'abs' -> LVAL_INT|LVAL_FLOAT - returns the absolute value of its argument */
Cell* builtin_abs(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT | VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    if (LVAL_AS_NUM(a->cell[0]) >= 0) {
        if (a->type == VAL_INT) { return make_val_int(a->int_v); }
        return make_val_real(a->real_v);
    }
    if (a->type == VAL_INT) { return make_val_int(-a->int_v); }
    return make_val_real(-a->real_v);
}

/* 'expt' -> LVAL_INT|LVAL_FLOAT - returns its first arg calculated
 * to the power of its second arg */
Cell* builtin_expt(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT | VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }

    long double base = LVAL_AS_NUM(a->cell[0]);
    const Cell* exp_val = a->cell[1];
    long double result;

    if (exp_val->type == VAL_INT) {
        /* integer exponent: use fast exponentiation */
        long long n = exp_val->int_v;
        result = 1.0;
        long long abs_n = n > 0 ? n : -n;

        while (abs_n > 0) {
            if (abs_n & 1) result *= base;
            base *= base;
            abs_n >>= 1;
        }
        if (n < 0) result = 1.0 / result;
    } else {
        /* float exponent: delegate to powl */
        result = powl(base, LVAL_AS_NUM(exp_val));
    }
    return make_lval_from_double(result);
}

/* 'modulo' -> LVAL_INT - returns the remainder of dividing the first argument
 * by the second, with the result having the same sign as the divisor.*/
Cell* builtin_modulo(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    long long r = a->cell[0]->int_v % a->cell[1]->int_v;
    if ((r != 0) && ((a->cell[1]->int_v > 0 && r < 0) || (a->cell[1]->int_v < 0 && r > 0))) {
        r += a->cell[1]->int_v;
    }
    return make_val_int(r);
}

/* 'quotient' -> LVAL_INT - returns the integer result of dividing
 * the first argument by the second, discarding any remainder.*/
Cell* builtin_quotient(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    return make_val_int(a->cell[0]->int_v / a->cell[1]->int_v);
}

/* 'remainder' -> LVAL_INT - returns the remainder of dividing the first argument
 * by the second, with the result having the same sign as the dividend.*/
Cell* builtin_remainder(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    return make_val_int(a->cell[0]->int_v % a->cell[1]->int_v);
}

Cell* builtin_lcm(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }

    const long long int x = a->cell[0]->int_v;
    const long long int y = a->cell[1]->int_v;

    Cell* gcd = builtin_gcd(e, make_sexpr_len2(make_val_int(x), make_val_int(y)));
    long long int tmp = x * y / gcd->int_v;
    /* return only positive value */
    if (tmp < 0) {
        tmp = -tmp;
    }
    cell_delete(gcd);
    return make_val_int(tmp);
}

Cell* builtin_gcd(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) return err;
    if ((err = CHECK_ARITY_EXACT(a, 2))) return err;

    long long x = a->cell[0]->int_v;
    long long y = a->cell[1]->int_v;

    while (x != 0) {
        const long long tmp = x;
        x = y % x;
        y = tmp;
    }

    /* return only positive value */
    if (y < 0) {
        y = -y;
    }
    return make_val_int(y);
}

/* 'max' -> */

/* 'min' -> */

/* -------------------------------------------------*
 *       Type identity predicate procedures         *
 * -------------------------------------------------*/

/* 'number?' -> VAL_BOOL - returns #t if obj is numeric, else #f  */
Cell* builtin_number_pred(Lex *e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX;
    if (a->cell[0]->type & mask) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

/* 'boolean?' -> LVAL_BOOL  - returns #t if obj is either #t or #f
    and returns #f otherwise. */
Cell* builtin_boolean_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_BOOL);
}

/* 'null?' -> VAL_BOOL - return #t if obj is null, else #f */
Cell* builtin_null_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_NIL);
}

/* 'pair?' -> VAL_BOOL - return #t if obj is a pair, else #f */
Cell* builtin_pair_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_PAIR);
}

/* 'procedure?' -> VAL_BOOL - return #t if obj is a procedure, else #f */
Cell* builtin_proc_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_PROC);
}

/* 'symbol?' -> VAL_BOOL - return #t if obj is a symbol, else #f */
Cell* builtin_sym_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_SYM);
}

/* 'string?' -> VAL_BOOL - return #t if obj is a string, else #f */
Cell* builtin_string_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_STR);
}

/* 'char?' -> VAL_BOOL - return #t if obj is a char, else #f */
Cell* builtin_char_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_CHAR);
}

/* 'vector?' -> VAL_BOOL - return #t if obj is a vector, else #f */
Cell* builtin_vector_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_VEC);
}

/* 'bytevector?' -> VAL_BOOL - return #t if obj is a byte vector, else #f */
Cell* builtin_byte_vector_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_BYTEVEC);
}

/* 'port?' -> VAL_BOOL - return #t if obj is a port, else #f */
Cell* builtin_port_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_PORT);
}

/* 'eof-object?' -> VAL_BOOL - return #t if obj is an eof, else #f */
Cell* builtin_eof_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_err("Not implemented yet");
}

/* ---------------------------------------*
 *      Numeric identity procedures       *
 * ---------------------------------------*/

/* 'exact?' -> VAL_BOOL -  */
Cell* builtin_exact(Lex *e, Cell* a) {
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
Cell* builtin_inexact(Lex *e, Cell* a) {
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

/* NOTE: these do not yet match the Scheme standard,
 * in terms of the 'tower of numeric types'. For now,
 * they simply return true if the underlying Cell type
 * matches */

/* 'complex?' -> VAL_BOOL -  */
Cell* builtin_complex(Lex *e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = VAL_COMPLEX;
    if (a->cell[0]->type & mask) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

/* 'real?' -> VAL_BOOL -  */
Cell* builtin_real(Lex *e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = VAL_REAL;
    if (a->cell[0]->type & mask) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

/* 'rational?' -> VAL_BOOL -  */
Cell* builtin_rational(Lex *e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = VAL_RAT;
    if (a->cell[0]->type & mask) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

/* 'integer?' -> VAL_BOOL -  */
Cell* builtin_integer(Lex *e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = VAL_INT;
    if (a->cell[0]->type & mask) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

/* 'exact-integer?' -> VAL_BOOL -  */
Cell* builtin_exact_integer(Lex *e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = VAL_INT;
    if ((a->cell[0]->type & mask) && (a->cell[0]->exact)) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}
/* 'finite?' -> VAL_BOOL -  */

/* 'infinite?' -> VAL_BOOL -  */


/* ---------------------------------------*
 *     Boolean and logical procedures     *
 * ---------------------------------------*/

/* 'not' -> LVAL_BOOL - returns #t if obj is false, and returns #f otherwise */
Cell* builtin_not(Lex* e, Cell* a) {
    Cell* err;
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const int is_false = (a->cell[0]->type == VAL_BOOL && a->cell[0]->boolean == 0);
    return make_val_bool(is_false);
}

/* 'boolean' -> LVAL_BOOL - converts any value to a strict boolean */
Cell* builtin_boolean(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    int result = (a->cell[0]->type == VAL_BOOL)
                 ? a->cell[0]->boolean
                 : 1; /* everything except #f is true */
    return make_val_bool(result);
}

/* 'and' -> LVAL_BOOL|ANY - if any expression evaluates to #f, then #f is
 * returned. Any remaining expressions are not evaluated. If all the expressions
 * evaluate to true values, the values of the last expression are returned.
 * If there are no expressions, then #t is returned.*/
Cell* builtin_and(Lex* e, Cell* a) {
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type == VAL_BOOL && a->cell[i]->boolean == 0) {
            /* first #f encountered → return a copy of it */
            return cell_copy(a->cell[i]);
        }
    }
    /* all truthy → return copy of last element */
    return cell_copy(a->cell[a->count - 1]);
}

/* 'or' -> LVAL_BOOL|ANY - the value of the first expression that evaluates
 * to true is returned. Any remaining expressions are not evaluated. If all
 * expressions evaluate to #f or if there are no expressions, #f is returned */
Cell* builtin_or(Lex* e, Cell* a) {
    for (int i = 0; i < a->count; i++) {
        if (!(a->cell[i]->type == VAL_BOOL && a->cell[i]->boolean == 0)) {
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

/* 'cons' -> LVAL_PAIR - returns a pair made from two arguments */
Cell* builtin_cons(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    return make_val_pair(cell_copy(a->cell[0]), cell_copy(a->cell[1]));
}

/* 'car' -> ANY - returns the first member of a pair */
Cell* builtin_car(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_PAIR|VAL_SEXPR);
    if (err) { return err; }

    if (a->cell[0]->type == VAL_PAIR) {
        return cell_copy(a->cell[0]->car);
    }
    /* This is for the case where the argument list was quoted.
     * It needs to be transformed into a list before taking the car */
    Cell* list = builtin_list(e, a->cell[0]);
    Cell* result = cell_copy(list->car);
    cell_delete(list);
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
    Cell* list = builtin_list(e, a->cell[0]);
    Cell* result = cell_copy(list->cdr);
    cell_delete(list);
    return result;
}

/* 'list' -> LVAL_PAIR - returns a nil-terminated proper list */
Cell* builtin_list(Lex* e, Cell* a) {
    /* start with nil */
    Cell* result = make_val_nil();

    /* build backwards so it comes out in the right order */
    for (int i = a->count - 1; i >= 0; i--) {
        result = make_val_pair(cell_copy(a->cell[i]), result);
    }
    return result;
}

/* 'length' -> LVAL_INT - returns the member count of a proper list */
Cell* builtin_list_length(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, VAL_PAIR|VAL_SEXPR);
    if (err) { return err; }

    if (a->cell[0]->type == VAL_PAIR) {
        int len = 0;
        const Cell* p = a->cell[0];
        while (p->type == VAL_PAIR) {
            len++;
            p = p->cdr;
        }
        if (p->type != VAL_NIL) {
            return make_val_err("Improper list");
        }
        return make_val_int(len);
    }
    if (a->cell[0]->type == VAL_SEXPR) {
        return make_val_int(a->cell[0]->count);
    }
    return make_val_err("Oops, this shouldn't be\n");
}

/* 'list-ref' -> ANY - returns the list member at the zero-indexed
 * integer specified in the second arg. First arg is the list to act on*/
Cell* builtin_list_ref(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    if (a->cell[1]->type != VAL_INT) {
        return make_val_err("list-ref: arg 2 must be an integer");
    }
    int i = (int)a->cell[1]->int_v;

    if (a->cell[0]->type == VAL_PAIR) {
        const Cell* p = a->cell[0];
        if (p->type != VAL_PAIR) {
            return make_val_err("list-ref: index out of bounds");
        }
        while (i > 0) {
            p = p->cdr;
            i--;
        }
        return cell_copy(p->car);
    }
    if (a->cell[0]->type == VAL_SEXPR) {
        /* else the list is buried in an LVAL_SEXPR */
        return cell_copy(a->cell[0]->cell[i]);
    }
    return make_val_err("list-ref: arg 1 must be list or pair.");
}

/*-------------------------------------------------------*
 *     Vector constructors, selectors, and procedures    *
 * ------------------------------------------------------*/


/*------------------------------------------------------------*
 *     Byte vector constructors, selectors, and procedures    *
 * -----------------------------------------------------------*/


/*-------------------------------------------------------*
 *     String constructors, selectors, and procedures    *
 * ------------------------------------------------------*/
