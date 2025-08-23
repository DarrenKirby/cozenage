#ifndef COZENAGE_OPS_H
#define COZENAGE_OPS_H

#include "environment.h"
#include "types.h"


l_val* builtin_add(l_env* e, l_val* a);
l_val* builtin_sub(l_env* e, l_val* a);
l_val* builtin_mul(l_env* e, l_val* a);
l_val* builtin_div(l_env* e, l_val* a);
//l_val* builtin_mod(l_env* e, l_val* a);
//l_val* builtin_pow(l_env* e, l_val* a);

#endif //COZENAGE_OPS_H
