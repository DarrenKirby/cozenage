
/* types.c */

#include "types.h"
#include "parser.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


double parse_number(const char* str, char* errbuf, size_t errbuf_sz, int* ok) {
    errno = 0;
    char* endptr;
    double val = strtod(str, &endptr);

    if (endptr == str) {
        /* No conversion performed at all */
        snprintf(errbuf, errbuf_sz, "invalid number: '%s'", str);
        *ok = 0;
        return 0.0;
    }

    if (errno == ERANGE) {
        /* Underflow or overflow */
        snprintf(errbuf, errbuf_sz, "number out of range: '%s'", str);
        *ok = 0;
        return 0.0;
    }

    /* Trailing non-numeric chars */
    if (*endptr != '\0') {
        snprintf(errbuf, errbuf_sz, "invalid trailing characters in number: '%s'", str);
        *ok = 0;
        return 0.0;
    }

    *ok = 1;
    return val;
}

l_val* lval_num(const double n) {
    l_val* v = malloc(sizeof(l_val));
    v->type = LVAL_NUM;
    v->num = n;
    return v;
}

l_val* lval_bool(const int b) {
    l_val* v = malloc(sizeof(l_val));
    v->type = LVAL_BOOL;
    v->boolean = b ? 1 : 0;
    return v;
}

l_val* lval_sym(const char* s) {
    l_val* v = malloc(sizeof(l_val));
    v->type = LVAL_SYM;
    v->sym = strdup(s);
    return v;
}

l_val* lval_str(const char* s) {
    l_val* v = malloc(sizeof(l_val));
    v->type = LVAL_STR;
    v->str = strdup(s);
    return v;
}

l_val* lval_sexpr(void) {
    l_val* v = malloc(sizeof(l_val));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

l_val* lval_qexpr(void) {
    l_val* v = malloc(sizeof(l_val));
    v->type = LVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

l_val* lval_err(const char* m) {
    l_val* v = malloc(sizeof(l_val));
    v->type = LVAL_ERR;
    v->str = strdup(m);
    return v;
}

/* Not a type constructor - despite the name
 * this is a helper function used by lval_sexpr
 * and lval_sexpr
 */
l_val* lval_add(l_val* v, l_val* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(l_val*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}

l_val *node_to_lval(Node *node) {
    if (!node) return NULL;
    char err_buf[128];

    switch (node->type) {
        case NODE_ATOM:
            /* Initial pound symbol */
            if (node->atom[0] == '#') {
                if (node->atom[1] == 'f') {  /* Also captures #false */
                    return lval_bool(0);
                }
                if (node->atom[1] == 't') {  /* Also captures #true */
                    return lval_bool(1);
                }
                /* Can add additional forms later (ie: #e #i #b #o #d #x) */
                snprintf(err_buf, sizeof(err_buf), "Unrecognized form: '%s'", node->atom);
                return lval_err(err_buf);
            }
            /* Looks like a number */
            if (isdigit(node->atom[0]) != 0 || node->atom[0] == '-' || node->atom[0] == '+' || node->atom[0] == '.') {
                int ok = 0;
                const double x = parse_number(node->atom, err_buf, sizeof(err_buf), &ok);

                if (!ok) {
                    return lval_err(err_buf);
                }
                return lval_num(x);
            }
            /* A string - note: still need to unescape */
            if (node->atom[0] == '"') {
                const size_t len = strlen(node->atom);

                /* Ensure str is properly terminated */
                if (node->atom[len - 1] != '"') {
                    snprintf(err_buf, sizeof(err_buf), "Unterminated string literal: '%s'", node->atom);
                    return lval_err(err_buf);
                }
                /* empty string or invalid input */
                if (len < 2) {
                    return lval_str("");
                }

                /* Allocate buffer for the inner string + null terminator */
                char* sub_str = malloc(len - 1);
                if (!sub_str) {
                    fprintf(stderr, "ENOMEM: malloc failed\n");
                    exit(EXIT_FAILURE);
                }

                /* Copy everything except the surrounding quotes */
                memcpy(sub_str, node->atom + 1, len - 2);
                sub_str[len-2] = '\0';

                /* Use sub_str in lval_str */
                l_val* s = lval_str(sub_str);
                free(sub_str);
                return s;
            }
            /* If here - treat as a symbol to look up in eval() */
            return lval_sym(node->atom);

        case NODE_LIST: {
            l_val* v = lval_sexpr();
            for (int i = 0; i < node->size; i++) {
                l_val *x = node_to_lval(node->list[i]);
                if (x != NULL) {
                    lval_add(v, x);
                }
            }
            return v;
        }
    }
    /* Should never get here */
    return NULL;
}

void lval_del(l_val* v) {
    switch (v->type) {
        case LVAL_NUM:
        case LVAL_BOOL:
            // nothing heap-allocated here
            break;

        case LVAL_SYM:
            free(v->sym);
            break;

        case LVAL_STR:
            free(v->str);
            break;

        case LVAL_SEXPR:
        case LVAL_QEXPR:
            for (int i = 0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }
            free(v->cell);
            break;

        case LVAL_FUN:
            // if you later add user-defined functions with captured
            // environments, you’d free those here.
            break;
    }
    free(v);
}
