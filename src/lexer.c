/*
 * 'src/lexer.c'
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

#include "lexer.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gc/gc.h>


typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

Scanner scanner;

void init_lexer(const char* source)
{
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}


static bool is_digit(const char c)
{
    return c >= '0' && c <= '9';
}


static bool is_whitespace(const char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}


static bool at_end()
{
    return *scanner.current == '\0';
}


static char advance()
{
    scanner.current++;
    return scanner.current[-1];
}


static char peek()
{
    return *scanner.current;
}


static char peekNext()
{
    if (at_end()) return '\0';
    return scanner.current[1];
}


static Token make_token(const TokenType type)
{
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}


static Token error_token(const char* message)
{
    Token token;
    token.type = T_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}


static void skip_whitespace()
{
    for (;;) {
        const char c = peek();
        switch (c) {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            scanner.line++;
            advance();
            break;
        /* Line comment. */
        case ';':
            while (peek() != '\n' && !at_end()) advance();
            break;
            /* Block comment. */
        case '#':
            if (peekNext() == '|')
            {
                /* Consume "#|". */
                advance(); advance();
                while (peek() != '|' && !at_end())
                {
                    if (peek() == '\n') scanner.line++;
                    advance();
                }
                /* Consume "|#". */
                advance(); advance();
                break;
            }
            return;
        default:
            return;
        }
    }
}


static Token string()
{
    while (peek() != '"' && !at_end()) {
        const char c = peek();

        if (c == '\\') {
            /* It's an escape character. */
            advance(); /* Consume the backslash. */

            /* Check for EOF right after the backslash. */
            if (at_end()) return error_token("Unterminated string.");

            /* If the escaped char is a newline, count it. */
            if (peek() == '\n') {
                scanner.line++;
            }

            /* Consume the escaped character.
               We don't care what it is, we just skip over it. */
            advance();
        } else if (c == '\n') {
            /* This is a *literal* (unescaped) newline in the string. */
            scanner.line++;
            advance();
        } else {
            /* Any other regular character. */
            advance();
        }
    }

    if (at_end()) return error_token("Unterminated string.");

    /* Consume the closing quote. */
    advance();
    return make_token(T_STRING);
}


static Token number()
{
    while (!is_whitespace(peek()) && !at_end() && peek() != ')') {
        advance();
    }
    return make_token(T_NUMBER);
}


static Token boolean()
{
    scanner.start = scanner.current;
    while (!is_whitespace(peek()) && !at_end() && peek() != ')') {
        advance();
    }
    return make_token(T_BOOLEAN);
}


static Token multi_word_identifier()
{
    while (peek() != '|' && !at_end()) {
        advance();
    }
    if (at_end()) return error_token("Unterminated multi-word identifier.");

    advance();
    return make_token(T_SYMBOL);
}


static Token symbol()
{
    while (!is_whitespace(peek()) && peek() != ')' && peek() != '(' && !at_end()) {
        advance();
    }
    return make_token(T_SYMBOL);
}


static Token character()
{
    scanner.start = scanner.current;
    while (!is_whitespace(peek()) && peek() != ')' && peek() != '(' && !at_end()) {
        advance();
    }
    return make_token(T_CHAR);
}


Token lex_token()
{
    skip_whitespace();
    scanner.start = scanner.current;

    if (at_end()) return make_token(T_EOF);

    const char c = advance();
    if (is_digit(c)) return number();

    switch (c) {
        case '(':
            return make_token(T_LEFT_PAREN);
        case ')':
            return make_token(T_RIGHT_PAREN);
        case '"':
            return string();
        case '\'':
            return make_token(T_QUOTE);
        case '`':
            return make_token(T_QUASIQUOTE);

        /* Either a number prefix, or symbol. */
        case '+':
        case '-': {
            /* -inf.0, +inf.0, +nan.0, and -nan.0 need special handling.
             * lex them as symbols, and deal with it in the parser. */
            if (peek() == 'i' && peekNext() == 'n') return symbol();
            if (peek() == 'n' && peekNext() == 'a') return symbol();
            if (is_digit(peek())) return number();
            return make_token(T_SYMBOL);
        }

        /* Comma, and comma at. */
        case ',': {
            switch (peek()) {
                case '@':
                    advance();
                    return make_token(T_COMMA_AT);
                default:
                    return make_token(T_COMMA);
            }
        }

        /* Multiple possibilities, depending on what follows the hash. */
        case '#': {
            switch (peek()) {
                /* Character literal. */
                case '\\':
                    advance();
                    return character();
                /* #t, #f, #true, #false. */
                case 't':
                case 'f':
                    return boolean();
                /* Exact or inexact, and numeric base literals. */
                case 'e':
                case 'i':
                case 'o':
                case 'd':
                case 'x':
                case 'b':
                    advance();
                    Token t = number();
                    t.start++;
                    t.length--;
                    return t;
                default: return make_token(T_HASH);
            }
        }
        /* eg: |dumb variable name| */
        case '|':
            return multi_word_identifier();
        /* If it's not something else, treat as a symbol/identifier. */
        default: return symbol();
    }
}


static TokenArray* init_token_array()
{
    /* Allocate space for the manager struct itself. */
    TokenArray *ta = GC_MALLOC(sizeof(TokenArray));
    if (ta == NULL) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }

    /* Allocate space for the actual tokens it will hold. */
    ta->tokens = GC_MALLOC(TA_CAPACITY * sizeof(Token));
    if (ta->tokens == NULL) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }

    ta->count = 0;
    ta->capacity = TA_CAPACITY;
    ta->position = 0;
    return ta;
}


static TokenArray* write_token_array(TokenArray* ta, const Token token)
{
    /* Check if we need to reallocate. */
    if (ta->count == ta->capacity) {
        ta->capacity *= 2;

        ta->tokens = GC_REALLOC(ta->tokens, ta->capacity * sizeof(Token));
        if (ta->tokens == NULL) {
            fprintf(stderr, "ENOMEM: GC_REALLOC failed\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Add the new token. */
    ta->tokens[ta->count] = token;
    ta->count++;

    return ta;
}


TokenArray* scan_all_tokens(const char* source)
{
    TokenArray *t_array = init_token_array();

    init_lexer(source);
    for (;;) {
        const Token token = lex_token();
        t_array = write_token_array(t_array, token);
        if (token.type == T_EOF) break;
    }

    return t_array;
}


void debug_lexer(const TokenArray* ta)
{
    int line = -1;

    for (int i = 0; i < ta->count; i++) {
        const Token token = ta->tokens[i];

        if (token.type == T_EOF) {
            break;
        }

        if (token.line != line) {
            printf("%4d ", token.line);
            line = token.line;
        } else {
            printf("   | ");
        }
        printf("%2d [ %.*s ]\n", token.type, token.length, token.start);
    }
    printf("token count: %d\n", ta->count);
    printf("token position: %d\n", ta->position);
}
