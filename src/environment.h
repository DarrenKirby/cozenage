/*
 * 'src/environment.h'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 Darren Kirby <darren@dragonbyte.ca>
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

#ifndef COZENAGE_ENVIRONMENT_H
#define COZENAGE_ENVIRONMENT_H

#include "hash.h"


typedef struct Ch_Env Ch_Env;

#define INITIAL_CHILD_ENV_CAPACITY 4

/* Wrapper which holds the current child-env (if any), and a
 * pointer to the global env hash table. */
typedef struct Lex {
    Ch_Env* local;     /* The current local scope. */
    ht_table* global;  /* The global hash table. */
} Lex;


/* Just parallel arrays for small, short-lived child environments. */
typedef struct Ch_Env {
    int count;              /* Number of occupied slots. */
    int capacity;           /* Allocated slots. */
    char** syms;            /* symbol names. */
    Cell** vals;            /* values. */
    Ch_Env* parent;         /* points to parent env, NULL if top-level. */
} Ch_Env;


/* Environment management. */
Lex* lex_initialize_global_env(void);
Lex* new_child_env(const Lex* parent_env);


/* Environment operations. */
Cell* lex_get(const Lex* e, const Cell* k);
void lex_put_local(Lex* e, const Cell* k, const Cell* v);
void lex_put_global(const Lex* e, const Cell* k, Cell* v);


/* Builtin helpers. */
Cell* lex_make_builtin(const char* name, Cell* (*func)(const Lex*, const Cell*));
Cell* lex_make_named_lambda(char* name, Cell* formals, Cell* body, Lex* env);
Cell* lex_make_lambda(Cell* formals, Cell* body, Lex* env);
Cell* lex_make_defmacro(char* name, Cell* formals, Cell* body, Lex* env);
void lex_add_builtin(const Lex* e, const char* name, Cell* (*func)(const Lex*, const Cell*));
void lex_add_builtins(const Lex* e);

#endif //COZENAGE_ENVIRONMENT_H
