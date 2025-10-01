/*
 * 'src/ops.h'
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

#ifndef COZENAGE_OPS_H
#define COZENAGE_OPS_H

#include "environment.h"


/* Basic arithmetic operators */
Cell* builtin_add(Lex* e, Cell* a);
Cell* builtin_sub(Lex* e, Cell* a);
Cell* builtin_mul(Lex* e, Cell* a);
Cell* builtin_div(Lex* e, Cell* a);

/* Numeric predicate procedures */
Cell* builtin_zero(Lex* e, Cell* a);
Cell* builtin_positive(Lex* e, Cell* a);
Cell* builtin_negative(Lex* e, Cell* a);
Cell* builtin_odd(Lex* e, Cell* a);
Cell* builtin_even(Lex* e, Cell* a);


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
/* Numeric identity predicate procedures */
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





/* Control features and list iteration procedures */
Cell* builtin_apply(Lex* e, Cell* a);
Cell* builtin_eval(Lex* e, Cell* a);
Cell* builtin_map(Lex* e, Cell* a);
Cell* builtin_filter(Lex* e, Cell* a);
Cell* builtin_foldl(Lex* e, Cell* a);


#endif //COZENAGE_OPS_H
