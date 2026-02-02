/*
 * 'src/base-lib/file_lib.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 - 2026 Darren Kirby <darren@dragonbyte.ca>
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
#include <sys/types.h>
#include <time.h>

#ifndef __APPLE__
#include <limits.h>
#include <sys/sysmacros.h>
#endif

#define TIME_SIZE 64


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


static char *filetype(const mode_t st_mode) {
    switch (st_mode & S_IFMT) {
        case S_IFBLK:
            return "block device";
        case S_IFCHR:
            return "character device";
        case S_IFDIR:
            return "directory";
        case S_IFIFO:
            return "FIFO/pipe";
        case S_IFLNK:
            return "symlink";
        case S_IFREG:
            return "regular file";
        case S_IFSOCK:
            return "socket";
        default:
            return "unknown";
    }
}


static char *format_time(const struct timespec *ts) {
    struct tm bdt;
    static char str[TIME_SIZE];

    if (localtime_r(&ts->tv_sec, &bdt) == NULL) {
        return "unknown";
    }

    snprintf(str, TIME_SIZE, "%i-%02i-%02i %02i:%02i:%02i.%09li %s",
        bdt.tm_year + 1900, bdt.tm_mon + 1, bdt.tm_mday,
        bdt.tm_hour, bdt.tm_min, bdt.tm_sec,
        ts->tv_nsec, bdt.tm_zone
    );
    return str;
}


#define FP_SPECIAL 1
/* Include set-user-ID, set-group-ID, and sticky
bit information in returned string */

#define PERM_STR_SIZE sizeof("rwxrwxrwx")

/* Return 'ls -l' style string for file permissions mask, This is from
 * 'The Linux Programming Interface' */
static char *file_perm_str(const mode_t perm) {
    static char str[PERM_STR_SIZE];
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const int flags = 1;
    snprintf(str, PERM_STR_SIZE, "%c%c%c%c%c%c%c%c%c",
    (perm & S_IRUSR) ? 'r' : '-', (perm & S_IWUSR) ? 'w' : '-',
    (perm & S_IXUSR) ?
    (((perm & S_ISUID) & (flags & FP_SPECIAL)) ? 's' : 'x') :
    (((perm & S_ISUID) & (flags & FP_SPECIAL)) ? 'S' : '-'),
    (perm & S_IRGRP) ? 'r' : '-', (perm & S_IWGRP) ? 'w' : '-',
    (perm & S_IXGRP) ?
    (((perm & S_ISGID) & (flags & FP_SPECIAL)) ? 's' : 'x') :
    (((perm & S_ISGID) & (flags & FP_SPECIAL)) ? 'S' : '-'),
    (perm & S_IROTH) ? 'r' : '-', (perm & S_IWOTH) ? 'w' : '-',
    (perm & S_IXOTH) ?
    (((perm & S_ISVTX) & (flags & FP_SPECIAL)) ? 't' : 'x') :
    (((perm & S_ISVTX) & (flags & FP_SPECIAL)) ? 'T' : '-'));
    return str;
}


/*-------------------------------------------------------*
 *        File type and other file/dir predicates        *
 * ------------------------------------------------------*/


static Cell* file_reg_file_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "reg-file?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "reg-file?"))) { return err; }

    const char* filename = a->cell[0]->str;
    const int8_t ft = f_get_type(filename);
    if (ft == F_ERR) {
        return make_cell_error(fmt_err("reg-file?: %s", strerror(errno)),
            OS_ERR);
    }

    if (ft == F_REG)
    {
        return True_Obj;
    }
    return False_Obj;
}


static Cell* file_directory_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "directory?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "directory?"))) { return err; }

    const char* filename = a->cell[0]->str;
    const int8_t ft = f_get_type(filename);
    if (ft == F_ERR) {
        return make_cell_error(fmt_err("directory?: %s", strerror(errno)),
            OS_ERR);
    }

    if (ft == F_DIR)
    {
        return True_Obj;
    }
    return False_Obj;
}


static Cell* file_symlink_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "symlink?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "symlink?"))) { return err; }

    const char* filename = a->cell[0]->str;
    const int8_t ft = f_get_type(filename);
    if (ft == F_ERR) {
        return make_cell_error(fmt_err("symlink?: %s", strerror(errno)),
            OS_ERR);
    }

    if (ft == F_LNK)
    {
        return True_Obj;
    }
    return False_Obj;
}


static Cell* file_char_device_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "char-device?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "char-device?"))) { return err; }

    const char* filename = a->cell[0]->str;
    const int8_t ft = f_get_type(filename);
    if (ft == F_ERR) {
        return make_cell_error(fmt_err("char-device?: %s", strerror(errno)),
            OS_ERR);
    }

    if (ft == F_CHR)
    {
        return True_Obj;
    }
    return False_Obj;
}


static Cell* file_block_device_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "block-device?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "block-device?"))) { return err; }

    const char* filename = a->cell[0]->str;
    const int8_t ft = f_get_type(filename);
    if (ft == F_ERR) {
        return make_cell_error(fmt_err("block-device?: %s", strerror(errno)),
            OS_ERR);
    }

    if (ft == F_BLK)
    {
        return True_Obj;
    }
    return False_Obj;
}


static Cell* file_pipe_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "fifo?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "fifo?"))) { return err; }

    const char* filename = a->cell[0]->str;
    const int8_t ft = f_get_type(filename);
    if (ft == F_ERR) {
        return make_cell_error(fmt_err("fifo?: %s", strerror(errno)),
            OS_ERR);
    }

    if (ft == F_FIFO)
    {
        return True_Obj;
    }
    return False_Obj;
}


static Cell* file_socket_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "socket?");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "socket?"))) { return err; }

    const char* filename = a->cell[0]->str;
    const int8_t ft = f_get_type(filename);
    if (ft == F_ERR) {
        return make_cell_error(fmt_err("socket?: %s", strerror(errno)),
            OS_ERR);
    }

    if (ft == F_SOCK)
    {
        return True_Obj;
    }
    return False_Obj;
}


/*  */
static Cell* file_file_exists_pred(const Lex* e, const Cell* a)
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


static Cell* file_rmdir(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "rmdir!");
    if (err) { return err; }
    err = check_arg_types(a, CELL_STRING, "rmdir!");
    if (err) { return err; }

    const char* path = a->cell[0]->str;

    if (rmdir(path) < 0) {
        return make_cell_error(
            fmt_err("rmdir!: %s", strerror(errno)),
            OS_ERR);
    }
    return True_Obj;
}


/* TODO - mkdir -p style mkdir procedure */
static Cell* file_mkdir(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "mkdir");
    if (err) { return err; }
    err = CHECK_ARITY_EXACT(a, 1, "mkdir");
    if (err) { return err; }

    const char* path = a->cell[0]->str;
    if (mkdir(path, 0755) < 0) {
        return make_cell_error(
            fmt_err("mkdir: %s", strerror(errno)),
            OS_ERR);
    }
    return True_Obj;
}



static Cell* file_unlink_file(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "unlink!");
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1, "unlink!"))) { return err; }

    const char* filename = a->cell[0]->str;
    if (unlink(filename) != 0) {
        return make_cell_error(
            fmt_err("unlink!: %s", strerror(errno)),
            OS_ERR);
    }
    return True_Obj;
}


/*-------------------------------------------------------*
 *                  file stat procedures                 *
 * ------------------------------------------------------*/

/* TODO: change this to return machine-readable time values
 * rather than human-readable. */
static Cell* file_stat(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "stat");
    if (err) { return err; }
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "stat: file path must be passed as a string",
            TYPE_ERR);
    }

    struct stat buf;
    if (stat(a->cell[0]->str, &buf) == -1) {
        return make_cell_error(
            fmt_err("stat: %s", strerror(errno)),
            OS_ERR);
    }

    Cell* result = make_cell_nil();
    int list_len;

#ifndef __linux__  /* Linux stat struct doesn't include birth time. */
    result = make_cell_pair(make_cell_pair(
        make_cell_symbol("st_birthtimespec"),
        make_cell_string(format_time(&buf.st_birthtimespec))), result);
    result->len = 1;
    result = make_cell_pair(make_cell_pair(
        make_cell_symbol("st_ctimespec"),
        make_cell_string(format_time(&buf.st_ctimespec))), result);
    result->len = 2;
    result = make_cell_pair(make_cell_pair(
        make_cell_symbol("st_mtimespec"),
        make_cell_string(format_time(&buf.st_mtimespec))), result);
    result->len = 3;
    result = make_cell_pair(make_cell_pair(
        make_cell_symbol("st_atimespec"),
        make_cell_string(format_time(&buf.st_atimespec))), result);
    result->len = 4;
    list_len = 5;
#else
    result = make_cell_pair(make_cell_pair(
        make_cell_symbol("st_ctime"),
        make_cell_string(format_time(&buf.st_ctim))), result);
    result->len = 1;
    result = make_cell_pair(make_cell_pair(
        make_cell_symbol("st_mtime"),
        make_cell_string(format_time(&buf.st_mtim))), result);
    result->len = 2;
    result = make_cell_pair(make_cell_pair(
        make_cell_symbol("st_atime"),
        make_cell_string(format_time(&buf.st_atim))), result);
    result->len = 3;
    list_len = 4;
#endif
    result = make_cell_pair(make_cell_pair(
    make_cell_symbol("st_gid"),
    make_cell_integer(buf.st_gid)), result);
    result->len = list_len;

    result = make_cell_pair(make_cell_pair(
    make_cell_symbol("st_uid"),
    make_cell_integer(buf.st_uid)), result);
    result->len = ++list_len;

    result = make_cell_pair(make_cell_pair(
        make_cell_symbol("st_mode"),
        make_cell_string(file_perm_str(buf.st_mode))), result);
    result->len = ++list_len;

    result = make_cell_pair(make_cell_pair(
    make_cell_symbol("st_nlink"),
    make_cell_integer(buf.st_nlink)), result);
    result->len = ++list_len;

    result = make_cell_pair(make_cell_pair(
    make_cell_symbol("st_ino"),
    make_cell_integer((long long)buf.st_ino)), result);
    result->len = ++list_len;

    Cell* min = make_cell_integer(minor(buf.st_dev));
    Cell* maj = make_cell_integer(major(buf.st_dev));

    result = make_cell_pair(make_cell_pair(
        make_cell_symbol("st_dev"),
        make_cell_pair(min, maj)), result);
    result->len = ++list_len;

    result = make_cell_pair(make_cell_pair(
    make_cell_symbol("st_blksize"),
    make_cell_integer(buf.st_blksize)), result);
    result->len = ++list_len;

    result = make_cell_pair(make_cell_pair(
    make_cell_symbol("st_blocks"),
    make_cell_integer(buf.st_blocks)), result);
    result->len = ++list_len;

    result = make_cell_pair(make_cell_pair(
    make_cell_symbol("st_size"),
    make_cell_integer(buf.st_size)), result);
    result->len = ++list_len;

    result = make_cell_pair(make_cell_pair(
    make_cell_symbol("type"),
    make_cell_string(filetype(buf.st_mode))), result);
    result->len = ++list_len;

    return result;
}

/* TODO:
 * lstat
 * file-size
 * file-mtime / file-atime / file-ctime
 * file-readable? / file-writable? / file-executable?
 * readlink
 * realpath
 * list-directory
 * rename!
 * copy-file
 * touch!
 * link! / symlink!
 * file-type
 * glob
 * path-absolute? / path-relative?
 * basename / dirname
 */


/* Register the procedures in the environment. */
void cozenage_library_init(const Lex* e)
{
    lex_add_builtin(e, "reg-file?", file_reg_file_pred);
    lex_add_builtin(e, "directory?", file_directory_pred);
    lex_add_builtin(e, "symlink?", file_symlink_pred);
    lex_add_builtin(e, "char-device?", file_char_device_pred);
    lex_add_builtin(e, "block-device?", file_block_device_pred);
    lex_add_builtin(e, "fifo?", file_pipe_pred);
    lex_add_builtin(e, "socket?", file_socket_pred);
    lex_add_builtin(e, "file-exists?", file_file_exists_pred);
    lex_add_builtin(e, "rmdir!", file_rmdir);
    lex_add_builtin(e, "mkdir", file_mkdir);
    lex_add_builtin(e, "unlink!", file_unlink_file);
    lex_add_builtin(e, "stat", file_stat);
}
