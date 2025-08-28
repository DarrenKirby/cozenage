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
#include <signal.h>


static volatile sig_atomic_t got_sigint = 0;
static volatile sig_atomic_t discard_line = 0;

static void sigint_handler(const int sig) {
    (void)sig;
    got_sigint = 1;
    printf("\n");
}

#ifdef __linux__
/* Callback for Ctrl-G binding */
int discard_continuation(const int count, const int key) {
    (void)count; (void)key;
    discard_line = 1;
    rl_replace_line("", 0);  /* clear current line buffer */
    rl_done = 1;             /* break out of readline() */
    return 0;
}
#endif

static char* read_multiline(const char* prompt, const char* cont_prompt) {
    char *line = NULL;
    char *input = NULL;
    size_t total_len = 0;
    int balance = 0;
    int in_string = 0;   /* track string literal state across lines */

    line = readline(prompt);
    if (!line) return NULL;
    if (got_sigint) {
        free(line);
        got_sigint = 0;
        return strdup("");  /* return empty input so REPL just re-prompts */
    }

    balance += paren_balance(line, &in_string);

    input = strdup(line);
    total_len = strlen(line);
    free(line);

    while (balance > 0 || in_string) {
        line = readline(cont_prompt);
        if (!line) break;
        if (got_sigint) {
            free(line);
            free(input);
            got_sigint = 0;
            return strdup("");  /* abort multiline and reset prompt */
        }

        balance += paren_balance(line, &in_string);

        const size_t line_len = strlen(line);
        char *tmp = realloc(input, total_len + line_len + 2);
        if (!tmp) { fprintf(stderr, "ENOMEM: malloc failed\n"); exit(EXIT_FAILURE); }
        input = tmp;
        input[total_len] = '\n';
        memcpy(input + total_len + 1, line, line_len + 1);

        total_len += line_len + 1;
        free(line);
    }

    return input;
}

/* read()
 * Take a line of input from a prompt and pass it
 * through a 2 step lexer/parser stream, and convert
 * it to an l_val struct.
 * */
Cell* coz_read(Lex* e) {
    char *input = read_multiline(PS1_PROMPT, PS2_PROMPT);
    /* reset bold input */
    printf("%s", ANSI_RESET);
    if (!input || strcmp(input, "exit") == 0) {
        printf("\n");
        lex_delete(e);
        exit(0);
    }

    Parser *p = parse_str(input);
    if (!p) { free(input); return NULL; }

    Cell *v = parse_tokens(p);

    free_tokens(p->array, p->size);
    free(p);
    if (v) add_history(input);
    free(input);

    return v;
}


/* Forward declaration to resolve circular dependency */
Cell* eval_sexpr(Lex* e, Cell* v);

/* coz_eval()
 * Evaluate an l_val in the given environment.
 * Literals evaluate to themselves; symbols are looked up.
 * S-expressions are recursively evaluated.
 */
Cell* coz_eval(Lex* e, Cell* v) {
    if (!v) return NULL;

    switch (v->type) {
        /* Symbols: look them up in the environment */
        case VAL_SYM: {
            Cell* x = lex_get(e, v);
            cell_delete(v);
            return x;
        }

        /* S-expressions: recursively evaluate */
        case VAL_SEXPR:
            return eval_sexpr(e, v);

        /* All literals evaluate to themselves */
        case VAL_INT:
        case VAL_REAL:
        case VAL_RAT:
        case VAL_COMPLEX:
        case VAL_BOOL:
        case VAL_CHAR:
        case VAL_STR:
        /* case LVAL_PAIR: is not necessary - no concept of 'pair literal' */
        case VAL_VEC:
        case VAL_BYTEVEC:
        case VAL_NIL:
        /* Functions, ports, continuations, and errors are returned as-is */
        case VAL_PROC:
        case VAL_PORT:
        case VAL_CONT:
        case VAL_ERR:
            return v;

        default:
            return make_val_err("Unknown l_val type in eval()");
    }
}

/* eval_sexpr()
 * Evaluate an S-expression.
 * 1) Evaluate each child.
 * 2) Handle empty or single-element S-expressions.
 * 3) Treat first element as function (symbol or builtin).
 */
Cell* eval_sexpr(Lex* e, Cell* v) {
    if (v->count == 0) return v;

    /* Grab first element without evaluating yet */
    Cell* first = cell_pop(v, 0);

    /* Special form: quote */
    if (first->type == VAL_SYM && strcmp(first->sym, "quote") == 0) {
        cell_delete(first);
        if (v->count != 1) {
            cell_delete(v);
            return make_val_err("quote takes exactly one argument");
        }
        return cell_take(v, 0);  /* return argument unevaluated */
    }

    /* Otherwise, evaluate first element normally (should become a function) */
    Cell* f = coz_eval(e, first);
    if (f->type != VAL_PROC) {
        printf(ANSI_RED_B);
        print_cell(f);
        printf(ANSI_RESET);
        cell_delete(f);
        cell_delete(v);
        return make_val_err("S-expression does not start with a procedure");
    }

    /* Now evaluate arguments (since it's not a special form) */
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = coz_eval(e, v->cell[i]);
        if (v->cell[i]->type == VAL_ERR) {
            cell_delete(f);
            return cell_take(v, i);
        }
    }

    /* Apply function */
    Cell* result = f->builtin(e, v);
    cell_delete(f);
    return result;
}

/* print()
 * Take the l_val produced by eval and print it in a
 * context-specific, meaningful way.
 * */
void coz_print(const Cell* v) {
    println_cell(v);
}

/* repl()
 * Read-Evaluate-Print loop
 * */
void repl() {
    Lex* e = lex_initialize();
    lex_add_builtins(e);

    for (;;) {
        Cell *val = coz_read(e);
        if (!val) {
            continue;
        }
        Cell *result = coz_eval(e, val);
        if (!result) {
            continue;
        }
        coz_print(result);
        cell_delete(result);
    }
}

int main(int argc, char** argv) {
    /* Print Version and Exit Information */
    printf("  %s%s%s Version %s\n", ANSI_BLUE_B, APP_NAME, ANSI_RESET, APP_VERSION);
    printf("  Press <Ctrl+d> or type 'exit' to quit\n\n");

    /* Set up keybinding and signal for Ctrl-G and CTRL-C */
#ifdef __linux__
    rl_bind_key('\007', discard_continuation);  /* 7 = Ctrl-G */
#endif
    signal(SIGINT, sigint_handler);

    /* Run until we don't */
    repl();
    return 0;
}
