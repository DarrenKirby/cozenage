#ifndef COZENAGE_PRINTER_H
#define COZENAGE_PRINTER_H

#include "parser.h"

void print_node(const Node *node, int indent);
void print_string(const Node *node);
void print_string_ln(const Node *node);

#endif //COZENAGE_PRINTER_H