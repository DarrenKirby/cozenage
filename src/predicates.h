/*
 * 'predicates.h'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 Darren Kirby <darren@dragonbyte.ca>
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

#ifndef COZENAGE_PREDICATES_H
#define COZENAGE_PREDICATES_H

#include "cell.h"


/* Type identity predicate procedures. */
Cell* builtin_number_pred(const Lex* e, const Cell* a);
Cell* builtin_boolean_pred(const Lex* e, const Cell* a);
Cell* builtin_null_pred(const Lex* e, const Cell* a);
Cell* builtin_pair_pred(const Lex* e, const Cell* a);
Cell* builtin_proc_pred(const Lex* e, const Cell* a);
Cell* builtin_list_pred(const Lex* e, const Cell* a);
Cell* builtin_sym_pred(const Lex* e, const Cell* a);
Cell* builtin_string_pred(const Lex* e, const Cell* a);
Cell* builtin_char_pred(const Lex* e, const Cell* a);
Cell* builtin_vector_pred(const Lex* e, const Cell* a);
Cell* builtin_bytevector_pred(const Lex* e, const Cell* a);
Cell* builtin_port_pred(const Lex* e, const Cell* a);
Cell* builtin_eof_pred(const Lex* e, const Cell* a);
/* Numeric identity predicate procedures. */
Cell* builtin_exact_pred(const Lex* e, const Cell* a);
Cell* builtin_inexact_pred(const Lex* e, const Cell* a);
Cell* builtin_complex(const Lex* e, const Cell* a);
Cell* builtin_real(const Lex* e, const Cell* a);
Cell* builtin_rational(const Lex* e, const Cell* a);
Cell* builtin_integer(const Lex* e, const Cell* a);
Cell* builtin_exact_integer(const Lex* e, const Cell* a);
Cell* builtin_bigint(const Lex* e, const Cell* a);
Cell* builtin_bigfloat(const Lex* e, const Cell* a);
/* Numeric predicate procedures. */
Cell* builtin_zero(const Lex* e, const Cell* a);
Cell* builtin_positive(const Lex* e, const Cell* a);
Cell* builtin_negative(const Lex* e, const Cell* a);
Cell* builtin_odd(const Lex* e, const Cell* a);
Cell* builtin_even(const Lex* e, const Cell* a);
/* Boolean object predicates. */
Cell* builtin_false_pred(const Lex* e, const Cell* a);
Cell* builtin_true_pred(const Lex* e, const Cell* a);

#endif //COZENAGE_PREDICATES_H
