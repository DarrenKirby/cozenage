#ifndef COZENAGE_OPS_H
#define COZENAGE_OPS_H

#include "environment.h"


Cell* builtin_add(Lex* e, Cell* a);
Cell* builtin_sub(Lex* e, Cell* a);
Cell* builtin_mul(Lex* e, Cell* a);
Cell* builtin_div(Lex* e, Cell* a);

Cell* builtin_eq_op(Lex* e, Cell* a);
Cell* builtin_gt_op(Lex* e, Cell* a);
Cell* builtin_lt_op(Lex* e, Cell* a);
Cell* builtin_gte_op(Lex* e, Cell* a);
Cell* builtin_lte_op(Lex* e, Cell* a);

Cell* builtin_zero(Lex* e, Cell* a);
Cell* builtin_positive(Lex* e, Cell* a);
Cell* builtin_negative(Lex* e, Cell* a);
Cell* builtin_odd(Lex* e, Cell* a);
Cell* builtin_even(Lex* e, Cell* a);

Cell* builtin_quote(Lex* e, Cell* a);
Cell* builtin_equal(Lex* e, Cell* a);
Cell* builtin_eqv(Lex* e, Cell* a);
Cell* builtin_eq(Lex* e, Cell* a);

Cell* builtin_abs(Lex* e, Cell* a);
Cell* builtin_expt(Lex* e, Cell* a);
Cell* builtin_modulo(Lex* e, Cell* a);
Cell* builtin_quotient(Lex* e, Cell* a);
Cell* builtin_remainder(Lex* e, Cell* a);

Cell* builtin_lcm(Lex* e, Cell* a);
Cell* builtin_gcd(Lex* e, Cell* a);

Cell* builtin_not(Lex* e, Cell* a);
Cell* builtin_boolean_pred(Lex* e, Cell* a);
Cell* builtin_boolean(Lex* e, Cell* a);
Cell* builtin_or(Lex* e, Cell* a);
Cell* builtin_and(Lex* e, Cell* a);

Cell* builtin_cons(Lex* e, Cell* a);
Cell* builtin_car(Lex* e, Cell* a);
Cell* builtin_cdr(Lex* e, Cell* a);
Cell* builtin_list(Lex* e, Cell* a);
Cell* builtin_list_length(Lex* e, Cell* a);
Cell* builtin_list_ref(Lex* e, Cell* a);

#endif //COZENAGE_OPS_H
