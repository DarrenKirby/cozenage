/*
 * 'src/lexer.h'
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

#ifndef COZENAGE_LEXER_H
#define COZENAGE_LEXER_H


typedef enum {
    /* Single char tokens */
    T_LEFT_PAREN ,
    T_RIGHT_PAREN,
    T_HASH,
    /* Multi-char tokens */
    T_NUMBER,
    T_CHAR,
    T_STRING,
    T_SYMBOL,
    T_BOOLEAN,
    T_QUOTE,
    T_QUASIQUOTE,
    /* Special tokens */
    T_ERROR,
    T_EOF
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

typedef struct {
    Token* tokens;
    int count;
    int capacity;
    int position;
} TokenArray;

#define TA_CAPACITY 8


TokenArray* scan_all_tokens(const char* source);
void debug_lexer(const TokenArray* ta);

#endif //COZENAGE_LEXER_H
