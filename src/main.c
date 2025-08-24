/* main.c - The main REPL */

#include "compat_readline.h"
#include "main.h"
#include "parser.h"
#include "printer.h"
#include "types.h"
#include "environment.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* read()
 * Take a line of input from a prompt and pass it
 * through a 2 step lexer/parser stream, and convert
 * it to an l_val struct.
 * */
l_val* coz_read(l_env* e) {
    char *input = readline(PROMPT_STRING);
    if (!input || strcmp(input, "exit") == 0) { printf("\n"); lenv_del(e); exit(0); }

    Parser *p = parse_str(input);
    if (!p) { free(input); return NULL; }

    l_val *v = parse_form(p);
    //----------DEBUG----------------
    //printf("Right after call to parse_form:\n");
    //println_lval(v);
    //===============================
    free_tokens(p->array);
    free(p);
    if (v) add_history(input);
    free(input);

    return v;
}

/* Forward declaration to resolve circular dependency */
l_val* eval_sexpr(l_env* e, l_val* v);

/* eval()
 * Evaluate an l_val in the given environment.
 * Literals evaluate to themselves; symbols are looked up.
 * S-expressions are recursively evaluated.
 */
l_val* coz_eval(l_env* e, l_val* v) {
    if (!v) return NULL;

    switch (v->type) {
        /* Symbols: look them up in the environment */
        case LVAL_SYM: {
            l_val* x = lenv_get(e, v);
            lval_del(v);
            return x;
        }

        /* S-expressions: recursively evaluate */
        case LVAL_SEXPR:
            return eval_sexpr(e, v);

        /* All literals evaluate to themselves */
        case LVAL_INT:
        case LVAL_FLOAT:
        case LVAL_RAT:
        case LVAL_COMPLEX:
        case LVAL_BOOL:
        case LVAL_CHAR:
        case LVAL_STR:
        case LVAL_VECT:
        case LVAL_BYTEVEC:
        case LVAL_NIL:
            return v;

        /* Functions, ports, continuations, and errors are returned as-is */
        case LVAL_FUN:
        case LVAL_PORT:
        case LVAL_CONT:
        case LVAL_ERR:
            return v;

        default:
            return lval_err("Unknown l_val type in eval()");
    }
}

/* eval_sexpr()
 * Evaluate an S-expression.
 * 1) Evaluate each child.
 * 2) Handle empty or single-element S-expressions.
 * 3) Treat first element as function (symbol or builtin).
 */
l_val* eval_sexpr(l_env* e, l_val* v) {
    if (DEBUG) {
        printf("Immediately after entering eval_sexpr:\n");
        println_lval(v);
    }
    if (v->count == 0) return v;

    /* Step 1: Evaluate the first element (the function/symbol) */
    l_val* f = coz_eval(e, lval_pop(v, 0));
    if (f->type != LVAL_FUN) {
        lval_del(f);
        lval_del(v);
        return lval_err("S-expression does not start with function");
    }

    /* Step 2: Decide whether to pre-evaluate arguments
    *  Special forms like 'quote' or user-defined macros do NOT evaluate args here
    * */
    int special_form = 0;
    if (f->name && strcmp(f->name, "quote") == 0) {
        special_form = 1;
    }

    if (!special_form) {
        /* Evaluate arguments */
        for (int i = 0; i < v->count; i++) {
            v->cell[i] = coz_eval(e, v->cell[i]);
            if (v->cell[i]->type == LVAL_ERR) {
                return lval_take(v, i);
            }
        }
    }
    /* Step 3: Call the function with v as arguments */
    l_val* result = f->builtin(e, v);
    lval_del(f);
    lval_del(v);
    return result;
}

/* print()
 * Take the l_val produced by eval and print it in a
 * context-specific, meaningful way.
 * */
void coz_print(const l_val* v) {
    println_lval(v);
}

/* repl()
 * Read-Evaluate-Print loop
 * */
void repl() {
    l_env* e = lenv_new();
    lenv_add_builtins(e);

    for (;;) {
        l_val *val = coz_read(e);
        if (!val) {
            continue;
        }
        l_val *result = coz_eval(e, val);
        if (!result) {
            continue;
        }
        coz_print(result);
        lval_del(result);
    }
}

int main(int argc, char** argv) {
    /* Print Version and Exit Information */
    printf("  %s%s%s Version %s\n", ANSI_BLUE_B, APP_NAME, ANSI_RESET, APP_VERSION);
    printf("  Press <Ctrl+d> to exit\n\n");

    /* Run until we don't */
    repl();
    return 0;
}
