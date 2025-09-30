#include "load_library.h"
#include <string.h>
#include <stdio.h>


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
    //{"read", lex_add_read_lib},
    {"write", lex_add_write_lib},
    {NULL, NULL}
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
    return make_val_err(buf, GEN_ERR);
}
