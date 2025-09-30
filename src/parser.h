#ifndef COZENAGE_PARSER_H
#define COZENAGE_PARSER_H

/* for debug output */
#define DEBUG 1

typedef struct Cell Cell;

typedef struct {
    char **array;
    int position;
    int size;
} Parser;

char **lexer(const char *input, int *count);
void free_tokens(char **tokens, int count);
Parser *parse_str(const char *input);
Cell *parse_tokens(Parser *p);
Cell *parse_atom(const char *tok);
int paren_balance(const char *s, int *in_string);

#endif //COZENAGE_PARSER_H
