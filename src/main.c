/* main.c */

#include "compat_readline.h"
#include "main.h"
#include "parser.h"
#include "printer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* read()
 * Take a line of input from a prompt and pass it
 * through a 2 step lexer/parser stream, then return
 * an AST in the form of nested Node objects.
 * */
Node * read(void) {
    char *input = readline(PROMPT_STRING);

    /* ctrl-d or exit to quit */
    if (input == NULL || strcmp(input, "exit") == 0) {
        printf("\n");
        exit(0);
    }

    Parser *p = parse_str(input);
    if (!p) {
        return NULL;
    }

    Node *ast = parse_form(p);
    if (!ast) {
        return NULL;
    }

    /* Only save input to history if it parses without error */
    add_history(input);
    free(input);
    free_tokens(p->array);
    free(p);

    return ast;
}

/* eval()
 * Take an AST Node and evaluate it in an environment,
 * producing a value.
 * */
Node *eval(Node *node) {
    if (!node) {
        return NULL;
    }
    return node;  // pass-through for now
}

/* print()
 * Take the value produced by eval and print it in a
 * context-specific, meaningful way.
 * */
void print(const Node *node) {
    print_node(node, 0);
    printf("\n");
    print_string_ln(node);
}

/* repl()
 * Read-Evaluate-Print loop
 * */
void repl() {
    for (;;) {
        Node *node = read();
        if (!node) {
            continue;
        }
        Node *result = eval(node);
        if (!result) {
            continue;
        }
        print(result);
        free_node(result);
    }
}

int main(int argc, char** argv) {
    /* Print Version and Exit Information */
    printf("  %s%s%s Version %s\n", ANSI_BLUE_B, APP_NAME, ANSI_RESET, APP_VERSION);
    printf("  Press <Ctrl+d> to exit\n\n");

    /* Run until we don't */
    repl();
    return 0;
}
