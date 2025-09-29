#include "file_lib.h"
#include "environment.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <gc/gc.h>
#include <sys/syslimits.h>


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
        Cell* f_err = make_val_err(strerror(errno), FILE_ERR);
        f_err->exact = FILE_ERR;
        return f_err;
    }
    return make_val_bool(1);
}

Cell* builtin_open_input_file(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) { return err; }
    err = CHECK_ARITY_EXACT(a, 1);
    if (err) { return err; }

    const char* filename = a->cell[0]->str;
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        return make_val_err(strerror(errno), FILE_ERR);
    }
    char *actual_path = GC_MALLOC(PATH_MAX);
    char *ptr = realpath(filename, actual_path);
    if (ptr == NULL) {
        fclose(fp);
        return make_val_err(strerror(errno), FILE_ERR);
    }

    Cell* p = make_val_port(ptr, fp, INPUT_PORT, TEXT_PORT);
    return p;
}

Cell* builtin_open_binary_input_file(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) { return err; }
    err = CHECK_ARITY_EXACT(a, 1);
    if (err) { return err; }

    const char* filename = a->cell[0]->str;
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        return make_val_err(strerror(errno), FILE_ERR);
    }
    Cell* p = make_val_port(filename, fp, INPUT_PORT, BINARY_PORT);
    return p;
}

void lex_add_file_lib(Lex* e) {
    lex_add_builtin(e, "file-exists?", builtin_file_exists);
    lex_add_builtin(e, "delete-file", builtin_delete_file);
    lex_add_builtin(e, "open-input-file", builtin_open_input_file);
    lex_add_builtin(e, "open-binary-input-file", builtin_open_binary_input_file);
}
