#ifndef COZENAGE_LOAD_LIBRARY_H
#define COZENAGE_LOAD_LIBRARY_H

#include "char_lib.h"
#include "complex_lib.h"
#include "coz_ext_lib.h"
#include "file_lib.h"
#include "inexact_lib.h"
#include "process_context_lib.h"

#define MAX_LOADED_LIBS 32

Cell* load_scheme_library(const char* lib_name, Lex* env);

#endif //COZENAGE_LOAD_LIBRARY_H