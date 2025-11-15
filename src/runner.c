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
#include "parser.h"
#include "eval.h"
#include "repl.h"
#include "repr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <gc/gc.h>


/* Check the file extension and print a warning if it's non-standard. */
static void check_and_warn_extension(const char *file_path)
{
    const char *ext = strrchr(file_path, '.');

    /* Does the file have an extension? Is the extension NOT .scm AND NOT .ss? */
    if (ext != NULL && strcmp(ext, ".scm") != 0 && strcmp(ext, ".ss") != 0) {
        /* Print the non-fatal warning to stderr */
        fprintf(stderr,
                "Warning: Running file '%s' which does not have the standard .scm or .ss extension.\n",
                file_path);
    }
}


char* read_file_to_string(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return nullptr;
    }

    /* Determine file size */
    fseek(file, 0, SEEK_END);
    const int64_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size == -1) {
        perror("Error getting file size");
        fclose(file);
        return nullptr;
    }

    /* Allocate memory for the string (+1 for null terminator) */
    char* buffer = GC_MALLOC_ATOMIC(file_size + 1);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return nullptr;
    }

    /* Read file content into the buffer */
    const size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != (size_t)file_size) {
        perror("Error reading file");
        fclose(file);
        return nullptr;
    }

    /* Null-terminate the string */
    buffer[file_size] = '\0';

    fclose(file);
    return buffer;
}


int run_file_script(const char *file_path, const lib_load_config load_libs)
{
    /* Check extension and issue non-fatal warning. */
    check_and_warn_extension(file_path);

    /* Initialize symbol table with initial size of 128. */
    symbol_table = ht_create(128);
    /* Initialize default ports. */
    init_default_ports();
    /* Initialize global singleton objects, nil, #t, #f, and EOF. */
    init_global_singletons();
    /* Initialize global environment. */
    Lex* e = lex_initialize_global_env();
    /* Load (scheme base) procedures into the environment. */
    lex_add_builtins(e);
    /* Initialize special form lookup table. */
    init_special_forms();
    /* Loads the CLI-specified R7RS libraries into the environment. */
    load_initial_libraries(e, load_libs);

    const char* input = read_file_to_string(file_path);
    if (input == NULL) {
        fprintf(stderr, "Fatal: could not open and read '%s'.\n", file_path);
        exit(EXIT_FAILURE);
    }

    TokenArray* ta = scan_all_tokens(input);
    const Cell* result = parse_all_expressions(e, ta, false);

    if (result->type == CELL_INTEGER) {
        exit((int)result->integer_v);
    }

    if (result->type == CELL_ERROR) {
        fprintf(stderr, "%s\n", cell_to_string(result, MODE_REPL));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_FAILURE);
}


Cell* parse_all_expressions(Lex* e, TokenArray* ta, const bool is_repl)
{
    while (ta->position <= ta->count) {
        Cell* expression = parse_tokens(ta);
        if (!expression) {
            break;
        }

        if (expression->type == CELL_ERROR) {
            return expression;
        }

        Cell* result = coz_eval(e, expression);

        if (result && result->type == CELL_ERROR) {
            return result;
        }

        if (result && is_repl) {
            coz_print(result);
        }

        /* Bump the token position */
        ta->position++;
    }
    /* No more expressions... */
    /* return null to get new REPL prompt */
    if (is_repl) {
        return nullptr;
    }
    /* Return success exit status to file runner */
    return make_cell_integer(0);
}
