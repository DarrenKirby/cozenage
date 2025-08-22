/* parser.c */

#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/* tokenize() -> char**
 * Take a line of input and return an array of strings
 * containing all the tokens.
 * */
char **lexer(const char *input, int *count) {

    /* Initialize with room for 8 tokens */
    int capacity = 8;
    char **tokens = malloc(capacity * sizeof(char *));
    if (!tokens) {
        fprintf(stderr, "ENOMEM: malloc failed\n");
        return NULL;
    }

    /* Keep track of the # of tokens we've created */
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

/* free_tokens() -> void
 * Destroy an array of tokens.
 * */
void free_tokens(char **tokens) {
    if (!tokens) {
        return;
    }
    free(tokens);
}

/*
 * free_node() -> void
 * Destroy a Node object.
 * */
void free_node(Node *node) {
    if (!node) return;
    if (node->type == NODE_ATOM) {
        free(node->atom);
    } else {  /* NODE_LIST */
        for (int i = 0; i < node->size; i++) {
            free_node(node->list[i]);
        }
        free(node->list);
    }
    free(node);
}

/* parse_str() -> Parser
 * Take a raw line, run it through the lexer,
 * then return the tokens in a Parser object.
 * */
Parser *parse_str(const char *input) {
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

/* parse_list() -> Node
 * Take a Parser object and return a
 * NODE_LIST type Node.
 * */
Node *parse_list(Parser *p) {
    /* Skip the '(' */
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
        elements[n++] = parse_form(p);
    }

    if (p->position >= p->size) {
        fprintf(stderr, "Error: unmatched '('\n");
        return NULL;
    }

    /* Skip the ')' */
    p->position++;

    Node *node = malloc(sizeof(Node));
    node->type = NODE_LIST;
    node->list = elements;
    node->size = n;
    node->atom = NULL;
    return node;
}

/* parse_atom() -> Node
 * Take a Parser object and return a
 * NODE_ATOM type Node.
 * */
Node *parse_atom(Parser *p) {
    const char *token = p->array[p->position++];
    Node *node = malloc(sizeof(Node));
    if (!node) {
        fprintf(stderr, "ENOMEM: Failed to allocate memory for atom\n");
        exit (EXIT_FAILURE);
    }
    node->type = NODE_ATOM;
    node->atom = strdup(token);
    node->list = NULL;
    node->size = 0;
    return node;
}

/* parse_form() -> Node
 * Take a Parser object and decide which form it is.
 * Dispatch it accordingly.
 * */
Node *parse_form(Parser *p) {
    if (p->position >= p->size) return NULL;

    const char *token = p->array[p->position];
    //debug:
    //printf("read_form: pos=%d token='%s'\n", p->position, token);
    if (strcmp(token, "(") == 0) {
        return parse_list(p);
    }
    if (strcmp(token, ")") == 0) {
        fprintf(stderr, "Error: unexpected ')'\n");
        return NULL;
    }
    return parse_atom(p);
}
