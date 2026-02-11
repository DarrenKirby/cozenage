/*
 * 'src/lexer.h'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 - 2026  Darren Kirby <darren@dragonbyte.ca>
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
    T_RIGHT_BRACKET,   /* ']' ... signals end of set. */
    T_RIGHT_BRACE,     /* '}' ... signals end of map. */
    /* Multi-char tokens */
    T_NUMBER,
    T_CHAR,
    T_STRING,
    T_SYMBOL,
    T_BOOLEAN,
    T_MAP_START,    /* '#[' ... signals start of set. */
    T_SET_START,    /* '#{' ... signals start of map. */
    T_QUOTE,
    T_QUASIQUOTE,
    T_COMMA,
    T_COMMA_AT,
    /* Special tokens */
    T_ERROR,
    T_EOF
} TokenType;

typedef struct {
    int length;
    int line;
    TokenType type;
    const char* start;
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
