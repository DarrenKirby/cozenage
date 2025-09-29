#include "coz_ext_lib.h"
#include "environment.h"
#include "ops.h"
#include "printer.h"
#include "types.h"

Cell* builtin_print_env(Lex* e, Cell* a) {
    (void)a;
    print_env(e);
    return make_val_bool(1);
}


void lex_add_coz_ext(Lex* e) {
    lex_add_builtin(e, "print-env", builtin_print_env);
    lex_add_builtin(e, "^", builtin_expt); /* non-standard alias for expt */
    lex_add_builtin(e, "%", builtin_modulo); /* non-standard alias for modulo */
}
