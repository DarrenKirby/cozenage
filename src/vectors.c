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

/* (vector obj ...)
* Returns a newly allocated vector whose elements contain the given arguments.
* It is analogous to list. */
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

/* (vector-length vector )
* Returns the number of elements in vector as an exact integer. */
Cell* builtin_vector_length(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_VECTOR);
    if (err) return err;

    return make_cell_integer(a->cell[0]->count);
}

/* (vector-ref vector k)
 * The vector-ref procedure returns the object and index k in vector */
Cell* builtin_vector_ref(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_VECTOR) {
        return make_cell_error(
            "vector-ref: arg 1 must be a vector",
            TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error(
            "vector-ref: arg 2 must be an integer",
            TYPE_ERR);
    }
    const int i = (int)a->cell[1]->integer_v;

    if (i >= a->cell[0]->count) {
        return make_cell_error(
            "vector-ref: index out of bounds",
            INDEX_ERR);
    }
    return a->cell[0]->cell[i];
}

/* (make-vector k)
 * (make-vector k fill)
 * Returns a newly allocated vector of k elements. If a second argument is given, then each element
 * is initialized to fill. Otherwise, the initial contents of each element is unspecified. */
Cell* builtin_make_vector(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_INTEGER) {
        return make_cell_error(
            "make-vector: arg 1 must be an integer",
            TYPE_ERR);
    }
    const long long n = a->cell[0]->integer_v;
    if (n < 1) {
        return make_cell_error(
            "make-vector: arg 1 must be non-negative",
            VALUE_ERR);
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

/* (list->vector list)
 * The list->vector procedure returns a newly created vector initialized to the elements of the list
 * 'list'. Order is preserved. */
Cell* builtin_list_to_vector(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_PAIR) {
        return make_cell_error(
            "list->vector: arg 1 must be a list",
            TYPE_ERR);
    }
    const int list_len = a->cell[0]->len;
    if (list_len == -1) {
        return make_cell_error(
            "list->vector: arg 1 must be a proper list",
            TYPE_ERR);
    }
    const Cell* lst = a->cell[0];
    Cell *vec = make_cell_vector();
    for (int i = 0; i < list_len; i++) {
        cell_add(vec, lst->car);
        lst = lst->cdr;
    }
    return vec;
}

/* (vector->list vector)
 * (vector->list vector start)
 * (vector->list vector start end)
 * The vector->list procedure returns a newly allocated list of the objects contained in the
 * elements of vector between start and end. Order is preserved. */
Cell* builtin_vector_to_list(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_VECTOR) {
        return make_cell_error(
            "vector->list: arg 1 must be a vector",
            TYPE_ERR);
    }
    int start = 0;
    int end = a->cell[0]->count;

    if (a->count == 2) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error(
                "vector->list: arg 2 must be an integer",
                TYPE_ERR);
        }
        start = (int)a->cell[1]->integer_v;
        if (start < 0) {
            return make_cell_error(
                "vector->list: arg 2 must be non-negative",
                VALUE_ERR);
        }
    }

    if (a->count == 3) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error(
                "vector->list: arg 2 must be an integer",
                TYPE_ERR);
        }
        start = (int)a->cell[1]->integer_v;
        end = (int)a->cell[2]->integer_v + 1;
        if (end < 0) {
            return make_cell_error(
                "vector->list: arg 2 must be non-negative",
                VALUE_ERR);
        }
        if (end > a->cell[0]->count) {
            return make_cell_error(
                "vector->list: index out of bounds",
                INDEX_ERR);
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

/* (vector-copy vector)
 * (vector-copy vector start)
 * (vector-copy vector start end)
 * Returns a newly allocated copy of the elements of the given vector between start and end. The
 * elements of the new vector are the same (in the sense of eqv?) as the elements of the old. */
Cell* builtin_vector_copy(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_VECTOR) {
        return make_cell_error(
            "vector->copy: arg 1 must be a vector",
            TYPE_ERR);
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

/* (vector->string vector)
 * (vector->string vector start)
 * (vector->string vector start end)
 * The vector->string procedure returns a newly allocated string of the objects contained in the
 * elements of vector between start and end. Order is preserved. */
Cell* builtin_vector_to_string(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_VECTOR) {
        return make_cell_error(
            "vector->string: arg must be a vector",
            TYPE_ERR);
    }
    int start = 0;
    int end = a->cell[0]->count;
    char* the_string = GC_MALLOC_ATOMIC(end * 4 + 1);
    if (a->count == 2) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error(
                "vector->string: arg2 must be an integer",
                TYPE_ERR);
        }
        start = (int)a->cell[1]->integer_v;
    }
    if (a->count == 3) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error(
                "vector->string: arg3 must be an integer",
                TYPE_ERR);
        }
        end = (int)a->cell[2]->integer_v;
    }

    int32_t j = 0;
    for (int i = start; i < end; i++) {
        const Cell* char_cell = a->cell[0]->cell[i];
        if (char_cell->type != CELL_CHAR) {
            return make_cell_error(
                "vector->string: vector must have only chars as members",
                TYPE_ERR);
        }
        const UChar32 code_point = char_cell->char_v;
        U8_APPEND_UNSAFE(the_string, j, code_point);
    }
    the_string[j] = '\0';
    return make_cell_string(the_string);
}

/* (string->vector string)
 * (string->vector string start )
 * (string->vector string start end)
 * It is an error if any element of vector between start and end is not a character. The
 * string->vector procedure returns a newly created vector initialized to the elements of the string
 * 'string' between start and end. Order is preserved. */
Cell* builtin_string_to_vector(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "string->vector: arg1 must be a string",
            TYPE_ERR);
    }

    const char* the_string = a->cell[0]->str;
    const size_t string_byte_len = strlen(the_string);

    /* Get optional start/end character indices from args */
    int start_char_idx = 0;
    int end_char_idx = -1; /* Use -1 to signify 'to the end' */

    if (a->count >= 2) {
        if (a->cell[1]->type != CELL_INTEGER) {
            return make_cell_error(
                "string->vector: arg2 must be an integer",
                TYPE_ERR);
        }
        start_char_idx = (int)a->cell[1]->integer_v;
    }
    if (a->count == 3) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error(
                "string->vector: arg3 must be an integer",
                TYPE_ERR);
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

/* (vector-set! vector k obj)
 * It is an error if k is not a valid index of vector. This procedure stores obj in the kth position
 * of vector */
Cell* builtin_vector_set_bang(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_VECTOR) {
        return make_cell_error(
            "vector->set!: arg must be a vector",
            TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error(
            "vector->set!: arg must be an integer",
            TYPE_ERR);
    }

    const long long idx = a->cell[1]->integer_v;
    const Cell* vec = a->cell[0];
    Cell* obj = a->cell[2];

    if (idx < 0 || idx >= a->cell[0]->count) {
        return make_cell_error(
            "vector->set!: index out of range",
            INDEX_ERR);
    }

    vec->cell[idx] = obj;
    return nullptr;
}

/* (vector-append vector ... )
 * Returns a newly allocated vector whose elements are the concatenation of the elements of the
 * given vectors. */
Cell* builtin_vector_append(const Lex* e, const Cell* a) {
    (void)e;
    /* No args returns an empty vector, not an error */
    if (a->count == 0) {
        return make_cell_vector();
    }
    /* Now that we know there's at least one arg, check that all args are vectors */
    Cell* err = check_arg_types(a, CELL_VECTOR);
    if (err) return err;

    /* One arg returns the arg */
    if (a->count == 1) {
        return a->cell[0];
    }

    /* Deal with multiple args */
    Cell* result = make_cell_vector();
    /* For each vector argument... */
    for (int i = 0; i < (int)a->count; i++) {
        const Cell* this_vec = a->cell[i];
        /* For each object in vector ... */
        for (int j = 0; j < (int)this_vec->count; j++) {
            cell_add(result, this_vec->cell[j]);
        }
    }
    return result;
}

Cell* builtin_vector_copy_bang(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    return make_cell_boolean(0);
}

/* (vector-fill! vector fill)
 * (vector-fill! vector fill start)
 * (vector-fill! vector fill start end)
 * The vector-fill! procedure stores fill in the elements of vector between start and end. */
Cell* builtin_vector_fill_bang(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 2, 4);
    if (err) return err;
    if (a->cell[0]->type != CELL_VECTOR) {
        return make_cell_error(
            "vector-fill!: arg 1 must be a vector",
            TYPE_ERR);
    }
    int start = 0;
    int end = a->cell[0]->count;
    Cell* fill = a->cell[1];
    if (a->count > 2) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error("vector-fill!: arg 3 must be an integer", TYPE_ERR);
        }
        start = (int)a->cell[2]->integer_v;
        if (start < 0 || start >= a->cell[0]->count) {
            return make_cell_error("vector-fill!: start out of range", INDEX_ERR);
        }
        if (a->count == 4) {
            if (a->cell[3]->type != CELL_INTEGER) {
                return make_cell_error("vector-fill!: arg 4 must be an integer", TYPE_ERR);
            }
            end = (int)a->cell[3]->integer_v;
            if (end < 0 || end >= a->cell[0]->count || end < start) {
                return make_cell_error("vector-fill!: end out of range", INDEX_ERR);
            }
        }
    }

    /* Args are good. Let's go! */
    for (int i = start; i < end; i++) {
        a->cell[0]->cell[i] = fill;
    }
    return nullptr;
}
