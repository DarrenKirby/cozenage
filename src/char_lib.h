#ifndef COZENAGE_CHAR_LIB_H
#define COZENAGE_CHAR_LIB_H

#include "types.h"

Cell* builtin_char_alphabetic(Lex* e, Cell* a);
Cell* builtin_char_whitespace(Lex* e, Cell* a);
Cell* builtin_char_numeric(Lex* e, Cell* a);
Cell* builtin_char_upper_case(Lex* e, Cell* a);
Cell* builtin_char_lower_case(Lex* e, Cell* a);
Cell* builtin_char_upcase(Lex* e, Cell* a);
Cell* builtin_char_downcase(Lex* e, Cell* a);
Cell* builtin_char_foldcase(Lex* e, Cell* a);
Cell* builtin_digit_value(Lex* e, Cell* a);
Cell* builtin_char_equal_ci(Lex* e, Cell* a);
Cell* builtin_char_lt_ci(Lex* e, Cell* a);
Cell* builtin_char_lte_ci(Lex* e, Cell* a);
Cell* builtin_char_gt_ci(Lex* e, Cell* a);
Cell* builtin_char_gte_ci(Lex* e, Cell* a);
Cell* builtin_string_downcase(Lex* e, Cell* a);
Cell* builtin_string_upcase(Lex* e, Cell* a);
Cell* builtin_string_foldcase(Lex* e, Cell* a);
void lex_add_char_lib(Lex* e);

#endif //COZENAGE_CHAR_LIB_H