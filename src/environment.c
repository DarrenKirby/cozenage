/*
 * 'src/environment.c'
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

#include "environment.h"
#include "numerics.h"
#include "types.h"
#include "comparators.h"
#include "control_features.h"
#include "bools.h"
#include "predicates.h"
#include "pairs.h"
#include "vectors.h"
#include "bytevectors.h"
#include "ports.h"
#include "strings.h"
#include "chars.h"
#include "symbols.h"
#include "errors.h"
#include "polymorph.h"
#include "repr.h"

#include <gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Initialize the global environment, and return a pointer to it. */
Lex* lex_initialize_global_env(void)
{
    ht_table* global_env = ht_create(256);
    Lex* e = GC_MALLOC(sizeof(Lex));
    e->local = nullptr;
    e->global = global_env;
    return e;
}


/* Initialize a new child environment */
Lex* new_child_env(const Lex* parent_env)
{
    Ch_Env* e = GC_MALLOC(sizeof(Ch_Env));
    e->count = 0;
    e->capacity = INITIAL_CHILD_ENV_CAPACITY;
    e->syms = GC_MALLOC(sizeof(char*) * e->capacity);
    e->vals = GC_MALLOC(sizeof(char*) * e->capacity);
    /* The new frame's parent is the PARENT'S LOCAL FRAME. */
    e->parent = parent_env->local;

    Lex* w = GC_MALLOC(sizeof(Lex));
    w->local = e; /* The new wrapper points to the new local frame. */
    w->global = parent_env->global;
    return w;
}


/* Retrieve a Cell* value from an environment. */
Cell* lex_get(const Lex* e, const Cell* k)
{
    if (!e || !k || k->type != CELL_SYMBOL) return nullptr;

    /* Search the entire local environment chain iteratively. */
    const Ch_Env* current_frame = e->local;
    while (current_frame != nullptr) {
        /* Linearly scan the symbols in the current frame. */
        for (int i = 0; i < current_frame->count; i++) {
            if (strcmp(current_frame->syms[i], k->sym) == 0) {
                return current_frame->vals[i];
            }
        }
        /* Not in this frame, move up to the parent frame. */
        current_frame = current_frame->parent;
    }

    /* If not found in any local frame, check the global environment. */
    Cell* result = ht_get(e->global, k->sym);
    if (result) {
        return result;
    }

    /* If not found anywhere, the symbol is unbound. */
    return make_cell_error(
        fmt_err("Unbound symbol: '%s'", k->sym),
        VALUE_ERR);
}


/* Place a Cell* value into a local environment. */
void lex_put_local(Lex* e, const Cell* k, const Cell* v)
{
    if (!e || !k || !v || k->type != CELL_SYMBOL) {
        fprintf(stderr, "lex_put: invalid arguments\n");
        return;
    }
    /* Check if we need to reallocate. */
    if (e->local->count == e->local->capacity) {
        e->local->capacity *= 2; /* Double the capacity */
        e->local->syms = GC_REALLOC(e->local->syms, sizeof(char*) * e->local->capacity);
        e->local->vals = GC_REALLOC(e->local->vals, sizeof(Cell*) * e->local->capacity);
        if (!e->local->syms || !e->local->vals) {
            fprintf(stderr, "ENOMEM: symbol_table_put failed\n");
            exit(EXIT_FAILURE);
        }
    }
    /* Check if symbol already exists. */
    for (int i = 0; i < e->local->count; i++) {
        if (strcmp(e->local->syms[i], k->sym) == 0) {
            /* Replace the old value with v. */
            e->local->vals[i] = (Cell*)v;
            return;
        }
    }
    /* Symbol not found; append new entry. */
    e->local->count++;
    e->local->syms[e->local->count - 1] = k->sym;
    e->local->vals[e->local->count - 1] = (Cell*)v;
}


/* Place a Cell* value in the global environment. */
void lex_put_global(const Lex* e, const Cell* k, Cell* v)
{
    if (!e || !k || !v || k->type != CELL_SYMBOL) {
        fprintf(stderr, "lex_put: invalid arguments\n");
        return;
    }
    ht_set(e->global, k->sym, v);
}


/* Populate the CELL_PROC struct of a Cell* object for builtin procedures. */
Cell* lex_make_builtin(const char* name, Cell* (*func)(const Lex*, const Cell*))
{
    Cell* c = GC_MALLOC(sizeof(Cell));
    c->type = CELL_PROC;
    c->f_name = GC_strdup(name);
    c->builtin = func;
    c->is_builtin = true;
    return c;
}


/* Populate the CELL_PROC struct of a Cell* object for a named lambda procedure. */
Cell* lex_make_named_lambda(char* name, Cell* formals, Cell* body, Lex* env)
{
    Cell* c = GC_MALLOC(sizeof(Cell)); /* Allocate Cell struct. */
    c->type = CELL_PROC;
    c->lambda= GC_MALLOC(sizeof(lambda)); /* Allocate lambda struct. */
    c->lambda->l_name = name;
    c->lambda->formals = formals;
    c->lambda->body = body;
    c->lambda->env = env;
    c->is_builtin = false;
    return c;
}


/* Populate the CELL_PROC struct of a Cell* object for an anonymous lambda procedure. */
Cell* lex_make_lambda(Cell* formals, Cell* body, Lex* env)
{
    Cell* c = GC_MALLOC(sizeof(Cell));
    c->type = CELL_PROC;
    c->lambda= GC_MALLOC(sizeof(lambda));
    c->lambda->l_name = nullptr;
    c->lambda->formals = formals;
    c->lambda->body = body;
    c->lambda->env = env;
    c->is_builtin = false;
    return c;
}


/* Populate the CELL_PROC struct of a Cell* object for a named lambda procedure. */
Cell* lex_make_defmacro(char* name, Cell* formals, Cell* body, Lex* env)
{
    Cell* c = GC_MALLOC(sizeof(Cell)); /* Allocate Cell struct. */
    c->type = CELL_MACRO;
    c->lambda= GC_MALLOC(sizeof(lambda)); /* Allocate lambda struct. */
    c->lambda->l_name = name;
    c->lambda->formals = formals;
    c->lambda->body = body;
    c->lambda->env = env;
    c->is_builtin = false;
    return c;
}


/* Register a procedure in the global environment. */
void lex_add_builtin(const Lex* e, const char* name, Cell* (*func)(const Lex*, const Cell*))
{
    Cell* fn = lex_make_builtin(name, func);
    const Cell* k = make_cell_symbol(name);
    lex_put_global(e, k, fn);
}

Cell* builtin_print_env(const Lex* e, const Cell* a) {
    (void)a;
    debug_print_env((Lex*)e);
    return USP_Obj;
}


/* Register all builtin procedures in the global environment. */
void lex_add_builtins(const Lex* e)
{
    /* Basic arithmetic operators. */
    lex_add_builtin(e, "+", builtin_add);
    lex_add_builtin(e, "-", builtin_sub);
    lex_add_builtin(e, "*", builtin_mul);
    lex_add_builtin(e, "/", builtin_div);
    /* Numeric comparison operators. */
    lex_add_builtin(e, "=", builtin_eq_op);
    lex_add_builtin(e, ">", builtin_gt_op);
    lex_add_builtin(e, "<", builtin_lt_op);
    lex_add_builtin(e, ">=", builtin_gte_op);
    lex_add_builtin(e, "<=", builtin_lte_op);
    /* Numeric predicate procedures. */
    lex_add_builtin(e, "zero?", builtin_zero);
    lex_add_builtin(e, "positive?", builtin_positive);
    lex_add_builtin(e, "negative?", builtin_negative);
    lex_add_builtin(e, "odd?", builtin_odd);
    lex_add_builtin(e, "even?", builtin_even);
    /* Equality and equivalence comparators. */
    lex_add_builtin(e, "eq?", builtin_eq);
    lex_add_builtin(e, "eqv?", builtin_eqv);
    lex_add_builtin(e, "equal?", builtin_equal);
    /* Generic numeric operations. */
    lex_add_builtin(e, "abs", builtin_abs);
    lex_add_builtin(e, "expt", builtin_expt);
    lex_add_builtin(e, "remainder", builtin_remainder);
    lex_add_builtin(e, "modulo", builtin_modulo);
    lex_add_builtin(e, "quotient", builtin_quotient);
    lex_add_builtin(e, "max", builtin_max);
    lex_add_builtin(e, "min", builtin_min);
    lex_add_builtin(e, "floor", builtin_floor);
    lex_add_builtin(e, "ceiling", builtin_ceiling);
    lex_add_builtin(e, "round", builtin_round);
    lex_add_builtin(e, "truncate", builtin_truncate);
    lex_add_builtin(e, "numerator", builtin_numerator);
    lex_add_builtin(e, "denominator", builtin_denominator);
    lex_add_builtin(e, "rationalize", builtin_rationalize);
    lex_add_builtin(e, "square", builtin_square);
    lex_add_builtin(e, "sqrt", builtin_sqrt);
    lex_add_builtin(e, "exact-integer-sqrt", builtin_exact_integer_sqrt);
    lex_add_builtin(e, "exact", builtin_exact);
    lex_add_builtin(e, "inexact", builtin_inexact);
    /* Type identity predicate procedures. */
    lex_add_builtin(e, "number?", builtin_number_pred);
    lex_add_builtin(e, "boolean?", builtin_boolean_pred);
    lex_add_builtin(e, "null?", builtin_null_pred);
    lex_add_builtin(e, "pair?", builtin_pair_pred);
    lex_add_builtin(e, "list?", builtin_list_pred);
    lex_add_builtin(e, "procedure?", builtin_proc_pred);
    lex_add_builtin(e, "symbol?", builtin_sym_pred);
    lex_add_builtin(e, "string?", builtin_string_pred);
    lex_add_builtin(e, "char?", builtin_char_pred);
    lex_add_builtin(e, "vector?", builtin_vector_pred);
    lex_add_builtin(e, "bytevector?", builtin_bytevector_pred);
    lex_add_builtin(e, "port?", builtin_port_pred);
    lex_add_builtin(e, "eof-object?", builtin_eof_pred);
    /* Numeric identity predicate procedures. */
    lex_add_builtin(e, "exact?", builtin_exact_pred);
    lex_add_builtin(e, "inexact?", builtin_inexact_pred);
    lex_add_builtin(e, "complex?", builtin_complex);
    lex_add_builtin(e, "real?", builtin_real);
    lex_add_builtin(e, "rational?", builtin_rational);
    lex_add_builtin(e, "integer?", builtin_integer);
    lex_add_builtin(e, "exact-integer?", builtin_exact_integer);
    lex_add_builtin(e, "bigint?", builtin_bigint);
    lex_add_builtin(e, "bigfloat?", builtin_bigfloat);
    lex_add_builtin(e, "infinite?", builtin_infinite);
    lex_add_builtin(e, "finite?", builtin_finite);
    lex_add_builtin(e, "nan?", builtin_nan);
    /* Boolean and logical procedures. */
    lex_add_builtin(e, "not", builtin_not);
    lex_add_builtin(e, "boolean=?", builtin_boolean);
    /* Pair/list procedures. */
    lex_add_builtin(e, "cons", builtin_cons);
    lex_add_builtin(e, "car", builtin_car);
    lex_add_builtin(e, "cdr", builtin_cdr);
    lex_add_builtin(e, "caar", builtin_caar);
    lex_add_builtin(e, "cadr", builtin_cadr);
    lex_add_builtin(e, "cdar", builtin_cdar);
    lex_add_builtin(e, "cddr", builtin_cddr);
    lex_add_builtin(e, "list", builtin_list);
    lex_add_builtin(e, "set-car!", builtin_set_car);
    lex_add_builtin(e, "set-cdr!", builtin_set_cdr);
    lex_add_builtin(e, "length", builtin_list_length);
    lex_add_builtin(e, "list-ref", builtin_list_ref);
    lex_add_builtin(e, "append", builtin_list_append);
    lex_add_builtin(e, "reverse", builtin_list_reverse);
    lex_add_builtin(e, "list-tail", builtin_list_tail);
    lex_add_builtin(e, "make-list", builtin_make_list);
    lex_add_builtin(e, "list-set!", builtin_list_set);
    lex_add_builtin(e, "memq", builtin_memq);
    lex_add_builtin(e, "memv", builtin_memv);
    lex_add_builtin(e, "member", builtin_member);
    lex_add_builtin(e, "assq", builtin_assq);
    lex_add_builtin(e, "assv", builtin_assv);
    lex_add_builtin(e, "assoc", builtin_assoc);
    lex_add_builtin(e, "list-copy", builtin_list_copy);
    lex_add_builtin(e, "filter", builtin_filter);
    lex_add_builtin(e, "foldl", builtin_foldl);
    lex_add_builtin(e, "zip", builtin_zip);
    /* Vector procedures. */
    lex_add_builtin(e, "vector", builtin_vector);
    lex_add_builtin(e, "vector-length", builtin_vector_length);
    lex_add_builtin(e, "vector-ref", builtin_vector_ref);
    lex_add_builtin(e, "make-vector", builtin_make_vector);
    lex_add_builtin(e, "list->vector", builtin_list_to_vector);
    lex_add_builtin(e, "vector->list", builtin_vector_to_list);
    lex_add_builtin(e, "vector-copy", builtin_vector_copy);
    lex_add_builtin(e, "vector-copy!", builtin_vector_copy_bang);
    lex_add_builtin(e, "vector->string", builtin_vector_to_string);
    lex_add_builtin(e, "string->vector", builtin_string_to_vector);
    lex_add_builtin(e, "vector-set!", builtin_vector_set_bang);
    lex_add_builtin(e, "vector-fill!", builtin_vector_fill_bang);
    lex_add_builtin(e, "vector-append", builtin_vector_append);
    /* Bytevector procedures. */
    lex_add_builtin(e, "bytevector", builtin_bytevector);
    lex_add_builtin(e, "bytevector-length", builtin_bytevector_length);
    lex_add_builtin(e, "bytevector-ref", builtin_bytevector_ref);
    lex_add_builtin(e, "bytevector-set!", builtin_bytevector_set_bang);
    lex_add_builtin(e, "make-bytevector", builtin_make_bytevector);
    lex_add_builtin(e, "bytevector-copy", builtin_bytevector_copy);
    lex_add_builtin(e, "bytevector-copy!", builtin_bytevector_copy_bang);
    lex_add_builtin(e, "bytevector-append", builtin_bytevector_append);
    lex_add_builtin(e, "utf8->string", builtin_utf8_string);
    lex_add_builtin(e, "string->utf8", builtin_string_utf8);
    /* Char procedures. */
    lex_add_builtin(e, "char->integer", builtin_char_to_int);
    lex_add_builtin(e, "integer->char", builtin_int_to_char);
    lex_add_builtin(e, "char=?", builtin_char_equal_pred);
    lex_add_builtin(e, "char<?", builtin_char_lt_pred);
    lex_add_builtin(e, "char<=?", builtin_char_lte_pred);
    lex_add_builtin(e, "char>?", builtin_char_gt_pred);
    lex_add_builtin(e, "char>=?", builtin_char_gte_pred);
    lex_add_builtin(e, "char-alphabetic?", builtin_char_alphabetic);
    lex_add_builtin(e, "char-whitespace?", builtin_char_whitespace);
    lex_add_builtin(e, "char-numeric?", builtin_char_numeric);
    lex_add_builtin(e, "char-upper-case?", builtin_char_upper_case);
    lex_add_builtin(e, "char-lower-case?", builtin_char_lower_case);
    lex_add_builtin(e, "char-upcase", builtin_char_upcase);
    lex_add_builtin(e, "char-downcase", builtin_char_downcase);
    lex_add_builtin(e, "char-foldcase", builtin_char_foldcase);
    lex_add_builtin(e, "digit-value", builtin_digit_value);
    lex_add_builtin(e, "char-ci=?", builtin_char_equal_ci);
    lex_add_builtin(e, "char-ci<?", builtin_char_lt_ci);
    lex_add_builtin(e, "char-ci<=?", builtin_char_lte_ci);
    lex_add_builtin(e, "char-ci>?", builtin_char_gt_ci);
    lex_add_builtin(e, "char-ci>=?", builtin_char_gte_ci);
    /* Symbol and string procedures. */
    lex_add_builtin(e, "features", builtin_features);
    lex_add_builtin(e, "symbol=?", builtin_symbol_equal_pred);
    lex_add_builtin(e, "symbol->string", builtin_symbol_to_string);
    lex_add_builtin(e, "string->symbol", builtin_string_to_symbol);
    lex_add_builtin(e, "string", builtin_string);
    lex_add_builtin(e, "string-length", builtin_string_length);
    lex_add_builtin(e, "string=?", builtin_string_eq_pred);
    lex_add_builtin(e, "string<?", builtin_string_lt_pred);
    lex_add_builtin(e, "string<=?", builtin_string_lte_pred);
    lex_add_builtin(e, "string>?", builtin_string_gt_pred);
    lex_add_builtin(e, "string>=?", builtin_string_gte_pred);
    lex_add_builtin(e, "string-append", builtin_string_append);
    lex_add_builtin(e, "string-ref", builtin_string_ref);
    lex_add_builtin(e, "make-string", builtin_make_string);
    lex_add_builtin(e, "string->list", builtin_string_list);
    lex_add_builtin(e, "list->string", builtin_list_string);
    lex_add_builtin(e, "substring", builtin_substring);
    lex_add_builtin(e, "string-copy", builtin_string_copy);
    lex_add_builtin(e, "string-copy!", builtin_string_copy_bang);
    lex_add_builtin(e, "string->number", builtin_string_number);
    lex_add_builtin(e, "number->string", builtin_number_string);
    lex_add_builtin(e, "string-downcase", builtin_string_downcase);
    lex_add_builtin(e, "string-upcase", builtin_string_upcase);
    lex_add_builtin(e, "string-foldcase", builtin_string_foldcase);
    lex_add_builtin(e, "string-ci=?", builtin_string_equal_ci);
    lex_add_builtin(e, "string-ci<?", builtin_string_lt_ci);
    lex_add_builtin(e, "string-ci<=?", builtin_string_lte_ci);
    lex_add_builtin(e, "string-ci>?", builtin_string_gt_ci);
    lex_add_builtin(e, "string-ci>=?", builtin_string_gte_ci);
    lex_add_builtin(e, "string-split", builtin_string_split);
    /* Control features. */
    lex_add_builtin(e, "eval", builtin_eval);
    lex_add_builtin(e, "apply", builtin_apply);
    lex_add_builtin(e, "map", builtin_map);
    lex_add_builtin(e, "vector-map", builtin_vector_map);
    lex_add_builtin(e, "string-map", builtin_string_map);
    lex_add_builtin(e, "for-each", builtin_foreach);
    lex_add_builtin(e, "vector-for-each", builtin_vector_foreach);
    lex_add_builtin(e, "string-for-each", builtin_string_foreach);
    lex_add_builtin(e, "load", builtin_load);
    lex_add_builtin(e, "command-line", builtin_command_line);
    lex_add_builtin(e, "exit", builtin_exit);
    /* Input/output and ports. */
    lex_add_builtin(e, "current-input-port", builtin_current_input_port);
    lex_add_builtin(e, "current-output-port", builtin_current_output_port);
    lex_add_builtin(e, "current-error-port", builtin_current_error_port);
    lex_add_builtin(e, "input-port?", builtin_input_port_pred);
    lex_add_builtin(e, "output-port?", builtin_output_port_pred);
    lex_add_builtin(e, "input-port-open?", builtin_input_port_open);
    lex_add_builtin(e, "output-port-open?", builtin_output_port_open);
    lex_add_builtin(e, "close-port", builtin_close_port);
    lex_add_builtin(e, "close-input-port", builtin_close_port); /* no distinction yet... */
    lex_add_builtin(e, "close-output-port", builtin_close_port);
    lex_add_builtin(e, "read-line", builtin_read_line);
    lex_add_builtin(e, "read-lines", builtin_read_lines);
    lex_add_builtin(e, "read-char", builtin_read_char);
    lex_add_builtin(e, "read-u8", builtin_read_char); /* Alias - no distinction between binary and text. */
    lex_add_builtin(e, "read-string", builtin_read_string);
    lex_add_builtin(e, "peek-char", builtin_peek_char);
    lex_add_builtin(e, "peek-u8", builtin_peek_char); /* Alias - no distinction between binary and text. */
    lex_add_builtin(e, "char-ready?", builtin_char_ready);
    lex_add_builtin(e, "u8-ready?", builtin_u8_ready);
    lex_add_builtin(e, "write-char", builtin_write_char);
    lex_add_builtin(e, "write-string", builtin_write_string);
    lex_add_builtin(e, "write-u8", builtin_write_u8);
    lex_add_builtin(e, "write-bytevector", builtin_write_bytevector);
    lex_add_builtin(e, "newline", builtin_newline);
    lex_add_builtin(e, "eof-object", builtin_eof);
    lex_add_builtin(e, "flush-output-port", builtin_flush_output_port);
    lex_add_builtin(e, "open-input-file", builtin_open_input_file);
    lex_add_builtin(e, "open-output-file", builtin_open_output_file);
    lex_add_builtin(e, "display", builtin_display);
    lex_add_builtin(e, "println", builtin_println);
    lex_add_builtin(e, "write", builtin_write);
    lex_add_builtin(e, "call-with-input-file", builtin_call_with_input_file);
    lex_add_builtin(e, "call-with-output-file", builtin_call_with_output_file);
    lex_add_builtin(e, "with-input-from-file", builtin_with_input_from_file);
    lex_add_builtin(e, "with-output-to-file", builtin_with_output_to_file);
    /* Error/debug procedures. */
    lex_add_builtin(e, "read-error?", builtin_read_error);
    lex_add_builtin(e, "file-error?", builtin_file_error);
    lex_add_builtin(e, "error-object?", builtin_error_object);
    lex_add_builtin(e, "raise", builtin_raise);
    lex_add_builtin(e, "gc-report", builtin_gc_report);
    lex_add_builtin(e, "print-env", builtin_print_env);
    /* Polymorphic procedures. */
    lex_add_builtin(e, "len", builtin_len);
    lex_add_builtin(e, "idx", builtin_idx);
    lex_add_builtin(e, "rev", builtin_rev);
}
