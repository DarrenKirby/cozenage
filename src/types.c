/*
 * 'src/types.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 - 2026 Darren Kirby <darren@dragonbyte.ca>
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

/* This file does not implement any builtin procedures, rather, it implements
 * a library of helper functions related to Cozenage types and procedures.
 * For example, it implements procedure arity and type checkers, numeric
 * promotion functions, helpers for building S-expr Cells of various lengths,
 * internal type converters, and the binary search lookup table for mapping
 * #\sigma style helpers to actual #\σ char literals. */

#include "bignum.h"
#include "cell.h"
#include "numerics.h"
#include "types.h"

#include <errno.h>
#include <gc.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unicode/ustring.h>

#define FMT_ERR_BUF_SIZE 512
#define CELL_MASK_NAME_BUF_SIZE 256

#ifdef CRITERION_TEST_BUILD
    /* During tests, STATIC compiles away to nothing, exposing the functions */
    #define STATIC
#else
    /* In standard builds, STATIC acts exactly like the static keyword */
    #define STATIC static
#endif

/* Not all systems provide this macro. */
#ifndef CMPLXL
#define CMPLXL(a,b) ((long double _Complex)((a) + (b) * I))
#endif


/* Global copies of argc and argv for use by (command-line). */
int g_argc;
char** g_argv;
/* Global is_repl used by many. */
bool is_repl;

/*---------------------------------------------------------*
 *      Type names to strings                              *
 * --------------------------------------------------------*/


/* The name may be bad. Can use this to format
 * a string anywhere a string is expected. */
const char* fmt_err(const char* fmt, ...)
{
    char buf[FMT_ERR_BUF_SIZE];
    va_list args = {};
    va_start(args, fmt);
    const int n = vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);

    if (n < 0) {
        return "internal error: failed to format error message";
    }
    return GC_strdup(buf);
}


typedef struct {
    uint32_t    flag;
    const char* name;
} cell_type_entry;


static const cell_type_entry CELL_TYPE_TABLE[] = {
    { .flag = CELL_INTEGER,    .name = "integer" },
    { .flag = CELL_REAL,       .name = "real" },
    { .flag = CELL_RATIONAL,   .name = "rational" },
    { .flag = CELL_COMPLEX,    .name = "complex" },
    { .flag = CELL_BOOLEAN,    .name = "bool" },
    { .flag = CELL_SYMBOL,     .name = "symbol" },
    { .flag = CELL_STRING,     .name = "string" },
    { .flag = CELL_SEXPR,      .name = "sexpr" },
    { .flag = CELL_NIL,        .name = "nil" },
    { .flag = CELL_PROC,       .name = "procedure" },
    { .flag = CELL_ERROR,      .name = "error" },
    { .flag = CELL_PAIR,       .name = "pair" },
    { .flag = CELL_VECTOR,     .name = "vector" },
    { .flag = CELL_CHAR,       .name = "char" },
    { .flag = CELL_BYTEVECTOR, .name = "byte vector" },
    { .flag = CELL_EOF,        .name = "eof" },
    { .flag = CELL_BIGINT,     .name = "bigint" },
    { .flag = CELL_BIGRAT,     .name = "bigrat" },
    { .flag = CELL_BIGFLOAT,   .name = "bigfloat" },
    { .flag = CELL_PROMISE,    .name = "promise" },
    { .flag = CELL_STREAM,     .name = "stream" },
    { .flag = CELL_MACRO,      .name = "macro" },
};

#define CELL_TYPE_TABLE_LEN (sizeof CELL_TYPE_TABLE / sizeof CELL_TYPE_TABLE[0])


/* Turn a single type into a string. */
const char* cell_type_name(const uint32_t t)
{
    for (size_t i = 0; i < CELL_TYPE_TABLE_LEN; i++) {
        if (CELL_TYPE_TABLE[i].flag == t) { return CELL_TYPE_TABLE[i].name; }
    }
    return "unknown";
}


/* Turn a mask (possibly multiple flags ORed together) into a string
   e.g. (CELL_INTEGER | CELL_REAL) -> "int|real". */
const char* cell_mask_types(const uint32_t mask)
{
    char buf[CELL_MASK_NAME_BUF_SIZE];
    size_t pos = 0;

    for (size_t i = 0; i < sizeof CELL_TYPE_TABLE / sizeof CELL_TYPE_TABLE[0]; i++) {
        if (!(mask & CELL_TYPE_TABLE[i].flag)) { continue; }

        const size_t n = strlen(CELL_TYPE_TABLE[i].name);
        if (pos + n + 2 > sizeof buf) { break; }   /* room for '|' and the NUL */

        if (pos > 0) { buf[pos++] = '|'; }
        memcpy(buf + pos, CELL_TYPE_TABLE[i].name, n);
        pos += n;
    }
    buf[pos] = '\0';
    return GC_strdup(buf);
}

/*---------------------------------------------------------*
 *      Procedure argument arity and type validators       *
 * --------------------------------------------------------*/


/* Return NULL if all args are valid, else return a CELL_ERROR. */
Cell* check_arg_types(const Cell* a, const int mask, const char* fname)
{
    for (int i = 0; i < a->count; i++) {
        const Cell* arg = a->cell[i];

        /* bitwise AND: if arg->type isn't in mask, it's invalid. */
        if (!(arg->type & mask)) {
            return make_cell_error(
            fmt_err("%s: bad type at arg %d: got %s, expected %s",
                     fname,
                     i+1,
                     cell_type_name(arg->type),
                     cell_mask_types(mask)), TYPE_ERR);
        }
    }
    return nullptr; /* all good. */
}


Cell* check_arg_arity(const Cell* a, const int exact, const int min, const int max, const char* fname)
{
    const int argc = a->count;

    if (exact >= 0 && argc != exact) {
        return make_cell_error(
            fmt_err(
                "%s: expected exactly %d arg%s, got %d", fname,
                exact, exact == 1 ? "" : "s", argc), ARITY_ERR);
    }
    if (min >= 0 && argc < min) {
        return make_cell_error(
            fmt_err("%s: expected at least %d arg%s, got %d", fname,
                 min, min == 1 ? "" : "s", argc), ARITY_ERR);
    }
    if (max >= 0 && argc > max) {
        return make_cell_error(fmt_err("%s: expected at most %d arg%s, got %d", fname,
                 max, max == 1 ? "" : "s", argc), ARITY_ERR);
    }
    return nullptr; /* all good */
}


int check_lambda_arity(const Cell* proc, const int expected) {
    if (proc->type != CELL_PROC) { return 0; }

    const Cell* formals = proc->lambda->formals;

    /* No arguments. */
    if (formals->count == 0) { return 0; }

    /* If formals is a symbol (variadic), it accepts anything. */
    if (formals->type == CELL_SYMBOL) { return 1; }

    int positional_count = 0;
    bool is_variadic = false;

    for (int i = 0; i < formals->count; i++) {
        if (formals->cell[i]->type == CELL_SYMBOL &&
            strcmp(formals->cell[i]->sym, ".") == 0) {
                is_variadic = true;
                break;
            }
        positional_count++;
    }

    if (is_variadic) {
        /* (a b . c) requires at least 2 args.
           The symbol after the dot (c) is the 'rest' list. */
        return expected >= positional_count;
    }

    /* Standard fixed-arity: count must match exactly. */
    return formals->count == expected;
}


/*---------------------------------------------------------*
 *       Numeric type promotion                            *
 * --------------------------------------------------------*/

/* Convertors. */
STATIC Cell* int_to_rat(const Cell* v)
{
    return make_cell_rational(v->integer_v, 1, 0);
}


STATIC Cell* int_to_real(const Cell* v)
{
    return make_cell_real((long double)v->integer_v);
}


STATIC Cell* bigint_to_real(const Cell* v)
{
    return make_cell_real(mpz_get_d(*v->bi));
}


STATIC Cell* rat_to_real(const Cell* v)
{
    return make_cell_real((long double)v->num / (long double)v->den);
}


STATIC Cell* bigrat_to_real(const Cell* v)
{
    return make_cell_real(mpq_get_d(*v->br));
}


STATIC Cell* to_complex(Cell* v)
{
    return make_cell_complex(v, make_cell_integer(0));
}


STATIC Cell* int_to_bigint(const Cell* v)
{
    return make_cell_bigint(nullptr, v, BIGINT_BASE);
}


STATIC Cell* int_to_bigrat(const Cell* v)
{
    return make_cell_bigrat(v->integer_v, 1, nullptr);
}


STATIC Cell* rat_to_bigrat(const Cell* v)
{
    return make_cell_bigrat(v->num, v->den, nullptr);
}


STATIC Cell* bigint_to_bigrat(const Cell* v)
{
    Cell* c = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    c->type = CELL_BIGRAT;
    c->exact = true;
    c->br = GC_MALLOC(sizeof(mpq_t));
    mpq_init(*c->br);
    mpq_set_z(*c->br, *v->bi);
    return c;
}


static Cell* promote_to(Cell *a, const Cell_t target)
{
    switch (target) {
        case CELL_BIGINT: {
            if (a->type == CELL_INTEGER) {
                return int_to_bigint(a);
            }
            if (a->type == CELL_BIGINT) { return a; }
            break;
        }

        case CELL_RATIONAL: {
            if (a->type == CELL_INTEGER) {
                return int_to_rat(a);
            }
            if (a->type == CELL_RATIONAL) { return a; }
            break;
        }

        case CELL_BIGRAT: {
            if (a->type == CELL_INTEGER) {
                return int_to_bigrat(a);
            }
            if (a->type == CELL_BIGINT) {
                return bigint_to_bigrat(a);
            }
            if (a->type == CELL_RATIONAL) {
                return rat_to_bigrat(a);
            }
            if (a->type == CELL_BIGRAT) { return a; }
            break;
        }

        case CELL_REAL: {
            if (a->type == CELL_INTEGER) {
                return int_to_real(a);
            }
            if (a->type == CELL_BIGINT) {
                return bigint_to_real(a);
            }
            if (a->type == CELL_RATIONAL) {
                return rat_to_real(a);
            }
            if (a->type == CELL_BIGRAT) {
                return bigrat_to_real(a);
            }
            if (a->type == CELL_REAL) { return a; }
            break;
        }

        case CELL_COMPLEX: {
            if (a->type == CELL_COMPLEX) {
                return a;
            }
            return to_complex(a);

        }

        default:
            ;
    }
    return a;
}


/* This is not ideal but necessary as the actual Cell type
 * enum uses powers of 2 for fast bitwise type checking. */
static NumType numeric_index(const Cell_t t)
{
    switch (t) {
        case CELL_INTEGER:  return NUM_INT;
        case CELL_BIGINT:   return NUM_BIGINT;
        case CELL_RATIONAL: return NUM_RATIONAL;
        case CELL_BIGRAT:   return NUM_BIGRAT;
        case CELL_REAL:     return NUM_REAL;
        case CELL_COMPLEX:  return NUM_COMPLEX;
        default:
            abort();
    }
}


static Cell_t promote_table[NUM_TYPE_COUNT][NUM_TYPE_COUNT] = {
    /*                 int            bigint        rat            bigrat        real          complex */
    /* int     */ { CELL_INTEGER,  CELL_BIGINT,  CELL_RATIONAL, CELL_BIGRAT,  CELL_REAL,    CELL_COMPLEX },
    /* bigint  */ { CELL_BIGINT,   CELL_BIGINT,  CELL_BIGRAT,   CELL_BIGRAT,  CELL_REAL,    CELL_COMPLEX },
    /* rat     */ { CELL_RATIONAL, CELL_BIGRAT,  CELL_RATIONAL, CELL_BIGRAT,  CELL_REAL,    CELL_COMPLEX },
    /* bigrat  */ { CELL_BIGRAT,   CELL_BIGRAT,  CELL_BIGRAT,   CELL_BIGRAT,  CELL_REAL,    CELL_COMPLEX },
    /* real    */ { CELL_REAL,     CELL_REAL,    CELL_REAL,     CELL_REAL,    CELL_REAL,    CELL_COMPLEX },
    /* complex */ { CELL_COMPLEX,  CELL_COMPLEX, CELL_COMPLEX,  CELL_COMPLEX, CELL_COMPLEX, CELL_COMPLEX },
};


/* Promote two numbers to the same type, modifying lhs and rhs in-place. */
void numeric_promote(Cell** lhs, Cell** rhs)
{
    Cell* a = *lhs;
    Cell* b = *rhs;

    /* Safety check. */
    if (a->type != CELL_ERROR && b->type != CELL_ERROR) {
        const Cell_t target = promote_table[numeric_index(a->type)]
                                           [numeric_index(b->type)];

        a = promote_to(a, target);
        b = promote_to(b, target);
    }

    *lhs = a;
    *rhs = b;
}

/*---------------------------------------------------------*
 *      Sexpr constructors and transformers                *
 * --------------------------------------------------------*/

/* Construct an S-expression with exactly one element. */
Cell* make_sexpr_len1(const Cell* a)
{
    Cell* v = make_cell_sexpr();
    v->count = 1;
    v->cell = GC_MALLOC(sizeof(Cell*));
    v->cell[0] = cell_copy(a);
    return v;
}


/* Construct an S-expression with exactly two elements. */
Cell* make_sexpr_len2(const Cell* a, const Cell* b)
{
    Cell* v = make_cell_sexpr();
    v->count = 2;
    v->cell = GC_MALLOC(sizeof(Cell*) * 2);
    v->cell[0] = cell_copy(a);
    v->cell[1] = cell_copy(b);
    return v;
}


/* Construct an S-expression with exactly three elements. */
Cell* make_sexpr_len3(const Cell* a, const Cell* b, const Cell* c)
{
    Cell* v = make_cell_sexpr();
    v->count = 3;
    v->cell = GC_MALLOC(sizeof(Cell*) * 3);
    v->cell[0] = cell_copy(a);
    v->cell[1] = cell_copy(b);
    v->cell[2] = cell_copy(c);
    return v;
}


/* Construct an S-expression with exactly four elements. */
Cell* make_sexpr_len4(const Cell* a, const Cell* b, const Cell* c, const Cell* d)
{
    Cell* v = make_cell_sexpr();
    v->count = 4;
    v->cell = GC_MALLOC(sizeof(Cell*) * 4);
    v->cell[0] = cell_copy(a);
    v->cell[1] = cell_copy(b);
    v->cell[2] = cell_copy(c);
    v->cell[3] = cell_copy(d);
    return v;
}


/* Convert a CELL_SEXPR to a CELL_PAIR linked-list. */
Cell* make_list_from_sexpr(Cell* c)
{

    /* Direct-return all the atomic types, and if it's already a list. */
    if (c->type & (CELL_INTEGER|CELL_REAL|CELL_RATIONAL|CELL_COMPLEX|CELL_PAIR|
                      CELL_BOOLEAN|CELL_CHAR|CELL_STRING|CELL_NIL|CELL_EOF|
                      CELL_PROC|CELL_PORT|CELL_ERROR|CELL_SYMBOL)) {
        return c;
    }

    /* Leave the top-level vector be, but convert internal members. */
    if (c->type == CELL_VECTOR) {
        Cell* result = make_cell_vector();
        for (int i = 0; i < c->count; i++) {
            cell_add(result, make_list_from_sexpr(c->cell[i]));
        }
        return result;
    }

    /* It is an S-expression. Check for improper list syntax. */
    int dot_pos = -1;
    if (c->count > 1) {
        const Cell* dot_candidate = c->cell[c->count - 2];
        if (dot_candidate->type == CELL_SYMBOL && strcmp(dot_candidate->sym, ".") == 0) {
            dot_pos = c->count - 2;
        }
    }

    /* Handle improper list. */
    if (dot_pos != -1) {
        /* The final cdr is the very last element in the S-expression. */
        Cell* final_cdr = make_list_from_sexpr(c->cell[c->count - 1]);

        /* Build the list chain backwards from the element before the dot. */
        Cell* list_head = final_cdr;
        for (int i = dot_pos - 1; i >= 0; i--) {
            Cell* element = make_list_from_sexpr(c->cell[i]);
            list_head = make_cell_pair(element, list_head);
        }
        return list_head;
    }

    /* Handle proper list. */
    Cell* list_head = make_cell_nil();
    const int len = c->count;

    for (int i = len - 1; i >= 0; i--) {
        /* Recursively call this function on each element to ensure
         * any nested S-expressions are also converted. */
        Cell* element = make_list_from_sexpr(c->cell[i]);

        /* Prepend the new element to the head of our list. */
        list_head = make_cell_pair(element, list_head);
        list_head->len = len - i;
    }
    return list_head;
}


Cell* make_sexpr_from_list(const Cell* v, const bool recurse)
{
    Cell* result = make_cell_sexpr();
    const Cell* p = v;

    while (p->type == CELL_PAIR) {
        Cell* item = p->car;
        if (recurse && item->type == CELL_PAIR) {
            cell_add(result, make_sexpr_from_list(item, true));
        } else {
            cell_add(result, item);
        }
        p = p->cdr;
    }

    /* Dotted tail: keep it as a single element. */
    if (p->type != CELL_NIL) {
        cell_add(result, (Cell*)p);
    }
    return result;
}


/* Constructs an S-expression from a C array of Cell pointers. */
Cell* make_sexpr_from_array(const int count, Cell** cells)
{
    Cell* v = make_cell_sexpr();
    v->count = count;
    v->cell = GC_MALLOC(sizeof(Cell*) * count);

    /* Copy each cell pointer from the source array. */
    for (int i = 0; i < count; i++) {
        v->cell[i] = cell_copy(cells[i]);
    }
    return v;
}


Cell* flatten_sexpr(const Cell* sexpr)
{
    /* Calculate the total size of the flattened S-expression. */
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

    /* Allocation. */
    Cell* result = make_cell_sexpr();
    result->count = new_count;
    if (new_count == 0) {
        return result;
    }
    result->cell = GC_MALLOC(sizeof(Cell*) * new_count);

    /* Populate the new S-expression's cell array. */
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


/* Helper to get a pointer to either the value in the Nth node of a list,
 * or the node itself if we are taking the tail.
 * Returns NULL if the index is out of bounds or the input is not a list. */
Cell* list_get_nth_cell_ptr(const Cell* list, const long n, const bool tail)
{
    if (n < 0) {
        return tail ? (Cell*)list : list->car;
    }
    const Cell* current = list;
    for (long i = 0; i < n; i++) {
        /* Make sure we are still on a pair before trying to get the cdr. */
        if (current->type != CELL_PAIR) {
            return nullptr;
        }
        current = current->cdr;
    }

    /* After the loop, `current` is the pair holding our desired element. */
    if (current->type != CELL_PAIR) {
        return nullptr;
    }
    /* If tail=false, the value we want is the CAR of this final pair.
     * If tail=true we want the cdr (assuming k-1 at caller). */
    return tail? current->cdr : current->car;
}

/*-------------------------------------------*
 *       Miscellaneous numeric helpers       *
 * ------------------------------------------*/

/* Helper to make C complex from Cell complex */
long double complex cell_to_c_complex(const Cell* c)
{
    const long double a = cell_to_long_double(c->real);
    const long double b = cell_to_long_double(c->imag);

    return CMPLXL(a, b);
}


/* Helper to check if a non-complex numeric cell has a value of zero. */
bool cell_is_real_zero(const Cell* c)
{
    if (!c) { return false; }
    switch (c->type) {
        case CELL_BIGINT:
            return mpz_cmp_si(*c->bi, 0) == 0;
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
bool cell_is_integer(const Cell* c)
{
    if (!c) { return false; }

    const long double num = cell_to_long_double(c);
    if (isinf(num)) {
        return false;
    }

    switch (c->type) {
        case CELL_INTEGER:
        case CELL_BIGINT:
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
bool cell_is_real(const Cell* c)
{
    if (!c) { return false; }

    const long double num = cell_to_long_double(c);
    if (isinf(num)) {
        return false;
    }

    switch (c->type) {
        case CELL_INTEGER:
        case CELL_BIGINT:
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
bool cell_is_positive(const Cell* c)
{
    if (!c) { return false; }

    if (c->type == CELL_BIGINT) {
        return mpz_sgn(*c->bi) == 1;
    }

    const long double num = cell_to_long_double(c);
    return num > 0;
}


/* Helper for negative? (< 0) */
bool cell_is_negative(const Cell* c)
{
    if (!c) { return false; }

    if (c->type == CELL_BIGINT) {
        return mpz_sgn(*c->bi) == -1;
    }

    const long double num = cell_to_long_double(c);
    return num < 0;
}


/* Helper for odd? */
bool cell_is_odd(const Cell* c)
{
    /* Must be an integer to be odd or even. */
    if (!cell_is_integer(c)) {
        return false;
    }

    /* inf.0 neither even nor odd */
    if (isinf(cell_to_long_double(c))) {
        return false;
    }

    /* Already determined c is an integer represented as a complex. */
    if (c->type == CELL_COMPLEX) {
        c = c->real;
    }

    long long val;
    switch (c->type) {
        case CELL_BIGINT: return mpz_odd_p(*c->bi) == 1;
        case CELL_INTEGER:  val = c->integer_v; break;
        case CELL_REAL: val = (long long)c->real_v; break;
        case CELL_RATIONAL:  val = c->num; break; /* Denominator is 1 if it's an integer. */
        default: return false; /* Unreachable. */
    }
    return val % 2 != 0;
}


/* Helper for even? */
bool cell_is_even(const Cell* c)
{
    /* Must be an integer to be odd or even. */
    if (!cell_is_integer(c)) {
        return false;
    }

    /* inf.0 neither even nor odd. */
    if (isinf(cell_to_long_double(c))) {
        return false;
    }

    /* Already determined c is an integer represented as a complex. */
    if (c->type == CELL_COMPLEX) {
        c = c->real;
    }

    long long val;
    switch (c->type) {
        case CELL_BIGINT: return mpz_even_p(*c->bi) == 1;
        case CELL_INTEGER:  val = c->integer_v; break;
        case CELL_REAL: val = (long long)c->real_v; break;
        case CELL_RATIONAL:  val = c->num; break; /* Denominator is 1 if it's an integer. */
        default: return false; /* Unreachable. */
    }
    return val % 2 == 0;
}


Cell* negate_numeric(const Cell* x)
{
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
        case CELL_BIGINT:
            return bigint_neg((Cell*)x);
        default:
            return make_cell_error(
                "negate numeric: bad arg type",
                TYPE_ERR);
    }
}


/* GCD helper. */
static long int gcd_ll(long int a, long int b)
{
    if (a < 0) { a = -a; }
    if (b < 0) { b = -b; }
    while (b != 0) {
        const long int t = b;
        b = a % b;
        a = t;
    }
    return a;
}


/* Simplify rational: reduce to the lowest terms, normalize sign. */
Cell* simplify_rational(Cell* v)
{
    if (v->type != CELL_RATIONAL) {
        return v; /* Nothing to do. */
    }

    const long int g = gcd_ll(v->num, v->den);
    v->num /= g;
    v->den /= g;

    /* Normalize sign: denominator always positive. */
    if (v->den < 0) {
        v->den = -v->den;
        v->num = -v->num;
    }

    if (v->den == 0) {
        /* Undefined fraction, return an error. */
        Cell* err = make_cell_error(
            "simplify_rational: denominator is zero!",
            GEN_ERR);
        return err;
    }
    if (v->num == v->den) {
        /* Return as integer 1. */
        return make_cell_integer(1);
    }
    if (v->den == 1) {
        /* Return as integer. */
        Cell* int_cell = make_cell_integer(v->num);
        return int_cell;
    }
    return v;
}


/* Helper for performing arithmetic on complex numbers. */
void complex_apply(const BuiltinFn fn, const Lex* e, Cell* result, const Cell* rhs)
{
    if (fn == builtin_add || fn == builtin_sub) {
        /* Addition/subtraction: elementwise using recursion. */
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

    /* Create temporary argument lists and perform calculations. */
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
        /* Create temporary argument lists for final operations. */
        const Cell* real_args = make_sexpr_len2(ac, bd);
        const Cell* imag_args = make_sexpr_len2(ad, bc);

        new_real = builtin_sub(e, real_args);
        new_imag = builtin_add(e, imag_args);
    }
    else if (fn == builtin_div) {
        /* Create temporary argument lists for denominator calculation. */
        const Cell* c_sq_args = make_sexpr_len2(c, c);
        const Cell* d_sq_args = make_sexpr_len2(d, d);

        const Cell* c_sq = builtin_mul(e, c_sq_args);
        const Cell* d_sq = builtin_mul(e, d_sq_args);

        const Cell* denom_args = make_sexpr_len2(c_sq, d_sq);
        const Cell* denom = builtin_add(e, denom_args);

        /* Create temporary argument lists for numerators. */
        const Cell* real_num_args = make_sexpr_len2(ac, bd);
        const Cell* imag_num_args = make_sexpr_len2(bc, ad);

        const Cell* real_num = builtin_add(e, real_num_args);
        const Cell* imag_num = builtin_sub(e, imag_num_args);

        /* Create temporary argument lists for final division. */
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
long double cell_to_long_double(const Cell* c)
{
    switch (c->type) {
        case CELL_INTEGER:
            return (long double)c->integer_v;
        case CELL_RATIONAL:
            return (long double)c->num / c->den;
        case CELL_REAL:
            return c->real_v;
        default:
            /* This case should not be reached if inputs are valid numbers. */
            return 0.0L;
    }
}


/* Helper to construct appropriate cell from a long double.
 * This function trampled correct exact/inexact flags, so
 * I've removed all calls to it. May remove entirely. */
Cell* make_cell_from_double(const long double d)
{
    if (d == floorl(d) && d >= LLONG_MIN && d <= LLONG_MAX) {
        return make_cell_integer((long long)d);
    }
    return make_cell_real(d);
}

/*-------------------------------------------*
 *       Named characters                    *
 * ------------------------------------------*/


/* Mapping of char names to Unicode codepoints.
 * This array MUST be kept sorted alphabetically
 * for the binary search. */
static const NamedChar named_chars[] = {
    {.name = "Alpha",     .codepoint = 0x0391},
    {.name = "Beta",      .codepoint = 0x0392},
    {.name = "Delta",     .codepoint = 0x0394},
    {.name = "Gamma",     .codepoint = 0x0393},
    {.name = "Iota",      .codepoint = 0x0399},
    {.name = "Lambda",    .codepoint = 0x039B},
    {.name = "Omega",     .codepoint = 0x03A9},
    {.name = "Omicron",   .codepoint = 0x039F},
    {.name = "Phi",       .codepoint = 0x03A6},
    {.name = "Pi",        .codepoint = 0x03A0},
    {.name = "Psi",       .codepoint = 0x03A8},
    {.name = "Rho",       .codepoint = 0x03A1},
    {.name = "Sigma",     .codepoint = 0x03A3},
    {.name = "Theta",     .codepoint = 0x0398},
    {.name = "Xi",        .codepoint = 0x039E},
    {.name = "alpha",     .codepoint = 0x03B1},
    {.name = "beta",      .codepoint = 0x03B2},
    {.name = "chi",       .codepoint = 0x03C7},
    {.name = "copy",      .codepoint = 0x00A9},
    {.name = "curren",    .codepoint = 0x00A4},
    {.name = "deg",       .codepoint = 0x00B0},
    {.name = "delta",     .codepoint = 0x03B4},
    {.name = "divide",    .codepoint = 0x00F7},
    {.name = "epsilon",   .codepoint = 0x03B5},
    {.name = "eta",       .codepoint = 0x03B7},
    {.name = "euro",      .codepoint = 0x20AC},
    {.name = "gamma",     .codepoint = 0x03B3},
    {.name = "iota",      .codepoint = 0x03B9},
    {.name = "iquest",    .codepoint = 0x00BF},
    {.name = "kappa",     .codepoint = 0x03BA},
    {.name = "lambda",    .codepoint = 0x03BB},
    {.name = "micro",     .codepoint = 0x00B5},
    {.name = "mu",        .codepoint = 0x03BC},
    {.name = "omega",     .codepoint = 0x03C9},
    {.name = "para",      .codepoint = 0x00B6},
    {.name = "phi",       .codepoint = 0x03C6},
    {.name = "pi",        .codepoint = 0x03C0},
    {.name = "plusnm",    .codepoint = 0x00B1},
    {.name = "pound",     .codepoint = 0x00A3},
    {.name = "psi",       .codepoint = 0x03C8},
    {.name = "reg",       .codepoint = 0x00AE},
    {.name = "rho",       .codepoint = 0x03C1},
    {.name = "sect",      .codepoint = 0x00A7},
    {.name = "sigma",     .codepoint = 0x03C3},
    {.name = "tau",       .codepoint = 0x03C4},
    {.name = "theta",     .codepoint = 0x03B8},
    {.name = "times",     .codepoint = 0x00D7},
    {.name = "xi",        .codepoint = 0x03BE},
    {.name = "yen",       .codepoint = 0x00A5},
    {.name = "zeta",      .codepoint = 0x03B6},
};


/* This function is required by bsearch to compare a key (the string name)
 * with an element in the NamedChar array. */
int compare_named_chars(const void* key, const void* element)
{
    const char* name_key = key;
    const NamedChar* char_element = element;
    return strcmp(name_key, char_element->name);
}


/* Returns a pointer to the found NamedChar, or NULL if not found. */
const NamedChar* find_named_char(const char* name)
{
    return bsearch(
        name,                                     /* The key to search for. */
        named_chars,                              /* The array to search in. */
        sizeof(named_chars) / sizeof(NamedChar),  /* Number of elements in the array. */
        sizeof(NamedChar),                        /* The size of each element. */
        compare_named_chars                       /* The comparison function. */
    );
}

/*-------------------------------------------*
 *       Unicode and UTF8                    *
 * ------------------------------------------*/


typedef struct {
    uint8_t  mask;     /* bits that identify the lead byte */
    uint8_t  tag;      /* value of those bits */
    uint8_t  payload;  /* lead-byte bits that carry code point data */
    uint32_t min_cp;   /* smallest code point needing this length (rejects overlong) */
} utf8_form;


static const utf8_form UTF8_FORMS[UTF8_MAX_LEN + 1] = {
    [1] = { .mask = 0x80, .tag = 0x00, .payload = 0x7F, .min_cp = 0x000000 },
    [2] = { .mask = 0xE0, .tag = 0xC0, .payload = 0x1F, .min_cp = 0x000080 },
    [3] = { .mask = 0xF0, .tag = 0xE0, .payload = 0x0F, .min_cp = 0x000800 },
    [4] = { .mask = 0xF8, .tag = 0xF0, .payload = 0x07, .min_cp = 0x010000 },
};


int utf8_len(const uint8_t first_byte) {
    for (int len = 1; len <= UTF8_MAX_LEN; len++) {
        if ((first_byte & UTF8_FORMS[len].mask) == UTF8_FORMS[len].tag) { return len; }
    }
    return -1;
}


int utf8_code_point_len(const uint32_t cp) {
    /* Bad value becomes U+FFFD. */
    if (!is_scalar_value(cp)) { return 3; }
    int len = 1;
    while (len < UTF8_MAX_LEN && cp >= UTF8_FORMS[len + 1].min_cp) { len++; }
    return len;
}


int utf8_encode(const UChar32 c, uint8_t* out_buf) {
    uint32_t cp = (uint32_t)c;   /* negatives become huge and fail below */
    if (!is_scalar_value(cp)) { cp = UNICODE_REPLACEMENT; }

    const int len = utf8_code_point_len(cp);
    out_buf[0] = (uint8_t)(UTF8_FORMS[len].tag | (cp >> (UTF8_CONT_BITS * (len - 1))));
    for (int i = 1; i < len; i++) {
        const int shift = UTF8_CONT_BITS * (len - 1 - i);
        out_buf[i] = (uint8_t)(UTF8_CONT_TAG | ((cp >> shift) & UTF8_CONT_MASK));
    }
    return len;
}


/* Decode one code point from the n bytes at s. Returns bytes consumed (1-4) and
 * stores the code point in *cp, or returns 0 if s is not a complete, valid,
 * shortest-form sequence. */
int utf8_decode(const uint8_t* s, const size_t n, UChar32* cp) {
    if (n == 0) { return 0; }
    const int len = utf8_len(s[0]);
    if (len < 1 || (size_t)len > n) { return 0; }

    uint32_t c = s[0] & UTF8_FORMS[len].payload;
    for (int i = 1; i < len; i++) {
        if ((s[i] & UTF8_MASK_TAG) != UTF8_CONT_TAG) { return 0; }
        c = (c << UTF8_CONT_BITS) | (s[i] & UTF8_CONT_MASK);
    }
    if (c < UTF8_FORMS[len].min_cp || !is_scalar_value(c)) { return 0; }

    *cp = (UChar32)c;
    return len;
}


uint32_t utf8_next(const uint8_t** p, const uint8_t* end) {
    if (*p >= end) { return UNICODE_REPLACEMENT; }

    UChar32 cp = U_SENTINEL;
    const int used = utf8_decode(*p, (size_t)(end - *p), &cp);
    if (used == 0) {
        (*p)++;                              /* skip one bad byte and resync */
        return UNICODE_REPLACEMENT;
    }
    *p += used;
    return (uint32_t)cp;
}


size_t utf8_prev_char(const char* str, size_t pos)
{
    if (pos == 0) { return 0; }
    pos--;
    while (pos > 0 && (str[pos] & UTF8_MASK_TAG) == UTF8_CONT_TAG) { pos--; }
    return pos;
}


size_t utf8_next_char(const char* str, size_t pos, const size_t len)
{
    if (pos >= len) { return len; }
    const size_t char_len = utf8_len((unsigned char)str[pos]);
    pos += char_len;
    if (pos > len) { return len; }
    return pos;
}


/* Move forward n codepoints and return byte offset. */
static size_t utf8_advance(const char* str, const size_t len, int n)
{
    size_t pos = 0;
    while (n-- > 0 && pos < len) {
        pos = utf8_next_char(str, pos, len);
    }
    return pos;
}


/* Move backward N codepoints and return byte offset. */
static size_t utf8_retreat(const char* str, size_t pos, int n)
{
    while (n-- > 0 && pos > 0) {
        pos = utf8_prev_char(str, pos);
    }
    return pos;
}


/* Returns the byte offset for the k-th character in a string. */
int32_t get_utf8_byte_offset(const Cell* s, const int32_t char_idx)
{
    if (s->ascii) { return char_idx; }

    if (char_idx <= s->char_count / 2) {
        return (int)utf8_advance(s->str, s->count, char_idx);
    }
    return (int)utf8_retreat(s->str, s->count,s->char_count - char_idx);
}


/* Calculate char-length of a UTF-8 string.
 * This is naïve, and will not correctly count multi-codepoint 'chars'. */
int32_t utf8_strlen(const char* s)
{
    int32_t len = 0;
    for (; *s; ++s) {
        if ((*s & UTF8_MASK_TAG) != UTF8_CONT_TAG) { ++len; }
    }
    return len;
}


/* Validate UTF-8 byte sequences for correctness and identify invalid characters using SIMD
 * (Single Instruction, Multiple Data) principles within a single register (SWAR). */
bool is_pure_ascii(const char *str, size_t len) {
    const unsigned char *ptr = (const unsigned char *)str;

    /* Alignment Loop: Process bytes one by one until pointer is 8-byte aligned.
       We check ((uintptr_t)ptr & 7) to see if we are aligned. */
    while (len > 0 && (uintptr_t)ptr & (WORD_SIZE - 1)) {
        if (*ptr & UTF8_CONT_TAG) { return false; }
        ptr++;
        len--;
    }

    /* Process 8 bytes at a time.
     * Cast to uint64_t pointer now that we know we are aligned. */
    const uint64_t *ptr64 = (const uint64_t *)ptr;
    // ReSharper disable once CppTooWideScope
    constexpr uint64_t high_bit_mask = 0x8080808080808080ULL;

    while (len >= WORD_SIZE) {
        /* If (chunk & mask) is non-zero, one of the bytes had the high bit set. */
        if (*ptr64 & high_bit_mask) {
            return false;
        }
        ptr64++;
        len -= WORD_SIZE;
    }

    /* Cleanup Loop: Process any remaining bytes (0-7 bytes left). */
    ptr = (const unsigned char *)ptr64;
    while (len > 0) {
        if (*ptr & UTF8_CONT_TAG) { return false; }
        ptr++;
        len--;
    }
    return true;
}


char* convert_to_utf8(const UChar* ustr)
{
    UErrorCode status = U_ZERO_ERROR;
    int32_t char_len = 0;
    char* result_utf8_str = nullptr;

    /* Get required buffer length in bytes. */
    u_strToUTF8(nullptr, 0, &char_len, ustr, -1, &status);

    if (status == U_BUFFER_OVERFLOW_ERROR) {
        status = U_ZERO_ERROR;
        result_utf8_str = (char*)GC_malloc(sizeof(char) * (char_len + 1));

        /* Actual Conversion. */
        u_strToUTF8(result_utf8_str, char_len + 1, nullptr, ustr, -1, &status);

        if (U_FAILURE(status)) {
            return nullptr;
        }
    }
    return result_utf8_str;
}


UChar* convert_to_utf16(const char* str)
{
    UErrorCode status = U_ZERO_ERROR;
    int32_t uchar_len = 0;
    UChar* my_utf16_str = nullptr;

    /* Pre-flight: Get the required buffer length in UChars. */
    u_strFromUTF8(nullptr, 0, &uchar_len, str, -1, &status);

    /* The pre-flight call sets an error code that we expect. */
    if (status == U_BUFFER_OVERFLOW_ERROR) {
        status = U_ZERO_ERROR; /* Reset status for the next call. */
        my_utf16_str = (UChar*)GC_malloc(sizeof(UChar) * (uchar_len + 1));
        if (!my_utf16_str) { return nullptr; }

        /* Call the function again with the allocated buffer. */
        u_strFromUTF8(my_utf16_str, uchar_len + 1, nullptr, str, -1, &status);

        if (U_FAILURE(status)) {
            return nullptr;
        }
    }
    return my_utf16_str;
}
