#ifndef COZENAGE_COMPLEX_LIB_H
#define COZENAGE_COMPLEX_LIB_H

#include "types.h"


Cell* builtin_real_part(Lex* e, Cell* a);
Cell* builtin_imag_part(Lex* e, Cell* a);
Cell* builtin_make_rectangular(Lex* e, Cell* a);
void lex_add_complex_lib(Lex* e);

#endif //COZENAGE_COMPLEX_LIB_H