#ifndef COZENAGE_EVAL_H
#define COZENAGE_EVAL_H

#include "types.h"
#include "environment.h"

Cell* apply_lambda(Cell* lambda, Cell* args);
Cell* coz_eval(Lex* e, Cell* v);
Cell* eval_sexpr(Lex* e, Cell* v);

#endif //COZENAGE_EVAL_H