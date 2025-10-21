/*
 * 'comparators.c'
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

#include "comparators.h"
#include "types.h"
#include <string.h>


/* -----------------------------*
 *     Comparison operators     *
 * -----------------------------*/

/* Helper for '=' which recursively compares complex numbers */
static int complex_eq_op(const Lex* e, const Cell* lhs, const Cell* rhs) {
    const Cell* args_real = make_sexpr_len2(lhs->real, rhs->real);
    const Cell* args_imag = make_sexpr_len2(lhs->imag, rhs->imag);

    const Cell* real_result = builtin_eq_op(e, args_real);
    const Cell* imag_result = builtin_eq_op(e, args_imag);

    const int eq = real_result->boolean_v && imag_result->boolean_v;
    return eq;
}

/* '=' -> CELL_BOOLEAN - returns true if all arguments are equal. */
Cell* builtin_eq_op(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int the_same = 0;
        Cell* lhs = a->cell[i];
        Cell* rhs = a->cell[i+1];
        numeric_promote(&lhs, &rhs);

        switch (lhs->type) {
            case CELL_INTEGER:
                if (lhs->integer_v == rhs->integer_v) { the_same = 1; }
                break;
            case CELL_RATIONAL:
                if (lhs->den == rhs->den && lhs->num == rhs->num) { the_same = 1; }
                break;
            case CELL_REAL:
                if (lhs->real_v == rhs->real_v) { the_same = 1; }
                break;
            case CELL_COMPLEX:
                if (complex_eq_op(e, lhs, rhs)) { the_same = 1; }
                break;
            default: ; /* this will never run as the types are pre-checked, but without the linter complains */
        }
        if (!the_same) {
            return make_cell_boolean(0);
        }
    }
    return make_cell_boolean(1);
}

/* '>' -> CELL_BOOLEAN - returns true if each argument is greater than the one that follows. */
Cell* builtin_gt_op(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_REAL|CELL_RATIONAL);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int ok = 0;
        Cell* lhs = a->cell[i];
        Cell* rhs = a->cell[i+1];
        numeric_promote(&lhs, &rhs);
        switch (lhs->type) {
            case CELL_INTEGER: {
                if (lhs->integer_v > rhs->integer_v) { ok = 1; }
                break;
            }
            case CELL_REAL: {
                if (lhs->real_v > rhs->real_v) { ok = 1; }
                break;
            }
            case CELL_RATIONAL: {
                if (lhs->num * rhs->den > lhs->den * rhs->num) { ok = 1; }
                break;
            }
            default: ;
        }
        if (!ok) {
            return make_cell_boolean(0);
        }
    }
    return make_cell_boolean(1);
}

/* '<' -> CELL_BOOLEAN - returns true if each argument is less than the one that follows. */
Cell* builtin_lt_op(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_REAL|CELL_RATIONAL);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int ok = 0;
        Cell* lhs = a->cell[i];
        Cell* rhs = a->cell[i+1];
        numeric_promote(&lhs, &rhs);
        switch (lhs->type) {
            case CELL_INTEGER: {
                if (lhs->integer_v < rhs->integer_v) { ok = 1; }
                break;
            }
            case CELL_REAL: {
                if (lhs->real_v < rhs->real_v) { ok = 1; }
                break;
            }
            case CELL_RATIONAL: {
                if (lhs->num * rhs->den < lhs->den * rhs->num) { ok = 1; }
                break;
            }
            default: ;
        }
        if (!ok) {
            return make_cell_boolean(0);
        }
    }
    return make_cell_boolean(1);
}

/* '>=' -> CELL_BOOLEAN - */
Cell* builtin_gte_op(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_REAL|CELL_RATIONAL);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int ok = 0;
        Cell* lhs = a->cell[i];
        Cell* rhs = a->cell[i+1];
        numeric_promote(&lhs, &rhs);
        switch (lhs->type) {
            case CELL_INTEGER: {
                if (lhs->integer_v >= rhs->integer_v) { ok = 1; }
                break;
            }
            case CELL_REAL: {
                if (lhs->real_v >= rhs->real_v) { ok = 1; }
                break;
            }
            case CELL_RATIONAL: {
                if (lhs->num * rhs->den >= (lhs->den * rhs->num)) { ok = 1; }
                break;
            }
            default: ;
        }
        if (!ok) {
            return make_cell_boolean(0);
        }
    }
    return make_cell_boolean(1);
}

/* '<=' -> CELL_BOOLEAN - */
Cell* builtin_lte_op(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_REAL|CELL_RATIONAL);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int ok = 0;
        Cell* lhs = a->cell[i];
        Cell* rhs = a->cell[i+1];
        numeric_promote(&lhs, &rhs);
        switch (lhs->type) {
            case CELL_INTEGER: {
                if (lhs->integer_v <= rhs->integer_v) { ok = 1; }
                break;
            }
            case CELL_REAL: {
                if (lhs->real_v <= rhs->real_v) { ok = 1; }
                break;
            }
            case CELL_RATIONAL: {
                if (lhs->num * rhs->den <= lhs->den * rhs->num) { ok = 1; }
                break;
            }
            default: ;
        }
        if (!ok) {
            return make_cell_boolean(0);
        }
    }
    return make_cell_boolean(1);
}

/* ------------------------------------------*
*    Equality and equivalence comparators    *
* -------------------------------------------*/

/* 'eq?' -> CELL_BOOLEAN - Tests whether its two arguments are the exact same object
 * (pointer equality). Typically used for symbols and other non-numeric atoms.
 * May not give meaningful results for numbers or characters, since distinct but
 * equal values aren’t guaranteed to be the same object. */
Cell* builtin_eq(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    const Cell* x = a->cell[0];
    const Cell* y = a->cell[1];

    /* Strict pointer equality */
    return make_cell_boolean(x == y);
}

/* 'eqv?' -> CELL_BOOLEAN - Like 'eq?', but also considers numbers and characters
 * with the same value as equivalent. (eqv? 2 2) is true, even if those 2s are
 * not the same object. Use when: you want a general-purpose equality predicate
 * that works for numbers, characters, and symbols, but you don’t need deep
 * structural comparison. */
Cell* builtin_eqv(const Lex* e, const Cell* a) {
    /* FIXME: (eqv? 10 10+0i) should return #t
     * Need to use numeric promotion, or perhaps just
     * interpret 10+0i as 10 at parser level */
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    const Cell* x = a->cell[0];
    const Cell* y = a->cell[1];

    if (x->type != y->type) return make_cell_boolean(0);

    switch (x->type) {
        case CELL_BOOLEAN: return make_cell_boolean(x->boolean_v == y->boolean_v);
        case CELL_INTEGER:  return make_cell_boolean(x->integer_v == y->integer_v);
        case CELL_REAL: return make_cell_boolean(x->real_v == y->real_v);
        case CELL_RATIONAL:  return make_cell_boolean(x->den == y->den && x->num == y->num);
        case CELL_COMPLEX: return make_cell_boolean(complex_eq_op(e, x, y));
        case CELL_CHAR: return make_cell_boolean(x->char_v == y->char_v);
        default:       return make_cell_boolean(x == y); /* fall back to identity */
    }
}

/* Helper for equal? */
static Cell* val_equal(const Lex* e, Cell* x, Cell* y) {
    /* FIXME: (eqv? 10 10+0i) should return #t
     * Need to use numeric promotion, or perhaps just
     * interpret 10+0i as 10 at parser level */
    if (x->type != y->type) return make_cell_boolean(0);

    switch (x->type) {
        case CELL_BOOLEAN: return make_cell_boolean(x->boolean_v == y->boolean_v);
        case CELL_CHAR: return make_cell_boolean(x->char_v == y->char_v);
        case CELL_INTEGER:  return make_cell_boolean(x->integer_v == y->integer_v);
        case CELL_REAL: return make_cell_boolean(x->real_v == y->real_v);
        case CELL_RATIONAL:  return make_cell_boolean((x->den == y->den) && (x->num == y->num));
        case CELL_SYMBOL:  return make_cell_boolean(strcmp(x->sym, y->sym) == 0);
        case CELL_STRING:  return make_cell_boolean(strcmp(x->str, y->str) == 0);
        case CELL_COMPLEX: return make_cell_boolean(complex_eq_op(e, x, y));

        case CELL_PAIR:
        case CELL_SEXPR:
        case CELL_VECTOR:
            if (x->type == CELL_PAIR) {
                x = make_sexpr_from_list(x);
                y = make_sexpr_from_list(y);
            }
            if (x->count != y->count) return make_cell_boolean(0);
            for (int i = 0; i < x->count; i++) {
                const Cell* eq = val_equal(e, x->cell[i], y->cell[i]);
                if (!eq->boolean_v) { return make_cell_boolean(0); }
            }
            return make_cell_boolean(1);

        default:
            return make_cell_boolean(0);
    }
}

/* 'equal?' -> CELL_BOOLEAN - Tests structural (deep) equality, comparing contents
 * recursively in lists, vectors, and strings. (equal? '(1 2 3) '(1 2 3)) → true,
 * even though the two lists are distinct objects.
 * Use when: you want to compare data structures by content, not identity.*/
Cell* builtin_equal(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    return val_equal(e, a->cell[0], a->cell[1]);
}
