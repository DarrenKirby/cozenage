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

    const int eq = real_result->b_val && imag_result->b_val;
    return eq;
}

/* '=' -> VAL_BOOL - returns true if all arguments are equal. */
Cell* builtin_eq_op(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int the_same = 0;
        Cell* lhs = a->cell[i];
        Cell* rhs = a->cell[i+1];
        numeric_promote(&lhs, &rhs);

        switch (lhs->type) {
            case VAL_INT:
                if (lhs->i_val == rhs->i_val) { the_same = 1; }
                break;
            case VAL_RAT:
                if (lhs->den == rhs->den && lhs->num == rhs->num) { the_same = 1; }
                break;
            case VAL_REAL:
                if (lhs->r_val == rhs->r_val) { the_same = 1; }
                break;
            case VAL_COMPLEX:
                if (complex_eq_op(e, lhs, rhs)) { the_same = 1; }
                break;
            default: ; /* this will never run as the types are pre-checked, but without the linter complains */
        }
        if (!the_same) {
            return make_val_bool(0);
        }
    }
    return make_val_bool(1);
}

/* '>' -> VAL_BOOL - returns true if each argument is greater than the one that follows. */
Cell* builtin_gt_op(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int ok = 0;
        Cell* lhs = a->cell[i];
        Cell* rhs = a->cell[i+1];
        numeric_promote(&lhs, &rhs);
        switch (lhs->type) {
            case VAL_INT: {
                if (lhs->i_val > rhs->i_val) { ok = 1; }
                break;
            }
            case VAL_REAL: {
                if (lhs->r_val > rhs->r_val) { ok = 1; }
                break;
            }
            case VAL_RAT: {
                if (lhs->num * rhs->den > lhs->den * rhs->num) { ok = 1; }
                break;
            }
            default: ;
        }
        if (!ok) {
            return make_val_bool(0);
        }
    }
    return make_val_bool(1);
}

/* '<' -> VAL_BOOL - returns true if each argument is less than the one that follows. */
Cell* builtin_lt_op(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int ok = 0;
        Cell* lhs = a->cell[i];
        Cell* rhs = a->cell[i+1];
        numeric_promote(&lhs, &rhs);
        switch (lhs->type) {
            case VAL_INT: {
                if (lhs->i_val < rhs->i_val) { ok = 1; }
                break;
            }
            case VAL_REAL: {
                if (lhs->r_val < rhs->r_val) { ok = 1; }
                break;
            }
            case VAL_RAT: {
                if (lhs->num * rhs->den < lhs->den * rhs->num) { ok = 1; }
                break;
            }
            default: ;
        }
        if (!ok) {
            return make_val_bool(0);
        }
    }
    return make_val_bool(1);
}

/* '>=' -> VAL_BOOL - */
Cell* builtin_gte_op(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int ok = 0;
        Cell* lhs = a->cell[i];
        Cell* rhs = a->cell[i+1];
        numeric_promote(&lhs, &rhs);
        switch (lhs->type) {
            case VAL_INT: {
                if (lhs->i_val >= rhs->i_val) { ok = 1; }
                break;
            }
            case VAL_REAL: {
                if (lhs->r_val >= rhs->r_val) { ok = 1; }
                break;
            }
            case VAL_RAT: {
                if (lhs->num * rhs->den >= (lhs->den * rhs->num)) { ok = 1; }
                break;
            }
            default: ;
        }
        if (!ok) {
            return make_val_bool(0);
        }
    }
    return make_val_bool(1);
}

/* '<=' -> VAL_BOOL - */
Cell* builtin_lte_op(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int ok = 0;
        Cell* lhs = a->cell[i];
        Cell* rhs = a->cell[i+1];
        numeric_promote(&lhs, &rhs);
        switch (lhs->type) {
            case VAL_INT: {
                if (lhs->i_val <= rhs->i_val) { ok = 1; }
                break;
            }
            case VAL_REAL: {
                if (lhs->r_val <= rhs->r_val) { ok = 1; }
                break;
            }
            case VAL_RAT: {
                if (lhs->num * rhs->den <= lhs->den * rhs->num) { ok = 1; }
                break;
            }
            default: ;
        }
        if (!ok) {
            return make_val_bool(0);
        }
    }
    return make_val_bool(1);
}

/* ------------------------------------------*
*    Equality and equivalence comparators    *
* -------------------------------------------*/

/* 'eq?' -> VAL_BOOL - Tests whether its two arguments are the exact same object
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
    return make_val_bool(x == y);
}

/* 'eqv?' -> VAL_BOOL - Like 'eq?', but also considers numbers and characters
 * with the same value as equivalent. (eqv? 2 2) is true, even if those 2s are
 * not the same object. Use when: you want a general-purpose equality predicate
 * that works for numbers, characters, and symbols, but you don’t need deep
 * structural comparison. */
Cell* builtin_eqv(const Lex* e, const Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    const Cell* x = a->cell[0];
    const Cell* y = a->cell[1];

    if (x->type != y->type) return make_val_bool(0);

    switch (x->type) {
        case VAL_BOOL: return make_val_bool(x->b_val == y->b_val);
        case VAL_INT:  return make_val_bool(x->i_val == y->i_val);
        case VAL_REAL: return make_val_bool(x->r_val == y->r_val);
        case VAL_RAT:  return make_val_bool((x->den == y->den) && (x->num == y->num));
        case VAL_COMPLEX: return make_val_bool(complex_eq_op(e, x, y));
        case VAL_CHAR: return make_val_bool(x->c_val == y->c_val);
        default:       return make_val_bool(x == y); /* fall back to identity */
    }
}

/* Helper for equal? */
Cell* val_equal(const Lex* e, const Cell* x, const Cell* y) {
    if (x->type != y->type) return make_val_bool(0);

    switch (x->type) {
        case VAL_BOOL: return make_val_bool(x->b_val == y->b_val);
        case VAL_CHAR: return make_val_bool(x->c_val == y->c_val);
        case VAL_INT:  return make_val_bool(x->i_val == y->i_val);
        case VAL_REAL: return make_val_bool(x->r_val == y->r_val);
        case VAL_RAT:  return make_val_bool((x->den == y->den) && (x->num == y->num));
        case VAL_SYM:  return make_val_bool(strcmp(x->sym, y->sym) == 0);
        case VAL_STR:  return make_val_bool(strcmp(x->str, y->str) == 0);
        case VAL_COMPLEX: return make_val_bool(complex_eq_op(e, x, y));

        case VAL_SEXPR:
        case VAL_VEC:
            if (x->count != y->count) return make_val_bool(0);
            for (int i = 0; i < x->count; i++) {
                const Cell* eq = val_equal(e, x->cell[i], y->cell[i]);
                if (!eq->b_val) { return make_val_bool(0); }
            }
            return make_val_bool(1);

        default:
            return make_val_bool(0);
    }
}

/* 'equal?' -> VAL_BOOL - Tests structural (deep) equality, comparing contents
 * recursively in lists, vectors, and strings. (equal? '(1 2 3) '(1 2 3)) → true,
 * even though the two lists are distinct objects.
 * Use when: you want to compare data structures by content, not identity.*/
Cell* builtin_equal(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    return val_equal(e, a->cell[0], a->cell[1]);
}
