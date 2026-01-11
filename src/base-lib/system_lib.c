/*
 * 'src/base-lib/system_lib.c'
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


#include "types.h"
#include "cell.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <gc/gc.h>


extern char **environ;


/* (get-pid)
 * Returns the process ID of the calling process. */
static Cell* system_get_pid(const Lex* e, const Cell* a)
{
    (void) e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-pid");
    if (err) { return err; }

    return make_cell_integer(getpid());
}


/* (get-ppid)
 * Returns the process ID of the parent of the calling process. */
static Cell* system_get_ppid(const Lex* e, const Cell* a)
{
    (void) e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-ppid");
    if (err) { return err; }

    return make_cell_integer(getppid());
}


/* (get-env-var string)
 * Returns the value of environmental variable 'string', or else
 * #false if the variable is unset. */
static Cell* system_get_env_var(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "get-env-var");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "get-env-var"))) { return err; }

    const char *env = getenv(a->cell[0]->str);
    if (env == NULL) {
        return False_Obj;
    }
    const char* var_string = GC_strdup(env);
    return make_cell_string(var_string);
}


/* (get-env-vars)
 * Returns an alist of all var=val pairs in the running process' environment. */
static Cell* system_get_env_vars(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-env-vars");
    if (err) { return err; }

    /* Start with nil. */
    Cell* result = make_cell_nil();
    int len = 0;

    for (char **env = environ; *env != NULL; env++) {
        /* Bad form to mutate env. */
        char* var_string = GC_strdup(*env);
        const char *var = strtok(var_string, "=");
        if (var == NULL) {
            break;
        }
        const char *val = strtok(nullptr, "=");
        if (val == NULL) {
            break;
        }
        Cell* vr = make_cell_string(var);
        Cell* vl = make_cell_string(val);
        result = make_cell_pair(make_cell_pair(vr, vl), result);
        result->len = len;
        len++;
    }
    return result;
}


/* (get-uid)
 * (get-gid)
 * (get-euid)
 * (get-egid)
 * The procedures return the user id, group id,
 * effective user id, and effective group id of the
 * running process, respectively. */
Cell* system_get_uid(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-uid");
    if (err) { return err; }
    return make_cell_integer(getuid());
}


Cell* system_get_gid(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-gid");
    if (err) { return err; }
    return make_cell_integer(getgid());
}


Cell* system_get_euid(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-euid");
    if (err) { return err; }
    return make_cell_integer(geteuid());
}


Cell* system_get_egid(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-egid");
    if (err) { return err; }
    return make_cell_integer(getegid());
}


/* (get-username)
 * Returns the username associated with the uid of the running process,
 * or #false if it cannoot be obtained. */
Cell* system_get_username(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get_username");
    if (err) { return err; }

    const uid_t uid = geteuid();
    const struct passwd *pw = getpwuid(uid);

    if (pw) {
        return make_cell_string(pw->pw_name);
    }
    return False_Obj;
}

Cell* system_get_groups(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-groups");
    if (err) { return err; }

    int ngroups = 15; /* Reasonable default. */
    gid_t *groups = GC_malloc(sizeof(gid_t) * ngroups);
    const uid_t uid = geteuid();
    const struct passwd *pw = getpwuid(uid);

    if (getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups) == -1) {
        /* Reallocate and call again if ngroups too small. */
        groups = GC_realloc(groups, sizeof(gid_t) * ngroups);
        getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups);
    }

    /* Start with nil. */
    Cell* result = make_cell_nil();
    int len = 0;

    for (int j = 0; j < ngroups; j++) {
        Cell* vr = make_cell_integer(groups[j]);
        Cell* vl;
        const struct group *gr = getgrgid(groups[j]);
        if (gr != NULL) {
            vl = make_cell_string(gr->gr_name);
        } else {
            vl = make_cell_string("");
        }
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
    lex_add_builtin(e, "get-env-var", system_get_env_var);
    lex_add_builtin(e, "get-env-vars", system_get_env_vars);
    lex_add_builtin(e, "get-uid", system_get_uid);
    lex_add_builtin(e, "get-gid", system_get_gid);
    lex_add_builtin(e, "get-euid", system_get_euid);
    lex_add_builtin(e, "get-egid", system_get_egid);
    lex_add_builtin(e, "get-username", system_get_username);
    lex_add_builtin(e, "get-groups", system_get_groups);
}
