#ifndef COZENAGE_INEXACT_LIB_H
#define COZENAGE_INEXACT_LIB_H

#include "types.h"


Cell* builtin_cos(Lex* e, Cell* a);
Cell* builtin_acos(Lex* e, Cell* a);
Cell* builtin_sin(Lex* e, Cell* a);
Cell* builtin_asin(Lex* e, Cell* a);
Cell* builtin_tan(Lex* e, Cell* a);
Cell* builtin_atan(Lex* e, Cell* a);
Cell* builtin_exp(Lex* e, Cell* a);
Cell* builtin_log(Lex* e, Cell* a);
Cell* builtin_log2(Lex* e, Cell* a);
Cell* builtin_log10(Lex* e, Cell* a);
Cell* builtin_sqrt(Lex* e, Cell* a);
Cell* builtin_cbrt(Lex* e, Cell* a);
void lex_add_inexact_lib(Lex* e);

#endif //COZENAGE_INEXACT_LIB_H