/*
 * 'src/predicates.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 -2026 Darren Kirby <darren@dragonbyte.ca>
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

#include <math.h>


/* -------------------------------------------------*
 *       Type identity predicate procedures         *
 * -------------------------------------------------*/


/* (number? obj)
 * Returns #t if obj is a number. Otherwise, #f is returned. */
Cell* builtin_number_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "number?");
    if (err) return err;

    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const int mask = CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_BIGINT|CELL_BIGFLOAT;
    if (a->cell[0]->type & mask) {
        return True_Obj;
    }
    return False_Obj;
}


/* (boolean? obj)
* The boolean? predicate returns #t if obj is either #t or #f and returns #f otherwise. */
Cell* builtin_boolean_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "boolean?");
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_BOOLEAN);
}


/* (null? obj)
 * Returns #t if obj is the empty list, otherwise returns #f. */
Cell* builtin_null_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "null?");
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_NIL);
}


/* (pair? obj)
 * The pair? predicate returns #t if obj is a pair, and otherwise returns #f*/
Cell *builtin_pair_pred(const Lex *e, const Cell *a)
{
    (void) e;
    Cell *err = CHECK_ARITY_EXACT(a, 1, "pair?");
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_PAIR);
}


/* (list? obj)
* Returns #t if obj is a list. Otherwise, it returns #f. By definition, all lists have finite length
* and are terminated by the empty list.*/
Cell* builtin_list_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "list?");
    if (err) return err;
    /* '() is a list */
    if (a->cell[0]->type == CELL_NIL) {
        return True_Obj;
    }

    /* If len is not -1, we can trust it's a proper list. */
    if (a->cell[0]->type == CELL_PAIR && a->cell[0]->len > 0) {
        return True_Obj;
    }

    /* If len is -1, this could be an improper list or a proper list
     * built with `cons`. We need to traverse it to find out. */
    const Cell* p = a->cell[0];
    while (p->type == CELL_PAIR) {
        p = p->cdr;
    }

    return p->type == CELL_NIL ? True_Obj : False_Obj;
}


/* (proc? obj)
 * Returns #t if obj is a lambda or builtin procedure. Otherwise, #f is returned. */
Cell* builtin_proc_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "procedure?");
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_PROC);
}


/* (symbol? obj)
 * Returns #t if obj is a symbol. Otherwise, #f is returned. */
Cell* builtin_sym_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "symbol?");
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_SYMBOL);
}


/* (string? obj)
 * Returns #t if obj is a string. Otherwise, #f is returned. */
Cell* builtin_string_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "string?");
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_STRING);
}


/* (char? obj)
 * Returns #t if obj is a char. Otherwise, #f is returned. */
Cell* builtin_char_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "char?");
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_CHAR);
}


/* (vector? obj)
 * Returns #t if obj is a vector. Otherwise, #f is returned. */
Cell* builtin_vector_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "vector?");
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_VECTOR);
}


/* (bytevector? obj)
 * Returns #t if obj is a bytevector. Otherwise, #f is returned. */
Cell* builtin_bytevector_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "bytevector?");
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_BYTEVECTOR);
}


/* (port? obj)
 * Returns #t if obj is a port. Otherwise, #f is returned. */
Cell* builtin_port_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "port?");
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_PORT);
}


/* (set? obj)
 * Returns #t if obj is a set. Otherwise, #f is returned. */
Cell* builtin_set_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "set?");
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_SET);
}


/* (map? obj)
 * Returns #t if obj is a map. Otherwise, #f is returned. */
Cell* builtin_map_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "map?");
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_MAP);
}


/* (eof-object? obj)
 * Returns #t if obj is the EOF! object. Otherwise, #f is returned. */
Cell* builtin_eof_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "eof-object?");
    if (err) return err;
    return make_cell_boolean(a->cell[0]->type == CELL_EOF);
}


/* ---------------------------------------*
 *      Numeric identity procedures       *
 * ---------------------------------------*/


/* 'exact?' -> CELL_BOOLEAN -  */
Cell* builtin_exact_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a,
        CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_BIGINT,
        "exact?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "exact?"))) { return err; }
    const Cell* z = a->cell[0];
    if (z->type == CELL_COMPLEX) {
        return make_cell_boolean(z->real->exact & z->imag->exact);
    }
    return make_cell_boolean(z->exact);
}


/* 'inexact?' -> CELL_BOOLEAN -  */
Cell* builtin_inexact_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a,
        CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_BIGINT,
        "inexact?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "inexact?"))) { return err; }

    if (a->cell[0]->type == CELL_COMPLEX) {
        return a->cell[0]->real->exact && a->cell[0]->imag->exact ?
        make_cell_boolean(false) : make_cell_boolean(true);
    }

    if (a->cell[0]->exact) {
        return False_Obj;
    }
    return True_Obj;
}


/* 'complex?' -> CELL_BOOLEAN - R7RS compliant */
Cell* builtin_complex(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "complex?");
    if (err) return err;

    /* All numbers are complex numbers. */
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const int mask = CELL_INTEGER|CELL_RATIONAL|CELL_REAL|CELL_COMPLEX|CELL_BIGINT;
    if (a->cell[0]->type & mask) {
        return True_Obj;
    }
    return False_Obj;
}


/* 'real?' -> CELL_BOOLEAN - R7RS compliant */
Cell* builtin_real(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "real?");
    if (err) return err;

    const Cell* arg = a->cell[0];
    switch (arg->type) {
        case CELL_INTEGER:
        case CELL_BIGINT:
        case CELL_RATIONAL:
        case CELL_REAL:
            return True_Obj;
        case CELL_COMPLEX:
            /* A complex number is real if its imaginary part is zero. */
            return make_cell_boolean(cell_is_real_zero(arg->imag));
        default:
            return False_Obj;
    }
}


/* 'rational?' -> CELL_BOOLEAN - R7RS compliant */
Cell* builtin_rational(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "rational?");
    if (err) return err;

    /* Non-numbers are not rational */
    if (check_arg_types(a,
        CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_BIGINT,
        "rational?")) {
        return False_Obj;
    }
    const Cell* arg = a->cell[0];

    /* A complex number is rational if its real part is exact
     * and its imaginary part is zero. */
    if (arg->type == CELL_COMPLEX) {
        return make_cell_boolean(arg->real->exact && cell_is_real_zero(arg->imag));
    }

    /* Exact numbers are rational */
    if (arg->exact) {
        return True_Obj;
    }

    /* Finite reals are rational */
    if (!arg->exact) {
        return make_cell_boolean(isfinite(cell_to_long_double(arg)));
    }

    return False_Obj;
}


/* 'integer?' -> CELL_BOOLEAN - R7RS compliant */
Cell* builtin_integer(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "integer?");
    if (err) return err;
    return make_cell_boolean(cell_is_integer(a->cell[0]));
}


/* 'exact-integer?' -> CELL_BOOLEAN - R7RS compliant */
Cell* builtin_exact_integer(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "exact-integer?");
    if (err) return err;

    const Cell* arg = a->cell[0];
    /* The value must be an integer AND the number must be exact. */
    return make_cell_boolean(cell_is_integer(arg) && arg->exact);
}


Cell* builtin_bigint(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "bigint?");
    if (err) return err;

    if (a->cell[0]->type == CELL_BIGINT) {
        return True_Obj;
    }
    return False_Obj;
}


Cell* builtin_bigrat(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "bigrat?");
    if (err) return err;

    if (a->cell[0]->type == CELL_BIGRAT) {
        return True_Obj;
    }
    return False_Obj;
}


Cell* builtin_bigfloat(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "bigfloat?");
    if (err) return err;

    if (a->cell[0]->type == CELL_BIGFLOAT) {
        return True_Obj;
    }
    return False_Obj;
}


/* ---------------------------------------*
 *       Numeric predicate procedures     *
 * ---------------------------------------*/

/* 'zero?' -> CELL_BOOLEAN - returns #t if arg is == 0 else #f */
Cell* builtin_zero(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a,
        CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_BIGINT,
        "zero?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "zero?"))) { return err; }

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
Cell* builtin_positive(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a,
        CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_BIGINT,
        "positive?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "positive?"))) { return err; }

    const Cell* val = a->cell[0];
    if (val->type == CELL_COMPLEX) {
        /* Must be a real number to be positive */
        if (!cell_is_real_zero(val->imag)) return make_cell_error(
            "positive?: expected real, got complex",
            VALUE_ERR);
        val = val->real;
    }

    return make_cell_boolean(cell_is_positive(val));
}


/* 'negative?' -> CELL_BOOLEAN - returns #t if arg is < 0 else #f */
Cell* builtin_negative(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a,
        CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_BIGINT,
        "negative?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "negative?"))) { return err; }

    const Cell* val = a->cell[0];
    if (val->type == CELL_COMPLEX) {
        /* Must be a real number to be negative */
        if (!cell_is_real_zero(val->imag)) return make_cell_error(
            "negative?: expected real, got complex",
            VALUE_ERR);
        val = val->real;
    }

    return make_cell_boolean(cell_is_negative(val));
}


/* 'odd?' -> CELL_BOOLEAN - returns #t if arg is an odd integer else #f */
Cell* builtin_odd(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a,
        CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_BIGINT,
        "odd?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "odd?"))) { return err; }

    if (!cell_is_integer(a->cell[0])) {
        return make_cell_error(
            "odd?: expected integer",
            VALUE_ERR);
    }

    return make_cell_boolean(cell_is_odd(a->cell[0]));
}


/* 'even?' -> CELL_BOOLEAN - returns #t if arg is an even integer else #f */
Cell* builtin_even(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a,
        CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_BIGINT,
        "even?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "even?"))) { return err; }

    if (!cell_is_integer(a->cell[0])) {
        return make_cell_error(
            "even?: expected integer",
            VALUE_ERR);
    }

    return make_cell_boolean(cell_is_even(a->cell[0]));
}


/* Note that these next two procedures check for the literal
 * boolean #true and #false objects, not just a check for 'truthiness'. */


/* (false? obj)
 * Returns true if obj is literal #false, else returns false. */
Cell* builtin_false_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "false?");
    if (err) { return err; }
    if (a->cell[0] == False_Obj) {
        return True_Obj;
    }
    return False_Obj;
}


/* (true? obj)
 * Returns true if obj is literal #true, else returns false. */
Cell* builtin_true_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "true?");
    if (err) { return err; }
    if (a->cell[0] == True_Obj) {
        return True_Obj;
    }
    return False_Obj;
}
