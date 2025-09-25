#ifndef COZENAGE_OPS_H
#define COZENAGE_OPS_H

#include "environment.h"

/* Basic arithmetic operators */
Cell* builtin_add(Lex* e, Cell* a);
Cell* builtin_sub(Lex* e, Cell* a);
Cell* builtin_mul(Lex* e, Cell* a);
Cell* builtin_div(Lex* e, Cell* a);
/* Comparison operators */
Cell* builtin_eq_op(Lex* e, Cell* a);
Cell* builtin_gt_op(Lex* e, Cell* a);
Cell* builtin_lt_op(Lex* e, Cell* a);
Cell* builtin_gte_op(Lex* e, Cell* a);
Cell* builtin_lte_op(Lex* e, Cell* a);
/* Numeric predicate procedures */
Cell* builtin_zero(Lex* e, Cell* a);
Cell* builtin_positive(Lex* e, Cell* a);
Cell* builtin_negative(Lex* e, Cell* a);
Cell* builtin_odd(Lex* e, Cell* a);
Cell* builtin_even(Lex* e, Cell* a);
/* Special forms */
Cell* builtin_quote(Lex* e, Cell* a);
Cell* builtin_define(Lex* e, Cell* a);
Cell* builtin_if(Lex* e, Cell* a);
Cell* builtin_when(Lex* e, Cell* a);
Cell* builtin_unless(Lex* e, Cell* a);
Cell* builtin_cond(Lex* e, Cell* a);
Cell* builtin_else(Lex* e, Cell* a);
Cell* builtin_import(Lex* e, Cell* a);
/* Equality and equivalence comparators */
Cell* builtin_equal(Lex* e, Cell* a);
Cell* builtin_eqv(Lex* e, Cell* a);
Cell* builtin_eq(Lex* e, Cell* a);
/* Generic numeric operations */
Cell* builtin_abs(Lex* e, Cell* a);
Cell* builtin_expt(Lex* e, Cell* a);
Cell* builtin_modulo(Lex* e, Cell* a);
Cell* builtin_quotient(Lex* e, Cell* a);
Cell* builtin_remainder(Lex* e, Cell* a);
Cell* builtin_lcm(Lex* e, Cell* a);
Cell* builtin_gcd(Lex* e, Cell* a);
Cell* builtin_max(Lex* e, Cell* a);
Cell* builtin_min(Lex* e, Cell* a);
Cell* builtin_floor(Lex* e, Cell* a);
Cell* builtin_ceiling(Lex* e, Cell* a);
Cell* builtin_round(Lex* e, Cell* a);
Cell* builtin_truncate(Lex* e, Cell* a);
Cell* builtin_numerator(Lex* e, Cell* a);
Cell* builtin_denominator(Lex* e, Cell* a);
Cell* builtin_square(Lex* e, Cell* a);
Cell* builtin_exact(Lex* e, Cell* a);
Cell* builtin_inexact(Lex* e, Cell* a);
/* Type identity predicate procedures */
Cell* builtin_number_pred(Lex* e, Cell* a);
Cell* builtin_boolean_pred(Lex* e, Cell* a);
Cell* builtin_null_pred(Lex* e, Cell* a);
Cell* builtin_pair_pred(Lex* e, Cell* a);
Cell* builtin_proc_pred(Lex* e, Cell* a);
Cell* builtin_sym_pred(Lex* e, Cell* a);
Cell* builtin_string_pred(Lex* e, Cell* a);
Cell* builtin_char_pred(Lex* e, Cell* a);
Cell* builtin_vector_pred(Lex* e, Cell* a);
Cell* builtin_byte_vector_pred(Lex* e, Cell* a);
Cell* builtin_port_pred(Lex* e, Cell* a);
Cell* builtin_eof_pred(Lex* e, Cell* a);
/* Numeric identity procedures */
Cell* builtin_exact_pred(Lex *e, Cell* a);
Cell* builtin_inexact_pred(Lex *e, Cell* a);
Cell* builtin_complex(Lex *e, Cell* a);
Cell* builtin_real(Lex *e, Cell* a);
Cell* builtin_rational(Lex *e, Cell* a);
Cell* builtin_integer(Lex *e, Cell* a);
Cell* builtin_exact_integer(Lex *e, Cell* a);
/* Boolean and logical procedures */
Cell* builtin_not(Lex* e, Cell* a);
Cell* builtin_boolean(Lex* e, Cell* a);
Cell* builtin_or(Lex* e, Cell* a);
Cell* builtin_and(Lex* e, Cell* a);
/* pair/list constructors, selectors, and procedures */
Cell* builtin_cons(Lex* e, Cell* a);
Cell* builtin_car(Lex* e, Cell* a);
Cell* builtin_cdr(Lex* e, Cell* a);
Cell* builtin_list(Lex* e, Cell* a);
Cell* builtin_list_length(Lex* e, Cell* a);
Cell* builtin_list_ref(Lex* e, Cell* a);
Cell* builtin_list_append(Lex* e, Cell* a);
Cell* builtin_list_reverse(Lex* e, Cell* a);
Cell* builtin_list_tail(Lex* e, Cell* a);
/* Vector constructors, selectors, and procedures */
Cell* builtin_vector(Lex* e, Cell* a);
Cell* builtin_vector_length(Lex* e, Cell* a);
Cell* builtin_vector_ref(Lex* e, Cell* a);
Cell* builtin_make_vector(Lex* e, Cell* a);
Cell* builtin_list_to_vector(Lex* e, Cell* a);
Cell* builtin_vector_to_list(Lex* e, Cell* a);
/* Bytevector constructors, selectors,and procedures */
Cell* builtin_bytevector(Lex* e, Cell* a);
Cell* builtin_bytevector_length(Lex* e, Cell* a);
/* Char constructors, selectors, and procedures */
Cell* builtin_char_to_int(Lex* e, Cell* a);
Cell* builtin_int_to_char(Lex* e, Cell* a);
/* String constructors, selectors, and procedures */
Cell* builtin_string_to_symbol(Lex* e, Cell* a);
Cell* builtin_symbol_to_string(Lex* e, Cell* a);

#endif //COZENAGE_OPS_H
