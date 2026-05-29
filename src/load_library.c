/*
 * 'src/load_library.c'
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

#include "environment.h"
#include "cell.h"
#include "load_library.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>


/* Define the library file extension based on the OS. */
#ifdef __APPLE__
    #define LIB_EXT "dylib"
#else
    #define LIB_EXT "so"
#endif


static bool name_in_list(const char* name, const char** list, int count)
{
    for (int i = 0; i < count; i++) {
        if (strcmp(name, list[i]) == 0) return true;
    }
    return false;
}


static void apply_import_spec(const Lex* env,
                               const CznExportTable* table,
                               const ImportSpec* spec)
{
    char buf[PROCEDURE_NAME_LENGTH];
    const char* prefix = spec->prefix ? spec->prefix : "";

    for (int i = 0; i < table->count; i++) {
        const char* scheme_name = table->exports[i].scheme_name;
        const CznBuiltinFn func = table->exports[i].func;

        /* Apply only/except filter. */
        if (spec->mode == IMPORT_ONLY) {
            if (!name_in_list(scheme_name, spec->filter_names, spec->filter_count))
                continue;
        } else if (spec->mode == IMPORT_EXCEPT) {
            if (name_in_list(scheme_name, spec->filter_names, spec->filter_count))
                continue;
        }

        /* Apply the rename: check if this binding has a rename entry.
         * Bindings not mentioned in the rename list pass through unchanged. */
        const char* effective_name = scheme_name;
        for (int j = 0; j < spec->rename_count; j++) {
            if (strcmp(scheme_name, spec->renames[j].from) == 0) {
                effective_name = spec->renames[j].to;
                break;
            }
        }

        /* Apply prefix and register. */
        snprintf(buf, sizeof(buf), "%s%s", prefix, effective_name);
        lex_add_builtin(env, buf, func);
    }
}


/*
 * This is the internal C function that handles module loading.
 * It takes a collection and library from an import-set,
 * the environment to load it into, and an import spec.
 *
 * Returns 1 on success, 0 on failure.
 */
int internal_cozenage_load_lib(const char* collection, const char* library,
                               const Lex* env, const ImportSpec* spec)
{
    char filepath[PATH_MAX];
    void* lib_handle = NULL;

    const char* env_path = getenv("COZENAGE_LIB_PATH");
    if (!env_path) {
        env_path = "";
    }

    /* Library Search Path Logic.
     * It looks in "./lib/" first, then tries a relative PATH "../lib/cozenage/".
     * It then checks if the COZENAGE_LIB_PATH ENV VAR has been set.
     * If none of these resolve, it will look in /usr/lib and /usr/lib64/
     * for regular/multilib Linux systems, and in /usr/local/lib/ for macOS and *BSD. */

    const char* search_paths[] = {
        "./lib/cozenage",
        "../lib/cozenage",
        env_path,
        "/usr/lib/cozenage",
#ifdef __linux__
        "/usr/lib64/cozenage",
#endif
        "/usr/local/lib/cozenage",
        nullptr
    };

    /* Iterate paths and try to load. */
    for (int i = 0; search_paths[i] != NULL; ++i) {
        if (search_paths[i][0] == '\0') continue;
        snprintf(filepath, sizeof(filepath), "%s/%s/%s.%s", search_paths[i], collection, library, LIB_EXT);
        lib_handle = dlopen(filepath, RTLD_LAZY);
        if (lib_handle) break;
    }

    if (!lib_handle) {
        /* Both failed. Report the error.
         * dlerror() returns a human-readable message. */
        fprintf(stderr, "Error loading library '%s %s': %s\n", collection, library, dlerror());
        return 0;
    }

    CznLibInitFunc init_func;
    *(void**)&init_func = dlsym(lib_handle, "cozenage_library_init");
    if (!init_func) {
        fprintf(stderr, "Error finding 'cozenage_library_init' in '%s': %s\n",
                filepath, dlerror());
        dlclose(lib_handle);
        return 0;
    }

    const CznExportTable* table = init_func();
    if (!table) {
        fprintf(stderr, "Library '%s %s' returned a null export table.\n", collection, library);
        dlclose(lib_handle);
        return 0;
    }

    apply_import_spec(env, table, spec);
    return 1;
}


/* This handles the libs specified to load from CLI args. */
void load_library(const char* libname, const Lex* env) {
    /* Use default spec. */
    const ImportSpec spec = {
        .mode         = IMPORT_ALL,
        .filter_names = nullptr,
        .filter_count = 0,
        .renames      = nullptr,
        .rename_count = 0,
        .prefix       = "",
    };

    /* Call the internal loader. */
    const int success = internal_cozenage_load_lib("base", libname, env, &spec);

    if (!success) {
        fprintf(stderr, "Error: unable to load ' base %s' library\n", libname);
    }
}
