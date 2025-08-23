#ifndef COZENAGE_ENVIRONMENT_H
#define COZENAGE_ENVIRONMENT_H


struct l_val;   // forward declaration
typedef struct l_val l_val;
struct l_env;

typedef l_val*(*l_builtin)(struct l_env*, l_val*);

typedef struct l_env {
    int count;
    char** syms;     // array of symbol strings
    l_builtin* funs; // array of function pointers
} l_env;

void lenv_add_builtins(l_env* e);
l_env* lenv_new(void);
void lenv_del(l_env* e);
l_val* lenv_get(l_env* e, const l_val* k);

#endif //COZENAGE_ENVIRONMENT_H
