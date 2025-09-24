/* types.c - definition of Cell constructors/destructors and helpers */

#include "types.h"
#include "parser.h"
#include "ops.h"
#include <gc.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "printer.h"


/* define the global nil */
Cell* val_nil = NULL;

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
        return make_val_err("Cannot have complex real or imaginary parts.");
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

Cell* make_val_err(const char* m) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = VAL_ERR;
    v->exact = GEN_ERR;
    v->str = GC_strdup(m);
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

    case VAL_PORT:
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
            //Cell* old = a;
            a = to_complex(a);
        }
        if (b->type != VAL_COMPLEX) {
            //Cell* old = b;
            b = to_complex(b);
        }
    }
    else if (a->type == VAL_REAL || b->type == VAL_REAL) {
        if (a->type == VAL_INT || a->type == VAL_RAT) {
            //Cell* old = a;
            a = (a->type == VAL_INT) ? int_to_real(a) : rat_to_real(a);
        }
        if (b->type == VAL_INT || b->type == VAL_RAT) {
            //Cell* old = b;
            b = (b->type == VAL_INT) ? int_to_real(b) : rat_to_real(b);
        }
    }
    else if (a->type == VAL_RAT || b->type == VAL_RAT) {
        if (a->type == VAL_INT) {
            //Cell* old = a;
            a = int_to_rat(a);
        }
        if (b->type == VAL_INT) {
            //Cell* old = b;
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

/*-------------------------------------------*
 *       Miscellaneous numeric helpers       *
 * ------------------------------------------*/

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
            return make_val_err("negate_numeric: Oops, this isn't right!");
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
        Cell* err = make_val_err("simplify_rational: denominator is zero!");
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

/**
 * A version of strdup that allocates memory using the garbage collector.
 */
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

/**
 * A version of strndup that allocates memory using the garbage collector.
 */
char* GC_strndup(const char* s, const size_t n) {
    // if (s == NULL) {
    //     return NULL;
    // }
    /* Find the actual length of the substring, up to n. */
    size_t len = strnlen(s, n);

    // Allocate GC-managed memory.
    char* new_str = (char*) GC_MALLOC_ATOMIC(len + 1);
    if (new_str == NULL) {
        return NULL;
    }

    // Copy the content and null-terminate.
    memcpy(new_str, s, len);
    new_str[len] = '\0';

    return new_str;
}
