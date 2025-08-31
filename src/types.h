#ifndef COZENAGE_TYPES_H
#define COZENAGE_TYPES_H

#include "parser.h"
#include "environment.h"


/* Convenience macros for readability */
#define CHECK_ARITY_EXACT(a, n) \
check_arg_arity((a), (n), -1, -1)

#define CHECK_ARITY_MIN(a, n) \
check_arg_arity((a), -1, (n), -1)

#define CHECK_ARITY_MAX(a, n) \
check_arg_arity((a), -1, -1, (n))

#define CHECK_ARITY_RANGE(a, lo, hi) \
check_arg_arity((a), -1, (lo), (hi))

#define VAL_AS_NUM(v) \
((v)->type == VAL_INT ? (long double)(v)->i_val : (v)->r_val)


typedef enum {
    VAL_INT     = 1 << 0,   /* integer */
    VAL_RAT     = 1 << 1,   /* rational */
    VAL_REAL    = 1 << 2,   /* real */
    VAL_COMPLEX = 1 << 3,   /* complex number */

    VAL_BOOL    = 1 << 4,   /* #t / #f */
    VAL_CHAR    = 1 << 5,   /* character */
    VAL_STR     = 1 << 6,   /* string */
    VAL_SYM     = 1 << 7,   /* symbol */

    VAL_PAIR    = 1 << 8,   /* cons cell */
    VAL_NIL     = 1 << 9,   /* '() empty list */
    VAL_VEC     = 1 << 10,  /* vector */
    VAL_BYTEVEC = 1 << 11,  /* byte vector */

    VAL_SEXPR   = 1 << 12,  /* a 'list' of types, used internally */
    VAL_PROC    = 1 << 13,  /* procedure */
    VAL_PORT    = 1 << 14,  /* port */
    VAL_CONT    = 1 << 15,  /* continuation (maybe) */

    VAL_ERR     = 1 << 16   /* error (interpreter-internal) */
} Cell_t;

typedef struct Cell {
    Cell_t type;               /* type of data the l_val holds */
    bool exact;                /* exact/inexact flag for numerics */

    union {
        long double r_val;    /* reals */
        long long int i_val;  /* integers */
        int b_val;            /* 0 = false, 1 = true */
        char c_val;           /* character literal #\a */
        char* sym;              /* symbols */
        char* str;              /* strings */

        struct {               /* pairs */
            Cell* car;           /* first member */
            Cell* cdr;           /* second member */
        };

        struct {               /* rationals */
            long int num;        /* numerator */
            long int den;        /* denominator */
        };

        struct {               /* complex numbers */
            Cell* real;          /* real part */
            Cell* imag;          /* imaginary part */
        };

        struct {               /* for compound types (sexpr, vectors, etc.) */
            Cell** cell;
            int count;
        };

        struct {                      /* built-in and user-defined procedures */
            char* name;                    /* optional, for printing name of builtins */
            Cell* (*builtin)(Lex*, Cell*); /* non-NULL → builtin, ignore formals/body/env */
            Cell* formals;                 /* non-NULL → user-defined lambda */
            Cell* body;                    /* S-expression for lambda */
            Lex* env;                      /* closure environment */
        };
    };
} Cell;

extern Cell* val_nil;  /* declare the global singleton */
typedef Cell* (*BuiltinFn)(Lex* e, Cell* args);

Cell* make_val_real(long double n);
Cell* make_val_int(long long n);
Cell* make_val_rat(long int num, long int den);
Cell* make_val_complex(Cell* real, Cell *imag);
Cell* make_val_bool(int b);
Cell* make_val_char(char c);
Cell* make_val_vect(void);
Cell* make_val_bytevec(void);
Cell* make_val_sym(const char* s);
Cell* make_val_str(const char* s);
Cell* make_val_sexpr(void);
Cell* make_val_nil(void);
Cell* make_val_pair(Cell* car, Cell* cdr);
Cell* make_val_err(const char* m);
Cell* cell_add(Cell* v, Cell* x);
void cell_delete(Cell* v);
Cell* cell_copy(const Cell* v);
Cell* cell_pop(Cell* v, int i);
Cell* cell_take(Cell* v, int i);
Cell* check_arg_types(const Cell* a, int mask);
Cell* check_arg_arity(const Cell* a, int exact, int min, int max);
void numeric_promote(Cell** lhs, Cell** rhs);
Cell* make_sexpr_len1(const Cell* a);
Cell* make_sexpr_len2(const Cell* a, const Cell* b);
Cell* negate_numeric(Cell* x);
Cell* simplify_rational(Cell* v);
long double cell_to_ld(Cell* c);
void complex_apply(BuiltinFn fn, Lex* e, Cell* result, Cell* rhs);

#endif //COZENAGE_TYPES_H
