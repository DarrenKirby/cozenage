/* types.c - definition of Cell constructors/destructors and helpers */

#include "types.h"
#include "parser.h"
#include "ops.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "printer.h"


/* define the global nil */
Cell* val_nil = NULL;

/*------------------------------------*
 *       Cell type constructors       *
 * -----------------------------------*/

Cell* make_val_nil(void) {
    if (!val_nil) {
        val_nil = malloc(sizeof(Cell));
        val_nil->type = VAL_NIL;
        /* no other fields needed */
    }
    return val_nil;
}

Cell* make_val_real(const long double n) {
    Cell* v = calloc(1, sizeof(Cell));
    v->type = VAL_REAL;
    v->exact = false;
    v->r_val = n;
    return v;
}

Cell* make_val_int(const long long int n) {
    Cell* v = calloc(1, sizeof(Cell));
    v->type = VAL_INT;
    v->exact = true;
    v->i_val = n;
    return v;
}

Cell* make_val_rat(const long int num, const long int den) {
    Cell* v = calloc(1, sizeof(Cell));
    v->type = VAL_RAT;
    v->exact = true;
    v->num = num;
    v->den = den;
    return simplify_rational(v);
}

Cell* make_val_complex(Cell* real, Cell *imag) {
    if (real->type == VAL_COMPLEX || imag->type == VAL_COMPLEX) {
        return make_val_err("Cannot have complex real or imaginary parts.");
    }
    Cell* v = calloc(1, sizeof(Cell));
    v->type = VAL_COMPLEX;
    v->real = real;
    v->imag = imag;
    v->exact = real->exact && imag->exact;
    return v;
}

Cell* make_val_bool(const int b) {
    Cell* v = calloc(1, sizeof(Cell));
    v->type = VAL_BOOL;
    v->b_val = b ? 1 : 0;
    return v;
}

Cell* make_val_sym(const char* s) {
    Cell* v = calloc(1, sizeof(Cell));
    v->type = VAL_SYM;
    v->sym = strdup(s);
    return v;
}

Cell* make_val_str(const char* s) {
    Cell* v = calloc(1, sizeof(Cell));
    v->type = VAL_STR;
    v->str = strdup(s);
    return v;
}

Cell* make_val_sexpr(void) {
    Cell* v = calloc(1, sizeof(Cell));
    v->type = VAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

Cell* make_val_char(char c) {
    Cell* v = calloc(1, sizeof(Cell));
    v->type = VAL_CHAR;
    v->c_val = c;
    return v;
}

Cell* make_val_pair(Cell* car, Cell* cdr) {
    Cell* v = calloc(1,sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: lval_pair failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = VAL_PAIR;
    v->car = car;
    v->cdr = cdr;
    return v;
}

Cell* make_val_vect(void) {
    Cell* v = calloc(1, sizeof(Cell));
    v->type = VAL_VEC;
    v->cell = NULL;
    v->count = 0;
    return v;
}

Cell* make_val_bytevec(void) {
    Cell* v = calloc(1, sizeof(Cell));
    v->type = VAL_BYTEVEC;
    v->cell = NULL;
    v->count = 0;
    return v;
}

Cell* make_val_err(const char* m) {
    Cell* v = calloc(1, sizeof(Cell));
    v->type = VAL_ERR;
    v->str = strdup(m);
    return v;
}

/*------------------------------------------------*
 *    Cell accessors, destructors, and helpers    *
 * -----------------------------------------------*/

Cell* cell_add(Cell* v, Cell* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(Cell*) * v->count);
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
       Do NOT call realloc(..., 0). */
    if (v->count == 0) {
        free(v->cell);
        v->cell = NULL;
    } else {
        /* Try to shrink the allocation; keep old pointer on OOM */
        Cell** tmp = realloc(v->cell, sizeof(Cell*) * v->count);
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

/* Recursively delete components of an l_val */
void cell_delete(Cell* v) {
    if (!v) return;

    switch (v->type) {
    case VAL_INT:
    case VAL_REAL:
    case VAL_RAT:
    case VAL_BOOL:
    case VAL_CHAR:
        /* nothing heap-allocated */
        break;

    case VAL_NIL:
        /* NIL is a singleton; never free */
        return;

    case VAL_SYM:
        free(v->sym);
        break;

    case VAL_STR:
    case VAL_ERR:
        free(v->str);
        break;

    case VAL_SEXPR:
    case VAL_VEC:
    case VAL_BYTEVEC:
        if (v->cell) {
            for (int i = 0; i < v->count; i++) {
                /* guard against NULL children just in case */
                if (v->cell[i]) cell_delete(v->cell[i]);
            }
            free(v->cell);
            v->cell = NULL;
        }
        v->count = 0;
        break;

    case VAL_PAIR:
        cell_delete(v->car);
        cell_delete(v->cdr);
        break;

    case VAL_COMPLEX:
        cell_delete(v->real);
        cell_delete(v->imag);
        break;

    case VAL_PROC:
        if (v->name) free(v->name);
        /* TODO: free env/body if user-defined later */
        break;

    case VAL_PORT:
    case VAL_CONT:
        /* not implemented yet */
        break;

    default:
        fprintf(stderr, "cell_delete: unknown type %d\n", v->type);
        break;
    }
    free(v);
}

/* Recursively deep-copy all components of a Cell */
Cell* cell_copy(const Cell* v) {
    if (!v) return NULL;

    Cell* copy = calloc(1, sizeof(Cell));
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
        copy->sym = strdup(v->sym);
        break;

    case VAL_STR:
    case VAL_ERR:
        copy->str = strdup(v->str);
        break;

    case VAL_PROC:
        copy->builtin = v->builtin;
        copy->name = v->name ? strdup(v->name) : NULL;
        /* TODO: deep copy env/body for lambdas */
        break;

    case VAL_SEXPR:
    case VAL_VEC:
    case VAL_BYTEVEC:
        copy->count = v->count;
        if (v->count) {
            copy->cell = malloc(sizeof(Cell*) * v->count);
        } else {
            copy->cell = NULL;
        }
        for (int i = 0; i < v->count; i++) {
            copy->cell[i] = cell_copy(v->cell[i]);
        }
        break;

    case VAL_NIL:
        /* return the singleton instead of allocating */
        free(copy);
        return make_val_nil();

    case VAL_PAIR: {
        copy->car = cell_copy(v->car);
        copy->cdr = cell_copy(v->cdr);
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

    case VAL_PORT:
    case VAL_CONT:
        /* shallow copy (all fields remain zeroed) */
        break;

    default:
        fprintf(stderr, "lval_copy: unknown type %d\n", v->type);
        free(copy);
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
        case VAL_PROC:     return "function";
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

/* Return NULL if all args are valid, else return an error lval* */
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
            return make_val_err(buf);
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
        return make_val_err(buf);
    }
    if (min >= 0 && argc < min) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "Arity error: expected at least %d arg%s, got %d",
                 min, min == 1 ? "" : "s", argc);
        return make_val_err(buf);
    }
    if (max >= 0 && argc > max) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "Arity error: expected at most %d arg%s, got %d",
                 max, max == 1 ? "" : "s", argc);
        return make_val_err(buf);
    }
    return NULL; /* all good */
}

/*---------------------------------------------------------*
 *       Helper functions for numeric type promotion       *
 * --------------------------------------------------------*/

/* Convertors */
Cell* int_to_rat(Cell* v) {
    return make_val_rat(v->i_val, 1);
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
            Cell* old = a;
            a = to_complex(a);
            cell_delete(old);
        }
        if (b->type != VAL_COMPLEX) {
            Cell* old = b;
            b = to_complex(b);
            cell_delete(old);
        }
    }
    else if (a->type == VAL_REAL || b->type == VAL_REAL) {
        if (a->type == VAL_INT || a->type == VAL_RAT) {
            Cell* old = a;
            a = (a->type == VAL_INT) ? int_to_real(a) : rat_to_real(a);
            cell_delete(old);
        }
        if (b->type == VAL_INT || b->type == VAL_RAT) {
            Cell* old = b;
            b = (b->type == VAL_INT) ? int_to_real(b) : rat_to_real(b);
            cell_delete(old);
        }
    }
    else if (a->type == VAL_RAT || b->type == VAL_RAT) {
        if (a->type == VAL_INT) {
            Cell* old = a;
            a = int_to_rat(a);
            cell_delete(old);
        }
        if (b->type == VAL_INT) {
            Cell* old = b;
            b = int_to_rat(b);
            cell_delete(old);
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
    v->cell = malloc(sizeof(Cell*) * 2);
    v->cell[0] = cell_copy(a);
    return v;
}

/* Construct an S-expression with exactly two elements */
Cell* make_sexpr_len2(const Cell* a, const Cell* b) {
    Cell* v = make_val_sexpr();
    v->count = 2;
    v->cell = malloc(sizeof(Cell*) * 2);
    v->cell[0] = cell_copy(a);
    v->cell[1] = cell_copy(b);
    return v;
}

/*-------------------------------------------*
 *       Miscellaneous numeric helpers       *
 * ------------------------------------------*/

Cell* negate_numeric(Cell* x) {
    check_arg_types(x, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    switch (x->type) {
        case VAL_INT: return make_val_int(-x->i_val);
        case VAL_RAT: return make_val_rat(-x->num, x->den);
        case VAL_REAL: return make_val_real(-x->r_val);
        case VAL_COMPLEX:
            return make_val_complex(
                negate_numeric(x->real),
                negate_numeric(x->imag)
            );
        default:
            return make_val_err("lval_neg: Oops, this isn't right!");
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

    if (v->den == 0) {
        /* undefined fraction, maybe return an error instead */
        return v;
    }

    long int g = gcd_ll(v->num, v->den);
    v->num /= g;
    v->den /= g;

    /* normalize sign: denominator always positive */
    if (v->den < 0) {
        v->den = -v->den;
        v->num = -v->num;
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

/* optimized helper for operating on complex numbers */
void complex_apply(BuiltinFn fn, Lex* e, Cell* result, Cell* rhs) {
    if (fn == builtin_mul) {
        long double a = cell_to_ld(result->real);
        long double b = cell_to_ld(result->imag);
        long double c = cell_to_ld(rhs->real);
        long double d = cell_to_ld(rhs->imag);

        long double real_val = a*c - b*d;
        long double imag_val = a*d + b*c;

        cell_delete(result->real);
        cell_delete(result->imag);

        result->real = make_val_real(real_val);
        result->imag = make_val_real(imag_val);
    }
    else if (fn == builtin_div) {
        long double a = cell_to_ld(result->real);
        long double b = cell_to_ld(result->imag);
        long double c = cell_to_ld(rhs->real);
        long double d = cell_to_ld(rhs->imag);

        long double denom = c*c + d*d;
        long double real_val = (a*c + b*d) / denom;
        long double imag_val = (b*c - a*d) / denom;

        cell_delete(result->real);
        cell_delete(result->imag);

        result->real = make_val_real(real_val);
        result->imag = make_val_real(imag_val);
    }
    else {
        /* addition/subtraction: elementwise using recursion */
        Cell* real_args = make_sexpr_len2(result->real, rhs->real);
        Cell* imag_args = make_sexpr_len2(result->imag, rhs->imag);

        Cell* new_real = fn(e, real_args);
        Cell* new_imag = fn(e, imag_args);

        cell_delete(real_args);
        cell_delete(imag_args);

        cell_delete(result->real);
        cell_delete(result->imag);

        result->real = new_real;
        result->imag = new_imag;
    }
}
