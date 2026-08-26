/*
 * 'src/load_library.h'
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

#ifndef COZENAGE_LOAD_LIBRARY_H
#define COZENAGE_LOAD_LIBRARY_H

#include "cell.h"

#define PROCEDURE_NAME_LENGTH 256

/* Define the library file extension based on the OS. */
#ifdef __APPLE__
    #define LIB_EXT "dylib"
#else
    #define LIB_EXT "so"
#endif

#define SCHEME_EXT "sls"

/* builtin function signature. */
typedef Cell* (*CznBuiltinFn)(const Lex*, const Cell*);

typedef struct {
    const char*  scheme_name;   /* Scheme-visible name, e.g. "current-second" */
    CznBuiltinFn func;
} CznExport;

typedef struct {
    const CznExport* exports;
    int              count;
} CznExportTable;

/* New init signature: pure declaration, no side effects. */
typedef const CznExportTable* (*CznLibInitFunc)();

typedef enum {
    IMPORT_ALL,
    IMPORT_ONLY,
    IMPORT_EXCEPT,
} ImportMode;

typedef struct {
    const char* from;
    const char* to;
} CznRename;

typedef struct {
    ImportMode   mode;
    const char** filter_names;   /* identifiers for only/except */
    int          filter_count;
    CznRename*   renames;        /* (old new) pairs for rename */
    int          rename_count;
    const char*  prefix;         /* "" means no prefix */
} ImportSpec;

char **get_load_paths();
//int internal_cozenage_load_lib(const char* collection, const char* library,
                             //  const Lex* env, const ImportSpec* spec);
Cell* parse_import_spec(const Cell* i_set, const char* mod, ImportSpec* spec);
void load_library(const char* libname, const Lex* env);
Cell* load_c_module(const Cell* libspec, const Lex* e, char *path, const ImportSpec *spec);
Cell* load_scheme_lib(const Cell* libspec, const Lex* e, char *path, const ImportSpec *spec);
#endif //COZENAGE_LOAD_LIBRARY_H
