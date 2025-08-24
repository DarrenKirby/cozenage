/* types.c - definition of l_val constructors/destructors and helpers */

#include "types.h"
#include "parser.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// double parse_float(const char* str, char* errbuf, size_t errbuf_sz, int* ok) {
//     errno = 0;
//     char* endptr;
//     const double val = strtod(str, &endptr);
//     if (endptr == str) {
//         /* No conversion performed at all */
//         snprintf(errbuf, errbuf_sz, "invalid number: '%s'", str);
//         *ok = 0;
//         return 0.0;
//     }
//     if (errno == ERANGE) {
//         /* Underflow or overflow */
//         snprintf(errbuf, errbuf_sz, "number out of range: '%s'", str);
//         *ok = 0;
//         return 0.0;
//     }
//     /* Trailing non-numeric chars */
//     if (*endptr != '\0') {
//         snprintf(errbuf, errbuf_sz, "invalid trailing characters in number: '%s'", str);
//         *ok = 0;
//         return 0.0;
//     }
//
//     *ok = 1;
//     return val;
// }
//
// long long int parse_int(const char* str, char* errbuf, size_t errbuf_sz, int* ok) {
//     errno = 0;
//     char* endptr;
//     const long long int val = strtoll(str, &endptr,  10);
//     if (endptr == str) {
//         /* No conversion performed at all */
//         snprintf(errbuf, errbuf_sz, "invalid number: '%s'", str);
//         *ok = 0;
//         return 0;
//     }
//
//     if (errno == ERANGE) {
//         /* Underflow or overflow */
//         snprintf(errbuf, errbuf_sz, "number out of range: '%s'", str);
//         *ok = 0;
//         return 0;
//     }
//     /* Trailing non-numeric chars */
//     if (*endptr != '\0') {
//         snprintf(errbuf, errbuf_sz, "invalid trailing characters in number: '%s'", str);
//         *ok = 0;
//         return 0;
//     }
//     *ok = 1;
//     return val;
// }

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
    v->float_n = n;
    return v;
}

l_val* lval_int(const long long int n) {
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_INT;
    v->int_n = n;
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
    l_val* v = calloc(1, sizeof(l_val)); // zero everything
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

l_val* lval_vect(void) {
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_VECT;
    v->cell = NULL;
    v->count = 0;
    return v;
}

l_val* lval_bytevect(void) {
    l_val* v = calloc(1, sizeof(l_val));
    v->type = LVAL_BYTEVEC;
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

/* Pop a value out of an s-expression at index i */
l_val* lval_pop(l_val* v, const int i) {
    /* Find the item at "i" */
    l_val* x = v->cell[i];

    /* Shift the memory after the item at "i" over the top */
    memmove(&v->cell[i], &v->cell[i+1],
        sizeof(l_val*) * (v->count-i-1));

    /* Decrease the count of items */
    v->count--;

    /* Reallocate the memory used */
    v->cell = realloc(v->cell, sizeof(l_val*) * v->count);
    return x;
}

/* Take an element out and delete the rest */
l_val* lval_take(l_val* v, const int i) {
    l_val* x = lval_pop(v, i);
    lval_del(v);
    return x;
}

void lval_del(l_val* v) {
    if (!v) return;

    switch (v->type) {
    case LVAL_INT:
    case LVAL_FLOAT:
    case LVAL_BOOL:
    case LVAL_COMPLEX:
    case LVAL_RAT:
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
    case LVAL_BYTEVEC:
        for (int i = 0; i < v->count; i++) {
            lval_del(v->cell[i]);
        }
        free(v->cell);
        break;

    case LVAL_PAIR:
        /* Not implemented yet */
        // lval_del(v->car);
        // lval_del(v->cdr);
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

l_val* lval_copy(const l_val* v) {
    if (!v) return NULL;

    l_val* copy = calloc(1, sizeof(l_val));  // zero-init for safety
    if (!copy) {
        fprintf(stderr, "ENOMEM: lval_copy failed\n");
        exit(EXIT_FAILURE);
    }

    copy->type = v->type;

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
    case LVAL_BYTEVEC:
        copy->count = v->count;
        copy->cell = v->count ? malloc(sizeof(l_val*) * v->count) : NULL;
        for (int i = 0; i < v->count; i++) {
            copy->cell[i] = lval_copy(v->cell[i]);
        }
        break;

    case LVAL_NIL:
        /* return the singleton instead of allocating */
        free(copy);
        return (l_val*)v; // or return global_nil if you prefer
        break;

    case LVAL_PAIR:
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
        case LVAL_INT:     return "int";
        case LVAL_FLOAT:   return "float";
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
        case LVAL_BYTEVEC: return "byte vector";
        default:           return "unknown";
    }
}

/* Turn a mask (possibly multiple flags ORed together) into a string
   e.g. (LVAL_INT | LVAL_FLOAT) -> "int|float" */
const char* lval_mask_types(const int mask) {
    static char buf[128];  /* static to return pointer safely */
    buf[0] = '\0';

    if (mask & LVAL_INT)      strcat(buf, "int|");
    if (mask & LVAL_FLOAT)    strcat(buf, "float|");
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
    if (mask & LVAL_BYTEVEC)  strcat(buf, "byte vector|");

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
                     "Bad type at arg%s %d: got %s, expected %s",
                     i == 1 ? "" : "s",
                     i,
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
