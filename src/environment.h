/*
 * 'src/environment.h'
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

#ifndef COZENAGE_ENVIRONMENT_H
#define COZENAGE_ENVIRONMENT_H


struct Cell;
typedef struct Cell Cell;

/* Just parallel arrays for now. Will optimize this later */
typedef struct Lex {
    int count;
    char** syms;         /* symbol names */
    Cell** vals;         /* values */
    struct Lex* parent;  /* points to parent env, NULL if top-level */
} Lex;

/* Environment management */
Lex* lex_initialize(void);
Lex* lex_new_child(Lex* parent);
void lex_delete(Lex* e);

/* Environment operations */
Cell* lex_get(const Lex* e, const Cell* k);
void lex_put(Lex* e, const Cell* k, const Cell* v);

/* Builtin helpers */
Cell* lex_make_builtin(const char* name, Cell* (*func)(Lex*, Cell*));
Cell* lex_make_named_lambda(const char* name, const Cell* formals, const Cell* body, Lex* env);
Cell* lex_make_lambda(const Cell* formals, const Cell* body, Lex* env);
void lex_add_builtin(Lex* e, const char* name, Cell* (*func)(Lex*, Cell*));
void lex_add_builtins(Lex* e);

#endif //COZENAGE_ENVIRONMENT_H
