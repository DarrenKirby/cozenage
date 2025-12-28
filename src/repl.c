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
#include "repr.h"
#include "symbols.h"
#include "lexer.h"
#include "runner.h"

#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <gc/gc.h>


int is_repl;
extern char **scheme_procedures;

static volatile sig_atomic_t got_sigint = 0;
#ifdef __linux__
static volatile sig_atomic_t discard_line = 0;
#endif


static void sigint_handler(const int sig)
{
    (void)sig;
    got_sigint = 1;
    printf("\n");
}


#ifdef __linux__
/* Callback for Ctrl-G binding */
int discard_continuation(const int count, const int key)
{
    (void)count; (void)key;
    discard_line = 1;
    rl_replace_line("", 0);  /* clear current line buffer */
    rl_done = 1;             /* break out of readline() */
    return 0;
}
#endif


/* Read in the history. */
static void read_history_from_file()
{
    const char *hf = tilde_expand(HIST_FILE);
    if (access(hf, R_OK) == -1) {
        /* create empty file if it does not exist */
        FILE* f = fopen(hf, "w");

        if (f == NULL) {
            fprintf(stderr, "Error: Could not open or create history file: `%s`.\n", HIST_FILE);
        }
        fclose(f);
    }
    read_history(hf);
}


/* Write out the history. */
void save_history_to_file()
{
    const char *hf = tilde_expand(HIST_FILE);
    write_history(hf);
}

/* Count parens to decide if we have a full expression,
 * or need to wait for more input. */
int paren_balance(const char *s, int *in_string)
{
    int balance = 0;
    int escaped = 0;
    int string = *in_string;  /* carry-over state from previous line. */

    for (const char *p = s; *p; p++) {
        if (string) {
            if (escaped) {
                escaped = 0;
            } else if (*p == '\\') {
                escaped = 1;
            } else if (*p == '"') {
                string = 0; /* string closed. */
            }
            continue;
        }

        /* not in a string. */
        if (*p == '"') {
            string = 1;
            escaped = 0;
        } else if (*p == '#' && *(p+1) == '\\') {
            /* char literal — skip this and next. */
            p++;
            if (*p && *(p+1)) p++;
        } else if (*p == '(') {
            balance++;
        } else if (*p == ')') {
            balance--;
        }
    }

    *in_string = string;  /* pass string-state back. */
    return balance;
}


/* Allow multi-line input in the REPL using
 * balanced parenthesis heuristic.*/
static char* read_multiline(const char* prompt, const char* cont_prompt)
{
#ifdef USE_GNU_READLINE
    rl_attempted_completion_function = completion_dispatcher;
#endif
    size_t total_len = 0;
    int balance = 0;
    int in_string = 0;   /* track string literal state across lines. */

    char *line = readline(prompt);
    if (!line) return nullptr;
    if (got_sigint) {
        free(line);
        got_sigint = 0;
        return GC_strdup("");  /* return empty input so REPL just re-prompts. */
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
            return GC_strdup("");  /* abort multiline and reset prompt. */
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


/* REPL output. */
void coz_print(const Cell* v)
{
    if (v->type == CELL_UNSPEC) {
        return;
    }
    fprintf(stdout, "%s\n", cell_to_string(v, MODE_REPL));
}


/* Print a prompt, return the input to the REPL. */
char* coz_read()
{
    char *input = read_multiline(PS1_PROMPT, PS2_PROMPT);
    /* reset bold input. */
    printf("%s", ANSI_RESET);
    if (!input) {
        printf("\n");
        save_history_to_file();
        exit(0);
    }
    /* Add expression to history. */
    if (input != nullptr) {
        add_history(input);
    }
    return input;
}


/* repl()
 * Read-Evaluate-Print loop. */
void repl(Lex* e)
{
    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
        /* Get the input. */
        const char* input = coz_read();
        /* Run it through the lexer. */
        TokenArray* ta = scan_all_tokens(input);
        /* Run it through the parser and evaluate. */
        Cell* result = parse_all_expressions(e, ta, true);
        /* Print either new prompt or error. */
        if (!result) {
            continue;
        }
        if (result->type == CELL_ERROR) {
            coz_print(result);
        }
    }
}


int run_repl(const lib_load_config load_libs)
{
    /* Print Version and Exit Information. */
    printf("  %s%s%s version %s\n", ANSI_BLUE_B, APP_NAME, ANSI_RESET, APP_VERSION);
    printf("  Press <Ctrl+d> or type '(exit)' to quit\n\n");

    /* Initialize the is_repl global flag. */
    is_repl = 1;

    /* Set up keybinding and signal for Ctrl-G and CTRL-C. */
#ifdef __linux__
    rl_bind_key('\007', discard_continuation);  /* 7 = Ctrl-G. */
#endif
    signal(SIGINT, sigint_handler);

    /* Initialize symbol table with initial size of 128. */
    symbol_table = ht_create(128);
    /* Load readline history. */
    read_history_from_file();
    /* Initialize default ports. */
    init_default_ports();
    /* Initialize global singleton objects. */
    init_global_singletons();
    /* Initialize global environment. */
    Lex* e = lex_initialize_global_env();
    /* Load base procedures into the environment. */
    lex_add_builtins(e);
    /* Loads the CLI-specified libraries into the environment. */
    load_initial_libraries(e, load_libs);

#ifdef USE_GNU_READLINE
    /* Bind the TAB key to the completion function. */
    rl_bind_key('\t', rl_complete);
    /* Load tab-completion candidate array from symbols in the environment. */
    populate_dynamic_completions(e);
#endif

    /* Initialize special form lookup table */
    init_special_forms();

    /* Run until we don't. */
    repl(e);
    return 0;
}
