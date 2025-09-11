#include "file_lib.h"
#include "environment.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>


/*
call-with-input-file
open-binary-input-file
open-input-file
with-input-from-file
call-with-output-file
open-binary-output-file
open-output-file
with-output-to-file
*/

Cell* builtin_file_exists(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const char* filename = a->cell[0]->str;
    if (access(filename, F_OK) == 0) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

Cell* builtin_delete_file(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    const char* filename = a->cell[0]->str;
    if (unlink(filename) != 0) {
        Cell* f_err = make_val_err(strerror(errno));
        f_err->exact = FILE_ERR;
        return f_err;
    }
    return make_val_bool(1);
}

void lex_add_file_lib(Lex* e) {
    lex_add_builtin(e, "file-exists?", builtin_file_exists);
    lex_add_builtin(e, "delete-file", builtin_delete_file);
}
