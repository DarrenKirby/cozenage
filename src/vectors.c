/*
 * 'vectors.c'
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

#include "vectors.h"
#include "types.h"
#include <string.h>
#include <gc/gc.h>
#include <unicode/utf8.h>


/*-------------------------------------------------------*
 *     Vector constructors, selectors, and procedures    *
 * ------------------------------------------------------*/

/* 'vector' -> CELL_VECTOR - returns a vector of all arg objects */
Cell* builtin_vector(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;

    Cell *vec = make_cell_vector();
    for (int i = 0; i < a->count; i++) {
        cell_add(vec, a->cell[i]);
    }
    return vec;
}

/* 'vector-length' -> CELL_INTEGER- returns the number of members of arg */
Cell* builtin_vector_length(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_VECTOR);
    if (err) return err;

    return make_cell_integer(a->cell[0]->count);
}

Cell* builtin_vector_ref(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_VECTOR) {
        return make_cell_error("vector-ref: arg 1 must be a vector", TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error("vector-ref: arg 2 must be an integer", TYPE_ERR);
    }
    const int i = (int)a->cell[1]->integer_v;

    if (i >= a->cell[0]->count) {
        return make_cell_error("vector-ref: index out of bounds", INDEX_ERR);
    }
    return a->cell[0]->cell[i];
}

Cell* builtin_make_vector(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_INTEGER) {
        return make_cell_error("make-vector: arg 1 must be an integer", TYPE_ERR);
    }
    long long n = a->cell[0]->integer_v;
    if (n < 1) {
        return make_cell_error("make-vector: arg 1 must be non-negative", VALUE_ERR);
    }
    Cell* fill;
    if (a->count == 2) {
        fill = a->cell[1];
    } else {
        fill = make_cell_integer(0);
    }
    Cell *vec = make_cell_vector();
    for (int i = 0; i < n; i++) {
        cell_add(vec, fill);
    }
    return vec;
}

Cell* builtin_list_to_vector(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_PAIR) {
        return make_cell_error("list->vector: arg 1 must be a list", TYPE_ERR);
    }
    const int list_len = a->cell[0]->len;
    if (list_len == -1) {
        return make_cell_error("list->vector: arg 1 must be a proper list", TYPE_ERR);
    }
    const Cell* lst = a->cell[0];
    Cell *vec = make_cell_vector();
    for (int i = 0; i < list_len; i++) {
        cell_add(vec, lst->car);
        lst = lst->cdr;
    }
    return vec;
}

Cell* builtin_vector_to_list(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_VECTOR) {
        return make_cell_error("vector->list: arg 1 must be a vector", TYPE_ERR);
    }
    int start = 0;
    int end = a->cell[0]->count;

    if (a->count == 2) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error("vector->list: arg 2 must be an integer", TYPE_ERR);
        }
        start = (int)a->cell[1]->integer_v;
        if (start < 0) {
            return make_cell_error("vector->list: arg 2 must be non-negative", VALUE_ERR);
        }
    }

    if (a->count == 3) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error("vector->list: arg 2 must be an integer", TYPE_ERR);
        }
        start = (int)a->cell[1]->integer_v;
        end = (int)a->cell[2]->integer_v + 1;
        if (end < 0) {
            return make_cell_error("vector->list: arg 2 must be non-negative", VALUE_ERR);
        }
        if (end > a->cell[0]->count) {
            return make_cell_error("vector->list: index out of bounds", INDEX_ERR);
        }
    }

    Cell* result = make_cell_nil();
    const Cell* vec = a->cell[0];
    for (int i = end - 1; i >= start; i--) {
        result = make_cell_pair(vec->cell[i], result);
        result->len = end - i;
    }
    return result;
}

/* 'vector-copy' -> CELL_VECTOR - returns a newly allocated copy of arg vector */
Cell* builtin_vector_copy(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_VECTOR) {
        return make_cell_error("vector->copy: arg 1 must be a vector", TYPE_ERR);
    }
    int start = 0;
    int end = a->cell[0]->count;

    if (a->count == 2) {
        start = (int)a->cell[1]->integer_v;
    }
    if (a->count == 3) {
        start = (int)a->cell[1]->integer_v;
        end = (int)a->cell[2]->integer_v;
    }
    Cell* vec = make_cell_vector();
    for (int i = start; i < end; i++) {
        cell_add(vec, a->cell[0]->cell[i]);
    }
    return vec;
}

/* 'vector->string' -> CELL_STRING - returns a str containing all char members
 * of arg */
Cell* builtin_vector_to_string(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_VECTOR) {
        return make_cell_error("vector->string: arg must be a vector", TYPE_ERR);
    }
    int start = 0;
    int end = a->cell[0]->count;
    char* the_string = GC_MALLOC_ATOMIC(end * 4 + 1);
    if (a->count == 2) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error("vector->string: arg2 must be an integer", TYPE_ERR);
        }
        start = (int)a->cell[1]->integer_v;
    }
    if (a->count == 3) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error("vector->string: arg3 must be an integer", TYPE_ERR);
        }
        end = (int)a->cell[2]->integer_v;
    }

    int32_t j = 0;
    for (int i = start; i < end; i++) {
        const Cell* char_cell = a->cell[0]->cell[i];
        if (char_cell->type != CELL_CHAR) {
            return make_cell_error("vector->string: vector must have only chars as members", TYPE_ERR);
        }
        const UChar32 code_point = char_cell->char_v;
        U8_APPEND_UNSAFE(the_string, j, code_point);
    }
    the_string[j] = '\0';
    return make_cell_string(the_string);
}

/* 'string->vector' -> CELL_VECTOR - returns a vector of all chars in arg */
Cell* builtin_string_to_vector(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error("string->vector: arg1 must be a string", TYPE_ERR);
    }

    const char* the_string = a->cell[0]->str;
    const size_t string_byte_len = strlen(the_string);

    /* Get optional start/end character indices from args */
    int start_char_idx = 0;
    int end_char_idx = -1; /* Use -1 to signify 'to the end' */

    if (a->count >= 2) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error("string->vector: arg2 must be an integer", TYPE_ERR);
        }
        start_char_idx = (int)a->cell[1]->integer_v;
    }
    if (a->count == 3) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error("string->vector: arg3 must be an integer", TYPE_ERR);
        }
        end_char_idx = (int)a->cell[2]->integer_v;
    }

    Cell* vec = make_cell_vector();
    int32_t byte_idx = 0;
    int32_t char_idx = 0;
    UChar32 code_point;

    /* Advance to the starting character position. */
    while (char_idx < start_char_idx && byte_idx < (int)string_byte_len) {
        U8_NEXT_UNSAFE(the_string, byte_idx, code_point);
        char_idx++;
    }

    /* Grab characters until we reach the end. */
    while (byte_idx < (int)string_byte_len) {
        /* Stop if we've reached the user-specified end character index */
        if (end_char_idx != -1 && char_idx >= end_char_idx) {
            break;
        }

        U8_NEXT_UNSAFE(the_string, byte_idx, code_point);
        cell_add(vec, make_cell_char(code_point));
        char_idx++;
    }
    return vec;
}
