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
#include <unistd.h>
#include <gc/gc.h>

#include "repr.h"
#include "types.h"


static bool name_in_list(const char* name, const char** list, const int count)
{
    for (int i = 0; i < count; i++) {
        if (strcmp(name, list[i]) == 0) return true;
    }
    return false;
}


#define MAX_SEARCH_PATHS 10

char **get_load_paths()
    /* Library Search Path Logic.
     * It looks in "./lib/cozenage" first, then tries a relative PATH "../lib/cozenage/".
     * It then checks if the COZENAGE_LIB_PATH ENV VAR has been set.
     * It then checks in $XDG_DATA_HOME/cozenage, or ~/.local/share/cozenage if it is not set.
     * If none of these resolve, it will look in /usr/lib and /usr/lib64/
     * for regular/multilib Linux systems, and in /usr/local/lib/ for macOS and *BSD. */
{
    char **paths = GC_MALLOC(MAX_SEARCH_PATHS * sizeof(char*));
    int i = 0;

    /*  Local relative paths. */
    paths[i++] = GC_STRDUP("./lib/cozenage");
    paths[i++] = GC_STRDUP("../lib/cozenage");

    /* Environment override. */
    const char *env_path = getenv("COZENAGE_LIB_PATH");
    if (env_path && env_path[0] != '\0') {
        paths[i++] = GC_STRDUP(env_path);
    }

    /* XDG Data Home / Fallback. */
    const char *xdg_data = getenv("XDG_DATA_HOME");
    char path_buf[PATH_MAX];

    if (xdg_data && xdg_data[0] != '\0') {
        snprintf(path_buf, sizeof(path_buf), "%s/cozenage", xdg_data);
        paths[i++] = GC_STRDUP(path_buf);
    } else {
        const char *home = getenv("HOME");
        if (home && home[0] != '\0') {
            snprintf(path_buf, sizeof(path_buf), "%s/.local/share/cozenage", home);
            paths[i++] = GC_STRDUP(path_buf);
        }
    }

    /* System paths. */
    paths[i++] = GC_STRDUP("/usr/lib/cozenage");
#ifdef __linux__
    paths[i++] = GC_STRDUP("/usr/lib64/cozenage");
#endif
    paths[i++] = GC_STRDUP("/usr/local/lib/cozenage");

    paths[i] = nullptr;
    return paths;
}


Cell* load_scheme_lib(const Cell* libspec, const Lex* e, char *path, const ImportSpec *spec)
{
    /* TODO: finish this... */
    (void)e;
    (void)spec;
    fprintf(stdout, "Inside load_scheme_lib\n");
    fprintf(stdout, "Library is: %s\n", path);
    fprintf(stdout, "libspec is: %s\n", cell_to_string(libspec, MODE_REPL));

    return True_Obj;
}


Cell* parse_import_spec(const Cell* i_set, const char* mod, ImportSpec* spec)
{
    /* Set defaults first. */
    spec->mode         = IMPORT_ALL;
    spec->filter_names = nullptr;
    spec->filter_count = 0;
    spec->renames      = nullptr;
    spec->rename_count = 0;
    spec->prefix       = "";

    if (mod) {
        if (strcmp(mod, "only") == 0) {
            spec->mode         = IMPORT_ONLY;
            spec->filter_count = i_set->count - 2;
            spec->filter_names = GC_malloc(spec->filter_count * sizeof(char*));
            for (int j = 0; j < spec->filter_count; j++)
                spec->filter_names[j] = i_set->cell[j + 2]->sym;

        } else if (strcmp(mod, "except") == 0) {
            spec->mode         = IMPORT_EXCEPT;
            spec->filter_count = i_set->count - 2;
            spec->filter_names = GC_malloc(spec->filter_count * sizeof(char*));
            for (int j = 0; j < spec->filter_count; j++)
                spec->filter_names[j] = i_set->cell[j + 2]->sym;

        } else if (strcmp(mod, "prefix") == 0) {
            if (i_set->count != 3) {
                return make_cell_error(
                    "import: 'prefix' requires exactly one argument",
                    SYNTAX_ERR);
            }
            spec->prefix = i_set->cell[2]->str;  /* string cell */

        } else if (strcmp(mod, "rename") == 0) {
            spec->rename_count = i_set->count - 2;
            spec->renames      = GC_malloc(spec->rename_count * sizeof(CznRename));
            for (int j = 0; j < spec->rename_count; j++) {
                const Cell* pair = i_set->cell[j + 2];
                if (pair->type != CELL_SEXPR || pair->count != 2) {
                    return make_cell_error(
                        "import: 'rename' expects (old-name new-name) pairs",
                        SYNTAX_ERR);
                }
                spec->renames[j].from = pair->cell[0]->sym;
                spec->renames[j].to   = pair->cell[1]->sym;
            }
        } else {
            return make_cell_error("import: unknown import modifier",
                SYNTAX_ERR);
        }
    }
    return True_Obj;
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


Cell* load_c_module(const Cell* libspec, const Lex* e, char* path, const ImportSpec *spec)
{
    void* lib_handle = NULL;

    /* Extract library identifier and library name from libspec. */
    const char* collection  = libspec->cell[0]->sym;
    const char* library = libspec->cell[1]->sym;

    lib_handle = dlopen(path, RTLD_LAZY);
    if (!lib_handle) {
        /* Report the error.
         * dlerror() returns a human-readable message. */
        return make_cell_error(fmt_err("failed to load library '%s %s': %s\n",
            collection, library, dlerror()), GEN_ERR);
    }

    CznLibInitFunc init_func;
    *(void**)&init_func = dlsym(lib_handle, "cozenage_library_init");
    if (!init_func) {
        Cell* err = make_cell_error(fmt_err("cannot find 'cozenage_library_init' in '%s': %s\n",
                path, dlerror()), GEN_ERR);
        dlclose(lib_handle);
        return err;
    }

    const CznExportTable* table = init_func();
    if (!table) {
        Cell* err = make_cell_error(fmt_err("Library '%s %s' returned a null export table.\n",
            collection, library), GEN_ERR);
        dlclose(lib_handle);
        return err;
    }

    apply_import_spec(e, table, spec);
    return True_Obj;
}


/* This handles the standard libs specified to load from CLI args. */
void load_library(const char* libname, const Lex* env) {
    const Cell* libspec = make_sexpr_len2(
      make_cell_symbol("base"),
      make_cell_symbol(libname)
    );

    char path[PATH_MAX];
    char **search_paths = get_load_paths();

    for (int j = 0; search_paths[j] != NULL; ++j) {
        if (search_paths[j][0] == '\0') continue;
        snprintf(path, sizeof(path), "%s/%s/%s.%s",
            search_paths[j], "base", libname, LIB_EXT);

        if (access(path, F_OK) == 0) {
            break;
        }
    }

    /* Call the internal loader. We need to create a
     * dummy, default ImportSpec even though it is not used. */
    const ImportSpec spec = {
        .mode         = IMPORT_ALL,
        .filter_names = nullptr,
        .filter_count = 0,
        .renames      = nullptr,
        .rename_count = 0,
        .prefix       = "",
    };
    const Cell* ret = load_c_module(libspec, env, path, &spec);

    /* There is no REPL, or anything else running at this point, and
     * nowhere to return an error, so just dump to stderr. */
    if (ret->type == CELL_ERROR) {
        fprintf(stderr, "Error: unable to load 'base %s' library\n", libname);
    }
}
