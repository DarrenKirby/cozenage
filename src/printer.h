#ifndef COZENAGE_PRINTER_H
#define COZENAGE_PRINTER_H

#include "parser.h"
#include "types.h"


void print_node(const Node *node, int indent);
void print_ast_string(const Node *node);
void print_ast_string_ln(const Node *node);
void print_lval(const l_val* v);

#endif //COZENAGE_PRINTER_H
