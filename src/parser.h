/*
 * 'src/parser.h'
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

#ifndef COZENAGE_PARSER_H
#define COZENAGE_PARSER_H

#include "cell.h"

/* for debug output */
#define DEBUG 0

typedef struct {
    char **array;
    int position;
    int size;
} Parser;

char **lexer(const char *input, int *count);
void free_tokens(char **tokens, int count);
Parser *parse_str(const char *input);
Cell *parse_tokens(Parser *p);
Cell *parse_atom(const char *tok);
int paren_balance(const char *s, int *in_string);

#endif //COZENAGE_PARSER_H
