/* ops.c - definitions of built-in functions */

#include "ops.h"
#include "types.h"
#include "environment.h"
#include <stdlib.h>


// Returns 1 if l_val is a number (int or float)
int lval_is_num(l_val* v) {
    return v->type == LVAL_INT || v->type == LVAL_FLOAT;
}

// Convert any l_val number to long double for calculation
long double lval_to_ld(l_val* v) {
    return (v->type == LVAL_INT) ? (long double)v->int_n : v->float_n;
}

// Returns 1 if any arg is float, else 0
int lval_any_float(l_val* a) {
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type == LVAL_FLOAT) return 1;
    }
    return 0;
}

// Check that all args are numeric; return error l_val* if not, else NULL
l_val* lval_check_numbers(l_val* a) {
    for (int i = 0; i < a->count; i++) {
        if (!lval_is_num(a->cell[i])) {
            return lval_err("Cannot operate on non-number!");
        }
    }
    return NULL;
}

// Create a result l_val based on float_seen flag
l_val* lval_make_num(const long double val, const int float_seen) {
    if (float_seen) return lval_float(val);
    return lval_int((long long)val);
}

/* Basic arithmetic operators
 *
 * The general flow of all these are:
 * 1. Validate that all args are numbers.
 * 2. Check if any float is present for type promotion.
 * 3. Apply all remaining args to the first arg.
 * 4. Free the l_val containing the args.
 * 5. Return an appropriately typed result.
 */

l_val* builtin_add(l_env* e, l_val* a) {
    if (lval_check_numbers(a)) return lval_check_numbers(a);
    const int float_seen = lval_any_float(a);
    long double result = lval_to_ld(a->cell[0]);
    for (int i = 1; i < a->count; i++) {
        result += lval_to_ld(a->cell[i]);
    }
    lval_del(a);
    return lval_make_num(result, float_seen);
}

l_val* builtin_sub(l_env* e, l_val* a) {
    if (lval_check_numbers(a)) return lval_check_numbers(a);
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
    if (lval_check_numbers(a)) return lval_check_numbers(a);
    const int float_seen = lval_any_float(a);
    long double result = lval_to_ld(a->cell[0]);
    for (int i = 1; i < a->count; i++) {
        result *= lval_to_ld(a->cell[i]);
    }
    lval_del(a);
    return lval_make_num(result, float_seen);
}

l_val* builtin_div(l_env* e, l_val* a) {
    if (lval_check_numbers(a)) return lval_check_numbers(a);
    const int float_seen = lval_any_float(a);
    long double result = lval_to_ld(a->cell[0]);
    for (int i = 1; i < a->count; i++) {
        const long double intermediate = lval_to_ld(a->cell[i]);
        if (intermediate == 0) {
            lval_del(a);
            return lval_err("Division by zero!");
        }
        result /= intermediate;
    }
    lval_del(a);
    return lval_make_num(result, float_seen);
}
