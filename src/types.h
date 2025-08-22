#ifndef COZENAGE_TYPES_H
#define COZENAGE_TYPES_H

typedef enum {
    LVAL_NUM   = 1 << 0,    /* number (int/float generalized to double) */
    LVAL_BOOL  = 1 << 1,    /* #true / #false */
    LVAL_SYM   = 1 << 2,    /* symbols */
    LVAL_STR   = 1 << 3,    /* string literals */
    LVAL_SEXPR = 1 << 4,    /* s-expressions (for code) */
    LVAL_QEXPR = 1 << 5,    /* quoted expressions (for data) */
    LVAL_FUN   = 1 << 6,    /* functions (builtins or lambdas) */
    LVAL_ERR   = 1 << 7     /* errors */
} l_val_t;

typedef struct l_val {
    int type;

    union {
        double num;         /* numbers */
        int boolean;        /* 0 = false, 1 = true */
        char* sym;          /* symbols */
        char* str;          /* strings */
        struct {            /* for sexpr/qexpr */
            struct l_val** cell;
            int count;
        };
        struct {            /* for builtins (later user lambdas) */
            struct l_val* (*builtin)(struct l_val*);
        };
    };
} l_val;

l_val* lval_num(double n);
l_val* lval_bool(int b);
l_val* lval_sym(const char* s);
l_val* lval_str(const char* s);
l_val* lval_sexpr(void);
l_val* lval_qexpr(void);
l_val* lval_err(const char* m);

#endif //COZENAGE_TYPES_H