#include "io_lib.h"
#include "printer.h"


Cell* builtin_display(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2);
    if (err) return err;
    println_cell(a->cell[0]);
    return make_val_bool(1);
}

void lex_add_write_lib(Lex* e) {
    lex_add_builtin(e, "display", builtin_display);
}

