/*
 * 'src/parser.c'
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

#include "main.h"
#include "parser.h"
#include "types.h"
#include <gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <wctype.h>
#include <unicode/uchar.h>
/* Linux needs this include, macOS and FreeBSD do not */
#ifdef __linux__
#include <ctype.h>
#endif


static long long parse_int_checked(const char* str, char* err_buf, const int base, int* ok) {
    errno = 0;
    char* end_ptr;
    const long long val = strtoll(str, &end_ptr, base);

    if (end_ptr == str) {
        snprintf(err_buf, 128, "Invalid numeric: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }
    if (errno == ERANGE || val > LLONG_MAX || val < LLONG_MIN) {
        snprintf(err_buf, 128, "Integer out of range: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }
    if (*end_ptr != '\0') {
        snprintf(err_buf, 128, "Invalid trailing characters in numeric: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }

    *ok = 1;
    return val;
}

static long double parse_float_checked(const char* str, char* err_buf, int* ok) {
    errno = 0;
    char* end_ptr;
    const long double val = strtold(str, &end_ptr);

    if (end_ptr == str) {
        snprintf(err_buf, 128, "Invalid numeric: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }
    if (errno == ERANGE) {
        snprintf(err_buf, 128, "Float out of range: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }
    /* early exit for nan.0, +inf.0, -inf.0 */
    if (*end_ptr == '.') {
        *ok = 1;
        return val;
    }
    /* Raises syntax error for things like "1234HELLO" */
    if (*end_ptr != '\0') {
        snprintf(err_buf, 128, "Invalid trailing characters in numeric: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }

    *ok = 1;
    return val;
}

static char* token_to_string(Token* token) {
    /* Allocate memory for the new string (+1 for the null terminator) */
    char* str = GC_MALLOC(token->length + 1);
    if (str == NULL) {
        fprintf(stderr, "Memory allocation failed in token_to_string.\n");
        exit(EXIT_FAILURE);
    }

    /* Copy the token's characters into the new string */
    memcpy(str, token->start, token->length);

    /* Add the null terminator */
    str[token->length] = '\0';

    return str;
}

static Cell* parse_number(char* token, const int line, int len) {
    int base = 10;   /* Default to base 10 */
    int exact = -1;  /* Default to unspecified */
    int ok = 0;      /* error flag */
    char err_buf[128] = {0};

    /* This oddball needs to get dispatched before
     * it hits the next if/else block */
     if (strcmp("inf.0", token) == 0) {
         return make_cell_real(parse_float_checked(token, err_buf, &ok));
     }

    /* Check for prefixes */
    char *tok;
    if (token[0] == 'b') {
        base = 2; len -= 1;
        tok = token + 1;
    } else if (token[0] == 'o') {
        base = 8; len -= 1;
        tok = token + 1;
    } else if (token[0] == 'd') {
        len -= 1;
        tok = token + 1;
    } else if (token[0] == 'x') {
        base = 16; len -= 1;
        tok = token + 1;
    } else if (token[0] == 'e') {
        exact = 1; len -= 1;
        tok = token + 1;
    } else if (token[0] == 'i') {
        exact = 0; len -= 1;
        tok = token + 1;
    } else {
        /* No prefix */
        tok = token;
    }

    /* Imaginary number */
    if (tok[len - 1] == 'i') {

        char *p = tok;

        /* strip trailing 'i' */
        p[len - 1] = '\0';

        Cell* r;
        Cell* i;

        /* Find the last '+' or '-' (but not the leading sign) */
        char *sep = nullptr;
        for (char *q = p + 1; *q; q++) {
            if (*q == '+' || *q == '-') {
                sep = q;
            }
        }

        if (!sep) {
            /* pure imaginary case: "12i", "-12i", "+12i", "i", "-i", "+i" */
            r = make_cell_integer(0);

            if (strcmp(p, "+") == 0 || strcmp(p, "") == 0) {
                i = make_cell_integer(1);
            } else if (strcmp(p, "-") == 0) {
                i = make_cell_integer(-1);
            } else {
                i = parse_number(p, line, len);
            }

        } else {
            /* real ± imag case: "23+10i", "23-10i", "-23+10i", etc. */
            const char sign = *sep;     /* save '+' or '-' */
            *sep = '\0';                /* terminate real part */
            char *real_str = p;

            /* imaginary part starts right after sep */
            const char *imag_digits = sep + 1;

            /* rebuild full imag string: e.g. "+10" or "-10" */
            char buf[64];
            snprintf(buf, sizeof(buf), "%c%s", sign, imag_digits);

            r = parse_number(real_str, line, len);
            i = parse_number(buf, line, len);
        }
        Cell* result = make_cell_complex(r, i);
        return result;
    }

    /* Rational number */
    if (strchr(tok, '/')) {
        char *p;
        p = GC_strdup(tok);

        const char *tok1 = strsep(&p, "/");
        const char *tok2 = strsep(&p, "/");

        if (p != NULL) {
            snprintf(err_buf, sizeof(err_buf), "Line %d: Invalid token: '%s%s%s'",
                line,
                ANSI_RED_B, tok, ANSI_RESET);
            return make_cell_error(err_buf, SYNTAX_ERR);
        }

        const long long n = parse_int_checked(tok1, err_buf, base, &ok);
        const long long d = parse_int_checked(tok2, err_buf, 10, &ok);

        if (d == 0) {
            fprintf(stderr, "Line %d: Invalid token: '%s'\n",line, tok);
            return make_cell_error(
                "Cannot have zero-value denominator in rational",
                VALUE_ERR);
        }

        /* Make inexact if specified by prefix */
        Cell* result = make_cell_rational(n, d, 1);
        if (exact == 0) {
            result->exact = 0;
        }
        return result;
    }

    /* Integers and reals */

    /* Try integer parsing if not base 10, or no decimal */
    if (base != 10 || !strchr(tok, '.')) {
        const long long i = parse_int_checked(tok, err_buf, base, &ok);
        if (ok) {
            Cell* result = make_cell_integer(i);
            if (exact == 0) {
                result->exact = 0;
            }
            return result;
        }
    }

    /* Otherwise, try float */
    if (base == 10) {
        const long double f = parse_float_checked(tok, err_buf, &ok);
        if (ok) {
            Cell* result = make_cell_real(f);
            if (exact == 1) {
                result->exact = 1;
            }
        return result;
        }
    }

    /* If parsing fails but there is a numeric-like string, return error */
    if (!ok) {
        fprintf(stderr, "Error in numeric on line %d:\n", line);
        return make_cell_error(err_buf, SYNTAX_ERR);
    }

    /* If we get here, something's really wrong */
    snprintf(err_buf, sizeof(err_buf),
        "Line %d: Unable to parse numeric token: '%s%s%s'",
        line, ANSI_RED_B, tok, ANSI_RESET);
    return make_cell_error(err_buf, SYNTAX_ERR);
}

static Cell* parse_string(const char* tok, const int len) {
    /* Remove open and closing quote */
    const char *str = GC_strndup(tok+1, len - 2);
    return make_cell_string(str);
}

static Cell* parse_boolean(const char* tok, const int line) {
    if (strcmp(tok, "t") == 0 ||
        strcmp(tok, "true") == 0) return make_cell_boolean(1);
    if (strcmp(tok, "f") == 0 ||
        strcmp(tok, "false") == 0) return make_cell_boolean(0);
    /* Otherwise an error */
    char err_buf[128] = {0};
    snprintf(err_buf, sizeof(err_buf), "Line %d: Unable to parse token: '%s#%s%s'",
                    line,
                    ANSI_RED_B, tok, ANSI_RESET);
    return make_cell_error(err_buf, SYNTAX_ERR);
}

static Cell* parse_symbol(char* tok, const int line, const int len) {
    /* This is kind of an ugly kludge, but I'm
     * not sure how to do it more elegantly. */
    if (strcmp(tok, "+inf.0") == 0 ||
        strcmp(tok, "-inf.0") == 0 ||
        strcmp(tok, "nan.0") == 0 ||
        strcmp(tok, "inf.0") == 0) {
        return parse_number(tok, line, len);
    }
    return make_cell_symbol(tok);
}

static Cell* parse_character(char* tok, const int line, const int len) {
    char err_buf[128] = {0};
    
    /* Handle the special '#\' -> space case */
    if (len == 0) {
        return make_cell_char(' ');
    }

    /* Check for multi-letter named characters and hex literals. */
    if (len > 1 || tok[0] == 'x') {
        /* Handle (R7RS required) named characters */
        if (strcmp(tok, "space") == 0) return make_cell_char(' ');
        if (strcmp(tok, "newline") == 0) return make_cell_char('\n');
        if (strcmp(tok, "alarm") == 0) return make_cell_char(0x7);
        if (strcmp(tok, "backspace") == 0) return make_cell_char(0x8);
        if (strcmp(tok, "delete") == 0) return make_cell_char(0x7f);
        if (strcmp(tok, "escape") == 0) return make_cell_char(0x1b);
        if (strcmp(tok, "null") == 0) return make_cell_char('\0');
        if (strcmp(tok, "return") == 0) return make_cell_char(0xd);
        if (strcmp(tok, "tab") == 0) return make_cell_char('\t');

        /* Check mapping of implementation-specific named chars */
        const NamedChar* named_char = find_named_char(tok);
        if (named_char) {
            return make_cell_char(named_char->codepoint);
        }

        /* Handle hex literals: #\x... */
        if (tok[0] == 'x' && len > 1) {
            char* end_ptr;
            const long code = strtol(tok + 1, &end_ptr, 16);

            if (*end_ptr != '\0' || code < 0 || code > 0x10FFFF) {
                snprintf(err_buf, sizeof(err_buf),
                    "Line %d, Invalid Unicode hex value: '%s%s%s'",
                    line, ANSI_RED_B, tok, ANSI_RESET);
                return make_cell_error(err_buf, VALUE_ERR);
            }
            return make_cell_char((UChar32)code);
        }
    }

    /* If it wasn't a recognized multi-letter name or hex,
     * THEN treat it as a single character literal. */
    UChar32 code_point;
    int32_t i = 0;

    /* Decode the first code point from the payload */
    U8_NEXT_UNSAFE(tok, i, code_point);

    /* If 'i' is not equal to the payload length, it means there was
     * more than one character after #\ (e.g., #\ab or #\λa),
     * which is an error according to the R7RS standard. */
    if (i != len) {
        snprintf(err_buf, sizeof(err_buf),
            "Line %d, Invalid character literal: '%s%s%s'",
            line, ANSI_RED_B, tok, ANSI_RESET);
        return make_cell_error(err_buf, SYNTAX_ERR);
    }

    return make_cell_char(code_point);
}

static Token *peek_new(const TokenArray *p) {
    if (p->position < p->count) return &p->tokens[p->position];
    return nullptr;
}

static Token *advance_new(TokenArray *p) {
    if (p->position < p->count) return &p->tokens[p->position++];
    return nullptr;
}

Cell* parse_tokens_new(TokenArray *ta) {
    /* First check that the expression is balanced */
    int left_count = 0, right_count = 0;

    for (int i = 0; i < ta->count; i++) {
        if (ta->tokens[i].type == T_LEFT_PAREN) left_count++;
        if (ta->tokens[i].type == T_RIGHT_PAREN) right_count++;
    }

    if (left_count != right_count) {
        return make_cell_error(
            "Expression has unbalanced parentheses.",
            SYNTAX_ERR);
    }

    /* Check token type and dispatch accordingly */
    Token *token = &ta->tokens[ta->position];
    if (!token) return nullptr;

    switch (token->type) {
        case T_EOF: return nullptr; /* We're done */
        /* Dispatch out the atoms, first */
        case T_NUMBER: return parse_number(token_to_string(token), token->line, token->length);
        case T_STRING: return parse_string(token_to_string(token), token->length);
        case T_SYMBOL: return parse_symbol(token_to_string(token), token->line, token->length);
        case T_BOOLEAN: return parse_boolean(token_to_string(token), token->line);
        case T_CHAR: return parse_character(token_to_string(token), token->line, token->length);
        case T_ERROR: return make_cell_error(token_to_string(token), SYNTAX_ERR);
        default: break;
    }

    /* It's a compound type */
    char err_buf[128] = {0};

    /* Just handle quote and quasiquote the same for now */
    if (token->type == T_QUOTE || token->type == T_QUASIQUOTE) {
        /* Grab the next token */
        Token* t = advance_new(ta);
        Cell *quoted = parse_tokens_new(ta);
        if (!quoted) {
            snprintf(err_buf, sizeof(err_buf),
                        "Line %d: Expected expression after quote: '%s%s%s'",
                        t->line, ANSI_RED_B, token_to_string(t), ANSI_RESET);
            return make_cell_error(err_buf, SYNTAX_ERR);
        }
        Cell *qexpr = make_cell_sexpr();
        cell_add(qexpr, make_cell_symbol("quote"));
        cell_add(qexpr, quoted);
        return qexpr;
    }

    /* Vector or bytevector */
    if (token->type == T_HASH) {
        token = advance_new(ta); /* Consume '#' */
        Cell* vec;

        if (peek_new(ta)->type == T_SYMBOL && strcmp("u8", token_to_string(peek_new(ta))) == 0) {
            /* Bytevector */
            vec = make_cell_bytevector();
            token = advance_new(ta); /* consume '#u8' */
        } else {
            /* Vector */
            vec = make_cell_vector();
        }

        if (peek_new(ta)->type != T_LEFT_PAREN) {
            snprintf(err_buf, sizeof(err_buf),
                        "Line %d: Expected '(' in vector literal: '%s%s%s'",
                        token->line, ANSI_RED_B, token_to_string(token), ANSI_RESET);
            return make_cell_error(err_buf, SYNTAX_ERR);
        }

        token = advance_new(ta); /* Consume '(' */
        while (peek_new(ta)->type != T_RIGHT_PAREN) {
            cell_add(vec, parse_tokens_new(ta));
            advance_new(ta);
        }

        if (!peek_new(ta)) {
            snprintf(err_buf, sizeof(err_buf),
                        "Line %d: Unmatched '(' in vector literal: '%s%s%s'",
                        token->line, ANSI_RED_B, token_to_string(token), ANSI_RESET);
            return make_cell_error(err_buf, SYNTAX_ERR);
        }

        /* Check for valid values in #u8() bytevector. */
        if (vec->type == CELL_BYTEVECTOR) {
            for (int i = 0; i < vec->count; i++) {
                if (vec->cell[i]->type != CELL_INTEGER) {
                    return make_cell_error(
                        "bytevector members must be integers",
                        VALUE_ERR);
                }
                if (vec->cell[i]->integer_v < 0 || vec->cell[i]->integer_v > 255) {
                    return make_cell_error(
                        " u8 bytevector members must be between 0 and 255 (inclusive)",
                        VALUE_ERR);
                }
            }
        }
        return vec;
    }

    /* S-expression */
    if (token->type == T_LEFT_PAREN) {
        token = advance_new(ta); /* Consume '(' */
        if (token->type == T_RIGHT_PAREN) {
            /* Unquoted nil is an error */
            snprintf(err_buf, sizeof(err_buf),
                        "Line %d: Empty S-expression.", token->line);
            return make_cell_error(err_buf, SYNTAX_ERR);
        }
        Cell *sexpr = make_cell_sexpr();

        while (peek_new(ta)->type != T_RIGHT_PAREN) {
            cell_add(sexpr, parse_tokens_new(ta));
            advance_new(ta);
        }
        if (!peek_new(ta)) {
            snprintf(err_buf, sizeof(err_buf),
                        "Line %d: Unmatched '('.", token->line);
            return make_cell_error(err_buf, SYNTAX_ERR);
        }
        return sexpr;
    }
    /* Should not ever get here, but maybe it's a hash with no vector? */
    snprintf(err_buf, sizeof(err_buf), "Line %d: bad token", token->line);
    return make_cell_error(err_buf, SYNTAX_ERR);
}
