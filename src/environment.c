
/* environment.c */

#include "environment.h"
#include "ops.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


l_env* lenv_new(void) {
    l_env* e = malloc(sizeof(l_env));
    e->count = 0;
    e->syms = NULL;
    e->funs = NULL;
    return e;
}

void lenv_del(l_env* e) {
    for (int i = 0; i < e->count; i++) {
        free(e->syms[i]);
    }
    free(e->syms);
    free(e->funs);
    free(e);
}

l_val* lenv_get(l_env* e, const l_val* k) {
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            // wrap the function pointer in an lval
            l_val* v = malloc(sizeof(l_val));
            v->type = LVAL_FUN;
            v->builtin = e->funs[i];
            return v;
        }
    }
    char err_buf[128];
    snprintf(err_buf, sizeof(err_buf), "Unbound symbol: '%s'", k->sym);
    return lval_err(err_buf);
}

void lenv_add_builtin(l_env* e, const char* name, const l_builtin func) {
    e->count++;
    e->syms = realloc(e->syms, sizeof(char*) * e->count);
    e->funs = realloc(e->funs, sizeof(l_builtin) * e->count);

    e->syms[e->count-1] = malloc(strlen(name)+1);
    strcpy(e->syms[e->count-1], name);

    e->funs[e->count-1] = func;
}

void lenv_add_builtins(l_env* e) {
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);
    //lenv_add_builtin(e, "%", builtin_mod);
    //lenv_add_builtin(e, "^", builtin_pow);
}
