/*
 * 'src/types.c'
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
#include "numerics.h"
#include <gc.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <unicode/ustring.h>


/* Turn a single type into a string (for error reporting) */
const char* cell_type_name(const int t) {
    switch (t) {
        case CELL_INTEGER:     return "integer";
        case CELL_REAL:        return "float";
        case CELL_RATIONAL:    return "rational";
        case CELL_COMPLEX:     return "complex";
        case CELL_BOOLEAN:     return "bool";
        case CELL_SYMBOL:      return "symbol";
        case CELL_STRING:      return "string";
        case CELL_SEXPR:       return "sexpr";
        case CELL_NIL:         return "nil";
        case CELL_PROC:        return "procedure";
        case CELL_ERROR:       return "error";
        case CELL_PAIR:        return "pair";
        case CELL_VECTOR:      return "vector";
        case CELL_CHAR:        return "char";
        case CELL_BYTEVECTOR:  return "byte vector";
        case CELL_EOF:         return "eof";
        default:               return "unknown";
    }
}

/* Turn a mask (possibly multiple flags ORed together) into a string
   e.g. (CELL_INTEGER | CELL_REAL) -> "int|real" */
const char* cell_mask_types(const int mask) {
    static char buf[128];  /* static to return pointer safely */
    buf[0] = '\0';

    if (mask & CELL_INTEGER)     strcat(buf, "integer|");
    if (mask & CELL_REAL)        strcat(buf, "real|");
    if (mask & CELL_RATIONAL)    strcat(buf, "rational|");
    if (mask & CELL_COMPLEX)     strcat(buf, "complex|");
    if (mask & CELL_BOOLEAN)     strcat(buf, "bool|");
    if (mask & CELL_SYMBOL)      strcat(buf, "symbol|");
    if (mask & CELL_STRING)      strcat(buf, "string|");
    if (mask & CELL_SEXPR)       strcat(buf, "sexpr|");
    if (mask & CELL_NIL)         strcat(buf, "nil|");
    if (mask & CELL_PROC)        strcat(buf, "procedure|");
    if (mask & CELL_ERROR)       strcat(buf, "error|");
    if (mask & CELL_PAIR)        strcat(buf, "pair|");
    if (mask & CELL_VECTOR)      strcat(buf, "vector|");
    if (mask & CELL_CHAR)        strcat(buf, "char|");
    if (mask & CELL_BYTEVECTOR)  strcat(buf, "byte vector|");
    if (mask & CELL_EOF)         strcat(buf, "eof|");

    /* remove trailing '|' */
    const size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '|') {
        buf[len-1] = '\0';
    }
    return buf;
}

/*---------------------------------------------------------*
 *      Procedure argument arity and type validators       *
 * --------------------------------------------------------*/

/* Return NULL if all args are valid, else return a CELL_ERROR */
Cell* check_arg_types(const Cell* a, const int mask) {
    for (int i = 0; i < a->count; i++) {
        const Cell* arg = a->cell[i];

        /* bitwise AND: if arg->type isn't in mask, it's invalid */
        if (!(arg->type & mask)) {
            char buf[128];
            snprintf(buf, sizeof(buf),
                     "bad type at arg %d: got %s, expected %s",
                     i+1,
                     cell_type_name(arg->type),
                     cell_mask_types(mask));
            return make_cell_error(buf, TYPE_ERR);
        }
    }
    return nullptr;
}

Cell* check_arg_arity(const Cell* a, const int exact, const int min, const int max) {
    const int argc = a->count;

    if (exact >= 0 && argc != exact) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "expected exactly %d arg%s, got %d",
                 exact, exact == 1 ? "" : "s", argc);
        return make_cell_error(buf, ARITY_ERR);
    }
    if (min >= 0 && argc < min) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "expected at least %d arg%s, got %d",
                 min, min == 1 ? "" : "s", argc);
        return make_cell_error(buf, ARITY_ERR);
    }
    if (max >= 0 && argc > max) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "expected at most %d arg%s, got %d",
                 max, max == 1 ? "" : "s", argc);
        return make_cell_error(buf, ARITY_ERR);
    }
    return nullptr; /* all good */
}

/*---------------------------------------------------------*
 *       Helper functions for numeric type promotion       *
 * --------------------------------------------------------*/

/* Convertors */
Cell* int_to_rat(const Cell* v) {
    return make_cell_rational(v->integer_v, 1, 0);
}

Cell* int_to_real(const Cell* v) {
    return make_cell_real((long double)v->integer_v);
}

Cell* rat_to_real(const Cell* v) {
    return make_cell_real((long double)v->num / (long double)v->den);
}

Cell* to_complex(Cell* v) {
    return make_cell_complex(v, make_cell_integer(0));
}

/* Promote two numbers to the same type, modifying lhs and rhs in-place. */
void numeric_promote(Cell** lhs, Cell** rhs) {
    Cell* a = *lhs;
    Cell* b = *rhs;

    if (a->type == CELL_COMPLEX|| b->type == CELL_COMPLEX) {
        if (a->type != CELL_COMPLEX) {
            a = to_complex(a);
        }
        if (b->type != CELL_COMPLEX) {
            b = to_complex(b);
        }
    }
    else if (a->type == CELL_REAL || b->type == CELL_REAL) {
        if (a->type == CELL_INTEGER || a->type == CELL_RATIONAL) {
            a = a->type == CELL_INTEGER ? int_to_real(a) : rat_to_real(a);
        }
        if (b->type == CELL_INTEGER || b->type == CELL_RATIONAL) {
            b = b->type == CELL_INTEGER ? int_to_real(b) : rat_to_real(b);
        }
    }
    else if (a->type == CELL_RATIONAL || b->type == CELL_RATIONAL) {
        if (a->type == CELL_INTEGER) {
            a = int_to_rat(a);
        }
        if (b->type == CELL_INTEGER) {
            b = int_to_rat(b);
        }
    }
    *lhs = a;
    *rhs = b;
}

/*---------------------------------------------------------*
 *      Helper functions for using builtins internally     *
 * --------------------------------------------------------*/

/* Construct an S-expression with exactly one element */
Cell* make_sexpr_len1(const Cell* a) {
    Cell* v = make_cell_sexpr();
    v->count = 1;
    v->cell = GC_MALLOC(sizeof(Cell*));
    v->cell[0] = cell_copy(a);
    return v;
}

/* Construct an S-expression with exactly two elements */
Cell* make_sexpr_len2(const Cell* a, const Cell* b) {
    Cell* v = make_cell_sexpr();
    v->count = 2;
    v->cell = GC_MALLOC(sizeof(Cell*) * 2);
    v->cell[0] = cell_copy(a);
    v->cell[1] = cell_copy(b);
    return v;
}

/* Construct an S-expression with exactly four elements */
Cell* make_sexpr_len4(const Cell* a, const Cell* b, const Cell* c, const Cell* d) {
    Cell* v = make_cell_sexpr();
    v->count = 4;
    v->cell = GC_MALLOC(sizeof(Cell*) * 4);
    v->cell[0] = cell_copy(a);
    v->cell[1] = cell_copy(b);
    v->cell[2] = cell_copy(c);
    v->cell[3] = cell_copy(d);
    return v;
}

Cell* make_sexpr_from_list(Cell* v) {
    Cell* result = make_cell_sexpr();
    result->count = v->len;
    result->cell = GC_MALLOC(sizeof(Cell*) * v->len);

    Cell* p = v;
    for (int i = 0; i < v->len; i++) {
        result->cell[i] = cell_copy(p->car);
        p = p->cdr;
    }
    return result;
}

/* Constructs an S-expression from a C array of Cell pointers. */
Cell* make_sexpr_from_array(const int count, Cell** cells) {
    Cell* v = make_cell_sexpr();
    v->count = count;
    v->cell = GC_MALLOC(sizeof(Cell*) * count);

    /* Copy each cell pointer from the source array */
    for (int i = 0; i < count; i++) {
        v->cell[i] = cell_copy(cells[i]);
    }

    return v;
}

Cell* flatten_sexpr(const Cell* sexpr) {
    /* Calculate the total size of the flattened S-expression */
    int new_count = 0;
    for (int i = 0; i < sexpr->count; i++) {
        const Cell* current_item = sexpr->cell[i];
        if (current_item->type == CELL_SEXPR) {
            /* If the item is an inner s-expr, add the count of its children. */
            new_count += current_item->count;
        } else {
            new_count++;
        }
    }

    /* Allocation */
    Cell* result = make_cell_sexpr();
    result->count = new_count;
    if (new_count == 0) {
        return result;
    }
    result->cell = GC_MALLOC(sizeof(Cell*) * new_count);

    /* Populate the new S-expression's cell array */
    int result_idx = 0;
    for (int i = 0; i < sexpr->count; i++) {
        const Cell* current_item = sexpr->cell[i];
        if (current_item->type == CELL_SEXPR) {
            /* It's an inner s-expr, so copy its children over. */
            for (int j = 0; j < current_item->count; j++) {
                result->cell[result_idx] = cell_copy(current_item->cell[j]);
                result_idx++;
            }
        } else {
            /* It's an atom, so copy it directly. */
            result->cell[result_idx] = cell_copy(current_item);
            result_idx++;
        }
    }
    return result;
}

/*-------------------------------------------*
 *       Miscellaneous numeric helpers       *
 * ------------------------------------------*/

/* Helper to check if a non-complex numeric cell has a value of zero. */
bool cell_is_real_zero(const Cell* c) {
    if (!c) return false;
    switch (c->type) {
        case CELL_INTEGER:
            return c->integer_v == 0;
        case CELL_RATIONAL:
            /* Assumes simplified rational where numerator would be 0. */
            return c->num == 0;
        case CELL_REAL:
            return c->real_v == 0.0L;
        default:
            return false;
    }
}

/* Helper to check if a cell represents an integer value, per R7RS tower. */
bool cell_is_integer(const Cell* c) {
    if (!c) return false;
    switch (c->type) {
        case CELL_INTEGER:
            return true;
        case CELL_RATIONAL:
            /* A simplified rational is an integer if its denominator is 1. */
            return c->den == 1;
        case CELL_REAL:
            /* A real is an integer if it has no fractional part. */
            return c->real_v == floorl(c->real_v);
        case CELL_COMPLEX:
            /* A complex is an integer if its imaginary part is zero
             * and its real part is an integer. */
            return cell_is_real_zero(c->imag) && cell_is_integer(c->real);
        default:
            return false;
    }
}

/* Checks if a number is real-valued (i.e., has a zero imaginary part). */
bool cell_is_real(const Cell* c) {
    if (!c) return false;
    switch (c->type) {
        case CELL_INTEGER:
        case CELL_RATIONAL:
        case CELL_REAL:
            return true;
        case CELL_COMPLEX:
            /* A complex number is real if its imaginary part is zero. */
            return cell_is_real_zero(c->imag);
        default:
            return false;
    }
}

/* Helper for positive? (> 0)
 * Note: R7RS 'positive?' is strictly greater than 0. */
bool cell_is_positive(const Cell* c) {
    if (!c) return false;

    const Cell* val_to_check = c;
    if (c->type == CELL_COMPLEX) {
        /* Must be a real number to be positive */
        if (!cell_is_real_zero(c->imag)) return false;
        val_to_check = c->real;
    }

    switch (val_to_check->type) {
        case CELL_INTEGER:  return val_to_check->integer_v > 0;
        case CELL_REAL: return val_to_check->real_v > 0.0L;
        case CELL_RATIONAL:  return val_to_check->num > 0; /* Assumes den is always positive */
        default:       return false;
    }
}

/* Helper for negative? (< 0) */
bool cell_is_negative(const Cell* c) {
    if (!c) return false;

    const Cell* val_to_check = c;
    if (c->type == CELL_COMPLEX) {
        /* Must be a real number to be negative */
        if (!cell_is_real_zero(c->imag)) return false;
        val_to_check = c->real;
    }

    switch (val_to_check->type) {
        case CELL_INTEGER:  return val_to_check->integer_v < 0;
        case CELL_REAL: return val_to_check->real_v < 0.0L;
        case CELL_RATIONAL:  return val_to_check->num < 0; /* Assumes den is always positive */
        default:       return false;
    }
}

/* Helper for odd? */
bool cell_is_odd(const Cell* c) {
    /* Must be an integer to be odd or even. */
    if (!cell_is_integer(c)) {
        return false;
    }

    const Cell* int_cell = (c->type == CELL_COMPLEX) ? c->real : c;

    long long val;
    switch (int_cell->type) {
        case CELL_INTEGER:  val = int_cell->integer_v; break;
        case CELL_REAL: val = (long long)int_cell->real_v; break;
        case CELL_RATIONAL:  val = int_cell->num; break; /* den is 1 if it's an integer */
        default: return false; /* Unreachable */
    }
    return (val % 2 != 0);
}

/* Helper for even? */
bool cell_is_even(const Cell* c) {
    /* Must be an integer to be odd or even. */
    if (!cell_is_integer(c)) {
        return false;
    }

    const Cell* int_cell = (c->type == CELL_COMPLEX) ? c->real : c;

    long long val;
    switch (int_cell->type) {
        case CELL_INTEGER:  val = int_cell->integer_v; break;
        case CELL_REAL: val = (long long)int_cell->real_v; break;
        case CELL_RATIONAL:  val = int_cell->num; break; /* den is 1 if it's an integer */
        default: return false; /* Unreachable */
    }
    return (val % 2 == 0);
}


Cell* negate_numeric(const Cell* x) {
    switch (x->type) {
        case CELL_INTEGER:
            return make_cell_integer(-x->integer_v);
        case CELL_RATIONAL:
            return make_cell_rational(-x->num, x->den, 1);
        case CELL_REAL:
            return make_cell_real(-x->real_v);
        case CELL_COMPLEX:
            return make_cell_complex(
                negate_numeric(x->real),
                negate_numeric(x->imag)
            );
        default:
            return make_cell_error("negate_numeric: Oops, this isn't right!", GEN_ERR);
    }
}

/* gcd helper, iterative and safe */
static long int gcd_ll(long int a, long int b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        const long int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

/* simplify rational: reduce to the lowest terms, normalize sign */
Cell* simplify_rational(Cell* v) {
    if (v->type != CELL_RATIONAL) {
        return v; /* nothing to do */
    }

    long int g = gcd_ll(v->num, v->den);
    v->num /= g;
    v->den /= g;

    /* normalize sign: denominator always positive */
    if (v->den < 0) {
        v->den = -v->den;
        v->num = -v->num;
    }

    if (v->den == 0) {
        /* undefined fraction, return an error */
        Cell* err = make_cell_error("simplify_rational: denominator is zero!", GEN_ERR);
        return err;
    }
    if (v->num == v->den) {
        /* Return as integer 1 */
        return make_cell_integer(1);
    }
    if (v->den == 1) {
        /* Return as integer */
        Cell* int_cell = make_cell_integer(v->num);
        return int_cell;
    }
    return v;
}

/* Helper: convert numeric Cell to long double */
long double cell_to_ld(const Cell* c) {
    switch (c->type) {
        case CELL_INTEGER:  return (long double)c->integer_v;
        case CELL_RATIONAL:  return (long double)c->num / (long double)c->den;
        case CELL_REAL: return c->real_v;
        default:       return 0.0L; /* should not happen */
    }
}

/* Helper for performing arithmetic on complex numbers */
void complex_apply(BuiltinFn fn, const Lex* e, Cell* result, const Cell* rhs) {
    if (fn == builtin_add || fn == builtin_sub) {
        /* addition/subtraction: elementwise using recursion */
        const Cell* real_args = make_sexpr_len2(result->real, rhs->real);
        const Cell* imag_args = make_sexpr_len2(result->imag, rhs->imag);

        Cell* new_real = fn(e, real_args);
        Cell* new_imag = fn(e, imag_args);

        result->real = new_real;
        result->imag = new_imag;
        return;
    }

    /* Pointers to the four numeric components (a, b, c, d). */
    const Cell* a = result->real;
    const Cell* b = result->imag;
    const Cell* c = rhs->real;
    const Cell* d = rhs->imag;

    /* Create temporary argument lists and perform calculations */
    const Cell* ac_args = make_sexpr_len2(a, c);
    const Cell* bd_args = make_sexpr_len2(b, d);
    const Cell* ad_args = make_sexpr_len2(a, d);
    const Cell* bc_args = make_sexpr_len2(b, c);

    const Cell* ac = builtin_mul(e, ac_args);
    const Cell* bd = builtin_mul(e, bd_args);
    const Cell* ad = builtin_mul(e, ad_args);
    const Cell* bc = builtin_mul(e, bc_args);

    Cell* new_real = nullptr;
    Cell* new_imag = nullptr;

    if (fn == builtin_mul) {
        /* Create temporary argument lists for final operations */
        const Cell* real_args = make_sexpr_len2(ac, bd);
        const Cell* imag_args = make_sexpr_len2(ad, bc);

        new_real = builtin_sub(e, real_args);
        new_imag = builtin_add(e, imag_args);
    }
    else if (fn == builtin_div) {
        /* Create temporary argument lists for denominator calculation */
        const Cell* c_sq_args = make_sexpr_len2(c, c);
        const Cell* d_sq_args = make_sexpr_len2(d, d);

        const Cell* c_sq = builtin_mul(e, c_sq_args);
        const Cell* d_sq = builtin_mul(e, d_sq_args);

        const Cell* denom_args = make_sexpr_len2(c_sq, d_sq);
        const Cell* denom = builtin_add(e, denom_args);

        /* Create temporary argument lists for numerators */
        const Cell* real_num_args = make_sexpr_len2(ac, bd);
        const Cell* imag_num_args = make_sexpr_len2(bc, ad);

        const Cell* real_num = builtin_add(e, real_num_args);
        const Cell* imag_num = builtin_sub(e, imag_num_args);

        /* Create temporary argument lists for final division */
        const Cell* final_real_args = make_sexpr_len2(real_num, denom);
        const Cell* final_imag_args = make_sexpr_len2(imag_num, denom);

        new_real = builtin_div(e, final_real_args);
        new_imag = builtin_div(e, final_imag_args);
    }
    /* Assign the newly calculated, type-correct components. */
    result->real = new_real;
    result->imag = new_imag;
}

/* Helper to convert any real-valued cell to a C long double. */
long double cell_to_long_double(const Cell* c) {
    switch (c->type) {
        case CELL_INTEGER:
            return (long double)c->integer_v;
        case CELL_RATIONAL:
            return (long double)c->num / c->den;
        case CELL_REAL:
            return c->real_v;
        default:
            /* This case should ideally not be reached if inputs are valid numbers. */
            return 0.0L;
    }
}

/* Helper to construct appropriate cell from a long double */
Cell* make_cell_from_double(const long double d) {
    if (d == floorl(d) && d >= LLONG_MIN && d <= LLONG_MAX) {
        return make_cell_integer((long long)d);
    }
    return make_cell_real(d);
}

/* A version of strdup that allocates memory using the garbage collector. */
char* GC_strdup(const char* s) {
    if (s == NULL) {
        return nullptr;
    }
    /* Allocate GC-managed memory for the new string. */
    size_t len = strlen(s) + 1;
    char* new_str = GC_MALLOC_ATOMIC(len);
    if (new_str == NULL) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    /* Copy the string content. */
    memcpy(new_str, s, len);
    return new_str;
}

/* A version of strndup that allocates memory using the garbage collector. */
char* GC_strndup(const char* s, const size_t n) {
    /* Find the actual length of the substring, up to n. */
    size_t len = strnlen(s, n);

    /* Allocate GC-managed memory. */
    char* new_str = GC_MALLOC_ATOMIC(len + 1);
    if (new_str == NULL) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }

    /* Copy the content and null-terminate. */
    memcpy(new_str, s, len);
    new_str[len] = '\0';

    return new_str;
}

/* Mapping of char names to Unicode codepoints.
 * This array MUST be kept sorted alphabetically. */
static const NamedChar named_chars[] = {
    {"Alpha",     0x0391},
    {"Beta",      0x0392},
    {"Delta",     0x0394},
    {"Gamma",     0x0393},
    {"Iota",      0x0399},
    {"Lambda",    0x039B},
    {"Omega",     0x03A9},
    {"Omicron",   0x039F},
    {"Phi",       0x03A6},
    {"Pi",        0x03A0},
    {"Psi",       0x03A8},
    {"Rho",       0x03A1},
    {"Sigma",     0x03A3},
    {"Theta",     0x0398},
    {"Xi",        0x039E},
    {"alpha",     0x03B1},
    {"beta",      0x03B2},
    {"chi",       0x03C7},
    {"copy",      0x00A9},
    {"curren",    0x00A4},
    {"deg",       0x00B0},
    {"delta",     0x03B4},
    {"divide",    0x00F7},
    {"epsilon",   0x03B5},
    {"eta",       0x03B7},
    {"euro",      0x20AC},
    {"gamma",     0x03B3},
    {"iota",      0x03B9},
    {"iquest",    0x00BF},
    {"kappa",     0x03BA},
    {"lambda",    0x03BB},
    {"micro",     0x00B5},
    {"mu",        0x03BC},
    {"omega",     0x03C9},
    {"para",      0x00B6},
    {"phi",       0x03C6},
    {"pi",        0x03C0},
    {"plusnm",    0x00B1},
    {"pound",     0x00A3},
    {"psi",       0x03C8},
    {"reg",       0x00AE},
    {"rho",       0x03C1},
    {"sect",      0x00A7},
    {"sigma",     0x03C3},
    {"tau",       0x03C4},
    {"theta",     0x03B8},
    {"times",     0x00D7},
    {"xi",        0x03BE},
    {"yen",       0x00A5},
    {"zeta",      0x03B6}
};

/* This function is required by bsearch to compare a key (the string name)
 * with an element in the NamedChar array. */
int compare_named_chars(const void* key, const void* element) {
    const char* name_key = key;
    const NamedChar* char_element = element;
    return strcmp(name_key, char_element->name);
}

/* Returns a pointer to the found NamedChar, or NULL if not found. */
const NamedChar* find_named_char(const char* name) {
    return bsearch(
        name,                                      /* The key to search for */
        named_chars,                              /* The array to search in */
        sizeof(named_chars) / sizeof(NamedChar),   /* Number of elements in the array */
        sizeof(NamedChar),                       /* The size of each element */
        compare_named_chars                            /* The comparison function */
    );
}

/* helper to get a pointer to the value in the Nth node of a list.
 * Returns NULL if the index is out of bounds or the input is not a list. */
Cell* list_get_nth_cell_ptr(const Cell* list, const long n) {
    const Cell* current = list;
    for (long i = 0; i < n; i++) {
        /* Make sure we are still on a pair before trying to get the cdr */
        if (current->type != CELL_PAIR) {
            return nullptr;
        }
        current = current->cdr;
    }

    /* After the loop, `current` is the pair holding our desired element */
    if (current->type != CELL_PAIR) {
        return nullptr;
    }
    /* The value we want is the CAR of this final pair */
    return current->car;
}

char* convert_to_utf8(const UChar* ustr) {
    UErrorCode status = U_ZERO_ERROR;
    int32_t char_len = 0;
    char* result_utf8_str = nullptr;

    /* Get required buffer length in bytes */
    u_strToUTF8(nullptr, 0, &char_len, ustr, -1, &status);

    if (status == U_BUFFER_OVERFLOW_ERROR) {
        status = U_ZERO_ERROR;
        result_utf8_str = (char*)GC_malloc(sizeof(char) * (char_len + 1));

        /* Actual Conversion */
        u_strToUTF8(result_utf8_str, char_len + 1, nullptr, ustr, -1, &status);

        if (U_FAILURE(status)) {
            return nullptr;
        }
    }
    return result_utf8_str;
}

UChar* convert_to_utf16(const char* str) {
    UErrorCode status = U_ZERO_ERROR;
    int32_t uchar_len = 0;
    UChar* my_utf16_str = nullptr;

    /* Pre-flight: Get the required buffer length in UChars */
    u_strFromUTF8(nullptr, 0, &uchar_len, str, -1, &status);

    /* The pre-flight call sets an error code that we expect. */
    if (status == U_BUFFER_OVERFLOW_ERROR) {
        status = U_ZERO_ERROR; // Reset status for the next call
        my_utf16_str = (UChar*)GC_malloc(sizeof(UChar) * (uchar_len + 1));
        if (!my_utf16_str) { return nullptr; }

        /* Call the function again with the allocated buffer */
        u_strFromUTF8(my_utf16_str, uchar_len + 1, nullptr, str, -1, &status);

        if (U_FAILURE(status)) {
            return nullptr;
        }
    }
    return my_utf16_str;
}
