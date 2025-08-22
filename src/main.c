
#include "compat_readline.h"
#include "main.h"
#include "parser.h"
#include "printer.h"
#include <stdio.h>
#include <stdlib.h>

Node * read(void) {
    char *input = readline(PROMPT_STRING);
    add_history(input);
    /* ctrl-d - exit */
    if (input == NULL) {
        printf("\n");
        exit(0);
    }
    Parser *p = read_str(input);
    free(input);

    Node *ast = read_form(p);

    free_tokens(p->array);
    free(p);

    return ast;
}

Node *eval(Node *node) {
    return node;  // pass-through for now
}

void print(const Node *node) {
    //print_node(node, 0);
    //printf("\n");
    print_string_ln(node);
}

void repl() {
    while (1) {
        Node *node = read();
        Node *result = eval(node);
        print(result);
        free_node(result);
    }
}

int main(int argc, char** argv) {
    /* Print Version and Exit Information */
    printf("  %s%s%s Version %s\n", ANSI_BLUE_B, APP_NAME, ANSI_RESET, APP_VERSION);
    printf("  Press <Ctrl+d> to exit\n\n");

    repl();
    return 0;
}
