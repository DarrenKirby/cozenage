/* main.c - The main REPL */

#include "compat_readline.h"
#include "main.h"
#include "parser.h"
#include "printer.h"
#include "types.h"
#include "environment.h"
#include "eval.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


static volatile sig_atomic_t got_sigint = 0;
#ifdef __linux__
static volatile sig_atomic_t discard_line = 0;
#endif

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

/* Allow multi-line input in the REPL */
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
 * the value to a Cell struct.
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

/* print()
 * Take the Cell produced by eval and print it in a
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
