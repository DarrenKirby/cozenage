#ifndef COZENAGE_PARSER_H
#define COZENAGE_PARSER_H

typedef struct {
    char **array;
    int position;
    int size;
} Parser;

typedef enum { NODE_ATOM = 1, NODE_LIST = 2 } NodeType;

typedef struct Node {
    NodeType type;
    char *atom;           // if ATOM
    struct Node **list;   // if LIST
    int size;             // number of elements in list
} Node;

char **lexer(const char *input, int *count);
void free_tokens(char **tokens);
void free_node(Node *node);
Parser *parse_str(const char *input);
Node *parse_list(Parser *p);
Node *parse_atom(Parser *p);
Node *parse_form(Parser *p);

#endif //COZENAGE_PARSER_H
