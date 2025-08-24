/* parser.c */

#include "parser.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


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
            tokens[n++] = strdup("'");  // emit a single-quote token
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
            p++; // skip opening quote
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
        /* Hash literals: booleans #t/#f, characters #\x */
        else if (*p == '#') {
            const char *start = p;
            p++;
            if (*p == 't' || *p == 'f') {
                p++;
            } else if (*p == '\\') {
                p++;
                if (*p) p++; // consume single char
            }
            long len = p - start;
            char *tok = malloc(len + 1);
            memcpy(tok, start, len);
            tok[len] = '\0';
            tokens[n++] = tok;
        }
        /* Symbol / number token */
        else {
            const char *start = p;
            while (*p && !isspace((unsigned char)*p) && *p != '(' && *p != ')' && *p != '"') {
                if (*p == ';') break; // comment start ends token
                p++;
            }
            long len = p - start;
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
void free_tokens(char **tokens) {
    if (!tokens) {
        return;
    }
    free(tokens);
}

/* parse_str() -> Parser
 * Take a raw line, run it through the lexer,
 * then return the tokens in a Parser object.
 * */
Parser *parse_str(const char *input) {
    /* Just bail if input is a comment */
    if (input[0] == ';') return NULL;

    int count;
    char **tokens = lexer(input, &count);
    if (!tokens) return NULL;

    Parser *p = malloc(sizeof(Parser));
    if (!p) { free_tokens(tokens); return NULL; }

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
    const char *tok = peek(p);
    if (!tok) return NULL;

    /* Handle quote (') */
    if (strcmp(tok, "'") == 0) {
        advance(p);  // consume '
        l_val *quoted = parse_form(p);
        if (!quoted) return lval_err("Expected expression after quote");
        l_val *qexpr = lval_sexpr();
        lval_add(qexpr, lval_sym("quote"));
        lval_add(qexpr, quoted);
        return qexpr;
    }

    /* Handle vector/byte vector literals */
    if (strcmp(tok, "#") == 0) {
        advance(p);  // consume '#'
        const char *next = peek(p);
        if (!next) return lval_err("Unexpected end after '#'");

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

        return lval_err("Unknown vector literal after '#'");
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

l_val *lval_atom_from_token(const char *tok) {
    if (!tok) return lval_err("NULL token");

    /* boolean */
    if (strcmp(tok, "#t") == 0) return lval_bool(1);
    if (strcmp(tok, "#f") == 0) return lval_bool(0);

    /* character literal */
    if (tok[0] == '#' && tok[1] == '\\') return lval_char(tok[2]);

    /* string literal */
    if (tok[0] == '"' && tok[strlen(tok)-1] == '"') {
        char *str = strndup(tok+1, strlen(tok)-2);
        return lval_str(str);
    }

    /* number (int or float) */
    char *end;
    long long i = strtoll(tok, &end, 10);
    if (*end == '\0') {
        return lval_int(i);
    }
    const long double f = strtold(tok, &end);
    if (*end == '\0') return lval_float(f);

    /* otherwise symbol */
    return lval_sym(tok);
}
