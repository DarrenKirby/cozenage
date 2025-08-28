/* environment.c - functions for getting and setting values in the environment
 *               - also, mapping from Scheme procedure to C implementation
 */

#include "environment.h"
#include "ops.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Initializes an environment, and returns a pointer to it. */
Lex* lex_initialize(void) {
    Lex* e = malloc(sizeof(Lex));
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

/* Delete the environment upon program exit */
void lex_delete(Lex* e) {
    if (!e) return;

    for (int i = 0; i < e->count; i++) {
        /* Free symbols properly */
        if (e->syms[i]) {
            free(e->syms[i]);
        }
        /* Free values */
        if (e->vals[i]) {
            cell_delete(e->vals[i]);
        }
    }
    free(e->syms);
    free(e->vals);
    free(e);
}


Cell* lex_get(const Lex* e, const Cell* k) {
    if (!e || !k || k->type != VAL_SYM) return NULL;

    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            return cell_copy(e->vals[i]);
        }
    }

    char buf[128];
    snprintf(buf, sizeof(buf), "Unbound symbol: '%s'", k->sym);
    return make_val_err(buf);
}

void lex_put(Lex* e, const Cell* k, const Cell* v) {
    if (!e || !k || !v || k->type != VAL_SYM) {
        fprintf(stderr, "lenv_put: invalid arguments\n");
        return;
    }

    /* Check if symbol already exists */
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            /* Free the old value and replace it with a copy of v */
            cell_delete(e->vals[i]);
            e->vals[i] = cell_copy(v);
            return;
        }
    }

    /* Symbol not found; append new entry */
    e->count++;
    e->syms = realloc(e->syms, sizeof(char*) * e->count);
    e->vals = realloc(e->vals, sizeof(Cell*) * e->count);
    if (!e->syms || !e->vals) {
        fprintf(stderr, "ENOMEM: lenv_put failed\n");
        exit(EXIT_FAILURE);
    }
    e->syms[e->count - 1] = strdup(k->sym);
    e->vals[e->count - 1] = cell_copy(v);
}

Cell* lval_builtin(const char* name,
                    Cell* (*func)(Lex*, Cell*)) {
    Cell* v = malloc(sizeof(Cell));
    v->type = VAL_PROC;
    v->builtin = func;
    v->name = strdup(name);   /* store name for printing */
    return v;
}

void lex_add_builtin(Lex* e, const char* name,
                      Cell* (*func)(Lex*, Cell*)) {
    Cell* fn = lval_builtin(name, func);
    Cell* k = make_val_sym(name);
    lex_put(e, k, fn);
    cell_delete(k);
    cell_delete(fn);  /* env makes its own copy */
}

void lex_add_builtins(Lex* e) {
    /* basic arithmatic operators */
    lex_add_builtin(e, "+", builtin_add);
    lex_add_builtin(e, "-", builtin_sub);
    lex_add_builtin(e, "*", builtin_mul);
    lex_add_builtin(e, "/", builtin_div);
    /* comparison operators */
    lex_add_builtin(e, "=", builtin_eq_op);
    lex_add_builtin(e, ">", builtin_gt_op);
    lex_add_builtin(e, "<", builtin_lt_op);
    lex_add_builtin(e, ">=", builtin_gte_op);
    lex_add_builtin(e, "<=", builtin_lte_op);
    /* general numeric procedures */
    lex_add_builtin(e, "zero?", builtin_zero);
    lex_add_builtin(e, "positive?", builtin_positive);
    lex_add_builtin(e, "negative?", builtin_negative);
    lex_add_builtin(e, "odd?", builtin_odd);
    lex_add_builtin(e, "even?", builtin_even);
    /* special forms */
    lex_add_builtin(e, "quote", builtin_quote);
    /* eq?, eql?, and equal? */
    lex_add_builtin(e, "eq?", builtin_eq);
    lex_add_builtin(e, "eqv?", builtin_eqv);
    lex_add_builtin(e, "equal?", builtin_equal);
    /* more numerics */
    lex_add_builtin(e, "abs", builtin_abs);
    lex_add_builtin(e, "expt", builtin_expt);
    lex_add_builtin(e, "^", builtin_expt); /* non-standard alias for expt */
    lex_add_builtin(e, "remainder", builtin_remainder);
    lex_add_builtin(e, "modulo", builtin_modulo);
    lex_add_builtin(e, "%", builtin_modulo); /* non-standard alias for modulo */
    lex_add_builtin(e, "quotient", builtin_quotient);
    lex_add_builtin(e, "lcm", builtin_lcm);
    lex_add_builtin(e, "gcd", builtin_gcd);
    /* logical operators */
    lex_add_builtin(e, "not", builtin_not);
    lex_add_builtin(e, "and", builtin_and);
    lex_add_builtin(e, "or", builtin_or);
    lex_add_builtin(e, "boolean?", builtin_boolean_pred);
    lex_add_builtin(e, "boolean", builtin_boolean);
    /* Pair/list constructors and selectors */
    lex_add_builtin(e, "cons", builtin_cons);
    lex_add_builtin(e, "car", builtin_car);
    lex_add_builtin(e, "cdr", builtin_cdr);
    lex_add_builtin(e, "list", builtin_list);
    lex_add_builtin(e, "length", builtin_list_length);
    lex_add_builtin(e, "list-ref", builtin_list_ref);
}
