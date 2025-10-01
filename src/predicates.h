/*
 * 'predicates.h'
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

#ifndef COZENAGE_PREDICATES_H
#define COZENAGE_PREDICATES_H

#include "types.h"


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
/* Numeric predicate procedures */
Cell* builtin_zero(Lex* e, Cell* a);
Cell* builtin_positive(Lex* e, Cell* a);
Cell* builtin_negative(Lex* e, Cell* a);
Cell* builtin_odd(Lex* e, Cell* a);
Cell* builtin_even(Lex* e, Cell* a);

#endif //COZENAGE_PREDICATES_H
