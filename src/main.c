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
    /* reset bold input */
    printf("%s", ANSI_RESET);
    if (!input || strcmp(input, "exit") == 0) { printf("\n"); lenv_del(e); exit(0); }

    Parser *p = parse_str(input);
    if (!p) { free(input); return NULL; }

    l_val *v = parse_form(p);

    free_tokens(p->array, p->size);
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
        /* case LVAL_PAIR: is not necessary - no concept of 'pair literal' */
        case LVAL_VECT:
        case LVAL_BYTEVEC:
        case LVAL_NIL:
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
    if (v->count == 0) return v;

    /* Grab first element without evaluating yet */
    l_val* first = lval_pop(v, 0);

    /* Special form: quote */
    if (first->type == LVAL_SYM && strcmp(first->sym, "quote") == 0) {
        lval_del(first);
        if (v->count != 1) {
            lval_del(v);
            return lval_err("quote takes exactly one argument");
        }
        return lval_take(v, 0);  // return argument unevaluated
    }

    /* Otherwise, evaluate first element normally (should become a function) */
    l_val* f = coz_eval(e, first);
    if (f->type != LVAL_FUN) {
        lval_del(f);
        lval_del(v);
        return lval_err("S-expression does not start with a procedure");
    }

    /* Now evaluate arguments (since it's not a special form) */
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = coz_eval(e, v->cell[i]);
        if (v->cell[i]->type == LVAL_ERR) {
            lval_del(f);
            return lval_take(v, i);
        }
    }

    /* Apply function */
    l_val* result = f->builtin(e, v);
    lval_del(f);
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
    printf("  Press <Ctrl+d> or type 'exit' to quit\n\n");

    /* Run until we don't */
    repl();
    return 0;
}
