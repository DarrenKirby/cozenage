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
#include <ctype.h>
#include <wctype.h>
#include <unicode/uchar.h>


long long parse_int_checked(const char* str, char* err_buf, const size_t err_buf_sz, const int base, int* ok) {
    errno = 0;
    char* end_ptr;
    const long long val = strtoll(str, &end_ptr, base);

    if (end_ptr == str) {
        snprintf(err_buf, err_buf_sz, "Invalid numeric: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }
    if (errno == ERANGE || val > LLONG_MAX || val < LLONG_MIN) {
        snprintf(err_buf, err_buf_sz, "Integer out of range: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }
    if (*end_ptr != '\0') {
        snprintf(err_buf, err_buf_sz, "Invalid trailing characters in numeric: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }

    *ok = 1;
    return val;
}

long double parse_float_checked(const char* str, char* err_buf, const size_t err_buf_sz, int* ok) {
    errno = 0;
    char* end_ptr;
    const long double val = strtold(str, &end_ptr);

    if (end_ptr == str) {
        snprintf(err_buf, err_buf_sz, "Invalid numeric: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }
    if (errno == ERANGE) {
        snprintf(err_buf, err_buf_sz, "Float out of range: '%s%s%s'",
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
        snprintf(err_buf, err_buf_sz, "Invalid trailing characters in numeric: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }

    *ok = 1;
    return val;
}

/* lexer() -> char**
 * Take a line of input and return an array of strings
 * containing all the tokens.
 * */
char **lexer(const char *input, int *count) {
    int capacity = 8;
    char **tokens = GC_MALLOC(capacity * sizeof(char *));
    if (tokens == NULL) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }

    int n = 0;
    const char *p = input;

    while (*p) {
        /* Skip whitespace (including newlines) */
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;

        /* Skip comments */
        if (*p == ';') {
            while (*p && *p != '\n') p++;
            continue;
        }

        /* Allocate more space if needed */
        if (n + 1 >= capacity) {
            capacity *= 2;
            char **tmp_tokens = GC_REALLOC(tokens, capacity * sizeof(char *));
            if (tmp_tokens == NULL) {
                fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
                exit(EXIT_FAILURE);
            }
            tokens = tmp_tokens;
        }

        /* quote char: ' */
        if (*p == '\'') {
            /* '() - null */
            if (*(p+1) == '(' && *(p+2) == ')') {
                tokens[n++] = GC_strdup("'()\0");
                p += 3;
            } else {
                tokens[n++] = GC_strdup("'");  /* emit a single-quote token */
                p++;
            }
            continue;
        }

        /* Parentheses as single-char tokens */
        if (*p == '(' || *p == ')') {
            const char buf[2] = {*p, '\0'};
            tokens[n++] = GC_strdup(buf);
            p++;
        }
        /* String literal */
        else if (*p == '"') {
            p++; /* skip opening quote */
            size_t buf_size = 64;
            char *tok = GC_MALLOC(buf_size);
            if (tok == NULL) {
                fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
                exit(EXIT_FAILURE);
            }
            size_t len = 0;
            tok[len++] = '"';
            while (*p && *p != '"') {
                if (*p == '\\' && *(p+1)) {
                    if (len + 2 >= buf_size) {
                        buf_size *= 2;
                        tok = GC_REALLOC(tok, buf_size);
                        if (tok == NULL) {
                            fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    tok[len++] = *p++;
                    tok[len++] = *p++;
                } else {
                    if (len + 1 >= buf_size) {
                        buf_size *= 2;
                        tok = GC_REALLOC(tok, buf_size);
                        if (tok == NULL) {
                            fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    tok[len++] = *p++;
                }
            }
            tok[len++] = '"';
            tok[len] = '\0';
            tokens[n++] = tok;
            if (*p == '"') p++; /* skip closing quote */
        }
        /* Hash literals: booleans #t/#f, characters #\x, numeric bases #b #o #d #x */
        else if (*p == '#') {
            const char *start = p;
            p++;  /* skip '#' */

            /* Consume entire token until whitespace, paren, or quote */
            while (*p && !isspace((unsigned char)*p) &&
                   *p != '(' && *p != ')' && *p != '"') {
                p++;
                   }

            const size_t len = p - start;
            char *tok = GC_MALLOC(len + 1);
            if (tok == NULL) {
                fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
                exit(EXIT_FAILURE);
            }
            memcpy(tok, start, len);
            tok[len] = '\0';
            tokens[n++] = tok;
        }

        /* Symbol / number token */
        else {
            const char *start = p;
            while (*p && !isspace((unsigned char)*p) && *p != '(' && *p != ')' && *p != '"') {
                if (*p == ';') break; /* comment start ends token */
                p++;
            }
            const long len = p - start;
            char *tok = GC_MALLOC(len + 1);
            memcpy(tok, start, len);
            tok[len] = '\0';
            tokens[n++] = tok;
        }
    }

    /* Null-terminate array */
    tokens[n] = nullptr;
    if (count) *count = n;

    /* Debug print */
    if (DEBUG) {
        for (int i = 0; i < n; i++) {
            printf("TOK[%d] = |%s|\n", i, tokens[i]);
        }
    }
    return tokens;
}

/* parse_str() -> Parser
 * Take a raw line, run it through the lexer,
 * then return the tokens in a Parser object.
 * */
Parser *parse_str(const char *input) {;
    int count;
    char **tokens = lexer(input, &count);

    Parser *p = GC_MALLOC(sizeof(Parser));
    if (p == NULL) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }

    p->array = tokens;
    p->position = 0;
    p->size = count;

    return p;
}

static const char *peek(const Parser *p) {
    if (p->position < p->size) return p->array[p->position];
    return nullptr;
}

static const char *advance(Parser *p) {
    if (p->position < p->size) return p->array[p->position++];
    return nullptr;
}

Cell* parse_tokens(Parser *p) {
    int left_count = 0, right_count = 0;

    for (int i = 0; i < p->size; i++) {
        if (strcmp(p->array[i], "(") == 0) left_count++;
        else if (strcmp(p->array[i], ")") == 0) right_count++;
    }

    if (left_count != right_count) {
        return make_val_err("Unbalanced parentheses", SYNTAX_ERR);
    }

    const char *tok = peek(p);
    if (!tok) return nullptr;

    /* Handle quote (') */
    if (strcmp(tok, "'") == 0) {
        advance(p);  /* consume ' */
        Cell *quoted = parse_tokens(p);
        if (!quoted) return make_val_err("Expected expression after quote", SYNTAX_ERR);
        Cell *qexpr = make_val_sexpr();
        cell_add(qexpr, make_val_sym("quote"));
        cell_add(qexpr, quoted);
        return qexpr;
    }

    /* byte vector: #u8( ... ) */
    if (strcmp(tok, "#u8") == 0) {
        advance(p); /* consume '#u8' */
        const char *paren = advance(p); /* must be '(' */
        if (!paren || strcmp(paren, "(") != 0)
            return make_val_err("Expected '(' after #u8", SYNTAX_ERR);
        Cell *bv = make_val_bytevec();
        while (peek(p) && strcmp(peek(p), ")") != 0) {
            cell_add(bv, parse_tokens(p));
        }
        if (!peek(p)) {
            return make_val_err("Unmatched '(' in bytevector literal", SYNTAX_ERR);
        }
        advance(p); /* skip ')' */

        /* ensure members in range */
        for (int i = 0; i < bv->count; i++) {
            if (bv->cell[i]->type != VAL_INT) {
                return make_val_err("bytevector members must be integers", VALUE_ERR);
            }
            if (bv->cell[i]->i_val < 0 || bv->cell[i]->i_val > 255) {
                return make_val_err("bytevector members must be between 0 and 255 (inclusive)", VALUE_ERR);
            }
        }
        return bv;
    }

    /* Handle vector literals */
    if (strcmp(tok, "#") == 0) {
        advance(p);  /* consume '#' */
        const char *next = peek(p);
        if (!next) return make_val_err("Invalid token: '#'", SYNTAX_ERR);

        /* plain vector: #( ... ) */
        if (strcmp(next, "(") == 0) {
            advance(p); /* consume '(' */
            Cell *vec = make_val_vect();
            while (peek(p) && strcmp(peek(p), ")") != 0) {
                cell_add(vec, parse_tokens(p));
            }
            if (!peek(p)) {
                return make_val_err("Unmatched '(' in vector literal", SYNTAX_ERR);
            }
            advance(p); /* skip ')' */
            return vec;
        }
        return make_val_err("Invalid token", SYNTAX_ERR);
    }

    /* Handle S-expressions '(' ... ')' */
    if (strcmp(tok, "(") == 0) {
        advance(p);  /* consume '(' */
        Cell *sexpr = make_val_sexpr();
        while (peek(p) && strcmp(peek(p), ")") != 0) {
            cell_add(sexpr, parse_tokens(p));
        }
        if (!peek(p)) {
            return make_val_err("Unmatched '('", SYNTAX_ERR);
        }

        advance(p); /* skip ')' */
        return sexpr;
    }

    /* Otherwise, atom (number, boolean, char, string, symbol) */
    advance(p);
    return parse_atom(tok);
}

Cell* parse_atom(const char *tok) {
    //printf("tok = |%s|\n", tok);
    if (!tok) return make_val_err("NULL token", GEN_ERR);
    char err_buf[128] = {0};
    const size_t len = strlen(tok);

    /* null - '() */
    if (strcmp(tok, "'()") == 0) {
        return make_val_nil();
    }

    /* Boolean */
    if (strcmp(tok, "#t") == 0 ||
        strcmp(tok, "#true") == 0) return make_val_bool(1);
    if (strcmp(tok, "#f") == 0 ||
        strcmp(tok, "#false") == 0) return make_val_bool(0);

    /* Character literal */
    if (tok[0] == '#' && tok[1] == '\\') {
        const char* payload = tok + 2;
        const size_t payload_len = len - 2;

        /* Handle the special '#\' -> space case */
        if (payload_len == 0) {
            return make_val_char(' ');
        }

        /* 1. Check for multi-letter named characters and hex literals FIRST. */
        if (payload_len > 1 || payload[0] == 'x') {
            /* Handle (R7RS required) named characters */
            if (strcmp(payload, "space") == 0) return make_val_char(' ');
            if (strcmp(payload, "newline") == 0) return make_val_char('\n');
            if (strcmp(payload, "alarm") == 0) return make_val_char(0x7);
            if (strcmp(payload, "backspace") == 0) return make_val_char(0x8);
            if (strcmp(payload, "delete") == 0) return make_val_char(0x7f);
            if (strcmp(payload, "escape") == 0) return make_val_char(0x1b);
            if (strcmp(payload, "null") == 0) return make_val_char('\0');
            if (strcmp(payload, "return") == 0) return make_val_char(0xd);
            if (strcmp(payload, "tab") == 0) return make_val_char('\t');

            /* Check mapping of implementation-specific named chars */
            const NamedChar* named_char = find_named_char(payload);
            if (named_char) {
                return make_val_char(named_char->codepoint);
            }

            /* Handle hex literals: #\x... */
            if (payload[0] == 'x' && payload_len > 1) {
                char* end_ptr;
                long code = strtol(payload + 1, &end_ptr, 16);

                if (*end_ptr != '\0' || code < 0 || code > 0x10FFFF) {
                    return make_val_err("Invalid Unicode hex value", VALUE_ERR);
                }
                return make_val_char((UChar32)code);
            }
        }

        /* If it wasn't a recognized multi-letter name or hex,
         * THEN treat it as a single character literal. */
        UChar32 code_point;
        int32_t i = 0;

        /* Decode the first code point from the payload */
        U8_NEXT_UNSAFE(payload, i, code_point);

        /* CRITICAL VALIDATION:
         * If 'i' is not equal to the payload length, it means there was
         * more than one character after #\ (e.g., #\ab or #\λa),
         * which is an error according to the R7RS standard. */
        if (i != (int)payload_len) {
            snprintf(err_buf, sizeof(err_buf), "Invalid character literal: '%s%s%s'",
                     ANSI_RED_B, tok, ANSI_RESET);
            return make_val_err(err_buf, SYNTAX_ERR);
        }

        return make_val_char(code_point);
    }

    /* String literal */
    if (tok[0] == '"' && tok[len-1] == '"') {
        const char *str = GC_strndup(tok+1, len-2);
        return make_val_str(str);
    }

    if ((tok[0] == '#' && strchr("bodx", tok[1])) ||  /* #b101, #o666, #d123, #x0ff */
        isdigit(tok[0]) ||                              /*  123,  5/4,  123+23i */
        (tok[0] == '+' && isdigit(tok[1])) ||           /* +123, +5/4, +123+23i */
        (tok[0] == '-' && isdigit(tok[1])) ||           /* -123, -5/4, -123+23i */
        strcmp(tok, "nan.0") == 0 ||
        strcmp(tok, "inf.0") == 0 ||
        strcmp(tok, "-inf.0") == 0 ||
        strcmp(tok, "+inf.0") == 0
        ) {
        int ok = 0; /* error flag */
        const char* num_start = tok;

        /* complex numbers */
        if (tok[len-1] == 'i') {
            char *copy = GC_strdup(tok);
            char *p = copy;

            /* strip trailing 'i' */
            p[len-1] = '\0';

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
                r = make_val_int(0);

                if (strcmp(p, "+") == 0 || strcmp(p, "") == 0) {
                    i = make_val_int(1);
                } else if (strcmp(p, "-") == 0) {
                    i = make_val_int(-1);
                } else {
                    i = parse_atom(p);
                }

            } else {
                /* real ± imag case: "23+10i", "23-10i", "-23+10i", etc. */
                const char sign = *sep;     /* save '+' or '-' */
                *sep = '\0';                /* terminate real part */
                const char *real_str = p;

                /* imaginary part starts right after sep */
                const char *imag_digits = sep + 1;

                /* rebuild full imag string: e.g. "+10" or "-10" */
                char buf[64];
                snprintf(buf, sizeof(buf), "%c%s", sign, imag_digits);

                r = parse_atom(real_str);
                i = parse_atom(buf);
            }
            Cell* result = make_val_complex(r, i);
            return result;
        }
        /* Rational numbers */
        if (strchr(tok, '/')) {
            char *p;
            p = GC_strdup(tok);

            const char *tok1 = strsep(&p, "/");
            const char *tok2 = strsep(&p, "/");

            if (p != NULL) {
                snprintf(err_buf, sizeof(err_buf), "Invalid token: '%s%s%s'",
                    ANSI_RED_B, tok, ANSI_RESET);
                return make_val_err(err_buf, SYNTAX_ERR);
            }

            const long long n = parse_int_checked(tok1, err_buf, sizeof(err_buf), 10, &ok);
            const long long d = parse_int_checked(tok2, err_buf, sizeof(err_buf), 10, &ok);

            if (d == 0) {
                return make_val_err("Cannot have zero-value denominator in rational", VALUE_ERR);
            }
            return make_val_rat(n, d, 1);
        }

        /* Numeric constants with optional base prefix */
        int base = 10;

        if (tok[0] == '#' && len > 2) {
            switch (tok[1]) {
            case 'b': base = 2; num_start = tok + 2; break;
            case 'o': base = 8; num_start = tok + 2; break;
            case 'd': base = 10; num_start = tok + 2; break;
            case 'x': base = 16; num_start = tok + 2; break;
                /* this will never run, but the linter
                 * complains about no default case */
            default: ;
            }
        }

        if (base != 10 || !strchr(tok, '.')) {
            /* Try integer parsing */
            const long long i = parse_int_checked(num_start, err_buf, sizeof(err_buf), base, &ok);
            if (ok) return make_val_int(i);
        }

        /* If no base prefix or decimal point detected, try float */
        if (base == 10) {
            const long double f = parse_float_checked(num_start, err_buf, sizeof(err_buf), &ok);
            if (ok) return make_val_real(f);
        }

        /* If parsing fails but there is a numeric-like string, return error */
        if (!ok) return make_val_err(err_buf, SYNTAX_ERR);
    }

    /* reject all other hash prefixes and single '#' */
    if (tok[0] == '#') {
        snprintf(err_buf, sizeof(err_buf), "Invalid token: '%s%s%s'",
            ANSI_RED_B, tok, ANSI_RESET);
        return make_val_err(err_buf, SYNTAX_ERR);
    }

    /* Otherwise, treat as symbol */
    return make_val_sym(tok);
}

/* Count '(' and ')' while ignoring:
   - anything inside string literals
   - character literals starting with "#\..." (including #\()/#\)),
   - line comments starting with ';' to end-of-line.
*/
int paren_balance(const char *s, int *in_string) {
    int balance = 0;
    int escaped = 0;
    int string = *in_string;  /* carry-over state from previous line */

    for (const char *p = s; *p; p++) {
        if (string) {
            if (escaped) {
                escaped = 0;
            } else if (*p == '\\') {
                escaped = 1;
            } else if (*p == '"') {
                string = 0; /* string closed */
            }
            continue;
        }

        /* not in a string */
        if (*p == '"') {
            string = 1;
            escaped = 0;
        } else if (*p == '#' && *(p+1) == '\\') {
            /* char literal — skip this and next */
            p++;
            if (*p && *(p+1)) p++;
        } else if (*p == '(') {
            balance++;
        } else if (*p == ')') {
            balance--;
        }
    }

    *in_string = string;  /* pass string-state back */
    return balance;
}
