/*
 * 'src/process_context_lib.c'
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

#include "process_context_lib.h"
#include "environment.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>


extern char **environ;

Cell* builtin_command_line(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    return make_val_err("not implemented yet", GEN_ERR);
}

Cell* builtin_exit(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_BOOL);
    if (err) { return err; }

    /* TODO: run dynamic-wind /after/ procedure here when implemented */
    if (a->count == 1) {
        if (a->cell[0]->type == VAL_BOOL) {
            const int es = a->cell[0]->b_val;
            if (es) {
                exit(0); /* flip boolean 1 (#t) to exit success (0) */
            }
            exit(1);
        }
        /* If not bool, int. Just return directly */
        exit((int)a->cell[0]->i_val);
    }
    exit(0); /* exit success if no arg */
}

Cell* builtin_emergency_exit(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_BOOL);
    if (err) { return err; }
    if (a->count == 1) {
        if (a->cell[0]->type == VAL_BOOL) {
            const int es = a->cell[0]->b_val;
            if (es) {
                exit(0); /* flip boolean 1 (#t) to exit success (0) */
            }
            exit(1);
        }
    /* If not bool, int. Just return directly */
        exit((int)a->cell[0]->i_val);
    }
    exit(0); /* exit success if no arg */
}

Cell* builtin_get_env_var(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const char *env = getenv(a->cell[0]->str);
    if (env == NULL) {
        return make_val_bool(0);
    }
    const char* var_string = GC_strdup(env);
    return make_val_str(var_string);
}

Cell* builtin_get_env_vars(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0);
    if (err) { return err; }

    /* start with nil */
    Cell* result = make_val_nil();
    int len = 0;

    for (char **env = environ; *env != NULL; env++) {
        /* Bad form to mutate env */
        char* var_string = GC_strdup(*env);
        const char *var = strtok(var_string, "=");
        const char *val = strtok(NULL, "=");
        Cell* vr = make_val_str(var);
        Cell* vl = make_val_str(val);
        result = make_val_pair(make_val_pair(vr, vl), result);
        result->len = len;
        len++;
    }
    return result;
}

void lex_add_proc_con_lib(Lex* e) {
    lex_add_builtin(e, "exit", builtin_exit);
    lex_add_builtin(e, "emergency-exit", builtin_emergency_exit);
    lex_add_builtin(e, "get-environment-variable", builtin_get_env_var);
    lex_add_builtin(e, "get-environment-variables", builtin_get_env_vars);
}
