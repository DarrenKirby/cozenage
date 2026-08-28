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
#include "eval.h"
#include "parser.h"
#include "runner.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <gc/gc.h>


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
    Lex *sandbox = lex_initialize_global_env();
    lex_add_builtins(sandbox);
    const char* input = read_file_to_string(path);
    TokenArray* ta = scan_all_tokens(input);
    const Cell* ast = parse_tokens(ta);

    if (ast->cell[0] != make_cell_symbol("define-library")) {
        return make_cell_error(fmt_err(
            "invalid library definition in %s", path),
            SYNTAX_ERR);
    }

    /* Ensure libspec matches library definition. */
    if (ast->cell[1]->cell[0] != libspec->cell[0] ||
        ast->cell[1]->cell[1] != libspec->cell[1]) {
        return make_cell_error(fmt_err(
            "unmatched collection/library spec in %s", path),
            SYNTAX_ERR);
    }

    Cell *exports = nullptr;
    Cell *definitions = nullptr;

    /* Start at index 2, iterate safely through the array */
    for (int k = 2; k < ast->count; k++) {
        const Cell* block_type = ast->cell[k]->cell[0];

        if (block_type == make_cell_symbol("import")) {
            Cell *res = coz_eval(sandbox, ast->cell[k]);
            if (res->type == CELL_ERROR) return res;
        }
        else if (block_type == make_cell_symbol("export")) {
            exports = ast->cell[k];
        }
        else if (block_type == make_cell_symbol("begin")) {
            definitions = ast->cell[k];
        }
    }

    /* Throw error if export list is not found. */
    if (!exports) {
        return make_cell_error(fmt_err(
            "missing export list in %s", path),
            SYNTAX_ERR);
    }

    /* Evaluate the library definitions in the sandbox. */
    Cell *l = coz_eval(sandbox, definitions);
    if (l->type == CELL_ERROR) {
        return l;
    }

    /* Iterate the export list, copy definitions from the sandbox to the global env,
     * and apply any import modifiers. */
    for (int i = 1; i < exports->count; i++) {
        const char* orig_name = exports->cell[i]->sym;
        bool skip = false;

        /* Apply ONLY and EXCEPT filters */
        if (spec->mode == IMPORT_ONLY) {
            skip = true; /* Assume skip unless explicitly found in the 'only' list */
            for (int j = 0; j < spec->filter_count; j++) {
                if (strcmp(orig_name, spec->filter_names[j]) == 0) {
                    skip = false;
                    break;
                }
            }
        } else if (spec->mode == IMPORT_EXCEPT) {
            for (int j = 0; j < spec->filter_count; j++) {
                if (strcmp(orig_name, spec->filter_names[j]) == 0) {
                    skip = true;
                    break;
                }
            }
        }

        if (skip) continue;

        /* Apply RENAME modifier */
        const char* dest_name = orig_name;
        for (int j = 0; j < spec->rename_count; j++) {
            if (strcmp(orig_name, spec->renames[j].from) == 0) {
                dest_name = spec->renames[j].to;
                break;
            }
        }

        /* Apply PREFIX modifier and construct the final Cell */
        const Cell* dest_cell = nullptr;
        if (spec->prefix && spec->prefix[0] != '\0') {
            /* Allocate string using atomic since it contains no pointers for libgc to trace */
            const size_t len = strlen(spec->prefix) + strlen(dest_name) + 1;
            char* prefixed_name = GC_malloc_atomic(len);
            snprintf(prefixed_name, len, "%s%s", spec->prefix, dest_name);
            dest_cell = make_cell_symbol(prefixed_name);
        } else if (dest_name != orig_name) {
            /* Renamed, but no prefix */
            dest_cell = make_cell_symbol((char*)dest_name);
        } else {
            /* Unchanged */
            dest_cell = exports->cell[i];
        }

        /* Fetch the object and bind it */
        Cell* obj = lex_get(sandbox, exports->cell[i]);
        if (!obj || obj->type == CELL_ERROR) {
            return make_cell_error(fmt_err(
                "exported symbol '%s' not defined in library", orig_name),
                VALUE_ERR);
        }
        lex_put_global(e, dest_cell, obj);
    }

    return True_Obj;
}


void init_import_spec(ImportSpec* spec) {
    spec->mode         = IMPORT_ALL;
    spec->filter_names = nullptr;
    spec->filter_count = 0;
    spec->renames      = nullptr;
    spec->rename_count = 0;
    spec->prefix       = "";
}

Cell* parse_import_spec(const Cell* node, ImportSpec* spec) {
    /* Base case: We reached (collection library) */
    if (node->count == 2 &&
        node->cell[0]->type == CELL_SYMBOL &&
        node->cell[1]->type == CELL_SYMBOL) {
        return True_Obj;
    }

    /* Recurse first so modifiers apply inside-out */
    Cell* inner_res = parse_import_spec(node->cell[1], spec);
    if (inner_res->type == CELL_ERROR) return inner_res;

    const char* mod = node->cell[0]->sym;

    if (strcmp(mod, "only") == 0) {
        spec->mode = IMPORT_ONLY;
        spec->filter_count = node->count - 2;
        spec->filter_names = GC_malloc(spec->filter_count * sizeof(char*));
        for (int j = 0; j < spec->filter_count; j++)
            spec->filter_names[j] = node->cell[j + 2]->sym;

    } else if (strcmp(mod, "except") == 0) {
        spec->mode = IMPORT_EXCEPT;
        spec->filter_count = node->count - 2;
        spec->filter_names = GC_malloc(spec->filter_count * sizeof(char*));
        for (int j = 0; j < spec->filter_count; j++)
            spec->filter_names[j] = node->cell[j + 2]->sym;

    } else if (strcmp(mod, "prefix") == 0) {
        if (node->count != 3) {
            return make_cell_error("import: 'prefix' requires exactly one argument", SYNTAX_ERR);
        }
        /* If nested prefixes exist, concatenate them */
        if (spec->prefix[0] != '\0') {
            const char* new_pref = node->cell[2]->str;
            size_t len = strlen(new_pref) + strlen(spec->prefix) + 1;
            char* combined = GC_malloc_atomic(len);
            snprintf(combined, len, "%s%s", new_pref, spec->prefix);
            spec->prefix = combined;
        } else {
            spec->prefix = node->cell[2]->str;
        }

    } else if (strcmp(mod, "rename") == 0) {
        /* Append new renames to existing renames if nested */
        int new_count = node->count - 2;
        int total_count = spec->rename_count + new_count;
        CznRename* combined = GC_malloc(total_count * sizeof(CznRename));

        /* Copy existing */
        for (int j = 0; j < spec->rename_count; j++) combined[j] = spec->renames[j];

        /* Add new */
        for (int j = 0; j < new_count; j++) {
            const Cell* pair = node->cell[j + 2];
            if (pair->type != CELL_SEXPR || pair->count != 2) {
                return make_cell_error("import: 'rename' expects (old new) pairs", SYNTAX_ERR);
            }
            combined[spec->rename_count + j].from = pair->cell[0]->sym;
            combined[spec->rename_count + j].to   = pair->cell[1]->sym;
        }
        spec->renames = combined;
        spec->rename_count = total_count;

    } else {
        return make_cell_error("import: unknown import modifier", SYNTAX_ERR);
    }

    return True_Obj;
}


// Cell* parse_import_spec(const Cell* i_set, const char* mod, ImportSpec* spec)
// {
//     /* Set defaults first. */
//     spec->mode         = IMPORT_ALL;
//     spec->filter_names = nullptr;
//     spec->filter_count = 0;
//     spec->renames      = nullptr;
//     spec->rename_count = 0;
//     spec->prefix       = "";
//
//     if (mod) {
//         if (strcmp(mod, "only") == 0) {
//             spec->mode         = IMPORT_ONLY;
//             spec->filter_count = i_set->count - 2;
//             spec->filter_names = GC_malloc(spec->filter_count * sizeof(char*));
//             for (int j = 0; j < spec->filter_count; j++)
//                 spec->filter_names[j] = i_set->cell[j + 2]->sym;
//
//         } else if (strcmp(mod, "except") == 0) {
//             spec->mode         = IMPORT_EXCEPT;
//             spec->filter_count = i_set->count - 2;
//             spec->filter_names = GC_malloc(spec->filter_count * sizeof(char*));
//             for (int j = 0; j < spec->filter_count; j++)
//                 spec->filter_names[j] = i_set->cell[j + 2]->sym;
//
//         } else if (strcmp(mod, "prefix") == 0) {
//             if (i_set->count != 3) {
//                 return make_cell_error(
//                     "import: 'prefix' requires exactly one argument",
//                     SYNTAX_ERR);
//             }
//             spec->prefix = i_set->cell[2]->str;  /* string cell */
//
//         } else if (strcmp(mod, "rename") == 0) {
//             spec->rename_count = i_set->count - 2;
//             spec->renames      = GC_malloc(spec->rename_count * sizeof(CznRename));
//             for (int j = 0; j < spec->rename_count; j++) {
//                 const Cell* pair = i_set->cell[j + 2];
//                 if (pair->type != CELL_SEXPR || pair->count != 2) {
//                     return make_cell_error(
//                         "import: 'rename' expects (old-name new-name) pairs",
//                         SYNTAX_ERR);
//                 }
//                 spec->renames[j].from = pair->cell[0]->sym;
//                 spec->renames[j].to   = pair->cell[1]->sym;
//             }
//         } else {
//             return make_cell_error("import: unknown import modifier",
//                 SYNTAX_ERR);
//         }
//     }
//     return True_Obj;
// }


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
            search_paths[j], "base", libname, C_LIB_EXT);

        if (access(path, F_OK) == 0) {
            break;
        }
    }

    /* Call the internal loader. We need to create a
     * dummy default ImportSpec even though it is not used. */
    ImportSpec spec;
    init_import_spec(&spec);

    const Cell* ret = load_c_module(libspec, env, path, &spec);

    /* There is no REPL, or anything else running at this point, and
     * nowhere to return an error, so just dump to stderr. */
    if (ret->type == CELL_ERROR) {
        fprintf(stderr, "Error: unable to load 'base %s' library\n", libname);
    }
}
