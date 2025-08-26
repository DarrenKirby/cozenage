/* ops.c - definitions of built-in functions */

#include "ops.h"
#include "types.h"
#include "environment.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


/* Return 1 if l_val is a number (int or float) */
int lval_is_num(const l_val* v) {
    return v->type == LVAL_INT || v->type == LVAL_FLOAT;
}

/* Convert any l_val number to long double for calculation */
long double lval_to_ld(const l_val* v) {
    return (v->type == LVAL_INT) ? (long double)v->int_n : v->float_n;
}

/* Return 1 if any arg is float, else 0 */
int lval_any_float(const l_val* a) {
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type == LVAL_FLOAT) return 1;
    }
    return 0;
}

/* Create a result l_val based on float_seen flag */
l_val* lval_make_num(const long double val, const int float_seen) {
    if (float_seen) return lval_float(val);
    return lval_int((long long)val);
}

/* Helper which determines if there is a meaningful fractional portion
 * in the result, and returns LVAL_INT or LVAL_FLOAT accordingly */
l_val* make_lval_from_double(const long double x) {
    // epsilon: what counts as “effectively an integer”
    const long double EPS = 1e-12L;

    const long double rounded = roundl(x);
    if (fabsl(x - rounded) < EPS) {
        // treat as integer
        return lval_int((long long)rounded);
    }
    // treat as float
    return lval_float(x);
}

/* Basic arithmetic operators: + - * /
 *
 * The general flow of all these are:
 * 1. Validate that all args are numbers.
 * 2. Check if any float is present for type promotion.
 * 3. Apply all remaining args to the first arg.
 * 4. Free the l_val containing the args.
 * 5. Return an appropriately typed result.
 */

l_val* builtin_add(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    /* identity law logic */
    if (a->count == 0) {
        return lval_int(0);
    }
    long double result = lval_to_ld(a->cell[0]);

    /* more identity law logic */
    if (a->count == 1) {
        return make_lval_from_double(result);
    }

    for (int i = 1; i < a->count; i++) {
        result += LVAL_AS_NUM(a->cell[i]);
    }
    return make_lval_from_double(result);
}

l_val* builtin_sub(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }
    long double result = lval_to_ld(a->cell[0]);
    /* Handle unary minus */
    if (a->count == 1) {
        result = -result;
    } else {
        for (int i = 1; i < a->count; i++) {
            result -= LVAL_AS_NUM(a->cell[i]);
        }
    }
    return make_lval_from_double(result);
}

l_val* builtin_mul(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    /* identity law logic */
    if (a->count == 0) {
        return lval_int(1);
    }
    long double result = lval_to_ld(a->cell[0]);
    /* more identity law logic */
    if (a->count == 1) {
        return make_lval_from_double(result);
    }

    for (int i = 1; i < a->count; i++) {
        result *= LVAL_AS_NUM(a->cell[i]);
    }
    return make_lval_from_double(result);
}

/* TODO: implement unary division reciprocal ie: (/ 5) -> 1/5 */
l_val* builtin_div(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }
    long double result = lval_to_ld(a->cell[0]);
    for (int i = 1; i < a->count; i++) {
        const long double intermediate = lval_to_ld(a->cell[i]);
        if (intermediate == 0) {
            return lval_err("Division by zero");
        }
        result /= intermediate;
    }
    return make_lval_from_double(result);
}

/* Comparison operators
 *
 * The general flow of all these are:
 * 1. Validate that all args are numbers.
 * 2. Check if any float is present for type promotion.
 * 3. Apply all remaining args to the first arg.
 * 4. Free the l_val containing the args.
 * 5. Return an appropriately typed result.
 */
l_val* builtin_eq_op(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        if (!(LVAL_AS_NUM(a->cell[i]) == LVAL_AS_NUM(a->cell[i+1]))) {
            return lval_bool(0);  /* false */
        }
    }
    return lval_bool(1);
}

l_val* builtin_gt_op(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        if (!(LVAL_AS_NUM(a->cell[i]) > LVAL_AS_NUM(a->cell[i+1]))) {
            return lval_bool(0);  /* false */
        }
    }
    return lval_bool(1);
}

l_val* builtin_lt_op(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        if (!(LVAL_AS_NUM(a->cell[i]) < LVAL_AS_NUM(a->cell[i+1]))) {
            return lval_bool(0);  /* false */
        }
    }
    return lval_bool(1);

}

l_val* builtin_gte_op(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        if (!(LVAL_AS_NUM(a->cell[i]) >= LVAL_AS_NUM(a->cell[i+1]))) {
            return lval_bool(0);  /* false */
        }
    }
    return lval_bool(1);
}

l_val* builtin_lte_op(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        if (!(LVAL_AS_NUM(a->cell[i]) <= LVAL_AS_NUM(a->cell[i+1]))) {
            return lval_bool(0);  /* false */
        }
    }
    return lval_bool(1);
}

/* generic unary numeric procedures */
l_val* builtin_zero(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long double result = lval_to_ld(a->cell[0]);
    if (result == 0 || result == 0.0) { return lval_bool(1); }
    return lval_bool(0);
}

l_val* builtin_positive(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long double result = lval_to_ld(a->cell[0]);
    if (result >= 0) { return lval_bool(1); }
    return lval_bool(0);
}

l_val* builtin_negative(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long double result = lval_to_ld(a->cell[0]);
    if (result < 0) { return lval_bool(1); }
    return lval_bool(0);
}

l_val* builtin_odd(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long long n = a->cell[0]->int_n;
    if (n % 2 == 0) { return lval_bool(0); }
    return lval_bool(1);
}

l_val* builtin_even(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long long n = a->cell[0]->int_n;
    if (n % 2 == 0) { return lval_bool(1); }
    return lval_bool(0);
}

/* builtin_quote()
 * Implements the Scheme 'quote special form.
 * Simply returns the first argument unevaluated.
 */
l_val* builtin_quote(l_env* e, l_val* a) {
    l_val* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    /* Take the first argument and do NOT evaluate it */
    //l_val* quoted = lval_take(a, 0);
    //return quoted;
    return lval_take(a, 0);
}

/*
* eq? → identity (mostly)
* eqv? → atomic value equality
* equal? → deep recursive structural equality
* */

/* Shallow identity: eq? */
l_val* builtin_eq(l_env* e, l_val* a) {
    l_val* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    const l_val* x = a->cell[0];
    const l_val* y = a->cell[1];

    int result = (x == y);
    if (!result && x->type == y->type) {
        if (x->type == LVAL_SYM)  result = (strcmp(x->sym, y->sym) == 0);
        if (x->type == LVAL_STR)  result = (strcmp(x->str, y->str) == 0);
        if (x->type == LVAL_CHAR) result = (x->char_val == y->char_val);
    }
    return lval_bool(result);
}

/* Slightly looser: eqv? (values equal for numbers, chars, symbols) */
l_val* builtin_eqv(l_env* e, l_val* a) {
    l_val* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    const l_val* x = a->cell[0];
    const l_val* y = a->cell[1];

    if (x->type != y->type) return lval_bool(0);

    switch (x->type) {
        case LVAL_INT:   return lval_bool(x->int_n == y->int_n);
        case LVAL_FLOAT: return lval_bool(x->float_n == y->float_n);
        case LVAL_CHAR:  return lval_bool(x->char_val == y->char_val);
        case LVAL_SYM:   return lval_bool(strcmp(x->sym, y->sym) == 0);
        case LVAL_STR:   return lval_bool(strcmp(x->str, y->str) == 0);
        default:         return lval_bool(x == y); // fall back to identity
    }
}

/* Deep recursive equality: equal? */
l_val* lval_equal(const l_val* x, const l_val* y) {
    if (x->type != y->type) return lval_bool(0);

    switch (x->type) {
        case LVAL_INT:   return lval_bool(x->int_n == y->int_n);
        case LVAL_FLOAT: return lval_bool(x->float_n == y->float_n);
        case LVAL_CHAR:  return lval_bool(x->char_val == y->char_val);
        case LVAL_SYM:   return lval_bool(strcmp(x->sym, y->sym) == 0);
        case LVAL_STR:   return lval_bool(strcmp(x->str, y->str) == 0);

        case LVAL_SEXPR:
        case LVAL_VECT:
            if (x->count != y->count) return lval_bool(0);
            for (int i = 0; i < x->count; i++) {
                l_val* eq = lval_equal(x->cell[i], y->cell[i]);
                if (!eq->boolean) { lval_del(eq); return lval_bool(0); }
                lval_del(eq);
            }
            return lval_bool(1);

        default:
            return lval_bool(0);
    }
}

l_val* builtin_equal(l_env* e, l_val* a) {
    l_val* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    return lval_equal(a->cell[0], a->cell[1]);
}

/* More numeric operations */

l_val* builtin_abs(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    if (LVAL_AS_NUM(a->cell[0]) >= 0) {
        if (a->type == LVAL_INT) { return lval_int(a->int_n); }
        return lval_float(a->float_n);
    }
    if (a->type == LVAL_INT) { return lval_int(-a->int_n); }
    return lval_float(-a->float_n);
}

l_val* builtin_expt(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }

    long double base = LVAL_AS_NUM(a->cell[0]);
    l_val* exp_val = a->cell[1];
    long double result;

    if (exp_val->type == LVAL_INT) {
        /* integer exponent: use fast exponentiation */
        long long n = exp_val->int_n;
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

l_val* builtin_modulo(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    long long r = a->cell[0]->int_n % a->cell[1]->int_n;
    if ((r != 0) && ((a->cell[1]->int_n > 0 && r < 0) || (a->cell[1]->int_n < 0 && r > 0))) {
        r += a->cell[1]->int_n;
    }
    return lval_int(r);
}

l_val* builtin_quotient(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    return lval_int(a->cell[0]->int_n / a->cell[1]->int_n);
}

l_val* builtin_remainder(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    return lval_int(a->cell[0]->int_n % a->cell[1]->int_n);
}

l_val* builtin_not(l_env* e, l_val* a) {
    l_val* err;
    /* Only #f (boolean false) returns true for `not`; everything else is false */
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const int is_false = (a->cell[0]->type == LVAL_BOOL && a->cell[0]->boolean == 0);
    return lval_bool(is_false);
}

l_val* builtin_boolean_pred(l_env* e, l_val* a) {
    l_val* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return lval_bool(a->cell[0]->type == LVAL_BOOL);
}

/* boolean: converts any value to a strict boolean */
l_val* builtin_boolean(l_env* e, l_val* a) {
    l_val* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    int result = (a->cell[0]->type == LVAL_BOOL)
                 ? a->cell[0]->boolean
                 : 1; // everything except #f is true
    return lval_bool(result);
}

l_val* builtin_and(l_env* e, l_val* a) {
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type == LVAL_BOOL && a->cell[i]->boolean == 0) {
            /* first #f encountered → return a copy of it */
            return lval_copy(a->cell[i]);
        }
    }
    /* all truthy → return copy of last element */
    return lval_copy(a->cell[a->count - 1]);
}

l_val* builtin_or(l_env* e, l_val* a) {
    for (int i = 0; i < a->count; i++) {
        if (!(a->cell[i]->type == LVAL_BOOL && a->cell[i]->boolean == 0)) {
            /* first truthy value → return a copy */
            return lval_copy(a->cell[i]);
        }
    }
    /* all false → return copy of last element (#f) */
    return lval_copy(a->cell[a->count - 1]);
}

/* pair/list constructors, selectors, and procedures */

l_val* builtin_cons(l_env* e, l_val* a) {
    l_val* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    return lval_pair(lval_copy(a->cell[0]), lval_copy(a->cell[1]));
}

l_val* builtin_car(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_PAIR);
    if (err) { return err; }
    return lval_copy(a->cell[0]->car);
}

l_val* builtin_cdr(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_PAIR);
    if (err) { return err; }
    return lval_copy(a->cell[0]->cdr);
}

l_val* builtin_list(l_env* e, l_val* a) {
    // start with nil
    l_val* result = lval_new_nil();

    // build backwards so it comes out in the right order
    for (int i = a->count - 1; i >= 0; i--) {
        result = lval_pair(lval_copy(a->cell[i]), result);
    }
    return result;
}

l_val* builtin_list_length(l_env* e, l_val* a) {
    l_val* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = lval_check_types(a, LVAL_PAIR);
    if (err) { return err; }

    int len = 0;
    const l_val* p = a->cell[0];
    while (p->type == LVAL_PAIR) {
        len++;
        p = p->cdr;
    }

    if (p->type != LVAL_NIL) {
        return lval_err("Improper list");
    }
    return lval_int(len);
}
