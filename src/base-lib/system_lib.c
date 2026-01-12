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
#include <errno.h>
#include <sys/utsname.h>
#include <sys/stat.h>


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
    int len = 1;

    for (char **env = environ; *env != NULL; env++) {
        /* Make a copy so we don't mutate env. */
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
static Cell* system_get_uid(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-uid");
    if (err) { return err; }
    return make_cell_integer(getuid());
}


static Cell* system_get_gid(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-gid");
    if (err) { return err; }
    return make_cell_integer(getgid());
}


static Cell* system_get_euid(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-euid");
    if (err) { return err; }
    return make_cell_integer(geteuid());
}


static Cell* system_get_egid(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-egid");
    if (err) { return err; }
    return make_cell_integer(getegid());
}


/* (get-username)
 * Returns the username associated with the uid of the running process,
 * or #false if it cannot be obtained. */
static Cell* system_get_username(const Lex* e, const Cell* a) {
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


/* BSD expects int*, glibc expects gid_t*.
 * We give BSD what it wants without lying about storage. */
static int portable_getgrouplist(const char *user, const gid_t basegid,
                      gid_t *groups, int *ngroups) {
#ifdef __linux__
    /* glibc-style API */
    return getgrouplist(user, basegid, groups, ngroups);
#else
    /* BSD-style API */
    int *igroups = GC_malloc(sizeof(int) * (*ngroups));
    const int ret = getgrouplist(user,
                           (int)basegid,
                           igroups,
                           ngroups);
    for (int i = 0; i < *ngroups; i++) {
        groups[i] = (gid_t)igroups[i];
    }
    return ret;
#endif
}


/* (get-groups)
 * Returns an alist of (gid . "groupname") pairs for all groups
 * associated with the current process' euid. */
static Cell* system_get_groups(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-groups");
    if (err) { return err; }

    long max = sysconf(_SC_NGROUPS_MAX);
    if (max < 0) max = 32;

    int ngroups = (int)max;
    gid_t *groups = GC_malloc(sizeof(gid_t) * ngroups);

    const uid_t uid = geteuid();
    const struct passwd *pw = getpwuid(uid);
    if (!pw) {
        return make_cell_error(
            "get-groups: getpwuid() call failed",
            OS_ERR);
    }

    const int ret = portable_getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups);
    if (ret == -1) {
        /* Nothing really to do here, as NGROUPS_MAX was used to set ngroups for the call.
         * BSD truncation: ngroups is "how many were stored"
         * glibc: ngroups is "required size" */
    }

    Cell* result = make_cell_nil();
    int len = 1;

    for (int j = 0; j < ngroups; j++) {
        Cell* vr = make_cell_integer(groups[j]);
        Cell* vl;
        const struct group *gr = getgrgid(groups[j]);
        if (gr) {
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


/* (get-cwd)
 * Returns the current working directory of the process
 * as a string. */
static Cell* system_get_cwd(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "get-cwd");
    if (err) { return err; }

    char pwd[PATH_MAX + 1];
    errno = 0;

    if (getcwd(pwd, sizeof(pwd)) == NULL) {
        return make_cell_error(
            fmt_err("get-cwd: getcwd() call failed: %s", strerror(errno)),
            OS_ERR);
    }
    return make_cell_string(pwd);
}


/* (chdir string)
 * Changes the CWD of the process to the path
 * represented by string. */
static Cell* system_chdir(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "chdir");
    if (err) { return err; }
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "chdir: path argument must be a string",
            TYPE_ERR);
    }

    char* path = a->cell[0]->str;
    /* TODO: tilde expand */
    if (chdir(path) == -1) {
        return make_cell_error(
            fmt_err("chdir: %s: %s", path, strerror(errno)),
            OS_ERR);
    }
    return True_Obj;
}


static Cell* system_uname(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "uname");
    if (err) { return err; }

    struct utsname uts;
    if (uname(&uts) == -1) {
        return make_cell_error(
            fmt_err("uname: %s", strerror(errno)),
            OS_ERR);
    }
    Cell* result = make_cell_nil();
    result = make_cell_pair(make_cell_string(uts.machine), result);
    result->len = 1;
    result = make_cell_pair(make_cell_string(uts.version), result);
    result->len = 2;
    result = make_cell_pair(make_cell_string(uts.release), result);
    result->len = 3;
    result = make_cell_pair(make_cell_string(uts.nodename), result);
    result->len = 4;
    result = make_cell_pair(make_cell_string(uts.sysname), result);
    result->len = 5;

    return result;
}


/* (chmod path mode)
 * Changes the mode bits, specified by integer (represented in octal) arg 'mode',
 * of the file path represented by 'path', passed as a string. */
static Cell* system_chmod(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "chmod");
    if (err) { return err; }
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "chmod: path argument must be a string",
            TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error(
            "chmod: mode argument must be an (octal) integer",
            TYPE_ERR);
    }
    if (chmod(a->cell[0]->str, (mode_t)a->cell[1]->integer_v) != 0) {
        make_cell_error(
            fmt_err("chmod: %s", strerror(errno)),
            OS_ERR);
    }
    return True_Obj;
}

/* TODO:
 * lchmod - in file lib?
 * chown - in file lib?
 * lchown - in file lib?
 * system equiv
 * popen/subprocess equiv
 * more?
 */


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
    /* May already be loaded from file lib, so check the global
     * environment before re-exporting the function. */
    if (!ht_get(e->global, "get-cwd")) {
        lex_add_builtin(e, "get-cwd", system_get_cwd);
    }
    lex_add_builtin(e, "chdir", system_chdir);
    lex_add_builtin(e, "uname", system_uname);
    lex_add_builtin(e, "chmod!", system_chmod);
}
