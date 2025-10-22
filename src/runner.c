/*
 * 'src/runner.c'
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

#include "runner.h"
#include "symbols.h"
#include "special_forms.h"
#include "load_library.h"
#include "parser.h"
#include "eval.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <gc/gc.h>


/* Checks the file extension and prints a warning if it's non-standard. */
static void check_and_warn_extension(const char *file_path) {
    const char *ext = strrchr(file_path, '.');

    /* Does the file have an extension? Is the extension NOT .scm AND NOT .ss? */
    if (ext != NULL && strcmp(ext, ".scm") != 0 && strcmp(ext, ".ss") != 0) {
        /* Print the non-fatal warning to stderr */
        fprintf(stderr,
                "Warning: Running file '%s' which does not have the standard .scm or .ss extension.\n",
                file_path);
    }
}

/* Loads the specified R7RS libraries into the environment. */
static void load_initial_libraries(const Lex* e, lib_load_config load_libs) {
    printf("Initializing interpreter environment...\n");

    if (load_libs.coz_ext) {
        (void)load_scheme_library("coz-ext", e);
    }
    if (load_libs.file) {
        (void)load_scheme_library("file", e);
    }
    if (load_libs.process_context) {
        (void)load_scheme_library("process_context", e);
    }
    if (load_libs.inexact) {
        (void)load_scheme_library("inexact", e);
    }
    if (load_libs.complex) {
        (void)load_scheme_library("complex", e);
    }
    if (load_libs.char_lib) {
        (void)load_scheme_library("char", e);
    }
    if (load_libs.read) {
        (void)load_scheme_library("read", e);
    }
    if (load_libs.write) {
        (void)load_scheme_library("write", e);
    }
    if (load_libs.eval) {
        (void)load_scheme_library("eval", e);
    }
    if (load_libs.cxr) {
        (void)load_scheme_library("cxr", e);
    }
    /* Cozenage libs */
    if (load_libs.coz_bits) {
        (void)load_scheme_library("bits", e);
    }
}

static char* collect_one_expression_from_file(FILE *input_file) {
    int c;
    int paren_depth = 0;

    /* Initial buffer size and allocation (will dynamically resize if needed) */
    size_t capacity = 1024;
    size_t length = 0;
    char *buffer = malloc(capacity);
    if (!buffer) {
        perror("Failed to allocate memory for expression buffer");
        return NULL;
    }

    /* Skip leading whitespace and comments until an expression starts or EOF */
    while ((c = fgetc(input_file)) != EOF) {
        if (c == ';') {
            /* Skip single-line comment */
            while ((c = fgetc(input_file)) != EOF && c != '\n') { ; }
            continue;
        }
        if (!isspace(c)) {
            /* Found the start of a token/expression, push the character back
             * and break the loop to start collecting. */
            ungetc(c, input_file);
            break;
        }
    }

    if (c == EOF) {
        free(buffer);
        return NULL;
    }

    /* Collect characters until the expression is balanced and complete */
    while ((c = fgetc(input_file)) != EOF) {
        /* Append Character to Buffer */
        if (length + 1 >= capacity) {
            capacity *= 2;
            char *new_buffer = realloc(buffer, capacity);
            if (!new_buffer) {
                perror("Failed to reallocate buffer");
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
        }
        buffer[length++] = (char)c;

        /* Update Parenthesis Depth */
        if (c == '(') {
            paren_depth++;
        } else if (c == ')') {
            paren_depth--;
        }

        /* If we hit a character that usually follows an expression (space, newline, EOF)
         * AND the top-level parenthesis are balanced (depth == 0)
         * AND we actually collected some characters (length > 0) */
        if (paren_depth == 0 && length > 0 && (isspace(fgetc(input_file)) || feof(input_file))) {
            /* Push the lookahead character back (space/newline/EOF) */
            ungetc(fgetc(input_file), input_file);
            break; /* Expression is complete */
        }
    }

    /* Null-terminate the string */
    buffer[length] = '\0';

    /* Check for unbalanced expressions at EOF */
    if (paren_depth != 0) {
        fprintf(stderr, "Error: Unbalanced expression found at end of file.\n");
        free(buffer);
        return NULL;
    }

    return buffer;
}

int run_file_script(const char *file_path, lib_load_config config) {
    FILE *script_file = NULL;
    int exit_status = EXIT_SUCCESS;

    /* Check extension and issue non-fatal warning */
    check_and_warn_extension(file_path);

    /* Open the file */
    script_file = fopen(file_path, "r");
    if (script_file == NULL) {
        perror("Error opening Scheme file");
        /* Cannot open the file, return failure */
        return EXIT_FAILURE;
    }

    /* Initialize symbol table with initial size of 128 */
    symbol_table = ht_create(128);
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
    load_initial_libraries(e, config);

    printf("Executing script file: %s\n", file_path);

    /* Loop through expressions and evaluate */
    char *expression_str = NULL;
    while ((expression_str = collect_one_expression_from_file(script_file)) != NULL) {

        Parser *p = parse_str(expression_str);
        Cell *expression = parse_tokens(p);

        /* I dunno, use GC here? */
        free(expression_str);

        /* Check if the parser failed */
        if (expression == NULL) {
            fprintf(stderr, "Fatal Syntax Error in script.\n");
            exit_status = EXIT_FAILURE;
            break;
        }

        /* Evaluate */
        const Cell* result = coz_eval(e, expression);

        /* Test for legitimate null return */
        if (!result) {
            continue;
        }

        /* Check for runtime errors during evaluation */
        if (result->type == CELL_ERROR) {
            fprintf(stderr, "Runtime error detected during script execution.\n");
            fprintf(stderr, "%s\n", result->error_v);
            exit_status = EXIT_FAILURE;
            break; // Stop execution on error
        }
    }

    /* Check if loop exited due to I/O error (not EOF) */
    if (!feof(script_file) && exit_status == EXIT_SUCCESS) {
        fprintf(stderr, "Script reader exited unexpectedly due to file I/O error.\n");
        exit_status = EXIT_FAILURE;
    }

    /* Cleanup */
    fclose(script_file);

    /* Delete this after testing*/
    if (exit_status == EXIT_SUCCESS) {
        printf("Script execution finished successfully.\n");
    } else {
        printf("Script execution finished with errors.\n");
    }

    return exit_status;
}
