#ifndef COZENAGE_PROCESS_CONTEXT_LIB_H
#define COZENAGE_PROCESS_CONTEXT_LIB_H

#include "types.h"

Cell* builtin_command_line(Lex* e, Cell* a);
Cell* builtin_exit(Lex* e, Cell* a);
Cell* builtin_emergency_exit(Lex* e, Cell* a);
Cell* builtin_get_env_var(Lex* e, Cell* a);
Cell* builtin_get_env_vars(Lex* e, Cell* a);
void lex_add_proc_con_lib(Lex* e);

#endif //COZENAGE_PROCESS_CONTEXT_LIB_H