/* environment.c - functions for getting and setting values in the environment
 *               - also, mapping from Scheme procedure to C implementation
 */

#include "environment.h"
#include "ops.h"
#include "types.h"
#include <gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Initializes an environment, and returns a pointer to it. */
Lex* lex_initialize(void) {
    Lex* top_env = GC_MALLOC(sizeof(Lex));
    top_env->count = 0;
    top_env->syms = NULL;
    top_env->vals = NULL;
    top_env->parent = NULL;  /* top-level has no parent */
    return top_env;
}

/* Initialize a new local environment */
Lex* lex_new_child(Lex* parent) {
    Lex* e = GC_MALLOC(sizeof(Lex));
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    e->parent = parent;
    return e;
}

Cell* lex_get(const Lex* e, const Cell* k) {
    if (!e || !k || k->type != VAL_SYM) return NULL;

    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            return cell_copy(e->vals[i]);
        }
    }

    /* Recurse into parent if not found */
    if (e->parent) {
        return lex_get(e->parent, k);
    }

    char buf[128];
    snprintf(buf, sizeof(buf), "Unbound symbol: '%s'", k->sym);
    return make_val_err(buf, GEN_ERR);
}

void lex_put(Lex* e, const Cell* k, const Cell* v) {
    if (!e || !k || !v || k->type != VAL_SYM) {
        fprintf(stderr, "lex_put: invalid arguments\n");
        return;
    }

    /* Check if symbol already exists */
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            /* Free the old value and replace it with a copy of v */
            e->vals[i] = cell_copy(v);
            return;
        }
    }

    /* Symbol not found; append new entry */
    e->count++;
    e->syms = GC_REALLOC(e->syms, sizeof(char*) * e->count);
    e->vals = GC_REALLOC(e->vals, sizeof(Cell*) * e->count);
    if (!e->syms || !e->vals) {
        fprintf(stderr, "ENOMEM: lex_put failed\n");
        exit(EXIT_FAILURE);
    }
    e->syms[e->count - 1] = GC_strdup(k->sym);
    e->vals[e->count - 1] = cell_copy(v);
}

Cell* lex_make_builtin(const char* name, Cell* (*func)(Lex*, Cell*)) {
    Cell* c = GC_MALLOC(sizeof(Cell));
    c->type = VAL_PROC;
    c->name = GC_strdup(name);
    c->builtin = func;
    c->formals = NULL;
    c->body = NULL;
    c->env = NULL;
    return c;
}

Cell* lex_make_named_lambda(const char* name, const Cell* formals, const Cell* body, Lex* env) {
    Cell* c = GC_MALLOC(sizeof(Cell));
    c->type = VAL_PROC;
    c->name = GC_strdup(name);  /* optional */
    c->builtin = NULL;
    c->formals = cell_copy(formals);
    c->body = cell_copy(body);
    c->env = env;  /* do NOT copy, just store pointer */
    return c;
}

Cell* lex_make_lambda(const Cell* formals, const Cell* body, Lex* env) {
    Cell* c = GC_MALLOC(sizeof(Cell));
    c->type = VAL_PROC;
    c->name = NULL;  /* optional */
    c->builtin = NULL;
    c->formals = cell_copy(formals);
    c->body = cell_copy(body);
    c->env = env;  /* do NOT copy, just store pointer */
    return c;
}

void lex_add_builtin(Lex* e, const char* name,
                      Cell* (*func)(Lex*, Cell*)) {
    const Cell* fn = lex_make_builtin(name, func);
    const Cell* k = make_val_sym(name);
    lex_put(e, k, fn);
}

void lex_add_builtins(Lex* e) {
    /* Basic arithmetic operators */
    lex_add_builtin(e, "+", builtin_add);
    lex_add_builtin(e, "-", builtin_sub);
    lex_add_builtin(e, "*", builtin_mul);
    lex_add_builtin(e, "/", builtin_div);
    /* Comparison operators */
    lex_add_builtin(e, "=", builtin_eq_op);
    lex_add_builtin(e, ">", builtin_gt_op);
    lex_add_builtin(e, "<", builtin_lt_op);
    lex_add_builtin(e, ">=", builtin_gte_op);
    lex_add_builtin(e, "<=", builtin_lte_op);
    /* Numeric predicate procedures */
    lex_add_builtin(e, "zero?", builtin_zero);
    lex_add_builtin(e, "positive?", builtin_positive);
    lex_add_builtin(e, "negative?", builtin_negative);
    lex_add_builtin(e, "odd?", builtin_odd);
    lex_add_builtin(e, "even?", builtin_even);
    /* Special forms */
    lex_add_builtin(e, "quote", builtin_quote);
    lex_add_builtin(e, "define", builtin_define);
    lex_add_builtin(e, "if", builtin_if);
    lex_add_builtin(e, "when", builtin_when);
    lex_add_builtin(e, "unless", builtin_unless);
    lex_add_builtin(e, "cond", builtin_cond);
    lex_add_builtin(e, "else", builtin_else);
    lex_add_builtin(e, "import", builtin_import);
    /* Equality and equivalence comparators */
    lex_add_builtin(e, "eq?", builtin_eq);
    lex_add_builtin(e, "eqv?", builtin_eqv);
    lex_add_builtin(e, "equal?", builtin_equal);
    /* Generic numeric operations */
    lex_add_builtin(e, "abs", builtin_abs);
    lex_add_builtin(e, "expt", builtin_expt);
    lex_add_builtin(e, "remainder", builtin_remainder);
    lex_add_builtin(e, "modulo", builtin_modulo);
    lex_add_builtin(e, "quotient", builtin_quotient);
    lex_add_builtin(e, "lcm", builtin_lcm);
    lex_add_builtin(e, "gcd", builtin_gcd);
    lex_add_builtin(e, "max", builtin_max);
    lex_add_builtin(e, "min", builtin_min);
    lex_add_builtin(e, "floor", builtin_floor);
    lex_add_builtin(e, "ceiling", builtin_ceiling);
    lex_add_builtin(e, "round", builtin_round);
    lex_add_builtin(e, "truncate", builtin_truncate);
    lex_add_builtin(e, "numerator", builtin_numerator);
    lex_add_builtin(e, "denominator", builtin_denominator);
    lex_add_builtin(e, "truncate-quotient", builtin_quotient);
    lex_add_builtin(e, "truncate-remainder", builtin_remainder);
    lex_add_builtin(e, "floor-remainder", builtin_modulo);
    lex_add_builtin(e, "square", builtin_square);
    lex_add_builtin(e, "exact", builtin_exact);
    lex_add_builtin(e, "inexact", builtin_inexact);
    /* Type identity predicate procedures */
    lex_add_builtin(e, "number?", builtin_number_pred);
    lex_add_builtin(e, "boolean?", builtin_boolean_pred);
    lex_add_builtin(e, "null?", builtin_null_pred);
    lex_add_builtin(e, "pair?", builtin_pair_pred);
    lex_add_builtin(e, "procedure?", builtin_proc_pred);
    lex_add_builtin(e, "symbol?", builtin_sym_pred);
    lex_add_builtin(e, "string?", builtin_string_pred);
    lex_add_builtin(e, "char?", builtin_char_pred);
    lex_add_builtin(e, "vector?", builtin_vector_pred);
    lex_add_builtin(e, "bytevector?", builtin_byte_vector_pred);
    lex_add_builtin(e, "port?", builtin_port_pred);
    lex_add_builtin(e, "eof-object?", builtin_eof_pred);
    /* Numeric identity predicate procedures */
    lex_add_builtin(e, "exact?", builtin_exact_pred);
    lex_add_builtin(e, "inexact?", builtin_inexact_pred);
    lex_add_builtin(e, "complex?", builtin_complex);
    lex_add_builtin(e, "real?", builtin_real);
    lex_add_builtin(e, "rational?", builtin_rational);
    lex_add_builtin(e, "integer?", builtin_integer);
    lex_add_builtin(e, "exact-integer?", builtin_exact_integer);
    /* Boolean and logical procedures */
    lex_add_builtin(e, "not", builtin_not);
    lex_add_builtin(e, "and", builtin_and);
    lex_add_builtin(e, "or", builtin_or);
    lex_add_builtin(e, "boolean", builtin_boolean);
    /* Pair/list procedures */
    lex_add_builtin(e, "cons", builtin_cons);
    lex_add_builtin(e, "car", builtin_car);
    lex_add_builtin(e, "cdr", builtin_cdr);
    lex_add_builtin(e, "list", builtin_list);
    lex_add_builtin(e, "length", builtin_list_length);
    lex_add_builtin(e, "list-ref", builtin_list_ref);
    lex_add_builtin(e, "append", builtin_list_append);
    lex_add_builtin(e, "reverse", builtin_list_reverse);
    lex_add_builtin(e, "list-tail", builtin_list_tail);
    /* Vector procedures */
    lex_add_builtin(e, "vector", builtin_vector);
    lex_add_builtin(e, "vector-length", builtin_vector_length);
    lex_add_builtin(e, "vector-ref", builtin_vector_ref);
    lex_add_builtin(e, "make-vector", builtin_make_vector);
    lex_add_builtin(e, "list->vector", builtin_list_to_vector);
    lex_add_builtin(e, "vector->list", builtin_vector_to_list);
    lex_add_builtin(e, "vector-copy", builtin_vector_copy);
    lex_add_builtin(e, "vector->string", builtin_vector_to_string);
    lex_add_builtin(e, "string->vector", builtin_string_to_vector);
    /* Bytevector procedures */
    lex_add_builtin(e, "bytevector", builtin_bytevector);
    lex_add_builtin(e, "bytevector-length", builtin_bytevector_length);
    lex_add_builtin(e, "bytevector-u8-ref", builtin_bytevector_ref);
    lex_add_builtin(e, "make-bytevector", builtin_make_bytevector);
    lex_add_builtin(e, "bytevector-copy", builtin_bytevector_copy);
    /* Char procedures */
    lex_add_builtin(e, "char->int", builtin_char_to_int);
    lex_add_builtin(e, "int->char", builtin_int_to_char);
    lex_add_builtin(e, "char=?", builtin_char_equal_pred);
    lex_add_builtin(e, "char<?", builtin_char_lt_pred);
    lex_add_builtin(e, "char<=?", builtin_char_lte_pred);
    lex_add_builtin(e, "char>?", builtin_char_gt_pred);
    lex_add_builtin(e, "char>=?", builtin_char_gte_pred);
    /* String procedures */
    lex_add_builtin(e, "symbol->string", builtin_symbol_to_string);
    lex_add_builtin(e, "string->symbol", builtin_string_to_symbol);
    lex_add_builtin(e, "string", builtin_string);
    lex_add_builtin(e, "string-length", builtin_string_length);
    lex_add_builtin(e, "string=?", builtin_string_eq_pred);
    lex_add_builtin(e, "string<?", builtin_string_lt_pred);
    lex_add_builtin(e, "string<=?", builtin_string_lte_pred);
    lex_add_builtin(e, "string>?", builtin_string_gt_pred);
    lex_add_builtin(e, "string>=?", builtin_string_gte_pred);
    /* Control features */
    lex_add_builtin(e, "apply", builtin_apply);
    lex_add_builtin(e, "eval", builtin_eval);
    lex_add_builtin(e, "map", builtin_map);
    lex_add_builtin(e, "filter", builtin_filter);
    /* input/output and ports */
    lex_add_builtin(e, "current-input-port", builtin_current_input_port);
    lex_add_builtin(e, "current-output-port", builtin_current_output_port);
    lex_add_builtin(e, "current-error-port", builtin_current_error_port);
}
