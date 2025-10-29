/*
 * 'chars.c'
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

#include "chars.h"
#include "types.h"
#include "comparators.h"
#include <gc/gc.h>


/*-------------------------------------------------------*
 *      Char constructors, selectors, and procedures     *
 * ------------------------------------------------------*/

/* (char->integer char )
 * Given a Unicode character, char->integer returns an exact integer between 0 and #xD7FF or
 * between #xE000 and #x10FFFF which is equal to the Unicode scalar value of that character. Given
 * a non-Unicode character, it returns an exact integer greater than #x10FFFF. This is true
 * independent of whether the implementation uses the Unicode representation internally. */
Cell* builtin_char_to_int(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_CHAR);
    if (err) return err;

    return make_cell_integer(a->cell[0]->char_v);
}

/* (integer->char n)
 * Given an exact integer that is the value returned by a character when char->integer is applied
 * to it, integer->char returns that character. */
Cell* builtin_int_to_char(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_INTEGER);
    if (err) return err;

    const UChar32 val = (int)a->cell[0]->integer_v;
    if (val >= 0xD800 && val <= 0xDFFF) {
        return make_cell_error("integer->char: invalid code point (surrogate)", VALUE_ERR);
    }
    if (val < 0 || val > 0x10FFFF) {
        return make_cell_error("integer->char: invalid code point", VALUE_ERR);
    }
    return make_cell_char(val);
}

/* These procedures return #t if the results of passing their arguments to char->integer are
 * respectively equal, monotonically increasing, monotonically decreasing, monotonically
 * non-decreasing, or monotonically non-increasing. */

/* (char=? char1 char2 char3 ... ) */
Cell* builtin_char_equal_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        cells[i] = make_cell_integer(a->cell[i]->char_v);
    }

    const Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_eq_op(e, cell_sexpr);
}

/* (char<? char1 char2 char3 ... ) */
Cell* builtin_char_lt_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        cells[i] = make_cell_integer(a->cell[i]->char_v);
    }

    const Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_lt_op(e, cell_sexpr);
}

/* (char<=? char1 char2 char3 ... )   */
Cell* builtin_char_lte_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        cells[i] = make_cell_integer(a->cell[i]->char_v);
    }

    const Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_lte_op(e, cell_sexpr);
}

/* (char>? char1 char2 char3 ... )   */
Cell* builtin_char_gt_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        cells[i] = make_cell_integer(a->cell[i]->char_v);
    }

    const Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_gt_op(e, cell_sexpr);
}

/* (char>=? char1 char2 char3 ... )   */
Cell* builtin_char_gte_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        cells[i] = make_cell_integer(a->cell[i]->char_v);
    }

    const Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_gte_op(e, cell_sexpr);
}
