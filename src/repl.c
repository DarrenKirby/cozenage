/*
 * 'src/repl.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025  Darren Kirby <darren@dragonbyte.ca>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "repl.h"
#include "compat_readline.h"
#include "main.h"
#include "cell.h"
#include "types.h"
#include "parser.h"
#include "repr.h"
#include "symbols.h"
#include "eval.h"
#include "lexer.h"
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <gc/gc.h>


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

static void read_history_from_file() {
    const char *hf = tilde_expand(HIST_FILE);
    if (access(hf, R_OK) == -1) {
        /* create empty file if it does not exist */
        FILE* f = fopen(hf, "w");

        if (f == NULL) {
            printf("Error: Could not read, open, or create history file `~/cozenage_history`.\n");
        }
        fclose(f);
    }
    read_history(hf);
}

static void save_history_to_file() {
    const char *hf = tilde_expand(HIST_FILE);
    write_history(hf);
}

/* Count '(' and ')' while ignoring:
   - anything inside string literals
   - character literals starting with "#\..." (including #\()/#\)),
   - line comments starting with ';' to end-of-line.
*/
int paren_balance(const char *s, int *in_string) {
    int balance = 0;
    int escaped = 0;
    int string = *in_string;  /* carry-over state from previous line */

    for (const char *p = s; *p; p++) {
        if (string) {
            if (escaped) {
                escaped = 0;
            } else if (*p == '\\') {
                escaped = 1;
            } else if (*p == '"') {
                string = 0; /* string closed */
            }
            continue;
        }

        /* not in a string */
        if (*p == '"') {
            string = 1;
            escaped = 0;
        } else if (*p == '#' && *(p+1) == '\\') {
            /* char literal — skip this and next */
            p++;
            if (*p && *(p+1)) p++;
        } else if (*p == '(') {
            balance++;
        } else if (*p == ')') {
            balance--;
        }
    }

    *in_string = string;  /* pass string-state back */
    return balance;
}


/* Allow multi-line input in the REPL */
static char* read_multiline(const char* prompt, const char* cont_prompt) {
    size_t total_len = 0;
    int balance = 0;
    int in_string = 0;   /* track string literal state across lines */

    char *line = readline(prompt);
    if (!line) return nullptr;
    if (got_sigint) {
        free(line);
        got_sigint = 0;
        return GC_strdup("");  /* return empty input so REPL just re-prompts */
    }

    balance += paren_balance(line, &in_string);

    char *input = GC_strdup(line);
    total_len = strlen(line);
    free(line);

    while (balance > 0 || in_string) {
        line = readline(cont_prompt);
        if (!line) break;
        if (got_sigint) {
            free(line);
            got_sigint = 0;
            return GC_strdup("");  /* abort multiline and reset prompt */
        }

        balance += paren_balance(line, &in_string);

        const size_t line_len = strlen(line);
        char *tmp = GC_REALLOC(input, total_len + line_len + 2);
        if (!tmp) { fprintf(stderr, "ENOMEM: malloc failed\n"); exit(EXIT_FAILURE); }
        input = tmp;
        input[total_len] = '\n';
        memcpy(input + total_len + 1, line, line_len + 1);

        total_len += line_len + 1;
        free(line);
    }
    return input;
}

/* print()
 * Take the Cell produced by eval and print
 * it formatted for the REPL
 * */
void coz_print(const Cell* v) {
    printf("%s\n", cell_to_string(v, MODE_REPL));
}

/* read()
 * Take a line of input from a prompt and pass it
 * through a 2 step lexer/parser stream, and convert
 * the value to a Cell struct.
 * */
Cell* coz_read(const Lex* e) {
    (void)e;
    char *input = read_multiline(PS1_PROMPT, PS2_PROMPT);
    /* reset bold input */
    printf("%s", ANSI_RESET);
    if (!input || strcmp(input, "exit") == 0) {
        printf("\n");
        save_history_to_file();
        exit(0);
    }

    TokenArray* ta = scan_all_tokens(input);
    Cell* parsed = parse_tokens_new(ta);
    if (!parsed) { return nullptr; }
    if (parsed->type == CELL_ERROR) {
        coz_print(parsed);
        return nullptr;
    }
    add_history(input);
    return parsed;
}

/* repl()
 * Read-Evaluate-Print loop */
void repl(Lex* e) {
    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
        Cell *val = coz_read(e);
        if (!val) {
            continue;
        }
        Cell *result = coz_eval(e, val);
        if (!result) {
            continue;
        }
        coz_print(result);
    }
}

int run_repl(const lib_load_config load_libs) {
    /* Print Version and Exit Information */
    printf("  %s%s%s version %s\n", ANSI_BLUE_B, APP_NAME, ANSI_RESET, APP_VERSION);
    printf("  Press <Ctrl+d> or type 'exit' to quit\n\n");

    /* Set up keybinding and signal for Ctrl-G and CTRL-C */
#ifdef __linux__
    rl_bind_key('\007', discard_continuation);  /* 7 = Ctrl-G */
#endif
    signal(SIGINT, sigint_handler);

    /* Initialize symbol table with initial size of 128 */
    symbol_table = ht_create(128);
    /* Load readline history */
    read_history_from_file();
    /* Initialize default ports */
    init_default_ports();
    /* Initialize global singleton objects, nil, #t, #f, and EOF */
    init_global_singletons();
    /* Initialize global environment */
    Lex* e = lex_initialize_global_env();
    /* Load (scheme base) procedures into the environment*/
    lex_add_builtins(e);
    /* Initialize special form lookup table */
    init_special_forms();
    /* Loads the CLI-specified R7RS libraries into the environment. */
    load_initial_libraries(e, load_libs);

    /* Run until we don't */
    repl(e);
    return 0;
}
