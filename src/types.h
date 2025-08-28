#ifndef COZENAGE_TYPES_H
#define COZENAGE_TYPES_H

#include "parser.h"
#include "environment.h"


/* Convenience macros for readability */
#define CHECK_ARITY_EXACT(a, n) \
lval_check_arity((a), (n), -1, -1)

#define CHECK_ARITY_MIN(a, n) \
lval_check_arity((a), -1, (n), -1)

#define CHECK_ARITY_MAX(a, n) \
lval_check_arity((a), -1, -1, (n))

#define CHECK_ARITY_RANGE(a, lo, hi) \
lval_check_arity((a), -1, (lo), (hi))

#define LVAL_AS_NUM(v) \
((v)->type == LVAL_INT ? (long double)(v)->int_n : (v)->float_n)


typedef enum {
    LVAL_INT      = 1 << 0,   /* integer */
    LVAL_FLOAT    = 1 << 1,   /* real (long double) */
    LVAL_RAT      = 1 << 2,   /* rational */
    LVAL_COMP     = 1 << 3,   /* complex number */

    LVAL_BOOL     = 1 << 4,   /* #t / #f */
    LVAL_CHAR     = 1 << 5,   /* character */
    LVAL_STR      = 1 << 6,   /* string */
    LVAL_SYM      = 1 << 7,   /* symbol */

    LVAL_PAIR     = 1 << 8,   /* cons cell */
    LVAL_NIL      = 1 << 9,   /* '() empty list */
    LVAL_VECT     = 1 << 10,  /* vector */
    LVAL_BYTE     = 1 << 11,  /* byte vector */

    LVAL_SEXPR    = 1 << 12,  /* a 'list' of types, used internally */
    LVAL_FUN      = 1 << 13,  /* procedure */
    LVAL_PORT     = 1 << 14,  /* port */
    LVAL_CONT     = 1 << 15,  /* continuation (maybe) */

    LVAL_ERR      = 1 << 16   /* error (interpreter-internal) */
} l_val_t;

typedef struct l_val {
    l_val_t type;               /* type of data the l_val holds */
    bool exact;                /* exact/inexact flag for numerics */

    union {
        long double float_n;     /* floats */
        long long int int_n;     /* integers */
        int boolean;             /* 0 = false, 1 = true */
        char char_val;           /* character literal #\a */
        char* sym;               /* symbols */
        char* str;               /* strings */

        struct {                 /* pairs */
            l_val* car;             /* first member */
            l_val* cdr;             /* second member */
        };

        struct {                 /* rationals */
            long int num;           /* numerator */
            long int den;           /* denominator */
        };

        struct {                 /* complex numbers */
            l_val* real;            /* real part */
            l_val* imag;            /* imaginary part */
        };

        struct {                 /* for compound types (sexpr, vectors, etc.) */
            l_val** cell;
            int count;
        };

        struct {                 /* for functions */
            char* name;          /* function name (optional, for builtins) */
            l_val* (*builtin)(l_env*, l_val*);
        };
    };
} l_val;

extern l_val* lval_nil;  /* declare the global singleton */

l_val* lval_float(long double n);
l_val* lval_int(long long n);
l_val* lval_rat(long int num, long int den);
l_val* lval_comp(l_val* real, l_val *imag);
l_val* lval_bool(int b);
l_val* lval_char(char c);
l_val* lval_vect(void);
l_val* lval_byte(void);
l_val* lval_sym(const char* s);
l_val* lval_str(const char* s);
l_val* lval_sexpr(void);
l_val* lval_new_nil(void);
l_val* lval_pair(l_val* car, l_val* cdr);
l_val* lval_err(const char* m);
l_val* lval_add(l_val* v, l_val* x);
void lval_del(l_val* v);
l_val* lval_copy(const l_val* v);
l_val* lval_pop(l_val* v, int i);
l_val* lval_take(l_val* v, int i);
l_val* lval_check_types(const l_val* a, int mask);
l_val* lval_check_arity(const l_val* a, int exact, int min, int max);
void numeric_promote(l_val** lhs, l_val** rhs);
l_val* lval_sexpr_from2(const l_val* a, const l_val* b);
l_val* lval_neg(l_val* x);
l_val* lval_rat_simplify(l_val* v);

#endif //COZENAGE_TYPES_H
