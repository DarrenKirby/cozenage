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
#include <unicode/uchar.h>


/*-------------------------------------------------------*
 *      Char constructors, selectors, and procedures     *
 * ------------------------------------------------------*/

/* (char->integer char )
 * Given a Unicode character, char->integer returns an exact integer between 0 and #xD7FF or
 * between #xE000 and #x10FFFF which is equal to the Unicode scalar value of that character. Given
 * a non-Unicode character, it returns an exact integer greater than #x10FFFF. This is true
 * independent of whether the implementation uses the Unicode representation internally. */
Cell* builtin_char_to_int(const Lex* e, const Cell* a)
{
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
Cell* builtin_int_to_char(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_INTEGER);
    if (err) return err;

    const UChar32 val = (int)a->cell[0]->integer_v;
    if (val >= 0xD800 && val <= 0xDFFF) {
        return make_cell_error(
            "integer->char: invalid code point (surrogate)",
            VALUE_ERR);
    }
    if (val < 0 || val > 0x10FFFF) {
        return make_cell_error(
            "integer->char: invalid code point",
            VALUE_ERR);
    }
    return make_cell_char(val);
}

/* These procedures return #t if the results of passing their arguments to char->integer are
 * respectively equal, monotonically increasing, monotonically decreasing, monotonically
 * non-decreasing, or monotonically non-increasing. */

/* (char=? char1 char2 char3 ... ) */
Cell* builtin_char_equal_pred(const Lex* e, const Cell* a)
{
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
Cell* builtin_char_lt_pred(const Lex* e, const Cell* a)
{
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
Cell* builtin_char_lte_pred(const Lex* e, const Cell* a)
{
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
Cell* builtin_char_gt_pred(const Lex* e, const Cell* a)
{
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
Cell* builtin_char_gte_pred(const Lex* e, const Cell* a)
{
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

/* These procedures return #t if their arguments are alphabetic, numeric, whitespace, upper case,
 * or lower case characters, respectively, otherwise they return #f.
 *
 * Specifically, they must return #t when applied to characters with the Unicode properties
 * Alphabetic, Numeric Digit, White Space, Uppercase, and Lowercase respectively, and #f when
 * applied to any other Unicode characters. Note that many Unicode characters are alphabetic but
 * neither upper nor lower case. */

/* (char-alphabetic? char) */
Cell* builtin_char_alphabetic(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_CHAR) {
        return make_cell_error(
            "char-alphabetic?: arg 1 must be a char",
            TYPE_ERR);
    }
    return make_cell_boolean(u_isalpha(a->cell[0]->char_v));
}

/* (char-whitespace? char) */
Cell* builtin_char_whitespace(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_CHAR) {
        return make_cell_error(
            "char-whitespace?: arg 1 must be a char",
            TYPE_ERR);
    }
    return make_cell_boolean(u_isspace(a->cell[0]->char_v));
}

/* (char-numeric? char) */
Cell* builtin_char_numeric(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_CHAR) {
        return make_cell_error(
            "char-numeric?: arg 1 must be a char",
            TYPE_ERR);
    }
    return make_cell_boolean(u_isdigit(a->cell[0]->char_v));
}

/* (char-upper-case? letter) */
Cell* builtin_char_upper_case(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_CHAR) {
        return make_cell_error(
            "char-upper-case?: arg 1 must be a char",
TYPE_ERR);
    }
    return make_cell_boolean(u_isupper(a->cell[0]->char_v));
}


/* (char-lower-case? letter) */
Cell* builtin_char_lower_case(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_CHAR) {
        return make_cell_error(
            "char-lower-case?: arg 1 must be a char",
            TYPE_ERR);
    }
    return make_cell_boolean(u_islower(a->cell[0]->char_v));
}

/* (char-upcase char)
 * The char-upcase procedure, given an argument that is the lowercase part of a Unicode casing pair,
 * returns the uppercase member of the pair, provided that both characters are supported by the
 * Scheme implementation. Note that language-sensitive casing pairs are not used. If the argument is
 * not the lowercase member of such a pair, it is returned. */
Cell* builtin_char_upcase(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_CHAR) {
        return make_cell_error(
            "char-upcase: arg 1 must be a char",
            TYPE_ERR);
    }
    return make_cell_char(u_toupper(a->cell[0]->char_v));
}

/* (char-downcase char)
 * The char-downcase procedure, given an argument that is the uppercase part of a Unicode casing
 * pair, returns the lowercase member of the pair, provided that both characters are supported by
 * the Scheme implementation. Note that language-sensitive casing pairs are not used. If the
 * argument is not the uppercase member of such a pair, it is returned. */
Cell* builtin_char_downcase(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_CHAR) {
        return make_cell_error(
            "char-downcase: arg 1 must be a char",
            TYPE_ERR);
    }
    return make_cell_char(u_tolower(a->cell[0]->char_v));
}

/* (char-foldcase char)
 * The char-foldcase procedure applies the Unicode simple case-folding algorithm to its argument and
 * returns the result. Note that language-sensitive folding is not used. If the argument is an
 * uppercase letter, the result will be either a lowercase letter or the same as the argument if the
 * lowercase letter does not exist or is not supported by the implementation. */
Cell* builtin_char_foldcase(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_CHAR) {
        return make_cell_error(
            "char-foldcase: arg 1 must be a char",
            TYPE_ERR);
    }
    const unsigned char c = a->cell[0]->char_v;
    return make_cell_char(u_foldCase(c, U_FOLD_CASE_DEFAULT));
}


/* (digit-value char)
 * This procedure returns the numeric value (0 to 9) of its argument if it is a numeric digit (that
 * is, if char-numeric? returns #t), or #f on any other character. */
Cell* builtin_digit_value(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_CHAR) {
        return make_cell_error(
            "digit-value: arg 1 must be a char",
            TYPE_ERR);
    }

    const int32_t value = u_charDigitValue(a->cell[0]->char_v);

    if (value == -1) {
        return False_Obj;
    }
    return make_cell_integer(value);
}

/*  */
Cell* builtin_char_equal_ci(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        const Cell* the_char = a->cell[i];
        const Cell* the_char_fc = builtin_char_foldcase(e, make_sexpr_len1(the_char));
        cells[i] = make_cell_integer(the_char_fc->char_v);
    }

    const Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_eq_op(e, cell_sexpr);
}

/*  */
Cell* builtin_char_lt_ci(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        const Cell* the_char = a->cell[i];
        const Cell* the_char_fc = builtin_char_foldcase(e, make_sexpr_len1(the_char));
        cells[i] = make_cell_integer(the_char_fc->char_v);
    }

    const Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_lt_op(e, cell_sexpr);
}

/*  */
Cell* builtin_char_lte_ci(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        const Cell* the_char = a->cell[i];
        const Cell* the_char_fc = builtin_char_foldcase(e, make_sexpr_len1(the_char));
        cells[i] = make_cell_integer(the_char_fc->char_v);
    }

    const Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_lte_op(e, cell_sexpr);
}

/*  */
Cell* builtin_char_gt_ci(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        const Cell* the_char = a->cell[i];
        const Cell* the_char_fc = builtin_char_foldcase(e, make_sexpr_len1(the_char));
        cells[i] = make_cell_integer(the_char_fc->char_v);
    }

    const Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_gt_op(e, cell_sexpr);
}

/*  */
Cell* builtin_char_gte_ci(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_CHAR);
    if (err) return err;

    Cell** cells = GC_MALLOC(sizeof(Cell*) * a->count);;
    for (int i = 0; i < a->count; i++) {
        const Cell* the_char = a->cell[i];
        const Cell* the_char_fc = builtin_char_foldcase(e, make_sexpr_len1(the_char));
        cells[i] = make_cell_integer(the_char_fc->char_v);
    }

    const Cell* cell_sexpr = make_sexpr_from_array(a->count, cells);
    return builtin_gte_op(e, cell_sexpr);
}
