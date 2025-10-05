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
#include "char_lib.h"
#include "complex_lib.h"
#include "coz_ext_lib.h"
#include "file_lib.h"
#include "inexact_lib.h"
#include "process_context_lib.h"
#include "io_lib.h"
#include "eval_lib.h"
#include "cxr_lib.h"
#include "coz_bits_lib.h"
#include <string.h>


char* loaded_libs[MAX_LOADED_LIBS];
int loaded_lib_count = 0;

/* map a name to a loader function */
typedef struct {
    const char* name;
    void (*init_func)(Lex* env);
} LibraryRegistryEntry;

/* The registry of all built-in libraries */
static const LibraryRegistryEntry library_registry[] = {
    {"complex", lex_add_complex_lib},
    {"coz-ext", lex_add_coz_ext},
    {"file", lex_add_file_lib},
    {"inexact", lex_add_inexact_lib},
    {"process-context", lex_add_proc_con_lib},
    {"char", lex_add_char_lib},
    {"eval", lex_add_eval_lib},
    {"write", lex_add_write_lib},
    {"cxr", lex_add_cxr_lib},
    {"coz-bits", lex_add_coz_bits_lib},
    {nullptr, nullptr}
};

/* look up and load a library */
Cell* load_scheme_library(const char* lib_name, Lex* env) {
    /* Check if already loaded to prevent redundant work */
    for (int i = 0; i < loaded_lib_count; i++) {
        if (strcmp(loaded_libs[i], lib_name) == 0) {
            return make_val_bool(1);
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
            return make_val_bool(1);
        }
    }

    /* If we get here, the library wasn't found. */
    char buf[512];
    snprintf(buf, 511, "library '%s' not found.", lib_name);
    return make_val_err(buf, VALUE_ERR);
}
