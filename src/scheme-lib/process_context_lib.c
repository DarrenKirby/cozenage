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

#include "repr.h"
#include "special_forms.h"


extern char **environ;
extern int is_repl;
extern int g_argc;
extern char **g_argv;

Cell* builtin_command_line(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    /* Return list of just empty string if using REPL */
    if (is_repl) {
        return make_cell_pair(make_cell_string(""), make_cell_nil());
    }
    /* Construct the list of args. */
    Cell* args = make_cell_sexpr();
    for (int i = 0; i < g_argc; i++) {
        cell_add(args, make_cell_string(g_argv[i]));
    }
    return sexpr_to_list(args);
}

Cell* builtin_exit(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_BOOLEAN);
    if (err) { return err; }

    /* TODO: run dynamic-wind /after/ procedure here when implemented */
    if (a->count == 1) {
        if (a->cell[0]->type == CELL_BOOLEAN) {
            const int es = a->cell[0]->boolean_v;
            if (es) {
                exit(0); /* flip boolean 1 (#t) to exit success (0) */
            }
            exit(1);
        }
        /* If not bool, int. Just return directly */
        exit((int)a->cell[0]->integer_v);
    }
    exit(0); /* exit success if no arg */
}

Cell* builtin_emergency_exit(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_BOOLEAN);
    if (err) { return err; }
    if (a->count == 1) {
        if (a->cell[0]->type == CELL_BOOLEAN) {
            const int es = a->cell[0]->boolean_v;
            if (es) {
                exit(0); /* flip boolean 1 (#t) to exit success (0) */
            }
            exit(1);
        }
    /* If not bool, int. Just return directly */
        exit((int)a->cell[0]->integer_v);
    }
    exit(0); /* exit success if no arg */
}

Cell* builtin_get_env_var(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const char *env = getenv(a->cell[0]->str);
    if (env == NULL) {
        return make_cell_boolean(0);
    }
    const char* var_string = GC_strdup(env);
    return make_cell_string(var_string);
}

Cell* builtin_get_env_vars(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    Cell* err = CHECK_ARITY_EXACT(a, 0);
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

void lex_add_proc_con_lib(const Lex* e) {
    lex_add_builtin(e, "command-line", builtin_command_line);
    lex_add_builtin(e, "exit", builtin_exit);
    lex_add_builtin(e, "emergency-exit", builtin_emergency_exit);
    lex_add_builtin(e, "get-environment-variable", builtin_get_env_var);
    lex_add_builtin(e, "get-environment-variables", builtin_get_env_vars);
}
