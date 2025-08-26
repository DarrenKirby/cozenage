#ifndef COZENAGE_ENVIRONMENT_H
#define COZENAGE_ENVIRONMENT_H


struct l_val;  // forward declaration
typedef struct l_val l_val;

typedef struct l_env {
    int count;
    char** syms;   // symbol names
    l_val** vals;  // values (can be LVAL_FUN, LVAL_INT, etc.)
} l_env;

/* Environment management */
l_env* lenv_new(void);
void lenv_del(l_env* e);

/* Environment operations */
l_val* lenv_get(const l_env* e, const l_val* k);
void lenv_put(l_env* e, const l_val* k, const l_val* v);

/* Builtin helpers */
l_val* lval_builtin(const char* name, l_val* (*func)(l_env*, l_val*));
void lenv_add_builtin(l_env* e, const char* name, l_val* (*func)(l_env*, l_val*));
void lenv_add_builtins(l_env* e);


#endif //COZENAGE_ENVIRONMENT_H
