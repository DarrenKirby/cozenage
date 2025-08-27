/* environment.c - functions for getting and setting values in the environment
 *               - also, mapping from lisp procedure to C implementation
 */

#include "environment.h"
#include "ops.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* lenv_new()
 * Initializes an environment, and returns a pointer to it.
 * */
l_env* lenv_new(void) {
    l_env* e = malloc(sizeof(l_env));
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

void lenv_del(l_env* e) {
    if (!e) return;

    for (int i = 0; i < e->count; i++) {
        // Free symbols properly
        if (e->syms[i]) {
            free(e->syms[i]);  // this will free v->sym as well
        }
        // Free values
        if (e->vals[i]) {
            lval_del(e->vals[i]);
        }
    }

    free(e->syms);
    free(e->vals);
    free(e);
}


l_val* lenv_get(const l_env* e, const l_val* k) {
    if (!e || !k || k->type != LVAL_SYM) return NULL;

    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            return lval_copy(e->vals[i]);
        }
    }

    char buf[128];
    snprintf(buf, sizeof(buf), "Unbound symbol: '%s'", k->sym);
    return lval_err(buf);
}

void lenv_put(l_env* e, const l_val* k, const l_val* v) {
    if (!e || !k || !v || k->type != LVAL_SYM) {
        fprintf(stderr, "lenv_put: invalid arguments\n");
        return;
    }

    /* Check if symbol already exists */
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            /* Free the old value and replace it with a copy of v */
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }

    /* Symbol not found; append new entry */
    e->count++;
    e->syms = realloc(e->syms, sizeof(char*) * e->count);
    e->vals = realloc(e->vals, sizeof(l_val*) * e->count);
    if (!e->syms || !e->vals) {
        fprintf(stderr, "ENOMEM: lenv_put failed\n");
        exit(EXIT_FAILURE);
    }
    e->syms[e->count - 1] = strdup(k->sym);
    e->vals[e->count - 1] = lval_copy(v);
}

l_val* lval_builtin(const char* name,
                    l_val* (*func)(l_env*, l_val*)) {
    l_val* v = malloc(sizeof(l_val));
    v->type = LVAL_FUN;
    v->builtin = func;
    v->name = strdup(name);   /* store name for printing */
    return v;
}

void lenv_add_builtin(l_env* e, const char* name,
                      l_val* (*func)(l_env*, l_val*)) {
    l_val* fn = lval_builtin(name, func);
    l_val* k = lval_sym(name);
    lenv_put(e, k, fn);
    lval_del(k);
    lval_del(fn);  /* env makes its own copy */
}

void lenv_add_builtins(l_env* e) {
    /* basic arithmatic operators */
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);
    /* comparison operators */
    lenv_add_builtin(e, "=", builtin_eq_op);
    lenv_add_builtin(e, ">", builtin_gt_op);
    lenv_add_builtin(e, "<", builtin_lt_op);
    lenv_add_builtin(e, ">=", builtin_gte_op);
    lenv_add_builtin(e, "<=", builtin_lte_op);
    /* general numeric procedures */
    lenv_add_builtin(e, "zero?", builtin_zero);
    lenv_add_builtin(e, "positive?", builtin_positive);
    lenv_add_builtin(e, "negative?", builtin_negative);
    lenv_add_builtin(e, "odd?", builtin_odd);
    lenv_add_builtin(e, "even?", builtin_even);
    /* special forms */
    lenv_add_builtin(e, "quote", builtin_quote);
    /* eq?, eql?, and equal? */
    lenv_add_builtin(e, "eq?", builtin_eq);
    lenv_add_builtin(e, "eqv?", builtin_eqv);
    lenv_add_builtin(e, "equal?", builtin_equal);
    /* more numerics */
    lenv_add_builtin(e, "abs", builtin_abs);
    lenv_add_builtin(e, "expt", builtin_expt);
    lenv_add_builtin(e, "^", builtin_expt); /* non-standard alias for expt */
    lenv_add_builtin(e, "remainder", builtin_remainder);
    lenv_add_builtin(e, "modulo", builtin_modulo);
    lenv_add_builtin(e, "%", builtin_modulo); /* non-standard alias for modulo */
    lenv_add_builtin(e, "quotient", builtin_quotient);
    /* logical operators */
    lenv_add_builtin(e, "not", builtin_not);
    lenv_add_builtin(e, "and", builtin_and);
    lenv_add_builtin(e, "or", builtin_or);
    lenv_add_builtin(e, "boolean?", builtin_boolean_pred);
    lenv_add_builtin(e, "boolean", builtin_boolean);
    /* Pair/list constructors and selectors */
    lenv_add_builtin(e, "cons", builtin_cons);
    lenv_add_builtin(e, "car", builtin_car);
    lenv_add_builtin(e, "cdr", builtin_cdr);
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "length", builtin_list_length);
    lenv_add_builtin(e, "list-ref", builtin_list_ref);
}
