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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>


/* Define the library file extension based on the OS */
#ifdef __APPLE__
    #define LIB_EXT "dylib"
#else
    #define LIB_EXT "so"
#endif

/* Define the function signature we expect to find */
typedef void (*CznLibInitFunc)(const Lex*);

/*
 * This is the internal C function that handles loading.
 * It takes a "logical" library name (e.g., "math") and
 * the environment to load it into.
 *
 * Returns 1 on success, 0 on failure.
 */
int internal_cozenage_load_lib(const char* libname, const Lex* env)
{
    char filepath[PATH_MAX];
    void* lib_handle = NULL;
    CznLibInitFunc init_func;

    const char* env_path = getenv("COZENAGE_LIB_PATH");
    if (!env_path) {
        env_path = "";
    }

    /* Library Search Path Logic
     * It looks in "./lib/" first, then tries a relative PATH "../lib/cozenage/".
     * It then checks if the COZENAGE_LIB_PATH ENV VAR has been set.
     * If none of these resolve, it will look in /usr/lib and /usr/lib64/
     * for regular/multilib Linux systems, and in /usr/local/lib/ for macOS and *BSD. */

    const char* search_paths[] = {
        "./lib",
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
        snprintf(filepath, sizeof(filepath), "%s/%s.%s", search_paths[i], libname, LIB_EXT);
        lib_handle = dlopen(filepath, RTLD_LAZY);
        if (lib_handle) break;
    }

    if (!lib_handle) {
        /* Both failed. Report the error.
         * dlerror() returns a human-readable message. */
        fprintf(stderr, "Error loading library '%s': %s\n", libname, dlerror());
        return 0;
    }

    /* We have a valid handle. Now, find the init function.
     * dlsym() searches for a symbol (function name) in the handle.
     * We cast the resulting 'void*' to our function pointer type. */
    *(void**)&init_func = dlsym(lib_handle, "cozenage_library_init");

    if (!init_func) {
        /* Could not find the init function. */
        fprintf(stderr, "Error finding 'cozenage_library_init' in '%s': %s\n", filepath, dlerror());
        dlclose(lib_handle);
        return 0;
    }

    /* We found the function. Now we call it. */
    init_func(env);

    /* NOTE: We don't call dlclose(lib_handle) here.
     * We want the library to stay in memory for the rest of
     * the interpreter's session, otherwise all the function
     * pointers we just registered will become invalid.
     * QUESTION: should we store the handles in a list so they can be unloaded later?
     * Or just assume loaded modules stay loaded for the life of the interpreter? */

    return 1; /* Success. */
}

Cell* load_library(const char* libname, const Lex* env)
{
    /* Call the internal loader. */
    const int success = internal_cozenage_load_lib(libname, env);

    /* Return a Cozenage value. */
    if (success) {
        return True_Obj;
    }
    return False_Obj;
}
