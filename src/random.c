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

#include "random.h"
#include "cell.h"
#include "special_forms.h"

#include <openssl/ssl.h>
#include <openssl/rand.h>


/* Random integer in [0, limit) */
unsigned int random_uint(const uint32_t limit) {
    union {
        uint32_t i;
        unsigned char c[sizeof(uint32_t)];
    } u;
    u.i = 0;
    if (!RAND_bytes(u.c, sizeof(u.c))) {
        fprintf(stderr, "Can't get random bytes!\n");
        exit(1);
    }

    const uint32_t t = -limit % limit;
    uint64_t m;
    uint32_t l;
    do {
        const uint32_t x = u.i;
        m = (uint64_t)x * (uint64_t)limit;
        l = (uint32_t)m;
    } while (l < t);
    return m >> 32;
}

/* Random double in [0.0, 1.0) */
double random_double() {
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
    return (u.i >> 11) * (1.0/9007199254740992.0);
}

//////// end of helpers

Cell* builtin_randint(const Lex* e, const Cell* a)
{
    (void)e;
    uint32_t limit = UINT32_MAX;
    if (a->count == 1) {
        limit = a->cell[0]->integer_v;
    }
    return make_cell_integer(random_uint(limit));
}

Cell* builtin_randbl(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    return make_cell_real(random_double());
}

Cell* builtin_shuffle(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_PAIR|CELL_VECTOR|CELL_SEXPR);
    if (err) return err;

    Cell* arr;
    bool list = false;
    if (a->cell[0]->type == CELL_PAIR) {
        list = true;
        arr = make_sexpr_from_list(a->cell[0]);
    } else {
        arr = a->cell[0];
    }

    /* Quoted list */
    if (a->cell[0]->type == CELL_SEXPR) {
        list = true;
    }

    const int32_t arr_size = arr->count;
    Cell* c_arr[arr_size];
    for (int i = 0; i < arr_size; i++) {
        c_arr[i] = arr->cell[i];
    }
    for (int i = 0; i < arr_size; i++) {
        uint32_t j = random_uint(arr->count);
        Cell* tmp = c_arr[i];
        c_arr[i] = c_arr[j];
        c_arr[j] = tmp;
    }
    Cell* sexp = make_sexpr_from_array(arr_size, c_arr);
    if (list) {
        return sexpr_to_list(sexp);
    }
    sexp->type = CELL_VECTOR;
    return sexp;
}
