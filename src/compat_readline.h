/*
* 'src/compat_readline.h'
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

#ifndef COZENAGE_COMPAT_READLINE_H
#define COZENAGE_COMPAT_READLINE_H


#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* Fake readline function */
char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

#else

#include <stdio.h>

/* editline does not provide tilde.h, so given certain systems'
 * propensity to provide fake readline stub files, this should
 * be a safe way to determine if we are actually using GNU readline */
#if __has_include(<readline/tilde.h.h>)
#  define HAS_GNU_READLINE 1
#  include <readline/readline.h>
#  include <readline/history.h>
#  include <readline/tilde.h>
#elif __has_include(<editline/readline.h>)
#  define HAS_GNU_READLINE 0
#  include <editline/readline.h>
#else
#  error "No readline-compatible headers found"
#endif

#endif

#endif //COZENAGE_COMPAT_READLINE_H
