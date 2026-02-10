/*
 * 'src/config.c'
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

#include "config.h"
#include "load_library.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>



void load_initial_libraries(const Lex* e, const lib_load_config load_libs) {
    if (load_libs.file) {
        (void)load_library("file", e);
    }
    if (load_libs.math) {
        (void)load_library("math", e);
    }
    if (load_libs.system) {
        (void)load_library("system", e);
    }
    if (load_libs.cxr) {
        (void)load_library("cxr", e);
    }
    if (load_libs.time) {
        (void)load_library("time", e);
    }
    if (load_libs.bits) {
        (void)load_library("bits", e);
    }
    if (load_libs.random) {
        (void)load_library("random", e);
    }
    if (load_libs.lazy) {
        (void)load_library("lazy", e);
    }
}


void init_history_path() {
    char path[PATH_MAX];
    const char *xdg_state = getenv("XDG_STATE_HOME");
    const char *home = getenv("HOME");

    if (xdg_state && strlen(xdg_state) > 0) {
        /* Use $XDG_STATE_HOME/cozenage/history */
        snprintf(path, sizeof(path), "%s/cozenage/history", xdg_state);
    } else if (home) {
        /* Fallback to ~/.local/state/cozenage/history */
        snprintf(path, sizeof(path), "%s/.local/state/cozenage/history", home);
    } else {
        /* Absolute emergency fallback. */
        strncpy(path, "/tmp/cozenage_history", sizeof(path));
    }

    /* Duplicate the string into the global variable. */
    cozenage_history_path = strdup(path);
}


/* TODO: find better spot for this, as it can be used by file_lib.c as well. */
static int mkdir_p(const char *path) {
    char temp[PATH_MAX];

    snprintf(temp, sizeof(temp), "%s", path);
    const size_t len = strlen(temp);
    if (temp[len - 1] == '/') {
        temp[len - 1] = 0;
    }

    for (char *p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0; /* Temporarily truncate. */
            if (mkdir(temp, S_IRWXU) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/'; /* Restore. */
        }
    }

    if (mkdir(temp, S_IRWXU) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}


void setup_history() {
    init_history_path();

    /* Isolate the directory part of the path. */
    char *dir_part = strdup(cozenage_history_path);
    char *last_slash = strrchr(dir_part, '/');

    if (last_slash) {
        *last_slash = '\0';

        /* Create the directory tree. */
        if (mkdir_p(dir_part) == 0) {
            /* Ensure the file itself exists. */
            FILE *f = fopen(cozenage_history_path, "a");
            if (f) fclose(f);
        }
    }
    free(dir_part);
}
