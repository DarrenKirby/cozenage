/*
 * 'predicates.c'
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

#include "predicates.h"
#include "types.h"


/* -------------------------------------------------*
 *       Type identity predicate procedures         *
 * -------------------------------------------------*/

/* 'number?' -> CELL_BOOLEAN - returns #t if obj is numeric, else #f  */
Cell* builtin_number_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX;
    if (a->cell[0]->type & mask) {
        return make_cell_boolean(1);
    }
    return make_cell_boolean(0);
}

/* (boolean? obj)
* The boolean? predicate returns #t if obj is either #t or #f and returns #f otherwise. */
Cell* builtin_boolean_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_BOOLEAN);
}

/* 'null?' -> CELL_BOOLEAN - return #t if obj is null, else #f */
Cell* builtin_null_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_NIL);
}

/* 'pair?' -> CELL_BOOLEAN - return #t if obj is a pair, else #f */
Cell* builtin_pair_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_PAIR);
}

/* 'procedure?' -> CELL_BOOLEAN - return #t if obj is a procedure, else #f */
Cell* builtin_proc_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_PROC);
}

/* 'symbol?' -> CELL_BOOLEAN - return #t if obj is a symbol, else #f */
Cell* builtin_sym_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_SYMBOL);
}

/* 'string?' -> CELL_BOOLEAN - return #t if obj is a string, else #f */
Cell* builtin_string_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_STRING);
}

/* 'char?' -> CELL_BOOLEAN - return #t if obj is a char, else #f */
Cell* builtin_char_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_CHAR);
}

/* 'vector?' -> CELL_BOOLEAN - return #t if obj is a vector, else #f */
Cell* builtin_vector_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_VECTOR);
}

/* 'bytevector?' -> CELL_BOOLEAN - return #t if obj is a byte vector, else #f */
Cell* builtin_byte_vector_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_BYTEVECTOR);
}

/* 'port?' -> CELL_BOOLEAN - return #t if obj is a port, else #f */
Cell* builtin_port_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_PORT);
}

/* 'eof-object?' -> CELL_BOOLEAN - return #t if obj is an eof, else #f */
Cell* builtin_eof_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_EOF);
}

/* ---------------------------------------*
 *      Numeric identity procedures       *
 * ---------------------------------------*/

/* 'exact?' -> CELL_BOOLEAN -  */
Cell* builtin_exact_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == CELL_COMPLEX) {
        if (a->cell[0]->exact && a->cell[1]->exact) {
            return make_cell_boolean(1);
        }
        return make_cell_boolean(0);
    }
    return make_cell_boolean(a->cell[0]->exact);
}

/* 'inexact?' -> CELL_BOOLEAN -  */
Cell* builtin_inexact_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == CELL_COMPLEX) {
        if (a->cell[0]->exact && a->cell[1]->exact) {
            return make_cell_boolean(0);
        }
        return make_cell_boolean(1);
    }
     if (a->cell[0]->exact) {
         return make_cell_boolean(0);
     }
    return make_cell_boolean(1);
}

/* 'complex?' -> CELL_BOOLEAN - R7RS compliant */
Cell* builtin_complex(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    /* All numbers are complex numbers. */
    const int mask = CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX;
    if (a->cell[0]->type & mask) {
        return make_cell_boolean(1);
    }
    return make_cell_boolean(0);
}

/* 'real?' -> CELL_BOOLEAN - R7RS compliant */
Cell* builtin_real(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const Cell* arg = a->cell[0];
    switch (arg->type) {
        case CELL_INTEGER:
        case CELL_RATIONAL:
        case CELL_REAL:
            return make_cell_boolean(1);
        case CELL_COMPLEX:
            /* A complex number is real if its imaginary part is zero. */
            return make_cell_boolean(cell_is_real_zero(arg->imag));
        default:
            return make_cell_boolean(0);
    }
}

/* 'rational?' -> CELL_BOOLEAN - R7RS compliant */
Cell* builtin_rational(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const Cell* arg = a->cell[0];
    switch (arg->type) {
        case CELL_INTEGER:
        case CELL_RATIONAL:
        case CELL_REAL:
            /* all reals are inherently rational. */
            return make_cell_boolean(1);
        case CELL_COMPLEX:
            /* A complex number is rational if its imaginary part is zero. */
            return make_cell_boolean(cell_is_real_zero(arg->imag));
        default:
            return make_cell_boolean(0);
    }
}

/* 'integer?' -> CELL_BOOLEAN - R7RS compliant */
Cell* builtin_integer(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_cell_boolean(cell_is_integer(a->cell[0]));
}

/* 'exact-integer?' -> CELL_BOOLEAN - R7RS compliant */
Cell* builtin_exact_integer(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const Cell* arg = a->cell[0];
    /* The value must be an integer AND the number must be exact. */
    return make_cell_boolean(cell_is_integer(arg) && arg->exact);
}

/* ---------------------------------------*
 *       Numeric predicate procedures     *
 * ---------------------------------------*/

/* 'zero?' -> CELL_BOOLEAN - returns #t if arg is == 0 else #f */
Cell* builtin_zero(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const Cell* arg = a->cell[0];
    bool is_zero;

    if (arg->type == CELL_COMPLEX) {
        is_zero = cell_is_real_zero(arg->real) && cell_is_real_zero(arg->imag);
    } else {
        is_zero = cell_is_real_zero(arg);
    }
    return make_cell_boolean(is_zero);
}

/* 'positive?' -> CELL_BOOLEAN - returns #t if arg is > 0 else #f */
Cell* builtin_positive(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    return make_cell_boolean(cell_is_positive(a->cell[0]));
}

/* 'negative?' -> CELL_BOOLEAN - returns #t if arg is < 0 else #f */
Cell* builtin_negative(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    return make_cell_boolean(cell_is_negative(a->cell[0]));
}

/* 'odd?' -> CELL_BOOLEAN - returns #t if arg is an odd integer else #f */
Cell* builtin_odd(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    return make_cell_boolean(cell_is_odd(a->cell[0]));
}

/* 'even?' -> CELL_BOOLEAN - returns #t if arg is an even integer else #f */
Cell* builtin_even(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    return make_cell_boolean(cell_is_even(a->cell[0]));
}

