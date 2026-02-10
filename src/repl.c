/*
 * 'src/repl.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 - 2026 Darren Kirby <darren@dragonbyte.ca>
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
#include "line_edit.h"
#include "main.h"
#include "cell.h"
#include "types.h"
#include "repr.h"
#include "symbols.h"
#include "lexer.h"
#include "runner.h"
#include "config.h"

#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <gc/gc.h>


int is_repl;
extern char **scheme_procedures;


/* Read in the history. */
static void read_history_from_file()
{
    if (read_history(cozenage_history_path) != 0) {
        fprintf(stderr, "Could not read history file: %s\n", cozenage_history_path);
    }
}


/* Write out the history. */
void save_history_to_file()
{
    if (write_history(cozenage_history_path) != 0) {
        fprintf(stderr, "Could not write history file: %s\n", cozenage_history_path);
    }
}


/* Count parens to decide if we have a full expression,
 * or need to wait for more input. */
int paren_balance(const char *s, int *in_string)
{
    int balance = 0;
    int escaped = 0;
    int string = *in_string;  /* Carry-over state from previous line. */

    for (const char *p = s; *p; p++) {
        if (string) {
            if (escaped) {
                escaped = 0;
            } else if (*p == '\\') {
                escaped = 1;
            } else if (*p == '"') {
                string = 0; /* String closed. */
            }
            continue;
        }

        /* Not in a string. */
        if (*p == '"') {
            string = 1;
            escaped = 0;
        } else if (*p == '#' && *(p+1) == '\\') {
            /* Char literal — skip this and next. */
            p++;
            if (*p && *(p+1)) p++;
        } else if (*p == '(') {
            balance++;
        } else if (*p == ')') {
            balance--;
        }
    }

    *in_string = string;  /* Pass string-state back. */
    return balance;
}


/* Allow multi-line input in the REPL using
 * balanced parenthesis heuristic.*/
static char* read_multiline(const char* prompt, const char* cont_prompt)
{
    size_t total_len = 0;
    int balance = 0;
    int in_string = 0;   /* Track string literal state across lines. */

    le_result r = readline(prompt);
    /* Reset bold input. */
    printf("%s", ANSI_RESET);

    switch (r.status) {
        case LE_EOF:
            /* Return null for clean exit. */
            return nullptr;
        case LE_INTERRUPT:
        case LE_ABORT:
            /* Return empty string for new prompt. */
            return GC_strdup("");
        default:
            ;
    }

    /* Should not ever be null here, but be defensive. */
    if (!r.line)
        return GC_strdup("");
    balance += paren_balance(r.line, &in_string);

    char *input = GC_strdup(r.line);
    total_len = strlen(r.line);

    while (balance > 0 || in_string) {
        r = readline(cont_prompt);
        /* Reset bold input. */
        printf("%s", ANSI_RESET);

        switch (r.status) {
            case LE_EOF:
                /* Return null for clean exit. */
                return nullptr;
            case LE_INTERRUPT:
                /* Loop again to get new line in multiline mode. */
                continue;
            case LE_ABORT:
                /* Return empty string to discard multiline and get new prompt. */
                return GC_strdup("");
            default:
                balance += paren_balance(r.line, &in_string);

                const size_t line_len = strlen(r.line);
                char *tmp = GC_REALLOC(input, total_len + line_len + 2);
                if (!tmp) { fprintf(stderr, "ENOMEM: malloc failed\n"); exit(EXIT_FAILURE); }
                input = tmp;
                input[total_len] = '\n';
                memcpy(input + total_len + 1, r.line, line_len + 1);

                total_len += line_len + 1;
        }
    }
    return input;
}


/* REPL output. */
void coz_print(const Cell* v)
{
    fprintf(stdout, "%s\n", cell_to_string(v, MODE_REPL));
}


/* Print a prompt, return the input to the REPL. */
char* coz_read()
{
    char *input = read_multiline(PS1_PROMPT, PS2_PROMPT);

    if (!input) {
        printf("\n");
        printf("Caught Ctrl-D ... exiting.\n");
        save_history_to_file();
        exit(0);
    }

    /* Add expression to history, if it is not empty. */
    if (input[0] != '\0') {
        add_history_entry(input);
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
    /* Initialize signal handler. */
    install_signal_handlers();
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
    /* Load tab-completion candidate array from symbols in the environment. */
    populate_dynamic_completions(e);
    /* Initialize special form lookup table */
    init_special_forms();

    /* Run until we don't. */
    repl(e);
    return 0;
}
