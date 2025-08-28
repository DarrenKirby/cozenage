/* parser.c */

#include "main.h"
#include "parser.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>


long long parse_int_checked(const char* str, char* err_buf, size_t err_buf_sz, const int base, int* ok) {
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

long double parse_float_checked(const char* str, char* err_buf, size_t err_buf_sz, int* ok) {
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

Cell *parse_tokens(Parser *p) {
    int left_count = 0, right_count = 0;

    for (int i = 0; i < p->size; i++) {
        if (strcmp(p->array[i], "(") == 0) left_count++;
        else if (strcmp(p->array[i], ")") == 0) right_count++;
    }

    if (left_count != right_count) {
        return make_val_err("Unbalanced parentheses");
    }

    const char *tok = peek(p);
    if (!tok) return NULL;

    /* Handle quote (') */
    if (strcmp(tok, "'") == 0) {
        advance(p);  /* consume ' */
        Cell *quoted = parse_tokens(p);
        if (!quoted) return make_val_err("Expected expression after quote");
        Cell *qexpr = make_val_sexpr();
        cell_add(qexpr, make_val_sym("quote"));
        cell_add(qexpr, quoted);
        return qexpr;
    }

    /* Handle vector/byte vector literals */
    if (strcmp(tok, "#") == 0) {
        advance(p);  /* consume '#' */
        const char *next = peek(p);
        if (!next) return make_val_err("Invalid token: '#'");

        /* plain vector: #( ... ) */
        if (strcmp(next, "(") == 0) {
            advance(p); /* consume '(' */
            Cell *vec = make_val_vect();
            while (peek(p) && strcmp(peek(p), ")") != 0) {
                cell_add(vec, parse_tokens(p));
            }
            if (!peek(p)) return make_val_err("Unmatched '(' in vector literal");
            advance(p); /* skip ')' */
            return vec;
        }

        /* byte vector: #u8( ... ) */
        if (strcmp(next, "u8") == 0) {
            advance(p); /* consume 'u8' */
            const char *paren = advance(p); /* must be '(' */
            if (!paren || strcmp(paren, "(") != 0)
                return make_val_err("Expected '(' after #u8");
            Cell *bv = make_val_bytevec();
            while (peek(p) && strcmp(peek(p), ")") != 0) {
                cell_add(bv, parse_tokens(p));
            }
            if (!peek(p)) return make_val_err("Unmatched '(' in bytevector literal");
            advance(p); /* skip ')' */
            return bv;
        }
        return make_val_err("Invalid token");
    }

    /* Handle S-expressions '(' ... ')' */
    if (strcmp(tok, "(") == 0) {
        advance(p);  /* consume '(' */
        Cell *sexpr = make_val_sexpr();
        while (peek(p) && strcmp(peek(p), ")") != 0) {
            cell_add(sexpr, parse_tokens(p));
        }
        if (!peek(p)) return make_val_err("Unmatched '('");
        advance(p); /* skip ')' */
        return sexpr;
    }

    /* Otherwise, atom (number, boolean, char, string, symbol) */
    advance(p);
    return parse_atom(tok);
}

Cell* parse_atom(const char *tok) {
    if (!tok) return make_val_err("NULL token");
    char err_buf[128] = {0};
    const size_t len = strlen(tok);

    /* Boolean */
    if (strcmp(tok, "#t") == 0 ||
        strcmp(tok, "#true") == 0) return make_val_bool(1);
    if (strcmp(tok, "#f") == 0 ||
        strcmp(tok, "#false") == 0) return make_val_bool(0);

    /* Character literal */
    if (tok[0] == '#' && tok[1] == '\\') {
        /* the oddball case where '#\ ' is considered a 'space'
         * char literal, but the lexer will only return '#\'
         * as the token */
        if (strcmp(tok, "#\\") == 0) {
            return make_val_char(' ');
        }
        /* single char in the ascii set */
        if (isascii(tok[2]) && len == 3) {
            return make_val_char(tok[2]);
        }
        /* named chars */
        if (strcmp(tok, "#\\alarm") == 0) {
            return make_val_char(0x7);
        }
        if (strcmp(tok, "#\\backspace") == 0) {
            return make_val_char(0x8);
        }
        if (strcmp(tok, "#\\delete") == 0) {
            return make_val_char(0x7f);
        }
        if (strcmp(tok, "#\\escape") == 0) {
            return make_val_char(0x1b);
        }
        if (strcmp(tok, "#\\newline") == 0) {
            return make_val_char('\n');
        }
        if (strcmp(tok, "#\\null") == 0) {
            return make_val_char('\0');
        }
        if (strcmp(tok, "#\\return") == 0) {
            return make_val_char(0xd);
        }
        if (strcmp(tok, "#\\space") == 0) {
            return make_val_char(' ');
        }
        if (strcmp(tok, "#\\tab") == 0) {
            return make_val_char('\t');
        }
        /* valid hex codes: 0x00 to 0x7f */
        if (tok[2] == 'x' && len >= 4) {
            unsigned int code = 0;
            const int items_read = sscanf(tok, "#\\x%x", &code);
            if (items_read != 1) { return make_val_err("Invalid token"); }
            if (code > 0x7f) {
                return make_val_err("Invalid ASCII hex value");
            }
            return make_val_char((char)code);
        }
    }

    /* String literal */
    if (tok[0] == '"' && tok[len-1] == '"') {
        const char *str = strndup(tok+1, len-2);
        return make_val_str(str);
    }

    if (tok[0] == '#' && strchr("bodx", tok[1]) || /* #b101, #o666, #d123, #xfff */
        isdigit(tok[0]) ||                           /* 123, 5/4 */
        tok[0] == '+' && isdigit(tok[1]) ||          /* +123 */
        tok[0] == '-' && isdigit(tok[1])             /* -123, -5/4 */
        ) {
        int ok = 0; /* error flag */

        /* Rational numbers */
        if (strchr(tok, '/')) {
            char *p, *to_free;
            p = to_free = strdup(tok);

            const char *tok1 = strsep(&p, "/");
            const char *tok2 = strsep(&p, "/");

            if (p != NULL) {
                snprintf(err_buf, sizeof(err_buf), "Invalid token: '%s%s%s'",
                    ANSI_RED_B, tok, ANSI_RESET);
                return make_val_err(err_buf);
            }

            const long long n = parse_int_checked(tok1, err_buf, sizeof(err_buf), 10, &ok);
            const long long d = parse_int_checked(tok2, err_buf, sizeof(err_buf), 10, &ok);

            free(to_free);
            return make_val_rat(n, d);
        }

        /* Numeric constants with optional base prefix */
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
            const long long i = parse_int_checked(numstart, err_buf, sizeof(err_buf), base, &ok);
            if (ok) return make_val_int(i);
        }

        /* If no base prefix or decimal point detected, try float */
        if (base == 10) {
            const long double f = parse_float_checked(numstart, err_buf, sizeof(err_buf), &ok);
            if (ok) return make_val_real(f);
        }

        /* If parsing fails but there is a numeric-like string, return error */
        if (!ok) return make_val_err(err_buf);
    }

    /* reject all other hash prefixes and single '#' */
    if (tok[0] == '#') {
        snprintf(err_buf, sizeof(err_buf), "Invalid token: '%s%s%s'",
            ANSI_RED_B, tok, ANSI_RESET);
        return make_val_err(err_buf);
    }

    /* Otherwise, treat as symbol */
    return make_val_sym(tok);
}

/* Count '(' and ')' while ignoring:
   - anything inside string literals (with \" escapes),
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
