/*
 * 'coz_bits_lib.c'
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

#include "coz_bits_lib.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>


/* Helper which returns a variable-width two's complement representation
 * of a signed long long */
char* format_twos_complement(const long long val) {
    /* Handle the zero case, which is special. */
    if (val == 0) {
        char* zero_str = malloc(2);
        strcpy(zero_str, "0");
        return zero_str;
    }

    /* Find the minimal number of bits (N) required. */
    int n_bits = 0;
    if (val > 0) {
        /* Find the smallest N such that val fits in N-1 bits. */
        for (int i = 1; i < 64; ++i) {
            if (val < (1LL << (i - 1))) {
                n_bits = i;
                break;
            }
        }
    } else {
        /* For negative numbers, we need a leading '1'.
         * We find the smallest N such that val fits in N bits. */
        for (int i = 1; i <= 64; ++i) {
            if (val >= -(1LL << (i - 1))) {
                n_bits = i;
                break;
            }
        }
    }

    /* Allocate memory for the string (N bits + 1 for null terminator). */
    char* bitstring = malloc(n_bits + 1);
    if (bitstring == NULL) return nullptr;

    /* Extract the lowest N bits from the value.
     * We work with the unsigned representation to avoid issues with
     * right-shifting negative numbers. */
    const unsigned long long u_val = (unsigned long long)val;
    for (int i = 0; i < n_bits; ++i) {
        /* Get the bit at position (n_bits - 1 - i) */
        const int bit_pos = n_bits - 1 - i;
        if (u_val >> bit_pos & 1) {
            bitstring[i] = '1';
        } else {
            bitstring[i] = '0';
        }
    }
    bitstring[n_bits] = '\0';
    return bitstring;
}

/*------------------------------------------------------------*
 *            (cozenage bits) library procedures              *
 * -----------------------------------------------------------*/

Cell* bits_right_shift(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    err = check_arg_types(a, CELL_INTEGER|CELL_SYMBOL);
    if (err) return err;

    const Cell* arg1 = a->cell[0];
    const Cell* arg2 = a->cell[1];

    bool bs = false;
    if (arg1->type == CELL_SYMBOL) {
        arg1 = bits_bitstring_to_int(e, make_sexpr_len1(arg1));
        bs = true;
    }
    const long long n = (long long)cell_to_long_double(arg1);
    const int shift_amount = (int)cell_to_long_double(arg2);
    Cell* result = make_cell_integer(n >> shift_amount);
    if (bs) {
        return bits_int_to_bitstring(e, make_sexpr_len1(result));
    }
    return result;
}

Cell* bits_left_shift(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    err = check_arg_types(a, CELL_INTEGER|CELL_SYMBOL);
    if (err) return err;

    const Cell* arg1 = a->cell[0];
    const Cell* arg2 = a->cell[1];

    bool bs = false;
    if (arg1->type == CELL_SYMBOL) {
        arg1 = bits_bitstring_to_int(e, make_sexpr_len1(arg1));
        bs = true;
    }
    const long long n = (long long)cell_to_long_double(arg1);
    const int shift_amount = (int)cell_to_long_double(arg2);
    Cell* result = make_cell_integer(n << shift_amount);
    if (bs) {
        return bits_int_to_bitstring(e, make_sexpr_len1(result));
    }
    return result;
}

Cell* bits_bitwise_and(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    err = check_arg_types(a, CELL_INTEGER|CELL_SYMBOL);
    if (err) return err;

    if (a->cell[0]->type == CELL_SYMBOL) {
        return make_cell_error("Bitstrings not implemented yet", GEN_ERR);
    }
    const long long lhs = (long long)cell_to_long_double(a->cell[0]);
    const long long rhs = (int)cell_to_long_double(a->cell[1]);
    return make_cell_integer(lhs & rhs);
}

Cell* bits_bitwise_or(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    err = check_arg_types(a, CELL_INTEGER|CELL_SYMBOL);
    if (err) return err;

    if (a->cell[0]->type == CELL_SYMBOL) {
        return make_cell_error("Bitstrings not implemented yet", GEN_ERR);
    }
    const long long lhs = (long long)cell_to_long_double(a->cell[0]);
    const long long rhs = (long long)cell_to_long_double(a->cell[1]);
    return make_cell_integer(lhs | rhs);
}

Cell* bits_bitwise_xor(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    err = check_arg_types(a, CELL_INTEGER|CELL_SYMBOL);
    if (err) return err;

    if (a->cell[0]->type == CELL_SYMBOL) {
        return make_cell_error("Bitstrings not implemented yet", GEN_ERR);
    }
    const long long lhs = (long long)cell_to_long_double(a->cell[0]);
    const long long rhs = (long long)cell_to_long_double(a->cell[1]);
    return make_cell_integer(lhs ^ rhs);
}

Cell* bits_bitwise_not(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_INTEGER|CELL_SYMBOL);
    if (err) return err;

    if (a->cell[0]->type == CELL_SYMBOL) {
        return make_cell_error("Bitstrings not implemented yet", GEN_ERR);
    }
    const long long val = (long long)cell_to_long_double(a->cell[0]);
    return make_cell_integer(~val);
}

Cell* bits_int_to_bitstring(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_INTEGER);
    if (err) return err;

    char* str = format_twos_complement(a->cell[0]->integer_v);
    /* 66 = 64 bit max size of long long + '\0' + 'b' prefix */
    char sym_str[66] = "b";
    strcat(sym_str, str);
    Cell* result = make_cell_symbol(sym_str);
    free(str);
    return result;
}

Cell* bits_bitstring_to_int(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_SYMBOL);
    if (err) return err;

    const char *binaryString = a->cell[0]->sym;
    const char *ros = binaryString + 1;
    char *end_ptr;
    if (ros[0] == '0') {
        const long long result = strtoll(ros, &end_ptr, 2);
        if (*end_ptr != '\0') {
            return make_cell_error("bitstring->int: invalid bitstring", VALUE_ERR);
        }
        return make_cell_integer(result);
    }
    const size_t len = strlen(ros);
    const long long positive_part = strtoll(ros + 1, &end_ptr, 2);
    if (*end_ptr != '\0') {
        return make_cell_error("bitstring->int: invalid bitstring", VALUE_ERR);
    }
    /* The value of the leading '1' (the negative part)
     * This is -(2^(len-1)) */
    const long long negative_part = -(1LL << (len - 1));

    return make_cell_integer(negative_part + positive_part);
}

void lex_add_coz_bits_lib(const Lex* e) {
    lex_add_builtin(e, ">>", bits_right_shift);
    lex_add_builtin(e, "<<", bits_left_shift);
    lex_add_builtin(e, "&", bits_bitwise_and);
    lex_add_builtin(e, "|", bits_bitwise_or);
    lex_add_builtin(e, "^", bits_bitwise_xor);
    lex_add_builtin(e, "~", bits_bitwise_not);
    lex_add_builtin(e, "bitstring->int", bits_bitstring_to_int);
    lex_add_builtin(e, "int->bitstring", bits_int_to_bitstring);
}
