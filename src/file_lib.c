/*
 * 'src/file_lib.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025  Darren Kirby <darren@dragonbyte.ca>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "file_lib.h"
#include "environment.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <gc/gc.h>
#include <limits.h>


/* TODO:
call-with-input-file
with-input-from-file
call-with-output-file
with-output-to-file
*/

/* 'file-exists?' -> VAL_BOOL - file exists predicate */
Cell* builtin_file_exists(const Lex* e, const Cell* a) {
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

/* 'delete-file -> VAL_BOOL - delete a file, and return a bool confirming outcome */
Cell* builtin_delete_file(const Lex* e, const Cell* a) {
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

/* 'open-input-file' -> VAL_PORT - open a file and bind it to a text port */
Cell* builtin_open_input_file(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) { return err; }
    err = CHECK_ARITY_EXACT(a, 1);
    if (err) { return err; }

    const char* filename = a->cell[0]->str;
    FILE *fp = fopen(filename, "r");
    if (!fp) {
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

/* 'open-binary-input-file' -> VAL_PORT - open a file and bind it to a binary port */
Cell* builtin_open_binary_input_file(const Lex* e, const Cell* a) {
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

Cell* builtin_open_output_file(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_STR);
    if (err) { return err; }
    err = CHECK_ARITY_RANGE(a, 1, 2);
    if (err) { return err; }

    char *mode = "w";
    const char* filename = a->cell[0]->str;
    if (a->count == 2 && a->cell[1]->type == VAL_STR) {
        mode = a->cell[1]->str;
    }
    FILE *fp = fopen(filename, mode);
    if (!fp) {
        return make_val_err(strerror(errno), FILE_ERR);
    }
    char *actual_path = GC_MALLOC(PATH_MAX);
    char *ptr = realpath(filename, actual_path);
    if (ptr == NULL) {
        fclose(fp);
        return make_val_err(strerror(errno), FILE_ERR);
    }

    Cell* p = make_val_port(filename, fp, OUTPUT_PORT, TEXT_PORT);
    return p;
}

Cell* builtin_open_binary_output_file(const Lex* e, const Cell* a) {
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
    Cell* p = make_val_port(filename, fp, OUTPUT_PORT, BINARY_PORT);
    return p;
}

/* Register the procedures in the environment */
void lex_add_file_lib(Lex* e) {
    lex_add_builtin(e, "file-exists?", builtin_file_exists);
    lex_add_builtin(e, "delete-file", builtin_delete_file);
    lex_add_builtin(e, "open-input-file", builtin_open_input_file);
    lex_add_builtin(e, "open-binary-input-file", builtin_open_binary_input_file);
    lex_add_builtin(e, "open-output-file", builtin_open_output_file);
    lex_add_builtin(e, "open-binary-output-file", builtin_open_binary_output_file);
}
