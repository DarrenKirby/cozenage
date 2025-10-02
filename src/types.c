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
#include "parser.h"
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


/* define the global nil */
Cell* val_nil = NULL;

/* default ports */
Cell* default_input_port  = NULL;
Cell* default_output_port = NULL;
Cell* default_error_port  = NULL;

void init_default_ports(void) {
    default_input_port  = make_val_port("stdin",  stdin,  INPUT_PORT, TEXT_PORT);
    default_output_port = make_val_port("stdout", stdout, OUTPUT_PORT, TEXT_PORT);
    default_error_port  = make_val_port("stderr", stderr, OUTPUT_PORT, TEXT_PORT);
}

/*------------------------------------*
 *       Cell type constructors       *
 * -----------------------------------*/

Cell* make_val_nil(void) {
    if (!val_nil) {
        val_nil = GC_MALLOC(sizeof(Cell));
        val_nil->type = VAL_NIL;
        /* no other fields needed */
    }
    return val_nil;
}

Cell* make_val_real(const long double n) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = VAL_REAL;
    v->exact = false;
    v->r_val = n;
    return v;
}

Cell* make_val_int(const long long int n) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = VAL_INT;
    v->exact = true;
    v->i_val = n;
    return v;
}

Cell* make_val_rat(const long int num, const long int den, const bool simplify) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = VAL_RAT;
    v->exact = true;
    v->num = num;
    v->den = den;
    if (simplify) {
        return simplify_rational(v);
    }
    return v;
}

Cell* make_val_complex(Cell* real, Cell *imag) {
    if (real->type == VAL_COMPLEX || imag->type == VAL_COMPLEX) {
        return make_val_err("Cannot have complex real or imaginary parts.", GEN_ERR);
    }
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = VAL_COMPLEX;
    v->real = real;
    v->imag = imag;
    v->exact = real->exact && imag->exact;
    return v;
}

Cell* make_val_bool(const int b) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = VAL_BOOL;
    v->b_val = b ? 1 : 0;
    return v;
}

Cell* make_val_sym(const char* s) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = VAL_SYM;
    v->exact = 1; /* Hack to flag env lookup of symbol: 1 = lookup 0 = don't */
    v->sym = GC_strdup(s);
    return v;
}

Cell* make_val_str(const char* s) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = VAL_STR;
    v->str = GC_strdup(s);
    return v;
}

Cell* make_val_sexpr(void) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = VAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

Cell* make_val_char(const UChar32 c) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = VAL_CHAR;
    v->c_val = c;
    return v;
}

Cell* make_val_pair(Cell* car, Cell* cdr) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = VAL_PAIR;
    v->car = car;
    v->cdr = cdr;
    v->len = -1;
    return v;
}

Cell* make_val_vect(void) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = VAL_VEC;
    v->cell = NULL;
    v->count = 0;
    return v;
}

Cell* make_val_bytevec(void) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = VAL_BYTEVEC;
    v->cell = NULL;
    v->count = 0;
    return v;
}

Cell* make_val_err(const char* m, err_t t) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = VAL_ERR;
    v->err_t = t;
    v->err = GC_strdup(m);
    return v;
}

Cell* make_val_port(const char* path, FILE* fh, const int io_t, const int stream_t) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->is_open = 1;
    v->type = VAL_PORT;
    v->path = GC_strdup(path);
    v->port_t = io_t;
    v->stream_t = stream_t;
    v->fh = fh;
    return v;
}

Cell* make_val_eof(void) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = VAL_EOF;
    return v;
}

/*------------------------------------------------*
 *    Cell accessors, destructors, and helpers    *
 * -----------------------------------------------*/

Cell* cell_add(Cell* v, Cell* x) {
    v->count++;
    v->cell = GC_REALLOC(v->cell, sizeof(Cell*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}

Cell* cell_pop(Cell* v, const int i) {
    if (i < 0 || i >= v->count) return NULL; /* defensive */

    /* Grab item */
    Cell* x = v->cell[i];

    /* Shift the memory after the item at "i" over the top */
    if (i < v->count - 1) {
        memmove(&v->cell[i], &v->cell[i+1],
                sizeof(Cell*) * (v->count - i - 1));
    }

    /* Decrease the count of items */
    v->count--;

    /* If there are no elements left, free the array and set to NULL.
       Do NOT call GC_REALLOC(..., 0). */
    if (v->count == 0) {
        v->cell = NULL;
    } else {
        /* Try to shrink the allocation; keep old pointer on OOM */
        Cell** tmp = GC_REALLOC(v->cell, sizeof(Cell*) * v->count);
        if (tmp) {
            v->cell = tmp;
        } /* else: on OOM we keep the old block (safe) */
    }
    return x;
}

/* Take an element out and delete the rest */
Cell* cell_take(Cell* v, const int i) {
    Cell* x = cell_pop(v, i);
    return x;
}

/* Recursively deep-copy all components of a Cell */
Cell* cell_copy(const Cell* v) {
    if (!v) return NULL;

    Cell* copy = GC_MALLOC(sizeof(Cell));
    if (!copy) {
        fprintf(stderr, "ENOMEM: cell_copy failed\n");
        exit(EXIT_FAILURE);
    }

    copy->type = v->type;
    copy->exact = v->exact;

    switch (v->type) {
    case VAL_INT:
        copy->i_val = v->i_val;
        break;
    case VAL_REAL:
        copy->r_val = v->r_val;
        break;
    case VAL_BOOL:
        copy->b_val = v->b_val;
        break;
    case VAL_CHAR:
        copy->c_val = v->c_val;
        break;

    case VAL_SYM:
        copy->sym = GC_strdup(v->sym);
        break;

    case VAL_STR:
    case VAL_ERR:
        copy->str = GC_strdup(v->str);
        break;

    case VAL_PROC:
        /* If it's a builtin, keep the function pointer; copy the name if present. */
        copy->builtin = v->builtin;
        copy->name = v->name ? GC_strdup(v->name) : NULL;

        if (v->builtin) {
            /* builtin: nothing else to deep-copy */
            copy->formals = NULL;
            copy->body = NULL;
            copy->env = NULL;
        } else {
            /* user lambda: deep copy formals and body; keep env pointer (closure) */
            copy->builtin = NULL;
            copy->formals = v->formals ? cell_copy(v->formals) : NULL;
            copy->body   = v->body   ? cell_copy(v->body)   : NULL;
            copy->env    = v->env;   /* DO NOT copy environments; share the pointer */
        }
        break;

    case VAL_SEXPR:
    case VAL_VEC:
    case VAL_BYTEVEC:
        copy->count = v->count;
        if (v->count) {
            copy->cell = GC_MALLOC(sizeof(Cell*) * v->count);
        } else {
            copy->cell = NULL;
        }
        for (int i = 0; i < v->count; i++) {
            copy->cell[i] = cell_copy(v->cell[i]);
        }
        break;

    case VAL_NIL:
        /* return the singleton instead of allocating */
        return make_val_nil();

    case VAL_PAIR: {
        copy->car = cell_copy(v->car);
        copy->cdr = cell_copy(v->cdr);
        copy->len = v->len;
        break;
        }

    case VAL_RAT: {
        copy->num = v->num;
        copy->den = v->den;
        break;
    }

    case VAL_COMPLEX: {
        copy->real = cell_copy(v->real);
        copy->imag = cell_copy(v->imag);
        break;
        }

    case VAL_PORT: {
        copy->is_open = v->is_open;
        copy->port_t = v->port_t;
        copy->stream_t = v->stream_t;
        copy->fh = v->fh;
        copy->path = GC_strdup(v->path);
        break;
    }

    case VAL_CONT:
        /* shallow copy (all fields remain zeroed) */
        break;

    default:
        fprintf(stderr, "cell_copy: unknown type %d\n", v->type);
        return NULL;
    }
    return copy;
}

/* Turn a single type into a string (for error reporting) */
const char* cell_type_name(const int t) {
    switch (t) {
        case VAL_INT:     return "integer";
        case VAL_REAL:   return "float";
        case VAL_RAT:     return "rational";
        case VAL_COMPLEX:    return "complex";
        case VAL_BOOL:    return "bool";
        case VAL_SYM:     return "symbol";
        case VAL_STR:     return "string";
        case VAL_SEXPR:   return "sexpr";
        case VAL_NIL:     return "nil";
        case VAL_PROC:     return "procedure";
        case VAL_ERR:     return "error";
        case VAL_PAIR:    return "pair";
        case VAL_VEC:    return "vector";
        case VAL_CHAR:    return "char";
        case VAL_BYTEVEC:    return "byte vector";
        default:           return "unknown";
    }
}

/* Turn a mask (possibly multiple flags ORed together) into a string
   e.g. (VAL_INT | VAL_REAL) -> "int|real" */
const char* cell_mask_types(const int mask) {
    static char buf[128];  /* static to return pointer safely */
    buf[0] = '\0';

    if (mask & VAL_INT)      strcat(buf, "integer|");
    if (mask & VAL_REAL)    strcat(buf, "real|");
    if (mask & VAL_RAT)      strcat(buf, "rational|");
    if (mask & VAL_COMPLEX)     strcat(buf, "complex|");
    if (mask & VAL_BOOL)     strcat(buf, "bool|");
    if (mask & VAL_SYM)      strcat(buf, "symbol|");
    if (mask & VAL_STR)      strcat(buf, "string|");
    if (mask & VAL_SEXPR)    strcat(buf, "sexpr|");
    if (mask & VAL_NIL)      strcat(buf, "nil|");
    if (mask & VAL_PROC)      strcat(buf, "procedure|");
    if (mask & VAL_ERR)      strcat(buf, "error|");
    if (mask & VAL_PAIR)     strcat(buf, "pair|");
    if (mask & VAL_VEC)     strcat(buf, "vector|");
    if (mask & VAL_CHAR)     strcat(buf, "char|");
    if (mask & VAL_BYTEVEC)     strcat(buf, "byte vector|");

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

/* Return NULL if all args are valid, else return an error lval*
 * 'a' is expected to be VAL_SEXPR holding the args */
Cell* check_arg_types(const Cell* a, const int mask) {
    for (int i = 0; i < a->count; i++) {
        const Cell* arg = a->cell[i];

        /* bitwise AND: if arg->type isn't in mask, it's invalid */
        if (!(arg->type & mask)) {
            char buf[128];
            snprintf(buf, sizeof(buf),
                     "Bad type at arg %d: got %s, expected %s",
                     i+1,
                     cell_type_name(arg->type),
                     cell_mask_types(mask));
            return make_val_err(buf, GEN_ERR);
        }
    }
    return NULL;
}

Cell* check_arg_arity(const Cell* a, const int exact, const int min, const int max) {
    const int argc = a->count;

    if (exact >= 0 && argc != exact) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "Arity error: expected exactly %d arg%s, got %d",
                 exact, exact == 1 ? "" : "s", argc);
        return make_val_err(buf, GEN_ERR);
    }
    if (min >= 0 && argc < min) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "Arity error: expected at least %d arg%s, got %d",
                 min, min == 1 ? "" : "s", argc);
        return make_val_err(buf, GEN_ERR);
    }
    if (max >= 0 && argc > max) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "Arity error: expected at most %d arg%s, got %d",
                 max, max == 1 ? "" : "s", argc);
        return make_val_err(buf, GEN_ERR);
    }
    return NULL; /* all good */
}

/*---------------------------------------------------------*
 *       Helper functions for numeric type promotion       *
 * --------------------------------------------------------*/

/* Convertors */
Cell* int_to_rat(Cell* v) {
    return make_val_rat(v->i_val, 1, 0);
}

Cell* int_to_real(Cell* v) {
    return make_val_real((long double)v->i_val);
}

Cell* rat_to_real(Cell* v) {
    return make_val_real((long double)v->num / (long double)v->den);
}

Cell* to_complex(Cell* v) {
    return make_val_complex(cell_copy(v), make_val_int(0));
}

/* Promote two numbers to the same type, modifying lhs and rhs in-place. */
void numeric_promote(Cell** lhs, Cell** rhs) {
    Cell* a = *lhs;
    Cell* b = *rhs;

    if (a->type == VAL_COMPLEX || b->type == VAL_COMPLEX) {
        if (a->type != VAL_COMPLEX) {
            a = to_complex(a);
        }
        if (b->type != VAL_COMPLEX) {
            b = to_complex(b);
        }
    }
    else if (a->type == VAL_REAL || b->type == VAL_REAL) {
        if (a->type == VAL_INT || a->type == VAL_RAT) {
            a = (a->type == VAL_INT) ? int_to_real(a) : rat_to_real(a);
        }
        if (b->type == VAL_INT || b->type == VAL_RAT) {
            b = (b->type == VAL_INT) ? int_to_real(b) : rat_to_real(b);
        }
    }
    else if (a->type == VAL_RAT || b->type == VAL_RAT) {
        if (a->type == VAL_INT) {
            a = int_to_rat(a);
        }
        if (b->type == VAL_INT) {
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
    Cell* v = make_val_sexpr();
    v->count = 1;
    v->cell = GC_MALLOC(sizeof(Cell*));
    v->cell[0] = cell_copy(a);
    return v;
}

/* Construct an S-expression with exactly two elements */
Cell* make_sexpr_len2(const Cell* a, const Cell* b) {
    Cell* v = make_val_sexpr();
    v->count = 2;
    v->cell = GC_MALLOC(sizeof(Cell*) * 2);
    v->cell[0] = cell_copy(a);
    v->cell[1] = cell_copy(b);
    return v;
}

/* Construct an S-expression with exactly four elements */
Cell* make_sexpr_len4(const Cell* a, const Cell* b, const Cell* c, const Cell* d) {
    Cell* v = make_val_sexpr();
    v->count = 4;
    v->cell = GC_MALLOC(sizeof(Cell*) * 4);
    v->cell[0] = cell_copy(a);
    v->cell[1] = cell_copy(b);
    v->cell[2] = cell_copy(c);
    v->cell[3] = cell_copy(d);
    return v;
}

Cell* make_sexpr_from_list(Cell* v) {
    Cell* result = make_val_sexpr();
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
    Cell* v = make_val_sexpr();
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
        if (current_item->type == VAL_SEXPR) {
            /* If the item is an inner s-expr, add the count of its children. */
            new_count += current_item->count;
        } else {
            new_count++;
        }
    }

    /* Allocation */
    Cell* result = make_val_sexpr();
    result->count = new_count;
    if (new_count == 0) {
        return result;
    }
    result->cell = GC_MALLOC(sizeof(Cell*) * new_count);

    /* Populate the new S-expression's cell array */
    int result_idx = 0;
    for (int i = 0; i < sexpr->count; i++) {
        const Cell* current_item = sexpr->cell[i];
        if (current_item->type == VAL_SEXPR) {
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
        case VAL_INT:
            return c->i_val == 0;
        case VAL_RAT:
            /* Assumes simplified rational where numerator would be 0. */
            return c->num == 0;
        case VAL_REAL:
            return c->r_val == 0.0L;
        default:
            return false;
    }
}

/* Helper to check if a cell represents an integer value, per R7RS tower. */
bool cell_is_integer(const Cell* c) {
    if (!c) return false;
    switch (c->type) {
        case VAL_INT:
            return true;
        case VAL_RAT:
            /* A simplified rational is an integer if its denominator is 1. */
            return c->den == 1;
        case VAL_REAL:
            /* A real is an integer if it has no fractional part. */
            return c->r_val == floorl(c->r_val);
        case VAL_COMPLEX:
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
        case VAL_INT:
        case VAL_RAT:
        case VAL_REAL:
            return true;
        case VAL_COMPLEX:
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
    if (c->type == VAL_COMPLEX) {
        /* Must be a real number to be positive */
        if (!cell_is_real_zero(c->imag)) return false;
        val_to_check = c->real;
    }

    switch (val_to_check->type) {
        case VAL_INT:  return val_to_check->i_val > 0;
        case VAL_REAL: return val_to_check->r_val > 0.0L;
        case VAL_RAT:  return val_to_check->num > 0; /* Assumes den is always positive */
        default:       return false;
    }
}

/* Helper for negative? (< 0) */
bool cell_is_negative(const Cell* c) {
    if (!c) return false;

    const Cell* val_to_check = c;
    if (c->type == VAL_COMPLEX) {
        /* Must be a real number to be negative */
        if (!cell_is_real_zero(c->imag)) return false;
        val_to_check = c->real;
    }

    switch (val_to_check->type) {
        case VAL_INT:  return val_to_check->i_val < 0;
        case VAL_REAL: return val_to_check->r_val < 0.0L;
        case VAL_RAT:  return val_to_check->num < 0; /* Assumes den is always positive */
        default:       return false;
    }
}

/* Helper for odd? */
bool cell_is_odd(const Cell* c) {
    /* Must be an integer to be odd or even. */
    if (!cell_is_integer(c)) {
        return false;
    }

    const Cell* int_cell = (c->type == VAL_COMPLEX) ? c->real : c;

    long long val;
    switch (int_cell->type) {
        case VAL_INT:  val = int_cell->i_val; break;
        case VAL_REAL: val = (long long)int_cell->r_val; break;
        case VAL_RAT:  val = int_cell->num; break; /* den is 1 if it's an integer */
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

    const Cell* int_cell = (c->type == VAL_COMPLEX) ? c->real : c;

    long long val;
    switch (int_cell->type) {
        case VAL_INT:  val = int_cell->i_val; break;
        case VAL_REAL: val = (long long)int_cell->r_val; break;
        case VAL_RAT:  val = int_cell->num; break; /* den is 1 if it's an integer */
        default: return false; /* Unreachable */
    }
    return (val % 2 == 0);
}


Cell* negate_numeric(Cell* x) {
    switch (x->type) {
        case VAL_INT: return make_val_int(-x->i_val);
        case VAL_RAT:
            return make_val_rat(-x->num, x->den, 1);
        case VAL_REAL:
            return make_val_real(-x->r_val);
        case VAL_COMPLEX:
            return make_val_complex(
                negate_numeric(x->real),
                negate_numeric(x->imag)
            );
        default:
            return make_val_err("negate_numeric: Oops, this isn't right!", GEN_ERR);
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
    if (v->type != VAL_RAT) {
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
        Cell* err = make_val_err("simplify_rational: denominator is zero!", GEN_ERR);
        return err;
    }
    if (v->num == v->den) {
        /* Return as integer 1 */
        return make_val_int(1);
    }
    if (v->den == 1) {
        /* Return as integer */
        Cell* int_cell = make_val_int(v->num);
        return int_cell;
    }
    return v;
}

/* Helper: convert numeric Cell to long double */
long double cell_to_ld(Cell* c) {
    switch (c->type) {
        case VAL_INT:  return (long double)c->i_val;
        case VAL_RAT:  return (long double)c->num / (long double)c->den;
        case VAL_REAL: return c->r_val;
        default:       return 0.0L; /* should not happen */
    }
}

/* Helper for performing arithmetic on complex numbers */
void complex_apply(BuiltinFn fn, Lex* e, Cell* result, Cell* rhs) {
    if (fn == builtin_add || fn == builtin_sub) {
        /* addition/subtraction: elementwise using recursion */
        Cell* real_args = make_sexpr_len2(result->real, rhs->real);
        Cell* imag_args = make_sexpr_len2(result->imag, rhs->imag);

        Cell* new_real = fn(e, real_args);
        Cell* new_imag = fn(e, imag_args);

        result->real = new_real;
        result->imag = new_imag;
        return;
    }

    /* Pointers to the four numeric components (a, b, c, d). */
    Cell* a = result->real;
    Cell* b = result->imag;
    Cell* c = rhs->real;
    Cell* d = rhs->imag;

    /* Create temporary argument lists and perform calculations */
    Cell* ac_args = make_sexpr_len2(a, c);
    Cell* bd_args = make_sexpr_len2(b, d);
    Cell* ad_args = make_sexpr_len2(a, d);
    Cell* bc_args = make_sexpr_len2(b, c);

    Cell* ac = builtin_mul(e, ac_args);
    Cell* bd = builtin_mul(e, bd_args);
    Cell* ad = builtin_mul(e, ad_args);
    Cell* bc = builtin_mul(e, bc_args);

    Cell* new_real = NULL;
    Cell* new_imag = NULL;

    if (fn == builtin_mul) {
        /* Create temporary argument lists for final operations */
        Cell* real_args = make_sexpr_len2(ac, bd);
        Cell* imag_args = make_sexpr_len2(ad, bc);

        new_real = builtin_sub(e, real_args);
        new_imag = builtin_add(e, imag_args);
    }
    else if (fn == builtin_div) {
        /* Create temporary argument lists for denominator calculation */
        Cell* c_sq_args = make_sexpr_len2(c, c);
        Cell* d_sq_args = make_sexpr_len2(d, d);

        Cell* c_sq = builtin_mul(e, c_sq_args);
        Cell* d_sq = builtin_mul(e, d_sq_args);

        Cell* denom_args = make_sexpr_len2(c_sq, d_sq);
        Cell* denom = builtin_add(e, denom_args);

        /* Create temporary argument lists for numerators */
        Cell* real_num_args = make_sexpr_len2(ac, bd);
        Cell* imag_num_args = make_sexpr_len2(bc, ad);

        Cell* real_num = builtin_add(e, real_num_args);
        Cell* imag_num = builtin_sub(e, imag_num_args);

        /* Create temporary argument lists for final division */
        Cell* final_real_args = make_sexpr_len2(real_num, denom);
        Cell* final_imag_args = make_sexpr_len2(imag_num, denom);

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
        case VAL_INT:
            return (long double)c->i_val;
        case VAL_RAT:
            return (long double)c->num / c->den;
        case VAL_REAL:
            return c->r_val;
        default:
            /* This case should ideally not be reached if inputs are valid numbers. */
            return 0.0L;
    }
}

/* Helper to construct appropriate cell from a long double */
Cell* make_cell_from_double(long double d) {
    if (d == floorl(d) && d >= LLONG_MIN && d <= LLONG_MAX) {
        return make_val_int((long long)d);
    }
    return make_val_real(d);
}

/* A version of strdup that allocates memory using the garbage collector. */
char* GC_strdup(const char* s) {
    if (s == NULL) {
        return NULL;
    }
    /* Allocate GC-managed memory for the new string. */
    size_t len = strlen(s) + 1;
    char* new_str = (char*) GC_MALLOC_ATOMIC(len);
    if (new_str == NULL) {
        /* Handle allocation failure if necessary */
        return NULL;
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
    char* new_str = (char*) GC_MALLOC_ATOMIC(len + 1);
    if (new_str == NULL) {
        return NULL;
    }

    /* Copy the content and null-terminate. */
    memcpy(new_str, s, len);
    new_str[len] = '\0';

    return new_str;
}

/* Mapping of char names to Unicode codepoints.
 * This array MUST be kept sorted alphabetically by the 'name' field. */
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
    const char* name_key = (const char*)key;
    const NamedChar* char_element = (const NamedChar*)element;
    return strcmp(name_key, char_element->name);
}

/* Returns a pointer to the found NamedChar, or NULL if not found. */
const NamedChar* find_named_char(const char* name) {
    return (const NamedChar*)bsearch(
        name,                                      /* The key to search for */
        named_chars,                              /* The array to search in */
        sizeof(named_chars) / sizeof(NamedChar),   /* Number of elements in the array */
        sizeof(NamedChar),                       /* The size of each element */
        compare_named_chars                            /* The comparison function */
    );
}

/* helper to get a pointer to the value in the Nth node of a list.
 * Returns NULL if the index is out of bounds or the input is not a list. */
Cell* list_get_nth_cell_ptr(Cell* list, long n) {
    Cell* current = list;
    for (long i = 0; i < n; i++) {
        /* Make sure we are still on a pair before trying to get the cdr */
        if (current->type != VAL_PAIR) {
            return NULL;
        }
        current = current->cdr;
    }

    /* After the loop, `current` should be the pair holding our desired element */
    if (current->type != VAL_PAIR) {
        return NULL;
    }
    /* The value we want is the CAR of this final pair */
    return current->car;
}

char* convert_to_utf8(const UChar* ustr) {
    UErrorCode status = U_ZERO_ERROR;
    int32_t char_len = 0;
    char* result_utf8_str = NULL;

    /* Get required buffer length in bytes */
    u_strToUTF8(NULL, 0, &char_len, ustr, -1, &status);

    if (status == U_BUFFER_OVERFLOW_ERROR) {
        status = U_ZERO_ERROR;
        result_utf8_str = (char*)GC_malloc(sizeof(char) * (char_len + 1));

        /* Actual Conversion */
        u_strToUTF8(result_utf8_str, char_len + 1, NULL, ustr, -1, &status);

        if (U_FAILURE(status)) {
            return NULL;
        }
    }
    return result_utf8_str;
}

UChar* convert_to_utf16(const char* str) {
    UErrorCode status = U_ZERO_ERROR;
    int32_t uchar_len = 0;
    UChar* my_utf16_str = NULL;

    /* Pre-flight: Get the required buffer length in UChars */
    u_strFromUTF8(NULL, 0, &uchar_len, str, -1, &status);

    /* The pre-flight call sets an error code that we expect. */
    if (status == U_BUFFER_OVERFLOW_ERROR) {
        status = U_ZERO_ERROR; // Reset status for the next call
        my_utf16_str = (UChar*)GC_malloc(sizeof(UChar) * (uchar_len + 1));
        if (!my_utf16_str) { return NULL; }

        /* Call the function again with the allocated buffer */
        u_strFromUTF8(my_utf16_str, uchar_len + 1, NULL, str, -1, &status);

        if (U_FAILURE(status)) {
            return NULL;
        }
    }
    return my_utf16_str;
}
