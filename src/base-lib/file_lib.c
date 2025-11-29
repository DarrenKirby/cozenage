/*
 * 'src/base-lib/file_lib.c'
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

#include "types.h"
#include "cell.h"

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef __APPLE__
#include <sys/syslimits.h>
#else
#include <limits.h>
#endif

/*-------------------------------------------------------*
 *         Local helpers for file/dir procedures         *
 * ------------------------------------------------------*/

typedef enum : uint8_t {
    F_REG,
    F_DIR,
    F_CHR,
    F_BLK,
    F_FIFO,
    F_LNK,
    F_SOCK,
    F_UK,     /* Unknown file type. */
    F_ERR     /* stat call error. */
} f_type;

static f_type f_get_type(const char* file)
{
    struct stat buf;
    if (lstat(file, &buf) < 0) {
        /* Signal error to caller. */
        return F_ERR;
    }
    if (S_ISREG(buf.st_mode)) return F_REG;
    if (S_ISDIR(buf.st_mode)) return F_DIR;
    if (S_ISLNK(buf.st_mode)) return F_LNK;
    if (S_ISCHR(buf.st_mode)) return F_CHR;
    if (S_ISBLK(buf.st_mode)) return F_BLK;
    if (S_ISFIFO(buf.st_mode)) return F_FIFO;
    if (S_ISSOCK(buf.st_mode)) return F_SOCK;

    return F_UK;
}


/*-------------------------------------------------------*
 *        File type and other file/dir predicates        *
 * ------------------------------------------------------*/

static Cell* reg_file_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "reg-file?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "reg-file?"))) { return err; }

    const char* filename = a->cell[0]->str;
    const int8_t ft = f_get_type(filename);
    if (ft == F_ERR) {
        return make_cell_error(strerror(errno),
            FILE_ERR);
    }

    if (ft == F_REG)
    {
        return True_Obj;
    }
    return False_Obj;
}

static Cell* directory_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "directory?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "directory?"))) { return err; }

    const char* filename = a->cell[0]->str;
    const int8_t ft = f_get_type(filename);
    if (ft == F_ERR) {
        return make_cell_error(strerror(errno),
            FILE_ERR);
    }

    if (ft == F_DIR)
    {
        return True_Obj;
    }
    return False_Obj;
}

static Cell* symlink_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "symlink?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "symlink?"))) { return err; }

    const char* filename = a->cell[0]->str;
    const int8_t ft = f_get_type(filename);
    if (ft == F_ERR) {
        return make_cell_error(strerror(errno),
            FILE_ERR);
    }

    if (ft == F_LNK)
    {
        return True_Obj;
    }
    return False_Obj;
}

static Cell* char_device_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "char-device?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "char-device?"))) { return err; }

    const char* filename = a->cell[0]->str;
    const int8_t ft = f_get_type(filename);
    if (ft == F_ERR) {
        return make_cell_error(strerror(errno),
            FILE_ERR);
    }

    if (ft == F_CHR)
    {
        return True_Obj;
    }
    return False_Obj;
}

static Cell* block_device_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "blk-device?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "blk-pred?"))) { return err; }

    const char* filename = a->cell[0]->str;
    const int8_t ft = f_get_type(filename);
    if (ft == F_ERR) {
        return make_cell_error(strerror(errno),
            FILE_ERR);
    }

    if (ft == F_BLK)
    {
        return True_Obj;
    }
    return False_Obj;
}

static Cell* pipe_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "fifo?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "fifo?"))) { return err; }

    const char* filename = a->cell[0]->str;
    const int8_t ft = f_get_type(filename);
    if (ft == F_ERR) {
        return make_cell_error(strerror(errno),
            FILE_ERR);
    }

    if (ft == F_FIFO)
    {
        return True_Obj;
    }
    return False_Obj;
}

static Cell* socket_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "socket?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "socket?"))) { return err; }

    const char* filename = a->cell[0]->str;
    const int8_t ft = f_get_type(filename);
    if (ft == F_ERR) {
        return make_cell_error(strerror(errno),
            FILE_ERR);
    }

    if (ft == F_SOCK)
    {
        return True_Obj;
    }
    return False_Obj;
}

/* 'file-exists?' -> CELL_BOOLEAN - file exists predicate */
static Cell* file_exists_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "file-exists?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "file-exists?"))) { return err; }

    const char* filename = a->cell[0]->str;
    if (access(filename, F_OK) == 0) {
        return True_Obj;
    }
    return False_Obj;
}

/*-------------------------------------------------------*
 *            Basic file operation procedures            *
 * ------------------------------------------------------*/

static Cell* get_cwd(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "getcwd");
    if (err) { return err; }

    char buf[PATH_MAX];
    if (getcwd(buf, PATH_MAX) == nullptr) {
        snprintf(buf, sizeof(buf), "getcwd: %s", strerror(errno));
        return make_cell_error(buf, FILE_ERR);
    }
    Cell* result = make_cell_string(buf);
    return result;
}

static Cell* rmdir__(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "rmdir");
    if (err) { return err; }
    err = check_arg_types(a, CELL_STRING, "rmdir");
    if (err) { return err; }

    const char* path = a->cell[0]->str;

    if (rmdir(path) < 0) {
        char buf[256];
        snprintf(buf, sizeof(buf), "rmdir: %s", strerror(errno));
        return make_cell_error(buf, FILE_ERR);
    }
    return True_Obj;
}

/* TODO - mkdir -p style mkdir procedure */
static Cell* mkdir__(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "mkdir");
    if (err) { return err; }
    err = CHECK_ARITY_EXACT(a, 1, "mkdir");
    if (err) { return err; }

    const char* path = a->cell[0]->str;
    if (mkdir(path, 0755) < 0) {
        char buf[256];
        snprintf(buf, sizeof(buf), "mkdir: %s", strerror(errno));
        return make_cell_error(buf, FILE_ERR);
    }
    return True_Obj;
}

/* 'delete-file -> CELL_BOOLEAN - delete a file, and return a bool confirming outcome. */
static Cell* unlink_file(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "unlink?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "unlink"))) { return err; }

    const char* filename = a->cell[0]->str;
    if (unlink(filename) != 0) {
        Cell* f_err = make_cell_error(strerror(errno), FILE_ERR);
        return f_err;
    }
    return True_Obj;
}

/* Register the procedures in the environment. */
void cozenage_library_init(const Lex* e)
{
    lex_add_builtin(e, "reg-file?", reg_file_pred);
    lex_add_builtin(e, "directory?", directory_pred);
    lex_add_builtin(e, "symlink?", symlink_pred);
    lex_add_builtin(e, "char-device?", char_device_pred);
    lex_add_builtin(e, "block-device?", block_device_pred);
    lex_add_builtin(e, "fifo?", pipe_pred);
    lex_add_builtin(e, "socket?", socket_pred);
    lex_add_builtin(e, "file-exists?", file_exists_pred);
    lex_add_builtin(e, "getcwd", get_cwd);
    lex_add_builtin(e, "rmdir", rmdir__); /* The odd name here is because of clash with rmdir C function */
    lex_add_builtin(e, "mkdir", mkdir__); /* ibid */
    lex_add_builtin(e, "unlink!", unlink_file);
}
