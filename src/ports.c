/*
 * 'ports.c'
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

#include "ports.h"
#include "types.h"
#include "strings.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wchar.h>
#include <sys/select.h>


/*-------------------------------------------------------*
 *                Input/output and ports                 *
 * ------------------------------------------------------*/

Cell* builtin_current_input_port(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    return default_input_port;
}

Cell* builtin_current_output_port(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    return default_output_port;
}

Cell* builtin_current_error_port(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    return default_error_port;
}

Cell* builtin_input_port_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_PORT || a->cell[0]->port->port_t != INPUT_PORT) {
        return make_cell_boolean(0);
    }
    return make_cell_boolean(1);
}

Cell* builtin_output_port_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_PORT || a->cell[0]->port->port_t != OUTPUT_PORT) {
        return make_cell_boolean(0);
    }
    return make_cell_boolean(1);
}

Cell* builtin_text_port_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_PORT || a->cell[0]->port->stream_t != TEXT_PORT) {
        return make_cell_boolean(0);
    }
    return make_cell_boolean(1);
}

Cell* builtin_binary_port_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_PORT || a->cell[0]->port->stream_t != BINARY_PORT) {
        return make_cell_boolean(0);
    }
    return make_cell_boolean(1);
}

Cell* builtin_input_port_open(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type == CELL_PORT ||
        a->cell[0]->port->port_t != INPUT_PORT ||
        a->cell[0]->is_open == true) {
        return make_cell_boolean(1);
        }
    return make_cell_boolean(0);
}

Cell* builtin_output_port_open(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type == CELL_PORT ||
        a->cell[0]->port->port_t != OUTPUT_PORT ||
        a->cell[0]->is_open == true) {
        return make_cell_boolean(1);
    }
    return make_cell_boolean(0);
}

Cell* builtin_close_port(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_PORT) {
        return make_cell_error(
            "arg1 is not a port",
            TYPE_ERR);
    }

    if (a->cell[0]->is_open == 1) {
        fflush(a->cell[0]->port->fh);
        fclose(a->cell[0]->port->fh);
        a->cell[0]->is_open = 0;
    }
    return make_cell_boolean(1);
}

Cell* builtin_read_line(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_PORT);
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_input_port(e, a);
    } else {
        port = a->cell[0];
    }

    if (port->is_open == 0 || port->port->port_t != INPUT_PORT)
        return make_cell_error(
            "port is not open for input",
            GEN_ERR);

    char *line = nullptr;
    size_t n = 0;
    const ssize_t len = getline(&line, &n, port->port->fh);
    if (len <= 0) { free(line); return make_cell_eof(); }
    /* remove newline if present */
    if (line[len-1] == '\n') line[len-1] = '\0';

    Cell* result = make_cell_string(line);
    free(line);
    return result;
}

Cell* builtin_read_char(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_PORT);
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_input_port(e, a);
    } else {
        port = a->cell[0];
    }

    if (port->is_open == 0 || port->port->port_t != INPUT_PORT)
        return make_cell_error(
            "port is not open for input",
            GEN_ERR);

    /* TODO: need to use feof() to tell EOF from error? */
    const int c = fgetwc(port->port->fh);
    if (c == EOF) {
        return make_cell_eof();
    }
    return make_cell_char(c);
}

Cell* builtin_peek_char(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_PORT);
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_input_port(e, a);
    } else {
        port = a->cell[0];
    }

    if (port->is_open == 0 || port->port->port_t != INPUT_PORT)
        return make_cell_error(
            "port is not open for input",
            GEN_ERR);

    /* TODO: need to use feof() to tell EOF from error? */
    const int c = fgetwc(port->port->fh);
    const int pb_c = ungetwc(c, port->port->fh);
    if (c == EOF) {
        return make_cell_eof();
    }
    if (pb_c == EOF) {
       return make_cell_error(
           "char pushback failed!",
           FILE_ERR);
    }
    return make_cell_char(c);
}

Cell* builtin_write_char(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_CHAR) {
        return make_cell_error(
            "arg1 must be a char",
            TYPE_ERR);
    }
    if (a->count == 2) {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error(
                "arg2 must be a port",
                TYPE_ERR);
        }
    }
    Cell* port;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        port = a->cell[1];
    }

    if (fputwc(a->cell[0]->char_v, port->port->fh) == EOF) {
        char buf[256];
        snprintf(buf, sizeof(buf), "write-char failed: %s", strerror(errno));
        return make_cell_error(buf, FILE_ERR);
    }
    return nullptr;
}

Cell* builtin_write_string(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 1, 4);
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error("arg1 must be a string", TYPE_ERR);
    }
    if (a->count >= 2) {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error("arg2 must be a port", TYPE_ERR);
        }
    }

    int start = 0;
    int end = 0;
    int num_chars = string_length(a->cell[0]);

    if (a->count >= 3) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error("arg3 must be an integer", TYPE_ERR);
        }
        start = (int)a->cell[2]->integer_v;
        if (a->count == 4) {
            if (a->cell[3]->type != CELL_INTEGER) {
                return make_cell_error("arg4 must be an integer", TYPE_ERR);
            }
            end = (int)a->cell[3]->integer_v;
        }
    }

    Cell* port;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        port = a->cell[1];
    }

    const char* in_string = &a->cell[0]->str[start];
    if (!end) {
        num_chars = num_chars - start;
    } else {
        num_chars = end - start;
    }
    const char* out_string = GC_strndup(in_string, num_chars);

    if (fputs(out_string, port->port->fh) == EOF) {
        return make_cell_error(strerror(errno), FILE_ERR);
    }
    /* No meaningful return value */
    return nullptr;
}

Cell* builtin_write_u8(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_INTEGER && (a->cell[0]->integer_v > 256 || a->cell[0]->integer_v < 0)) {
        return make_cell_error("arg1 must be an unsigned byte: 0 >= n <= 255", TYPE_ERR);
    }
    if (a->count == 2) {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error("arg2 must be a port", TYPE_ERR);
        }
    }
    Cell* port;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        port = a->cell[1];
    }
    int byte = (int)a->cell[0]->integer_v;
    if (putc(byte, port->port->fh) == EOF) {
        char buf[256];
        snprintf(buf, sizeof(buf), "write-u8 failed: %s", strerror(errno));
        return make_cell_error(strerror(errno), FILE_ERR);
    }
    return nullptr;
}

Cell* builtin_write_bytevector(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 4);
    if (err) return err;
    if (a->cell[0]->type != CELL_BYTEVECTOR) {
        return make_cell_error("arg1 must be a bytevector", TYPE_ERR);
    }
    if (a->count == 2) {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error("arg2 must be a port", TYPE_ERR);
        }
    }
    int start = 0;
    int end = 0;
    int num_bytes = a->cell[0]->count;

    if (a->count >= 3) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error("arg3 must be an integer", TYPE_ERR);
        }
        start = (int)a->cell[2]->integer_v;
        if (a->count == 4) {
            if (a->cell[3]->type != CELL_INTEGER) {
                return make_cell_error("arg4 must be an integer", TYPE_ERR);
            }
            end = (int)a->cell[3]->integer_v;
        }
    }
    Cell* port;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        port = a->cell[1];
    }

    const Cell* bv = a->cell[0];
    if (!end) {
        num_bytes = num_bytes - start;
    } else {
        num_bytes = end - start;
    }
    for (int i = start; i < num_bytes; i++) {
        const int byte = (int)bv->cell[i]->integer_v;
        if (putc(byte, port->port->fh) == EOF) {
            char buf[256];
            snprintf(buf, sizeof(buf), "write-bytevector failed: %s", strerror(errno));
            return make_cell_error(strerror(errno), FILE_ERR);
        }
    }
    return nullptr;
}

Cell* builtin_newline(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1);
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_output_port(e, a);
    } else {
        port = a->cell[0];
    }
    if (fputs("\n", port->port->fh) == EOF) {
        return make_cell_error(strerror(errno), FILE_ERR);
    }
    /* No meaningful return value */
    return nullptr;
}

Cell* builtin_eof(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 0);
    if (err) return err;

    return make_cell_eof();
}

/* (read-error? obj) */
Cell* builtin_read_error(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const Cell* obj = a->cell[0];
    if (obj->type != CELL_ERROR) {
        return make_cell_boolean(0);
    }

    if (obj->err_t != READ_ERR) {
        return make_cell_boolean(0);
    }

    return make_cell_boolean(1);
}

/* (file-error? obj) */
Cell* builtin_file_error(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const Cell* obj = a->cell[0];
    if (obj->type != CELL_ERROR) {
        return make_cell_boolean(0);
    }

    if (obj->err_t != FILE_ERR) {
        return make_cell_boolean(0);
    }

    return make_cell_boolean(1);
}

/* (flush-output-port) procedure
(flush-output-port port ) */
Cell* builtin_flush_output_port(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1);
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_output_port(e, a);
    } else {
        err = check_arg_types(a, CELL_PORT);
        if (err) return err;
        port = a->cell[0];
    }

    int es;
    if ((es = fflush(port->port->fh)) != 0) {
        return make_cell_error(strerror(es), FILE_ERR);
    }
    return nullptr;
}

/* A simple function to check if a character is ready on a FILE* stream.
 * This directly implements the logic for char-ready? and u8-ready? */
static int is_char_ready(FILE *fp) {
    /* Get the underlying file descriptor */
    const int fd = fileno(fp);
    if (fd < 0) {
        return -1;
    }

    fd_set readfds;
    struct timeval timeout;

    /* Clear the set and add our file descriptor to it */
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    /* Set the timeout to 0, so select() returns immediately */
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    /* Monitor the file descriptor for readiness
     * The first argument is the highest-numbered file descriptor plus 1. */
    const int result = select(fd + 1, &readfds, nullptr, nullptr, &timeout);

    if (result == -1) {
        return -1;
    }

    /* result will be 1 if data is ready, 0 otherwise.
     * FD_ISSET is technically the most correct check. */
    return result > 0 && FD_ISSET(fd, &readfds);
}

Cell* builtin_char_ready(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1);
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_output_port(e, a);
    } else {
        err = check_arg_types(a, CELL_PORT);
        if (err) return err;
        port = a->cell[0];
    }

    const int result = is_char_ready(port->port->fh);
    if (result == -1) {
        return make_cell_error(
            "char-ready?: bad file descriptor",
            FILE_ERR);
    }
    return result ? make_cell_boolean(1) : make_cell_boolean(0);
}


Cell* builtin_u8_ready(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1);
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_output_port(e, a);
    } else {
        err = check_arg_types(a, CELL_PORT);
        if (err) return err;
        port = a->cell[0];
    }

    const int result = is_char_ready(port->port->fh);
    if (result == -1) {
        return make_cell_error(
            "u8-ready?: bad file descriptor",
            FILE_ERR);
    }
    return result ? make_cell_boolean(1) : make_cell_boolean(0);
}
