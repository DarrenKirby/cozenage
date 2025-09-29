#ifndef COZENAGE_FILE_LIB_H
#define COZENAGE_FILE_LIB_H

#include "environment.h"
#include "types.h"

Cell* builtin_file_exists(Lex* e, Cell* a);
Cell* builtin_delete_file(Lex* e, Cell* a);
Cell* builtin_open_input_file(Lex* e, Cell* a);
Cell* builtin_open_binary_input_file(Lex* e, Cell* a);
void lex_add_file_lib(Lex* e);

#endif //COZENAGE_FILE_LIB_H