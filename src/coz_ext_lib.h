#ifndef COZENAGE_COZ_EXT_LIB_H
#define COZENAGE_COZ_EXT_LIB_H

#include "environment.h"

Cell* builtin_print_env(Lex* e, Cell* a);
void lex_add_coz_ext(Lex* e);

#endif //COZENAGE_COZ_EXT_LIB_H
