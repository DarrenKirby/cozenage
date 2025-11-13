/*
 * 'src/load_library.c'
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

#include "load_library.h"
#include "base-lib/file_lib.h"
#include "base-lib/proc_env_lib.h"
#include "base-lib/cxr_lib.h"
#include "base-lib/time_lib.h"
#include "base-lib/bits_lib.h"
#include "base-lib/math_lib.h"
#include <string.h>


char* loaded_libs[MAX_LOADED_LIBS];
int loaded_lib_count = 0;

/* map a name to a loader function */
typedef struct {
    const char* name;
    void (*init_func)(const Lex* env);
} LibraryRegistryEntry;

/* The registry of all built-in libraries */
static const LibraryRegistryEntry library_registry[] = {
    {"file", lex_add_file_lib},
    {"proc-env", lex_add_proc_env_lib},
    {"cxr", lex_add_cxr_lib},
    {"time",lex_add_time_lib},
    {"bits", lex_add_bits_lib},
    {"math", lex_add_math_lib},
    {nullptr, nullptr}
};

/* look up and load a library */
Cell* load_library(const char* lib_name, const Lex* env) {
    /* Check if already loaded to prevent redundant work */
    for (int i = 0; i < loaded_lib_count; i++) {
        if (strcmp(loaded_libs[i], lib_name) == 0) {
            return make_cell_boolean(1);
        }
    }
    /* Find the library in the registry */
    for (int i = 0; library_registry[i].name != NULL; i++) {
        if (strcmp(library_registry[i].name, lib_name) == 0) {
            /* Found it. Call its initializer function. */
            library_registry[i].init_func(env);

            /* Mark it as loaded */
            if (loaded_lib_count < MAX_LOADED_LIBS) {
                loaded_libs[loaded_lib_count++] = strdup(lib_name);
            }
            return make_cell_boolean(1);
        }
    }
    /* If we get here, the library wasn't found. */
    char buf[512];
    snprintf(buf, 511, "library '%s' not found.", lib_name);
    return make_cell_error(buf, VALUE_ERR);
}
