#include "io_lib.h"
#include "printer.h"
#include "ops.h"
#include <string.h>
#include <errno.h>

Cell* builtin_display(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2);
    if (err) return err;

    Cell* port = NULL;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        if (a->cell[1]->type != VAL_PORT) {
            return make_val_err("arg1 must be a port", GEN_ERR);
        }
        port = a->cell[1];
    }
    if (fputs(a->cell[0]->str, port->fh) == EOF) {
        return make_val_err(strerror(errno), FILE_ERR);
    }
    return NULL;
}

void lex_add_write_lib(Lex* e) {
    lex_add_builtin(e, "display", builtin_display);
}

