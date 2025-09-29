#ifndef COZENAGE_IO_LIB_H
#define COZENAGE_IO_LIB_H

#include "types.h"


Cell* builtin_display(Lex* e, Cell* a);
void lex_add_write_lib(Lex* e);
void lex_add_read_lib(Lex* e);

#endif //COZENAGE_IO_LIB_H