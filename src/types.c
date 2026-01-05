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

#include "types.h"
#include "cell.h"
#include "numerics.h"
#include "bignum.h"

#include <gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <unicode/ustring.h>


const char* fmt_err(const char *fmt, ...)
{
    static char buf[512];
    va_list args;
    va_start(args, fmt);
    if (vsnprintf(buf, 512, fmt, args) < 0) {
        fprintf(stderr, "vsnprintf failed!: %s\n", strerror(errno));
    }
    return buf;
}


/* Turn a single type into a string (for error reporting). */
const char* cell_type_name(const int t)
{
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
        case CELL_BIGINT:      return "bigint";
        case CELL_BIGFLOAT:    return "bigfloat";
        case CELL_PROMISE:     return "promise";
        case CELL_STREAM:      return "stream";
        case CELL_MACRO:       return "macro";
        default:               return "unknown";
    }
}


/* Turn a mask (possibly multiple flags ORed together) into a string
   e.g. (CELL_INTEGER | CELL_REAL) -> "int|real" */
const char* cell_mask_types(const int mask)
{
    static char buf[128];  /* static to return pointer safely. */
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
    if (mask & CELL_BIGINT)      strcat(buf, "bigint|");
    if (mask & CELL_BIGFLOAT)    strcat(buf, "bigfloat|");
    if (mask & CELL_PROMISE)     strcat(buf, "promise|");
    if (mask & CELL_STREAM)      strcat(buf, "stream|");
    if (mask & CELL_MACRO)       strcat(buf, "macro|");

    /* Remove trailing '|'. */
    const size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '|') {
        buf[len-1] = '\0';
    }
    return buf;
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
    if (proc->type != CELL_PROC) return 0;

    const Cell* formals = proc->lambda->formals;

    /* No arguments. */
    if (formals->count == 0) return 0;

    /* If formals is a symbol (variadic), it accepts anything. */
    if (formals->type == CELL_SYMBOL) return 1;

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
 *       Helper functions for numeric type promotion       *
 * --------------------------------------------------------*/

/* Convertors. */
static Cell* int_to_rat(const Cell* v)
{
    return make_cell_rational(v->integer_v, 1, 0);
}


static Cell* int_to_real(const Cell* v)
{
    return make_cell_real((long double)v->integer_v);
}


static Cell* rat_to_real(const Cell* v)
{
    return make_cell_real((long double)v->num / (long double)v->den);
}


static Cell* to_complex(Cell* v)
{
    return make_cell_complex(v, make_cell_integer(0));
}


static Cell* to_bigint(const Cell* v)
{
    return make_cell_bigint(nullptr, v, 10);
}


/* Promote two numbers to the same type, modifying lhs and rhs in-place. */
void numeric_promote(Cell** lhs, Cell** rhs)
{
    Cell* a = *lhs;
    Cell* b = *rhs;

    if (a->type == CELL_BIGINT || b->type == CELL_BIGINT) {
        if (a->type != CELL_BIGINT) {
            a = to_bigint(a);
        }
        if (b->type != CELL_BIGINT) {
            b = to_bigint(b);
        }
    } else if (a->type == CELL_COMPLEX || b->type == CELL_COMPLEX) {
        if (a->type != CELL_COMPLEX) {
            a = to_complex(a);
        }
        if (b->type != CELL_COMPLEX) {
            b = to_complex(b);
        }
    } else if (a->type == CELL_REAL || b->type == CELL_REAL) {
        if (a->type == CELL_INTEGER || a->type == CELL_RATIONAL) {
            a = a->type == CELL_INTEGER ? int_to_real(a) : rat_to_real(a);
        }
        if (b->type == CELL_INTEGER || b->type == CELL_RATIONAL) {
            b = b->type == CELL_INTEGER ? int_to_real(b) : rat_to_real(b);
        }
    } else if (a->type == CELL_RATIONAL || b->type == CELL_RATIONAL) {
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


Cell* make_sexpr_from_list(Cell* v, const bool recurse)
{
    Cell* result = make_cell_sexpr();
    int count;

    /* A proper list is the simple case. */
    if (v->len != -1) {
        count = v->len;
        result->cell = GC_MALLOC(sizeof(Cell*) * count);
        const Cell* p = v;
        for (int i = 0; i < count; i++) {
            if (recurse && p->car->type == CELL_PAIR) {
                cell_add(result, make_sexpr_from_list(p->car, true));
            } else {
                cell_add(result, p->car);
            }
            p = p->cdr;
        }
        return result;
    }

    /* Improper List Handling. */
    count = 0;
    const Cell* traverser = v;

    /* Count the number of pairs in the chain. */
    while (traverser != NULL && traverser->type == CELL_PAIR) {
        count++;
        traverser = traverser->cdr;
    }

    /* Allocate space for all the cars from the pairs PLUS the final `cdr`. */
    result->cell = GC_MALLOC(sizeof(Cell*) * (count + 1));

    /* Copy the car of each pair. */
    Cell* p = v;
    for (int i = 0; i < count; i++) {
        Cell* item = p->car;

        if (recurse && item->type == CELL_PAIR) {
            cell_add(result, make_sexpr_from_list(item, true));
        } else {
            cell_add(result, item);
        }
        p = p->cdr;
    }

    /* Handling the tail of an improper list. */
    if (v->len == -1) {
        /* Found a dot. p is the tail. */
        if (recurse && (p->type == CELL_PAIR || p->type == CELL_SEXPR)) {

            const int old_count = result->count;
            const int tail_count = p->type == CELL_SEXPR ? p->count : p->len;
            const int new_total = old_count + tail_count;

            result->cell = GC_REALLOC(result->cell, sizeof(Cell*) * new_total);

            if (p->type == CELL_SEXPR) {
                /* Dissolve the S-expression directly into the result. */
                for (int i = 0; i < tail_count; i++) {
                    result->cell[old_count + i] = p->cell[i];
                }
            } else {
                /* Dissolve the pair-chain into the result. */
                const Cell* tail_p = p;
                for (int i = 0; i < tail_count; i++) {
                    result->cell[old_count + i] = tail_p->car;
                    tail_p = tail_p->cdr;
                }
            }
            result->count = new_total;
        } else {
            cell_add(result, p);
        }
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

/*-------------------------------------------*
 *       Miscellaneous numeric helpers       *
 * ------------------------------------------*/

/* Helper to make C complex from Cell complex */
long double complex cell_to_c_complex(const Cell* c)
{
    long double a = cell_to_long_double(c->real);
    long double b = cell_to_long_double(c->imag);

    return CMPLXL(a, b);
}


/* Helper to check if a non-complex numeric cell has a value of zero. */
bool cell_is_real_zero(const Cell* c)
{
    if (!c) return false;
    switch (c->type) {
        case CELL_BIGINT:
            if (mpz_cmp_si(*c->bi, 0) == 0) return true;
            return false;
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
    if (!c) return false;

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
    if (!c) return false;

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
    if (!c) return false;

    if (c->type == CELL_BIGINT) {
        return mpz_sgn(*c->bi) == 1 ? true : false;
    }

    const long double num = cell_to_long_double(c);
    return num > 0 ? true : false;
}


/* Helper for negative? (< 0) */
bool cell_is_negative(const Cell* c)
{
    if (!c) return false;

    if (c->type == CELL_BIGINT) {
        return mpz_sgn(*c->bi) == -1 ? true : false;
    }

    const long double num = cell_to_long_double(c);
    return num < 0 ? true : false;
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
        case CELL_BIGINT: return mpz_odd_p(*c->bi) == 1 ? true : false;
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
        case CELL_BIGINT: return mpz_even_p(*c->bi) == 1 ? true : false;
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
    if (a < 0) a = -a;
    if (b < 0) b = -b;
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
            /* This case should ideally not be reached if inputs are valid numbers. */
            return 0.0L;
    }
}


/* Helper to construct appropriate cell from a long double. */
Cell* make_cell_from_double(const long double d)
{
    if (d == floorl(d) && d >= LLONG_MIN && d <= LLONG_MAX) {
        return make_cell_integer((long long)d);
    }
    return make_cell_real(d);
}


/* A version of strdup that allocates memory using the garbage collector. */
char* GC_strdup(const char* s)
{
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


char* GC_strndup(const char* s, size_t byte_len)
{
    /* +1 for the null terminator (for C-compatibility/printing). */
    char* new_str = (char*)GC_MALLOC_ATOMIC(byte_len + 1);
    if (new_str == NULL) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }

    memcpy(new_str, s, byte_len);
    new_str[byte_len] = '\0'; /* Always safety-terminate. */
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
        name,                                      /* The key to search for. */
        named_chars,                              /* The array to search in. */
        sizeof(named_chars) / sizeof(NamedChar),   /* Number of elements in the array. */
        sizeof(NamedChar),                        /* The size of each element. */
        compare_named_chars                            /* The comparison function. */
    );
}


/* helper to get a pointer to the value in the Nth node of a list.
 * Returns NULL if the index is out of bounds or the input is not a list. */
Cell* list_get_nth_cell_ptr(const Cell* list, const long n)
{
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
    /* The value we want is the CAR of this final pair. */
    return current->car;
}


/* Helpers for dealing with strings and Unicode. */

int32_t string_length_utf8(const char* s)
{
    const int32_t len_bytes = (int)strlen(s);

    int32_t i = 0;
    int32_t code_point_count = 0;
    UChar32 c;

    /* Iterate through the string one code point at a time. */
    while (i < len_bytes) {
        U8_NEXT(s, i, len_bytes, c);
        /* A negative value for 'c' indicates an invalid UTF-8 sequence. */
        if (c < 0) {
            return -1;
        }
        code_point_count++;
    }
    return code_point_count;
}


/* Validate UTF-8 byte sequences for correctness and identify invalid characters using SIMD (Single Instruction,
 * Multiple Data) principles within a single register (SWAR). */
bool is_pure_ascii(const char *str, size_t len) {
    const unsigned char *ptr = (const unsigned char *)str;

    /* Alignment Loop: Process bytes one by one until pointer is 8-byte aligned.
       We check ((uintptr_t)ptr & 7) to see if we are aligned. */
    while (len > 0 && (uintptr_t)ptr & 7) {
        if (*ptr & 0x80) return false;
        ptr++;
        len--;
    }

    /* Process 8 bytes at a time.
       Cast to uint64_t pointer now that we know we are aligned. */
    const uint64_t *ptr64 = (const uint64_t *)ptr;
    // ReSharper disable once CppTooWideScope
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const uint64_t high_bit_mask = 0x8080808080808080ULL;

    while (len >= 8) {
        /* If (chunk & mask) is non-zero, one of the bytes had the high bit set. */
        if (*ptr64 & high_bit_mask) {
            return false;
        }
        ptr64++;
        len -= 8;
    }

    /* Cleanup Loop: Process any remaining bytes (0-7 bytes left). */
    ptr = (const unsigned char *)ptr64;
    while (len > 0) {
        if (*ptr & 0x80) return false;
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
