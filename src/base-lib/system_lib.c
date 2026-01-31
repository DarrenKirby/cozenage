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

/* These includes and defines are all for uptime. */
#ifdef  __linux__
#include <sys/sysinfo.h>
#else
#include <sys/sysctl.h>
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#include <sys/types.h>
#include <sys/param.h>
#include <vm/vm_param.h>
#endif

#define ONE_DAY     86400
#define ONE_HOUR    3600
#define ONE_MINUTE  60
#define LOADS_SCALE 65536.0


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


/* (uptime)
 * Returns information about the system's uptime and load average. The information is returned in a list of length 3.
 * The first item is an integer representing uptime in seconds. The second item is a human-readable uptime string of
 * the form "up 31 days 16:37". The third item is itself a three item list of floats which represent the 1, 5, and 15-
 * minute load average figures in that order. */
Cell* system_uptime(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "uptime");
    if (err) { return err; }

    uint32_t days, hours, minutes;
    uint32_t uptime_in_days, uptime_in_hours;
    uint64_t uptime_in_seconds;
    float av1, av2, av3;
#ifdef __linux__
    /* Get uptime Linux */
    struct sysinfo s_info;
    if (sysinfo(&s_info) == -1) {
        return make_cell_error(
            fmt_err("uptime: sysinfo read failed: %s", strerror(errno)),
            OS_ERR);
    }

    uptime_in_seconds = s_info.uptime;

    /* Get load average Linux */
    av1 = s_info.loads[0] / LOADS_SCALE;
    av2 = s_info.loads[1] / LOADS_SCALE;
    av3 = s_info.loads[2] / LOADS_SCALE;
#else
    /* Get uptime OS X / *BSD */
    struct timeval boot_time;
    size_t len = sizeof(boot_time);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };

    if (sysctl(mib, 2, &boot_time, &len, NULL, 0) < 0 ) {
        return make_cell_error(
            "uptime: failed to get uptime from OS",
            OS_ERR);
    }

    const time_t boot_sec = boot_time.tv_sec;
    const time_t current_sec = time(nullptr);
    uptime_in_seconds = (long)difftime(current_sec, boot_sec);

    /* Get load average *BSD */
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
    double loadavg[3];
    if (getloadavg(loadavg, nitems(loadavg)) == -1) {
        return make_cell_error(
            "uptime: failed to get load average from OS",
            OS_ERR);
    }
    av1 = loadavg[0];
    av2 = loadavg[1];
    av3 = loadavg[2];

    /* Get load average OS X */
#else
    struct loadavg loads;
    size_t load_len = sizeof(loads);
    int mib2[2] = { CTL_VM, VM_LOADAVG };

    if (sysctl(mib2, 2, &loads, &load_len, NULL, 0) < 0) {
        return make_cell_error(
            "uptime: failed to get load average from OS",
            OS_ERR);
    }

    av1 = (float)loads.ldavg[0] / (float)loads.fscale;
    av2 = (float)loads.ldavg[1] / (float)loads.fscale;
    av3 = (float)loads.ldavg[2] / (float)loads.fscale;
#endif
#endif

    /* Factor seconds into larger units */
    days = uptime_in_seconds / ONE_DAY;
    uptime_in_days = uptime_in_seconds - (days * ONE_DAY);
    hours = uptime_in_days / ONE_HOUR;
    uptime_in_hours = uptime_in_days - hours * ONE_HOUR;
    minutes = uptime_in_hours / ONE_MINUTE;

    /* Format human-readable string. */
    char s_buffer[256];
    snprintf(s_buffer, sizeof(s_buffer), "up %d day%s %02d:%02d",
        days, (days != 1) ? "s" : "", hours, minutes);
    Cell* up_s = make_cell_string(s_buffer);

    /* Raw seconds. */
    Cell* ip_i = make_cell_integer((long long)uptime_in_seconds);

    /* Load scale. */
    Cell* ls_1 = make_cell_real(av1);
    Cell* ls_2 = make_cell_real(av2);
    Cell* ls_3 = make_cell_real(av3);

    /* Organize results into list. */
    Cell* result = make_cell_nil();

    Cell* load_list = make_cell_nil();
    load_list = make_cell_pair(ls_3, load_list);
    load_list = make_cell_pair(ls_2, load_list);
    load_list = make_cell_pair(ls_1, load_list);

    result = make_cell_pair(load_list, result);
    result = make_cell_pair(up_s, result);
    result = make_cell_pair(ip_i, result);

    return result;
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
    lex_add_builtin(e, "uptime", system_uptime);
}
