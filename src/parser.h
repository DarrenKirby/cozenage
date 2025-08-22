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

char **tokenize(const char *input, int *count);
Parser *read_str(const char *input);
Node *read_form(Parser *p);
void free_tokens(char **tokens);
void free_node(Node *node);

#endif //COZENAGE_PARSER_H