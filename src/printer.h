#ifndef COZENAGE_PRINTER_H
#define COZENAGE_PRINTER_H

#include "parser.h"
#include "environment.h"


void print_cell(const Cell* v);
void println_cell(const Cell* v);
void print_env(const Lex* e);

#endif //COZENAGE_PRINTER_H
