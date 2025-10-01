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


/* -------------------------------------------------*
 *       Type identity predicate procedures         *
 * -------------------------------------------------*/

/* 'number?' -> VAL_BOOL - returns #t if obj is numeric, else #f  */
Cell* builtin_number_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX;
    if (a->cell[0]->type & mask) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

/* 'boolean?' -> VAL_BOOL  - returns #t if obj is either #t or #f
    and returns #f otherwise. */
Cell* builtin_boolean_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_BOOL);
}

/* 'null?' -> VAL_BOOL - return #t if obj is null, else #f */
Cell* builtin_null_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_NIL);
}

/* 'pair?' -> VAL_BOOL - return #t if obj is a pair, else #f */
Cell* builtin_pair_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_PAIR);
}

/* 'procedure?' -> VAL_BOOL - return #t if obj is a procedure, else #f */
Cell* builtin_proc_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_PROC);
}

/* 'symbol?' -> VAL_BOOL - return #t if obj is a symbol, else #f */
Cell* builtin_sym_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_SYM);
}

/* 'string?' -> VAL_BOOL - return #t if obj is a string, else #f */
Cell* builtin_string_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_STR);
}

/* 'char?' -> VAL_BOOL - return #t if obj is a char, else #f */
Cell* builtin_char_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_CHAR);
}

/* 'vector?' -> VAL_BOOL - return #t if obj is a vector, else #f */
Cell* builtin_vector_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_VEC);
}

/* 'bytevector?' -> VAL_BOOL - return #t if obj is a byte vector, else #f */
Cell* builtin_byte_vector_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_BYTEVEC);
}

/* 'port?' -> VAL_BOOL - return #t if obj is a port, else #f */
Cell* builtin_port_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_PORT);
}

/* 'eof-object?' -> VAL_BOOL - return #t if obj is an eof, else #f */
Cell* builtin_eof_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_EOF);
}

/* ---------------------------------------*
 *      Numeric identity procedures       *
 * ---------------------------------------*/

/* 'exact?' -> VAL_BOOL -  */
Cell* builtin_exact_pred(Lex *e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == VAL_COMPLEX) {
        if (a->cell[0]->exact && a->cell[1]->exact) {
            return make_val_bool(1);
        }
        return make_val_bool(0);
    }
    return make_val_bool(a->cell[0]->exact);
}

/* 'inexact?' -> VAL_BOOL -  */
Cell* builtin_inexact_pred(Lex *e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == VAL_COMPLEX) {
        if (a->cell[0]->exact && a->cell[1]->exact) {
            return make_val_bool(0);
        }
        return make_val_bool(1);
    }
     if (a->cell[0]->exact) {
         return make_val_bool(0);
     }
    return make_val_bool(1);
}

/* 'complex?' -> VAL_BOOL - R7RS compliant */
Cell* builtin_complex(Lex *e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    /* All numbers are complex numbers. */
    const int mask = VAL_INT|VAL_RAT|VAL_REAL|VAL_COMPLEX;
    if (a->cell[0]->type & mask) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

/* 'real?' -> VAL_BOOL - R7RS compliant */
Cell* builtin_real(Lex *e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const Cell* arg = a->cell[0];
    switch (arg->type) {
        case VAL_INT:
        case VAL_RAT:
        case VAL_REAL:
            return make_val_bool(1);
        case VAL_COMPLEX:
            /* A complex number is real if its imaginary part is zero. */
            return make_val_bool(cell_is_real_zero(arg->imag));
        default:
            return make_val_bool(0);
    }
}

/* 'rational?' -> VAL_BOOL - R7RS compliant */
Cell* builtin_rational(Lex *e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const Cell* arg = a->cell[0];
    switch (arg->type) {
        case VAL_INT:
        case VAL_RAT:
        case VAL_REAL:
            /* all reals are inherently rational. */
            return make_val_bool(1);
        case VAL_COMPLEX:
            /* A complex number is rational if its imaginary part is zero. */
            return make_val_bool(cell_is_real_zero(arg->imag));
        default:
            return make_val_bool(0);
    }
}

/* 'integer?' -> VAL_BOOL - R7RS compliant */
Cell* builtin_integer(Lex *e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(cell_is_integer(a->cell[0]));
}

/* 'exact-integer?' -> VAL_BOOL - R7RS compliant */
Cell* builtin_exact_integer(Lex *e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const Cell* arg = a->cell[0];
    /* The value must be an integer AND the number must be exact. */
    return make_val_bool(cell_is_integer(arg) && arg->exact);
}

/* ---------------------------------------*
 *       Numeric predicate procedures     *
 * ---------------------------------------*/

/* 'zero?' -> VAL_BOOL - returns #t if arg is == 0 else #f */
Cell* builtin_zero(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const Cell* arg = a->cell[0];
    bool is_zero;

    if (arg->type == VAL_COMPLEX) {
        is_zero = cell_is_real_zero(arg->real) && cell_is_real_zero(arg->imag);
    } else {
        is_zero = cell_is_real_zero(arg);
    }
    return make_val_bool(is_zero);
}

/* 'positive?' -> VAL_BOOL - returns #t if arg is > 0 else #f */
Cell* builtin_positive(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    return make_val_bool(cell_is_positive(a->cell[0]));
}

/* 'negative?' -> VAL_BOOL - returns #t if arg is < 0 else #f */
Cell* builtin_negative(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    return make_val_bool(cell_is_negative(a->cell[0]));
}

/* 'odd?' -> VAL_BOOL - returns #t if arg is an odd integer else #f */
Cell* builtin_odd(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    return make_val_bool(cell_is_odd(a->cell[0]));
}

/* 'even?' -> VAL_BOOL - returns #t if arg is an even integer else #f */
Cell* builtin_even(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    return make_val_bool(cell_is_even(a->cell[0]));
}

