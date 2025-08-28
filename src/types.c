/* types.c - definition of l_val constructors/destructors and helpers */

#include "types.h"
#include "parser.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* define the global nil */
l_val* lval_nil = NULL;

/* L_VAL constructors.... */

l_val* lval_new_nil(void) {
    if (!lval_nil) {
        lval_nil = malloc(sizeof(l_val));
        lval_nil->type = LVAL_NIL;
        /* no other fields needed */
    }
    return lval_nil;
}

l_val* lval_float(const long double n) {
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_FLOAT;
    v->exact = false;
    v->float_n = n;
    return v;
}

l_val* lval_int(const long long int n) {
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_INT;
    v->exact = true;
    v->int_n = n;
    return v;
}

l_val* lval_rat(const long int num, const long int den) {
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_RAT;
    v->exact = true;
    v->num = num;
    v->den = den;
    return lval_rat_simplify(v);
}

l_val* lval_comp(l_val* real, l_val *imag) {
    if (real->type == LVAL_COMP || imag->type == LVAL_COMP) {
        return lval_err("Cannot have complex real or imaginary parts.");
    }
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_COMP;
    v->real = real;
    v->imag = imag;
    v->exact = real->exact && imag->exact;
    return v;
}

l_val* lval_bool(const int b) {
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_BOOL;
    v->boolean = b ? 1 : 0;
    return v;
}

l_val* lval_sym(const char* s) {
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_SYM;
    v->sym = strdup(s);
    return v;
}

l_val* lval_str(const char* s) {
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_STR;
    v->str = strdup(s);
    return v;
}

l_val* lval_sexpr(void) {
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

l_val* lval_char(char c) {
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_CHAR;
    v->char_val = c;
    return v;
}

l_val* lval_pair(l_val* car, l_val* cdr) {
    l_val* v = calloc(1,sizeof(l_val));
    if (!v) {
        fprintf(stderr, "ENOMEM: lval_pair failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = LVAL_PAIR;
    v->car = car;
    v->cdr = cdr;
    return v;
}

l_val* lval_vect(void) {
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_VECT;
    v->cell = NULL;
    v->count = 0;
    return v;
}

l_val* lval_byte(void) {
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_BYTE;
    v->cell = NULL;
    v->count = 0;
    return v;
}

l_val* lval_err(const char* m) {
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_ERR;
    v->str = strdup(m);
    return v;
}

/* Not a type constructor - despite the name
 * this is a helper function used by lval_sexpr.
 */
l_val* lval_add(l_val* v, l_val* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(l_val*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}

l_val* lval_pop(l_val* v, const int i) {
    if (i < 0 || i >= v->count) return NULL; /* defensive */

    /* Grab item */
    l_val* x = v->cell[i];

    /* Shift the memory after the item at "i" over the top */
    if (i < v->count - 1) {
        memmove(&v->cell[i], &v->cell[i+1],
                sizeof(l_val*) * (v->count - i - 1));
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
        l_val** tmp = realloc(v->cell, sizeof(l_val*) * v->count);
        if (tmp) {
            v->cell = tmp;
        } /* else: on OOM we keep the old block (safe) */
    }
    return x;
}


/* Take an element out and delete the rest */
l_val* lval_take(l_val* v, const int i) {
    l_val* x = lval_pop(v, i);
    return x;
}

/* Recursively delete components of an l_val */
void lval_del(l_val* v) {
    if (!v) return;

    switch (v->type) {
    case LVAL_INT:
    case LVAL_FLOAT:
    case LVAL_RAT:
    case LVAL_BOOL:
    case LVAL_CHAR:
        /* nothing heap-allocated */
        break;

    case LVAL_NIL:
        /* NIL is a singleton; never free */
        return;

    case LVAL_SYM:
        free(v->sym);
        break;

    case LVAL_STR:
    case LVAL_ERR:
        free(v->str);
        break;

    case LVAL_SEXPR:
    case LVAL_VECT:
    case LVAL_BYTE:
        if (v->cell) {
            for (int i = 0; i < v->count; i++) {
                /* guard against NULL children just in case */
                if (v->cell[i]) lval_del(v->cell[i]);
            }
            free(v->cell);
            v->cell = NULL;
        }
        v->count = 0;
        break;

    case LVAL_PAIR:
        lval_del(v->car);
        lval_del(v->cdr);
        break;

    case LVAL_COMP:
        lval_del(v->real);
        lval_del(v->imag);
        break;

    case LVAL_FUN:
        if (v->name) free(v->name);
        /* TODO: free env/body if user-defined later */
        break;

    case LVAL_PORT:
    case LVAL_CONT:
        /* not implemented yet */
        break;

    default:
        fprintf(stderr, "lval_del: unknown type %d\n", v->type);
        break;
    }
    free(v);
}

/* Recursively deep-copy all components of an l_val */
l_val* lval_copy(const l_val* v) {
    if (!v) return NULL;

    l_val* copy = calloc(1, sizeof(l_val));  // zero-init for safety
    if (!copy) {
        fprintf(stderr, "ENOMEM: lval_copy failed\n");
        exit(EXIT_FAILURE);
    }

    copy->type = v->type;
    copy->exact = v->exact;

    switch (v->type) {
    case LVAL_INT:
        copy->int_n = v->int_n;
        break;
    case LVAL_FLOAT:
        copy->float_n = v->float_n;
        break;
    case LVAL_BOOL:
        copy->boolean = v->boolean;
        break;
    case LVAL_CHAR:
        copy->char_val = v->char_val;
        break;

    case LVAL_SYM:
        copy->sym = strdup(v->sym);
        break;

    case LVAL_STR:
    case LVAL_ERR:
        copy->str = strdup(v->str);
        break;

    case LVAL_FUN:
        copy->builtin = v->builtin;
        copy->name = v->name ? strdup(v->name) : NULL;
        /* TODO: deep copy env/body for lambdas */
        break;

    case LVAL_SEXPR:
    case LVAL_VECT:
    case LVAL_BYTE:
        copy->count = v->count;
        if (v->count) {
            copy->cell = malloc(sizeof(l_val*) * v->count);
        } else {
            copy->cell = NULL;
        }
        for (int i = 0; i < v->count; i++) {
            copy->cell[i] = lval_copy(v->cell[i]);
        }
        break;

    case LVAL_NIL:
        /* return the singleton instead of allocating */
        free(copy);
        return lval_new_nil();

    case LVAL_PAIR: {
        copy->car = lval_copy(v->car);
        copy->cdr = lval_copy(v->cdr);
        break;
        }

    case LVAL_RAT: {
        copy->num = v->num;
        copy->den = v->den;
        break;
    }

    case LVAL_COMP: {
        copy->real = lval_copy(v->real);
        copy->imag = lval_copy(v->imag);
        break;
        }

    case LVAL_PORT:
    case LVAL_CONT:
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
const char* lval_type_name(const int t) {
    switch (t) {
        case LVAL_INT:     return "integer";
        case LVAL_FLOAT:   return "float";
        case LVAL_RAT:     return "rational";
        case LVAL_COMP:    return "complex";
        case LVAL_BOOL:    return "bool";
        case LVAL_SYM:     return "symbol";
        case LVAL_STR:     return "string";
        case LVAL_SEXPR:   return "sexpr";
        case LVAL_NIL:     return "nil";
        case LVAL_FUN:     return "function";
        case LVAL_ERR:     return "error";
        case LVAL_PAIR:    return "pair";
        case LVAL_VECT:    return "vector";
        case LVAL_CHAR:    return "char";
        case LVAL_BYTE:    return "byte vector";
        default:           return "unknown";
    }
}

/* Turn a mask (possibly multiple flags ORed together) into a string
   e.g. (LVAL_INT | LVAL_FLOAT) -> "int|float" */
const char* lval_mask_types(const int mask) {
    static char buf[128];  /* static to return pointer safely */
    buf[0] = '\0';

    if (mask & LVAL_INT)      strcat(buf, "integer|");
    if (mask & LVAL_FLOAT)    strcat(buf, "float|");
    if (mask & LVAL_RAT)      strcat(buf, "rational|");
    if (mask & LVAL_COMP)     strcat(buf, "complex|");
    if (mask & LVAL_BOOL)     strcat(buf, "bool|");
    if (mask & LVAL_SYM)      strcat(buf, "symbol|");
    if (mask & LVAL_STR)      strcat(buf, "string|");
    if (mask & LVAL_SEXPR)    strcat(buf, "sexpr|");
    if (mask & LVAL_NIL)      strcat(buf, "nil|");
    if (mask & LVAL_FUN)      strcat(buf, "function|");
    if (mask & LVAL_ERR)      strcat(buf, "error|");
    if (mask & LVAL_PAIR)     strcat(buf, "pair|");
    if (mask & LVAL_VECT)     strcat(buf, "vector|");
    if (mask & LVAL_CHAR)     strcat(buf, "char|");
    if (mask & LVAL_BYTE)     strcat(buf, "byte vector|");

    /* remove trailing '|' */
    const size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '|') {
        buf[len-1] = '\0';
    }
    return buf;
}

/* Return NULL if all args are valid, else return an error lval* */
l_val* lval_check_types(const l_val* a, const int mask) {
    for (int i = 0; i < a->count; i++) {
        const l_val* arg = a->cell[i];

        /* bitwise AND: if arg->type isn't in mask, it's invalid */
        if (!(arg->type & mask)) {
            char buf[128];
            snprintf(buf, sizeof(buf),
                     "Bad type at arg %d: got %s, expected %s",
                     i+1,
                     lval_type_name(arg->type),
                     lval_mask_types(mask));
            return lval_err(buf);
        }
    }
    return NULL;
}

l_val* lval_check_arity(const l_val* a, const int exact, const int min, const int max) {
    const int argc = a->count;

    if (exact >= 0 && argc != exact) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "Arity error: expected exactly %d arg%s, got %d",
                 exact, exact == 1 ? "" : "s", argc);
        return lval_err(buf);
    }
    if (min >= 0 && argc < min) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "Arity error: expected at least %d arg%s, got %d",
                 min, min == 1 ? "" : "s", argc);
        return lval_err(buf);
    }
    if (max >= 0 && argc > max) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "Arity error: expected at most %d arg%s, got %d",
                 max, max == 1 ? "" : "s", argc);
        return lval_err(buf);
    }
    return NULL; /* all good */
}

/* Helper functions for numeric type promotion */

/* Convertors */
l_val* int_to_rat(l_val* v) {
    return lval_rat(v->int_n, 1);
}

l_val* int_to_float(l_val* v) {
    return lval_float((long double)v->int_n);
}

l_val* rat_to_float(l_val* v) {
    return lval_float((long double)v->num / (long double)v->den);
}

l_val* to_complex(l_val* v) {
    return lval_comp(lval_copy(v), lval_int(0));
}

/* Promote two numbers to the same type, modifying lhs and rhs in-place.
   Returns 0 on success, nonzero on error. */
void numeric_promote(l_val** lhs, l_val** rhs) {
    l_val* a = *lhs;
    l_val* b = *rhs;

    /* If either is complex, promote other to complex */
    if (a->type == LVAL_COMP || b->type == LVAL_COMP) {
        if (a->type != LVAL_COMP) a = to_complex(a);
        if (b->type != LVAL_COMP) b = to_complex(b);
    }
    /* If either is float, promote other to float */
    else if (a->type == LVAL_FLOAT || b->type == LVAL_FLOAT) {
        if (a->type == LVAL_INT) a = int_to_float(a);
        else if (a->type == LVAL_RAT) a = rat_to_float(a);

        if (b->type == LVAL_INT) b = int_to_float(b);
        else if (b->type == LVAL_RAT) b = rat_to_float(b);
    }
    /* If either is rat, promote other to rat */
    else if (a->type == LVAL_RAT || b->type == LVAL_RAT) {
        if (a->type == LVAL_INT) a = int_to_rat(a);
        if (b->type == LVAL_INT) b = int_to_rat(b);
    }
    /* else both are ints, nothing to do */

    *lhs = a;
    *rhs = b;
    //return 0;
}

/* Construct an S-expression with exactly two elements */
l_val* lval_sexpr_from2(const l_val* a, const l_val* b) {
    l_val* v = lval_sexpr();
    v->count = 2;
    v->cell = malloc(sizeof(l_val*) * 2);
    v->cell[0] = lval_copy(a);
    v->cell[1] = lval_copy(b);
    return v;
}

l_val* lval_neg(l_val* x) {
    lval_check_types(x, LVAL_INT|LVAL_FLOAT|LVAL_RAT|LVAL_COMP);
    switch (x->type) {
        case LVAL_INT: return lval_int(-x->int_n);
        case LVAL_RAT: return lval_rat(-x->num, x->den);
        case LVAL_FLOAT: return lval_float(-x->float_n);
        case LVAL_COMP:
            return lval_comp(
                lval_neg(x->real),
                lval_neg(x->imag)
            );
        default:
            return lval_err("lval_neg: Oops, this isn't right!");
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
l_val* lval_rat_simplify(l_val* v) {
    if (v->type != LVAL_RAT) {
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
