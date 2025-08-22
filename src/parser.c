/* parser.c */

#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char **tokenize(const char *input, int *count) {
    int capacity = 8;

    char **tokens = malloc(capacity * sizeof(char *));
    if (!tokens) {
        fprintf(stderr, "ENOMEM: malloc failed\n");
        return NULL;
    }

    int n = 0;
    const char *p = input;

    while (*p) {
        /* Skip whitespace */
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;

        /* Allocate more space if needed */
        if (n >= capacity) {
            char **tmp_tokens = NULL;
            capacity *= 2;
            tmp_tokens = realloc(tokens, capacity * sizeof(char *));
            if (!tmp_tokens) {
                fprintf(stderr, "ENOMEM: realloc failed\n");
                exit(EXIT_FAILURE);
            }
            tokens = tmp_tokens;
        }

        if (*p == '(' || *p == ')') {
            /* Single-char token */
            const char buf[2] = {*p, '\0'};
            tokens[n++] = strdup(buf);
            p++;
        } else {
            /* Symbol/number token */
            const char *start = p;
            while (*p && !isspace((unsigned char)*p) && *p != '(' && *p != ')') {
                p++;
            }
            long int len = p - start;
            char *tok = malloc(len + 1);
            memcpy(tok, start, len);
            tok[len] = '\0';
            tokens[n++] = tok;
        }
    }
    //debug
    //for (int i = 0; i < n; i++) {
    //    printf("TOK[%d] = '%s'\n", i, tokens[i]);
    //}

    /* Null-terminate array */
    tokens[n] = NULL;
    if (count) *count = n;
    return tokens;
}

void free_tokens(char **tokens) {
    if (!tokens) {
        return;
    }
    free(tokens);
}

void free_node(Node *node) {
    if (!node) return;
    if (node->type == NODE_ATOM) {
        free(node->atom);
    } else {  // LIST
        for (int i = 0; i < node->size; i++) {
            free_node(node->list[i]);
        }
        free(node->list);
    }
    free(node);
}

Parser *read_str(const char *input) {
    int count;
    char **tokens = tokenize(input, &count);
    if (!tokens) return NULL;

    Parser *p = malloc(sizeof(Parser));
    if (!p) { free_tokens(tokens); return NULL; }

    p->array = tokens;
    p->position = 0;
    p->size = count;

    return p;
}

Node *read_list(Parser *p) {
    // Skip the '('
    p->position++;
    Node **elements = NULL;
    Node **tmp_elements = NULL;
    int capacity = 4;
    int n = 0;

    elements = malloc(capacity * sizeof(Node*));
    if (!elements) {
        fprintf(stderr, "ENOMEM: Failed to allocate memory for list\n");
        return NULL;
    }

    while (p->position < p->size && strcmp(p->array[p->position], ")") != 0) {
        if (n >= capacity) {
            capacity *= 2;
            tmp_elements = realloc(elements, capacity * sizeof(Node*));
            if (!tmp_elements) {
                fprintf(stderr, "ENOMEM: Failed to reallocate memory for list\n");
                exit(EXIT_FAILURE);
            }
            elements = tmp_elements;
        }
        elements[n++] = read_form(p);
    }

    if (p->position >= p->size) {
        fprintf(stderr, "Error: unmatched '('\n");
        exit(1);
    }

    // Skip the ')'
    p->position++;

    Node *node = malloc(sizeof(Node));
    node->type = NODE_LIST;
    node->list = elements;
    node->size = n;
    node->atom = NULL;
    return node;
}

Node *read_atom(Parser *p) {
    const char *token = p->array[p->position++];
    Node *node = malloc(sizeof(Node));
    node->type = NODE_ATOM;
    node->atom = strdup(token);
    node->list = NULL;
    node->size = 0;
    return node;
}

Node *read_form(Parser *p) {
    if (p->position >= p->size) return NULL;

    const char *token = p->array[p->position];
    //debug:
    //printf("read_form: pos=%d token='%s'\n", p->position, token);
    if (strcmp(token, "(") == 0) {
        return read_list(p);
    }
    if (strcmp(token, ")") == 0) {
        fprintf(stderr, "Error: unexpected ')'\n");
        exit(1);
    }
    return read_atom(p);
}
