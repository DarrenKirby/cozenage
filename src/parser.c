/* parser.c */

#include "parser.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "main.h"

long long parse_int_checked(const char* str, char* errbuf, size_t errbuf_sz, int base, int* ok) {
    errno = 0;
    char* endptr;
    const long long val = strtoll(str, &endptr, base);

    if (endptr == str) {
        snprintf(errbuf, errbuf_sz, "Invalid numeric: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }
    if (errno == ERANGE || val > LLONG_MAX || val < LLONG_MIN) {
        snprintf(errbuf, errbuf_sz, "Integer out of range: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }
    if (*endptr != '\0') {
        snprintf(errbuf, errbuf_sz, "Invalid trailing characters in numeric: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }

    *ok = 1;
    return val;
}

long double parse_float_checked(const char* str, char* errbuf, size_t errbuf_sz, int* ok) {
    errno = 0;
    char* endptr;
    const long double val = strtold(str, &endptr);

    if (endptr == str) {
        snprintf(errbuf, errbuf_sz, "Invalid numeric: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }
    if (errno == ERANGE) {
        snprintf(errbuf, errbuf_sz, "Float out of range: '%s%s%s'",
            ANSI_RED_B, str, ANSI_RESET);
        *ok = 0; return 0;
    }
    if (*endptr != '\0') {
        snprintf(errbuf, errbuf_sz, "Invalid trailing characters in numeric: '%s%s%s'",
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
    char **tokens = malloc(capacity * sizeof(char *));
    if (!tokens) {
        fprintf(stderr, "ENOMEM: malloc failed\n");
        return NULL;
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
        if (n >= capacity) {
            capacity *= 2;
            char **tmp_tokens = realloc(tokens, capacity * sizeof(char *));
            if (!tmp_tokens) {
                fprintf(stderr, "ENOMEM: realloc failed\n");
                exit(EXIT_FAILURE);
            }
            tokens = tmp_tokens;
        }

        /* quote char: ' */
        if (*p == '\'') {
            tokens[n++] = strdup("'");  /* emit a single-quote token */
            p++;
            continue;
        }

        /* Parentheses as single-char tokens */
        if (*p == '(' || *p == ')') {
            char buf[2] = {*p, '\0'};
            tokens[n++] = strdup(buf);
            p++;
        }
        /* String literal */
        else if (*p == '"') {
            p++; /* skip opening quote */
            const char *start = p;
            size_t buf_size = 64;
            char *tok = malloc(buf_size);
            if (!tok) exit(EXIT_FAILURE);
            size_t len = 0;
            tok[len++] = '"';
            while (*p && *p != '"') {
                if (*p == '\\' && *(p+1)) {
                    if (len + 2 >= buf_size) {
                        buf_size *= 2;
                        tok = realloc(tok, buf_size);
                        if (!tok) exit(EXIT_FAILURE);
                    }
                    tok[len++] = *p++;
                    tok[len++] = *p++;
                } else {
                    if (len + 1 >= buf_size) {
                        buf_size *= 2;
                        tok = realloc(tok, buf_size);
                        if (!tok) exit(EXIT_FAILURE);
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
            p++;  // skip '#'

            /* Consume entire token until whitespace, paren, or quote */
            while (*p && !isspace((unsigned char)*p) &&
                   *p != '(' && *p != ')' && *p != '"') {
                p++;
                   }

            const size_t len = p - start;
            char *tok = malloc(len + 1);
            if (!tok) exit(EXIT_FAILURE);
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
            char *tok = malloc(len + 1);
            memcpy(tok, start, len);
            tok[len] = '\0';
            tokens[n++] = tok;
        }
    }

    /* Null-terminate array */
    tokens[n] = NULL;
    if (count) *count = n;

    /* Debug print */
    if (DEBUG) {
        for (int i = 0; i < n; i++) {
            printf("TOK[%d] = '%s'\n", i, tokens[i]);
        }
    }
    return tokens;
}

/* free_tokens() -> void
 * Destroy an array of tokens.
 * */
void free_tokens(char **tokens, const int count) {
    if (!tokens) return;
    for (int i = 0; i < count; i++) {
        free(tokens[i]);  /* Free each strdup’d string */
    }
    free(tokens); /* Then free the array */
}

/* parse_str() -> Parser
 * Take a raw line, run it through the lexer,
 * then return the tokens in a Parser object.
 * */
Parser *parse_str(const char *input) {;
    int count;
    char **tokens = lexer(input, &count);
    if (!tokens) return NULL;

    Parser *p = malloc(sizeof(Parser));
    if (!p) { free_tokens(tokens, count); return NULL; }

    p->array = tokens;
    p->position = 0;
    p->size = count;

    return p;
}

static const char *peek(Parser *p) {
    if (p->position < p->size) return p->array[p->position];
    return NULL;
}

static const char *advance(Parser *p) {
    if (p->position < p->size) return p->array[p->position++];
    return NULL;
}

l_val *parse_form(Parser *p) {
    int left_count = 0, right_count = 0;

    for (int i = 0; i < p->size; i++) {
        if (strcmp(p->array[i], "(") == 0) left_count++;
        else if (strcmp(p->array[i], ")") == 0) right_count++;
    }

    if (left_count != right_count) {
        return lval_err("Unbalanced parentheses");
    }

    const char *tok = peek(p);
    if (!tok) return NULL;

    /* Handle quote (') */
    if (strcmp(tok, "'") == 0) {
        advance(p);  /* consume ' */
        l_val *quoted = parse_form(p);
        if (!quoted) return lval_err("Expected expression after quote");
        l_val *qexpr = lval_sexpr();
        lval_add(qexpr, lval_sym("quote"));
        lval_add(qexpr, quoted);
        return qexpr;
    }

    /* Handle vector/byte vector literals */
    if (strcmp(tok, "#") == 0) {
        advance(p);  /* consume '#' */
        const char *next = peek(p);
        if (!next) return lval_err("Invalid token: '#'");

        /* plain vector: #( ... ) */
        if (strcmp(next, "(") == 0) {
            advance(p); /* consume '(' */
            l_val *vec = lval_vect();
            while (peek(p) && strcmp(peek(p), ")") != 0) {
                lval_add(vec, parse_form(p));
            }
            if (!peek(p)) return lval_err("Unmatched '(' in vector literal");
            advance(p); /* skip ')' */
            return vec;
        }

        /* byte vector: #u8( ... ) */
        if (strcmp(next, "u8") == 0) {
            advance(p); /* consume 'u8' */
            const char *paren = advance(p); /* must be '(' */
            if (!paren || strcmp(paren, "(") != 0)
                return lval_err("Expected '(' after #u8");
            l_val *bv = lval_bytevect();
            while (peek(p) && strcmp(peek(p), ")") != 0) {
                lval_add(bv, parse_form(p));
            }
            if (!peek(p)) return lval_err("Unmatched '(' in bytevector literal");
            advance(p); /* skip ')' */
            return bv;
        }

        return lval_err("Invalid token");
    }

    /* Handle S-expressions '(' ... ')' */
    if (strcmp(tok, "(") == 0) {
        advance(p);  /* consume '(' */
        l_val *sexpr = lval_sexpr();
        while (peek(p) && strcmp(peek(p), ")") != 0) {
            lval_add(sexpr, parse_form(p));
        }
        if (!peek(p)) return lval_err("Unmatched '('");
        advance(p); /* skip ')' */
        return sexpr;
    }

    /* Otherwise, atom (number, boolean, char, string, symbol) */
    advance(p);
    return lval_atom_from_token(tok);
}

l_val* lval_atom_from_token(const char *tok) {
    if (!tok) return lval_err("NULL token");
    char errbuf[128] = {0};
    const size_t len = strlen(tok);

    /* Boolean */
    if (strcmp(tok, "#t") == 0 ||
        strcmp(tok, "#true") == 0) return lval_bool(1);
    if (strcmp(tok, "#f") == 0 ||
        strcmp(tok, "#false") == 0) return lval_bool(0);

    /* Character literal */
    if (tok[0] == '#' && tok[1] == '\\') {
        /* the oddball case where '#\ ' is considered a 'space'
         * char literal, but the lexer will only return '#\'
         * as the token */
        if (strcmp(tok, "#\\") == 0) {
            return lval_char(' ');
        }
        /* single char in the ascii set */
        if (isascii(tok[2]) && len == 3) {
            return lval_char(tok[2]);
        }
        /* named chars */
        if (strcmp(tok, "#\\alarm") == 0) {
            return lval_char(0x7);
        }
        if (strcmp(tok, "#\\backspace") == 0) {
            return lval_char(0x8);
        }
        if (strcmp(tok, "#\\delete") == 0) {
            return lval_char(0x7f);
        }
        if (strcmp(tok, "#\\escape") == 0) {
            return lval_char(0x1b);
        }
        if (strcmp(tok, "#\\newline") == 0) {
            return lval_char('\n');
        }
        if (strcmp(tok, "#\\null") == 0) {
            return lval_char('\0');
        }
        if (strcmp(tok, "#\\return") == 0) {
            return lval_char(0xd);
        }
        if (strcmp(tok, "#\\space") == 0) {
            return lval_char(' ');
        }
        if (strcmp(tok, "#\\tab") == 0) {
            return lval_char('\t');
        }
        /* valid hex codes: 0x00 to 0x7f */
        if (tok[2] == 'x' && len >= 4) {
            unsigned int code = 0;
            const int items_read = sscanf(tok, "#\\x%x", &code);
            if (items_read != 1) { return lval_err("Invalid token"); }
            if (code > 0x7f) {
                return lval_err("Invalid ASCII hex value");
            }
            return lval_char((char)code);
        }
    }

    /* String literal */
    if (tok[0] == '"' && tok[len-1] == '"') {
        const char *str = strndup(tok+1, len-2);
        return lval_str(str);
    }

    if (tok[0] == '#' && strchr("bodx", tok[1]) || /* #b101, #o666, #d123, #xfff */
        isdigit(tok[0]) ||                           /* 123 */
        tok[0] == '+' && isdigit(tok[1]) ||          /* +123 */
        tok[0] == '-' && isdigit(tok[1])             /* -123 */
        ) {
        /* Numeric constants with optional base prefix */
        int ok = 0;
        const char* numstart = tok;
        int base = 10;

        if (tok[0] == '#' && len > 2) {
            switch (tok[1]) {
            case 'b': base = 2; numstart = tok + 2; break;
            case 'o': base = 8; numstart = tok + 2; break;
            case 'd': base = 10; numstart = tok + 2; break;
            case 'x': base = 16; numstart = tok + 2; break;
                /* this will never run, but the linter
                 * complains about no default case */
            default: ;
            }
        }

        if (base != 10 || !strchr(tok, '.')) {
            /* Try integer parsing */
            const long long i = parse_int_checked(numstart, errbuf, sizeof(errbuf), base, &ok);
            if (ok) return lval_int(i);
        }

        /* If no base prefix or decimal point detected, try float */
        if (base == 10) {
            const long double f = parse_float_checked(numstart, errbuf, sizeof(errbuf), &ok);
            if (ok) return lval_float(f);
        }

        /* If parsing fails but there is a numeric-like string, return error */
        if (!ok) return lval_err(errbuf);
    }

    /* reject all other hash prefixes and single '#' */
    if (tok[0] == '#') {
        snprintf(errbuf, sizeof(errbuf), "Invalid token: '%s%s%s'",
            ANSI_RED_B, tok, ANSI_RESET);
        return lval_err(errbuf);
    }

    /* Otherwise, treat as symbol */
    return lval_sym(tok);
}
