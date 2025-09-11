#include "coz_ext_lib.h"
#include "environment.h"
#include "ops.h"


void lex_add_coz_ext(Lex* e) {
    lex_add_builtin(e, "^", builtin_expt); /* non-standard alias for expt */
    lex_add_builtin(e, "%", builtin_modulo); /* non-standard alias for modulo */
}
