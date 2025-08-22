/* printer.c */

#include "printer.h"
#include "parser.h"
#include <stdio.h>

void print_node(const Node *node, const int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
    if (node->type == NODE_ATOM) {
        printf("%s\n", node->atom);
    } else {
        printf("(\n");
        for (int i = 0; i < node->size; i++) {
            print_node(node->list[i], indent + 1);
        }
        for (int i = 0; i < indent; i++) printf("  ");
        printf(")\n");
    }
}

void print_string(const Node *node) {
    if (node->type == NODE_ATOM) {
        printf("%s", node->atom);
    } else {  // LIST
        printf("(");
        for (int i = 0; i < node->size; i++) {
            print_string(node->list[i]);
            if (i < node->size - 1) {
                printf(" ");
            }
        }
        printf(")");
    }
}

void print_string_ln(const Node *node) {
    print_string(node);
    printf("\n");
}