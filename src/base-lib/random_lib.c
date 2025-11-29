/*
 * 'random.c'
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

#include "types.h"
#include "cell.h"

#include <openssl/ssl.h>
#include <openssl/rand.h>


/* Random integer in [0, limit)
 * Implements the Lemire method. */
static unsigned int rand_uint(const uint32_t limit) {
    const uint32_t t = -limit % limit;

    union {
        uint32_t i;
        unsigned char c[sizeof(uint32_t)];
    } u;

    uint64_t m;
    uint32_t l;
    u.i = 0; /* This is just to shut the linter up. */

    do {
        /* Get a new random number on each iteration. */
        if (!RAND_bytes(u.c, sizeof(u.c))) {
            fprintf(stderr, "Can't get random bytes!\n");
            exit(1);
        }

        const uint32_t x = u.i;
        m = (uint64_t)x * (uint64_t)limit;
        l = (uint32_t)m;
    } while (l < t); /* Reject if the lower bits fall in the biased range. */

    /* The upper 32 bits of the product are the unbiased, scaled result. */
    return m >> 32;
}

#define RAND_DOUBLE_SCALE 9007199254740992.0 /* 2⁵³ */

/* Random double in [0.0, 1.0). */
static double rand_double() {
    union {
        uint64_t i;
        unsigned char c[sizeof(uint64_t)];
    } u;

    u.i = 0;
    if (!RAND_bytes(u.c, sizeof(u.c))) {
        fprintf(stderr, "Can't get random bytes!\n");
        exit(1);
    }
    /* 53 bits / 2**53 */
    return (double)(u.i >> 11) * (1.0/RAND_DOUBLE_SCALE);
}

//////// end of helpers

static Cell* random_randint(const Lex* e, const Cell* a)
{
    (void)e;
    uint32_t limit = UINT32_MAX;
    if (a->count == 1) {
        limit = a->cell[0]->integer_v;
    }
    return make_cell_integer(rand_uint(limit));
}

static Cell* random_randbl(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    return make_cell_real(rand_double());
}

/* Random double in [min, max). */
static Cell* random_uniform(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "rand-uniform");
    if (err) return err;
    err = check_arg_types(a, CELL_INTEGER|CELL_RATIONAL|CELL_REAL, "rand-uniform");
    if (err) return err;

    const long double min = cell_to_long_double(a->cell[0]);
    const long double max = cell_to_long_double(a->cell[1]);

    return make_cell_real(min + (max - min) * rand_double());
}

/* Implements a 'modern' version of the
 * Fisher-Yates shuffle. */
static Cell* random_shuffle(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "shuffle");
    if (err) return err;
    err = check_arg_types(a, CELL_PAIR|CELL_VECTOR|CELL_SEXPR, "shuffle");
    if (err) return err;

    Cell* arr;
    bool list = false;
    if (a->cell[0]->type == CELL_PAIR) {
        list = true;
        arr = make_sexpr_from_list(a->cell[0]);
    } else {
        arr = a->cell[0];
    }

    /* Quoted list. */
    if (a->cell[0]->type == CELL_SEXPR) {
        list = true;
    }

    const int32_t arr_size = arr->count;
    Cell* c_arr[arr_size];
    for (int i = 0; i < arr_size; i++) {
        c_arr[i] = arr->cell[i];
    }
    for (int i = arr_size - 1; i > 0; i--) {
        /* Pick a random index from 0 to i (inclusive). */
        uint32_t j = rand_uint(i + 1);

        /* Swap the element at i with the randomly chosen element at j. */
        Cell* tmp = c_arr[i];
        c_arr[i] = c_arr[j];
        c_arr[j] = tmp;
    }

    Cell* sexp = make_sexpr_from_array(arr_size, c_arr);
    if (list) {
        return make_list_from_sexpr(sexp);
    }
    sexp->type = CELL_VECTOR;
    return sexp;
}

static Cell* random_choice(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "rand-choice");
    if (err) return err;
    err = check_arg_types(a, CELL_PAIR|CELL_VECTOR|CELL_SEXPR, "rand-choice");
    if (err) return err;

    Cell* arr;
    if (a->cell[0]->type == CELL_PAIR) {
        arr = make_sexpr_from_list(a->cell[0]);
    } else {
        arr = a->cell[0];
    }

    const int32_t arr_size = arr->count;
    Cell* c_arr[arr_size];
    for (int i = 0; i < arr_size; i++) {
        c_arr[i] = arr->cell[i];
    }

    return c_arr[rand_uint(arr_size)];
}

static Cell* random_choices(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "rand-choices");
    if (err) return err;
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const int mask = CELL_PAIR|CELL_VECTOR|CELL_SEXPR;
    if (!(a->cell[0]->type & mask)) {
        return make_cell_error("rand-choices: arg1 must be a list or vector", TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error("rand-choices: arg2 must be an integer", TYPE_ERR);
    }

    Cell* arr;
    bool list = false;
    if (a->cell[0]->type == CELL_PAIR) {
        list = true;
        arr = make_sexpr_from_list(a->cell[0]);
    } else {
        arr = a->cell[0];
    }

    /* Quoted list. */
    if (a->cell[0]->type == CELL_SEXPR) {
        list = true;
    }

    const int32_t k = (int)a->cell[1]->integer_v;
    const int32_t arr_size = arr->count;
    Cell* c_arr[arr_size];
    for (int i = 0; i < arr_size; i++) {
        c_arr[i] = arr->cell[i];
    }

    Cell* result_arr = make_cell_sexpr();
    for (int i = 0; i < k; i++) {
        cell_add(result_arr, c_arr[rand_uint(arr_size)]);
    }

    if (list) {
        return make_list_from_sexpr(result_arr);
    }
    result_arr->type = CELL_VECTOR;
    return result_arr;
}

void cozenage_library_init(const Lex* e)
{
    lex_add_builtin(e, "rand-int", random_randint);
    lex_add_builtin(e, "rand-dbl", random_randbl);
    lex_add_builtin(e, "rand-uniform", random_uniform);
    lex_add_builtin(e, "shuffle", random_shuffle);
    lex_add_builtin(e, "rand-choice", random_choice);
    lex_add_builtin(e, "rand-choices", random_choices);
}
