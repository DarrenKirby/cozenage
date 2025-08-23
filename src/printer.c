/* printer.c - print ASTs (Node objects) and l_vals */

#include "printer.h"
#include "parser.h"
#include "types.h"
#include <stdio.h>


/* print_node() -> void
 * Take a Node object and display the AST.
 * For debugging.
 * */
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

/* print_string() ->void
 * Print the Node object as response into the REPL.
 * */
void print_ast_string(const Node *node) {
    if (node->type == NODE_ATOM) {
        printf("%s", node->atom);
    } else {  /* NODE_LIST */
        printf("(");
        for (int i = 0; i < node->size; i++) {
            print_ast_string(node->list[i]);
            if (i < node->size - 1) {
                printf(" ");
            }
        }
        printf(")");
    }
}

/* print_string_ln() -> void
 * Wrapper which prints a newline.
 * */
void print_ast_string_ln(const Node *node) {
    print_ast_string(node);
    printf("\n");
}

void print_lval(const l_val* v) {
    switch (v->type) {
        case LVAL_FLOAT:
            printf("%Lf\n", v->float_n);
            return;
            case LVAL_INT:
            printf("%lld\n", v->int_n);
            return;
        case LVAL_BOOL:
            printf("%s\n", v->boolean == 1 ? "#true" : "#false");
            return;
        case LVAL_ERR:
        case LVAL_STR:
            printf("%s\n", v->str);
            return;
        //case LVAL_STR:
        //    printf("%s\n", v->str);
        //    return;
        case LVAL_SYM:
            printf("%s\n", v->sym);
            return;
        default:
            printf("(unknown type: %d)\n", v->type);
    }
}
