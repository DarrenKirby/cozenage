/*
 * 'src/base-lib/system_lib.c'
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


#include "types.h"
#include "cell.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>


extern char **environ;

static Cell* system_get_pid(const Lex* e, const Cell* a)
{
    (void) e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-pid");
    if (err) { return err; }

    return make_cell_integer(getpid());
}

static Cell* system_get_ppid(const Lex* e, const Cell* a)
{
    (void) e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-ppid");
    if (err) { return err; }

    return make_cell_integer(getppid());
}

static Cell* system_get_env_var(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "get-environment-variable");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "get-environment-variable"))) { return err; }

    const char *env = getenv(a->cell[0]->str);
    if (env == NULL) {
        return False_Obj;
    }
    const char* var_string = GC_strdup(env);
    return make_cell_string(var_string);
}

static Cell* system_get_env_vars(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-environment-variables");
    if (err) { return err; }

    /* start with nil */
    Cell* result = make_cell_nil();
    int len = 0;

    for (char **env = environ; *env != NULL; env++) {
        /* Bad form to mutate env */
        char* var_string = GC_strdup(*env);
        const char *var = strtok(var_string, "=");
        const char *val = strtok(nullptr, "=");
        Cell* vr = make_cell_string(var);
        Cell* vl = make_cell_string(val);
        result = make_cell_pair(make_cell_pair(vr, vl), result);
        result->len = len;
        len++;
    }
    return result;
}

void cozenage_library_init(const Lex* e)
{
    lex_add_builtin(e, "get-pid", system_get_pid);
    lex_add_builtin(e, "get-ppid", system_get_ppid);
    lex_add_builtin(e, "get-environment-variable", system_get_env_var);
    lex_add_builtin(e, "get-environment-variables", system_get_env_vars);
}
