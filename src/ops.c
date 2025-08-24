/* ops.c - definitions of built-in functions */

#include "ops.h"
#include "types.h"
#include "environment.h"
#include <string.h>


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
        lval_del(a);
        return lval_int(0);
    }
    const int float_seen = lval_any_float(a);
    long double result = lval_to_ld(a->cell[0]);

    /* more identity law logic */
    if (a->count == 1) {
        lval_del(a);
        return lval_make_num(result, float_seen);
    }

    for (int i = 1; i < a->count; i++) {
        result += lval_to_ld(a->cell[i]);
    }
    lval_del(a);
    return lval_make_num(result, float_seen);
}

l_val* builtin_sub(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }
    const int float_seen = lval_any_float(a);
    long double result = lval_to_ld(a->cell[0]);
    /* Handle unary minus */
    if (a->count == 1) {
        result = -result;
    } else {
        for (int i = 1; i < a->count; i++) {
            result -= lval_to_ld(a->cell[i]);
        }
    }
    lval_del(a);
    return lval_make_num(result, float_seen);
}

l_val* builtin_mul(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    /* identity law logic */
    if (a->count == 0) {
        lval_del(a);
        return lval_int(1);
    }
    const int float_seen = lval_any_float(a);
    long double result = lval_to_ld(a->cell[0]);

    /* more identity law logic */
    if (a->count == 1) {
        lval_del(a);
        return lval_make_num(result, float_seen);
    }

    for (int i = 1; i < a->count; i++) {
        result *= lval_to_ld(a->cell[i]);
    }
    lval_del(a);
    return lval_make_num(result, float_seen);
}

/* TODO: implement unary division reciprocal ie: (/ 5) -> 1/5 */
l_val* builtin_div(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }
    const int float_seen = lval_any_float(a);
    long double result = lval_to_ld(a->cell[0]);
    for (int i = 1; i < a->count; i++) {
        const long double intermediate = lval_to_ld(a->cell[i]);
        if (intermediate == 0) {
            lval_del(a);
            return lval_err("CozError: Division by zero");
        }
        result /= intermediate;
    }
    lval_del(a);
    return lval_make_num(result, float_seen);
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
    if ((err = CHECK_ARITY_MIN(a, 2))) { return err; }
    /* FIXME: finish these functions! */
    return lval_err("Not implemented yet");
}

l_val* builtin_gt_op(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 2))) { return err; }
    /* FIXME: finish these functions! */
    return lval_err("Not implemented yet");
}

l_val* builtin_lt_op(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 2))) { return err; }
    /* FIXME: finish these functions! */
    return lval_err("Not implemented yet");
}

l_val* builtin_gte_op(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 2))) { return err; }
    /* FIXME: finish these functions! */
    return lval_err("Not implemented yet");
}

l_val* builtin_lte_op(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 2))) { return err; }
    /* FIXME: finish these functions! */
    return lval_err("Not implemented yet");
}

/* generic unary numeric procedures */
l_val* builtin_zero(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long double result = lval_to_ld(a->cell[0]);
    lval_del(a);
    if (result == 0 || result == 0.0) { return lval_bool(1); }
    return lval_bool(0);
}

l_val* builtin_positive(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long double result = lval_to_ld(a->cell[0]);
    lval_del(a);
    if (result >= 0) { return lval_bool(1); }
    return lval_bool(0);
}

l_val* builtin_negative(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT | LVAL_FLOAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long double result = lval_to_ld(a->cell[0]);
    lval_del(a);
    if (result < 0) { return lval_bool(1); }
    return lval_bool(0);
}

l_val* builtin_odd(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long long n = a->cell[0]->int_n;
    lval_del(a);
    if (n % 2 == 0) { return lval_bool(0); }
    return lval_bool(1);
}

l_val* builtin_even(l_env* e, l_val* a) {
    l_val* err = lval_check_types(a, LVAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long long n = a->cell[0]->int_n;
    lval_del(a);
    if (n % 2 == 0) { return lval_bool(1); }
    return lval_bool(0);
}

/* builtin_quote()
 * Implements the Scheme 'quote special form.
 * Simply returns the first argument unevaluated.
 */
l_val* builtin_quote(l_env* e, l_val* a) {
    if (a->count != 1) {
        lval_del(a);
        return lval_err("quote expects exactly 1 argument");
    }
    /* Take the first argument and do NOT evaluate it */
    l_val* quoted = lval_take(a, 0);
    return quoted;
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
