#ifndef COZENAGE_TYPES_H
#define COZENAGE_TYPES_H

#include "parser.h"
#include "environment.h"


typedef enum {
    LVAL_FLOAT = 1 << 0,    /* float represented as long double */
    LVAL_INT   = 1 << 1,    /* integer represented as long long */
    LVAL_BOOL  = 1 << 2,    /* #true / #false */
    LVAL_SYM   = 1 << 3,    /* symbols */
    LVAL_STR   = 1 << 4,    /* string literals */
    LVAL_SEXPR = 1 << 5,    /* s-expressions (for code) */
    LVAL_QEXPR = 1 << 6,    /* quoted expressions (for data) */
    LVAL_FUN   = 1 << 7,    /* functions (builtins or lambdas) */
    LVAL_ERR   = 1 << 8,    /* errors */
} l_val_t;

typedef struct l_val {
    int type;

    union {
        long double float_n;  /* floats */
        long long int int_n;  /* integers */
        int boolean;          /* 0 = false, 1 = true */
        char* sym;            /* symbols */
        char* str;            /* strings */
        struct {              /* for sexpr/qexpr */
            struct l_val** cell;
            int count;
        };
        struct {            /* for builtins (later user lambdas) */
            struct l_val* (*builtin)(l_env*, struct l_val*);
        };
    };
} l_val;

l_val* lval_float(long double n);
l_val* lval_int(long long n);
l_val* lval_bool(int b);
l_val* lval_sym(const char* s);
l_val* lval_str(const char* s);
l_val* lval_sexpr(void);
l_val* lval_qexpr(void);
l_val* lval_err(const char* m);
l_val *node_to_lval(Node *node);
void lval_del(l_val* v);
l_val* lval_pop(l_val* v, int i);
l_val* lval_take(l_val* v, int i);

#endif //COZENAGE_TYPES_H
