#ifndef COZENAGE_PARSER_H
#define COZENAGE_PARSER_H

/* for debug output */
#define DEBUG 0

typedef struct l_val l_val;

typedef struct {
    char **array;
    int position;
    int size;
} Parser;

char **lexer(const char *input, int *count);
void free_tokens(char **tokens, int count);
Parser *parse_str(const char *input);
l_val *parse_form(Parser *p);
l_val *lval_atom_from_token(const char *tok);


#endif //COZENAGE_PARSER_H
