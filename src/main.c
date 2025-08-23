/* main.c */

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
 * through a 2 step lexer/parser stream, build an
 * AST in the form of nested Node objects, then
 * convert it to an l_val struct.
 * */
l_val* read(l_env* e) {
    char *input = readline(PROMPT_STRING);

    /* ctrl-d or exit to quit */
    if (input == NULL || strcmp(input, "exit") == 0) {
        printf("\n");
        lenv_del(e);
        exit(0);
    }

    Parser *p = parse_str(input);
    if (!p) {
        return NULL;
    }

    Node *ast = parse_form(p);
    if (!ast) {
        return NULL;
    }

    l_val *v = node_to_lval(ast);
    if (!v) {
        return NULL;
    }

    /* Only save input to history if it parses without error */
    add_history(input);
    free(input);
    free_tokens(p->array);
    free(p);
    free_node(ast);

    return v;
}

/* Forward declaration resolve circular dependency */
l_val* eval_sexpr(l_env* e, l_val* v);

/* eval()
 * Take an l_val struct and evaluate it in an environment,
 * producing a value, which is packed into another l_val.
 * */
l_val* eval(l_env* e, l_val* v) {
    if (!v) return NULL;

    switch (v->type) {
        case LVAL_SYM: {
            // look up symbol in environment
            l_val* x = lenv_get(e, v);
            lval_del(v);
            return x;
        }
        case LVAL_SEXPR:
            return eval_sexpr(e, v);
        default:
            // literals (numbers, strings, bools, etc.) just return themselves
            return v;
    }
}

/* This does not belong here, need to move it to an eval.c file
 * at some point */
l_val* eval_sexpr(l_env* e, l_val* v) {
    // 1) eval children
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = eval(e, v->cell[i]);
        if (v->cell[i]->type == LVAL_ERR) {
            // return the error and free container
            return lval_take(v, i);
        }
    }

    // 2) empty / single
    if (v->count == 0) { return v; }
    if (v->count == 1) { return lval_take(v, 0); }

    // 3) pop the function, leaving args in v
    l_val* f = lval_pop(v, 0);
    if (f->type != LVAL_FUN) {
        lval_del(f);
        lval_del(v);
        return lval_err("S-expression does not start with function");
    }

    // 4) call builtin with the remaining list; then clean up
    l_val* result = f->builtin(e, v);
    lval_del(f);
    return result;
}


/* print()
 * Take the l_val produced by eval and print it in a
 * context-specific, meaningful way.
 * */
void print(l_val* v) {
    print_lval(v);
}

/* repl()
 * Read-Evaluate-Print loop
 * */
void repl() {
    l_env* e = lenv_new();
    lenv_add_builtins(e);

    for (;;) {
        l_val *val = read(e);
        if (!val) {
            continue;
        }
        l_val *result = eval(e, val);
        if (!result) {
            continue;
        }
        print(result);
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
