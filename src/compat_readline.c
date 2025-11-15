/*
 * 'compat_readline.c'
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

#include "compat_readline.h"

#ifdef USE_GNU_READLINE

#include "hash.h"
#include <string.h>
#include <stdlib.h>
#include <gc/gc.h>


char **scheme_procedures = NULL;

void populate_dynamic_completions(const Lex* e)
{
    int symbol_count = 0;
    /* Iterate once to get number of symbols. */
    hti it = ht_iterator(e->global);
    while (ht_next(&it)) {
        symbol_count++;
    }

    /* Special forms have to be added manually. */
    char* special_forms[] = { "quote", "define", "lambda", "let", "let*", "letrec", "set!", "if",
        "when", "unless", "cond", "else", "begin", "import", "and", "or" };
    /* Why tho, does CLion always think this is C++ code? */
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const int num_sfs = sizeof(special_forms) / sizeof(special_forms[0]);

    /* Allocate space for 'symbol_count' pointers to strings, plus one for NULL, plus num_sfs for the SF. */
    scheme_procedures = GC_MALLOC(sizeof(char*) * (symbol_count + 1 + num_sfs));
    if (!scheme_procedures) {
        perror("ENOMEM: malloc failed");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    for (int j = 0; j < num_sfs; j++) {
        scheme_procedures[i] = GC_strdup(special_forms[j]);
        i++;
    }
    /* Iterate second time to copy symbol names */
    it = ht_iterator(e->global);
    while (ht_next(&it)) {
        scheme_procedures[i] = GC_strdup(it.key);
        i++;
    }

    /* For debugging */
    // for (int k = 0; k < symbol_count + num_sfs; k++) {
    //     printf("found sym[%d]: %s\n", k, scheme_procedures[k]);
    // }

    /* The list must be NULL-terminated for the generator to know when to stop. */
    scheme_procedures[i] = NULL;
}

char* scheme_procedure_generator(const char *text, const int state)
{
    static int list_index, len;
    char *name;

    /* If this is the first call for this completion, reset the state. */
    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    /* Iterate through the procedure list and return the next match. */
    while ((name = scheme_procedures[list_index++])) {
        if (strncmp(name, text, len) == 0) {
            return strdup(name); // Readline will free this for us.
        }
    }

    /* No more matches found. */
    return NULL;
}

char** completion_dispatcher(const char *text, const int start, const int end)
{
    (void)end;
    rl_attempted_completion_over = 1; /* Prevents completion on an empty line. */

    bool in_string = false;
    /* Iterate through the line buffer up to the point of completion. */
    for (int i = 0; i < start; i++) {
        /* If we find a double quote... */
        if (rl_line_buffer[i] == '"') {
            /* ...check if it's escaped. If it is (preceded by a '\'),
             * we ignore it. We also check i > 0 to avoid reading before the buffer. */
            if (i > 0 && rl_line_buffer[i - 1] == '\\') {
                continue;
            }
            /* Otherwise, toggle our state. */
            in_string = !in_string;
        }
    }

    /* Now, use the state to decide which completer to use. */
    if (in_string) {
        /* We are inside a string, so use the filename completer.
         * Passing NULL as the generator uses the default filename completer. */
        return rl_completion_matches(text, rl_filename_completion_function);
    }
    /* We are not in a string, so use our custom procedure completer. */
    return rl_completion_matches(text, scheme_procedure_generator);
}

#endif
