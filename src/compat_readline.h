/*
* 'src/compat_readline.h'
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

#ifndef COZENAGE_COMPAT_READLINE_H
#define COZENAGE_COMPAT_READLINE_H


#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* Fake readline function. */
char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = GC_MALLOC(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

/* Fake add_history function. */
void add_history(char* unused) {}

#else

#ifdef USE_GNU_READLINE

// ReSharper disable once CppUnusedIncludeDirective
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <readline/tilde.h>
#include "environment.h"


char* scheme_procedure_generator(const char *text, int state);
char** completion_dispatcher(const char *text, int start, int end);
void populate_dynamic_completions(const Lex* e);

#else

#include <readline/readline.h>
#include <readline/history.h>

#endif

#endif

#endif //COZENAGE_COMPAT_READLINE_H
