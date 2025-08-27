#ifndef COZENAGE_OPS_H
#define COZENAGE_OPS_H

#include "environment.h"


l_val* builtin_add(l_env* e, l_val* a);
l_val* builtin_sub(l_env* e, l_val* a);
l_val* builtin_mul(l_env* e, l_val* a);
l_val* builtin_div(l_env* e, l_val* a);

l_val* builtin_eq_op(l_env* e, l_val* a);
l_val* builtin_gt_op(l_env* e, l_val* a);
l_val* builtin_lt_op(l_env* e, l_val* a);
l_val* builtin_gte_op(l_env* e, l_val* a);
l_val* builtin_lte_op(l_env* e, l_val* a);

l_val* builtin_zero(l_env* e, l_val* a);
l_val* builtin_positive(l_env* e, l_val* a);
l_val* builtin_negative(l_env* e, l_val* a);
l_val* builtin_odd(l_env* e, l_val* a);
l_val* builtin_even(l_env* e, l_val* a);

l_val* builtin_quote(l_env* e, l_val* a);
l_val* builtin_equal(l_env* e, l_val* a);
l_val* builtin_eqv(l_env* e, l_val* a);
l_val* builtin_eq(l_env* e, l_val* a);

l_val* builtin_abs(l_env* e, l_val* a);
l_val* builtin_expt(l_env* e, l_val* a);
l_val* builtin_modulo(l_env* e, l_val* a);
l_val* builtin_quotient(l_env* e, l_val* a);
l_val* builtin_remainder(l_env* e, l_val* a);

l_val* builtin_not(l_env* e, l_val* a);
l_val* builtin_boolean_pred(l_env* e, l_val* a);
l_val* builtin_boolean(l_env* e, l_val* a);
l_val* builtin_or(l_env* e, l_val* a);
l_val* builtin_and(l_env* e, l_val* a);

l_val* builtin_cons(l_env* e, l_val* a);
l_val* builtin_car(l_env* e, l_val* a);
l_val* builtin_cdr(l_env* e, l_val* a);
l_val* builtin_list(l_env* e, l_val* a);
l_val* builtin_list_length(l_env* e, l_val* a);
l_val* builtin_list_ref(l_env* e, l_val* a);

#endif //COZENAGE_OPS_H
