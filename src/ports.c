/*
 * 'src/ports.c'
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

#include "ports.h"
#include "eval.h"
#include "types.h"
#include "strings.h"
#include "repr.h"
#include "vectors.h"
#include "bytevectors.h"
#include "buffer.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wchar.h>
#include <gc/gc.h>
#include <sys/select.h>

#define R_EOF (-1)
#define R_ERR (-2)
#define R_OK 0


/* TODO: many (most?) of these read/write procedures do not exhaustively check
 * for the correct port type and flags. They almost certainly should. */


/* These next 24 static functions are the actual handlers for the builtin
 * generic I/O procedures. They are specific to whether the operation is
 * on text or binary data, and whether the backing store is a file, string,
 * or bytevector. */

/*
 * Next 6: textual file-backed functions.
 *
 */
static int file_gets(const Cell* p, int* err) {
    const wint_t wc = fgetwc(p->port->fh);
    if (wc == WEOF) {
        /* Regular EOF. */
        if (feof(p->port->fh)) {
            return R_EOF;
        }
        /* Error from fgetwc(). */
        *err = errno;
        return R_ERR;
    }
    return wc;
}


static int file_puts(const int c, const Cell* p, int* err) {
    if (fputwc(c, p->port->fh) == WEOF) {
        *err = errno;
        return R_ERR;
    }
    return R_OK;
}


static int file_getm(const int chars_to_read, void* buf, const Cell* p, int* err) {
    char *dest = buf;
    int buf_idx = 0;
    int chars_read = 0;

    while (chars_read < chars_to_read) {
        static unsigned short mask[] = {192, 224, 240};
        unsigned short i = 0;
        unsigned short j = 0;

        /* Read a byte. */
        const int c = getc(p->port->fh);
        if (c == EOF) {
            if (feof(p->port->fh)) {
                dest[buf_idx] = '\0';
                break;
            }
            /* An actual error from getc. */
            *err = errno;
            return R_ERR;
        }
        dest[buf_idx] = (char)c;

        /* Check how many more bytes need to be read for character. */
        if ((dest[buf_idx] & mask[0]) == mask[0]) i++;
        if ((dest[buf_idx] & mask[1]) == mask[1]) i++;
        if ((dest[buf_idx] & mask[2]) == mask[2]) i++;
        /* Increment buffer pointer for the next read. */
        buf_idx++;

        /* Read subsequent character bytes. */
        while (j < i) {
            j++;
            dest[buf_idx] = (char)getc(p->port->fh);
            buf_idx++;
        }
        chars_read++;
    }

    if (chars_read == 0) {
        return R_EOF;
    }
    return chars_read;
}


static int file_putm(const void* buf, const Cell* p, int* err) {
    if (fputs(buf, p->port->fh) == EOF) {
        *err = errno;
        return R_ERR;
    }
    return R_OK;
}


static int file_peek(const Cell* p, int* err) {
    const wint_t wc = fgetwc(p->port->fh);
    if (wc == WEOF) {
        /* Regular EOF. */
        if (feof(p->port->fh)) {
            return R_EOF;
        }
        /* Error from fgetwc(). */
        *err = errno;
        return R_ERR;
    }
    /* Push back the char. */
    const wint_t pb_c = ungetwc(wc, p->port->fh);
    if (pb_c == WEOF) {
        *err = errno;
        return READ_ERR;
    }
    return wc;
}

static void file_close(Cell* p) {
    if (p->is_open) {
        fflush(p->port->fh);
        fclose(p->port->fh);
        p->is_open = 0;
    }
}


/*
 * Next 6: textual string-backed functions.
 *
 */
static int string_gets(const Cell* p, int* err) {
    *err = 0;
    if (p->port->data->length == p->port->index) {
        return R_EOF;
    }

    const int ch = (unsigned char)p->port->data->buffer[p->port->index];
    p->port->index++;
    return ch;
}


static int string_puts(const int c, const Cell* p, int* err) {
    *err = 0;
    sb_append_char(p->port->data, (char)c);
    return R_OK;
}


static int string_getm(const int chars_to_read, void* buf, const Cell* p, int* err) {
    *err = 0;
    char* dest = buf;
    memcpy(dest, &p->port->data->buffer[p->port->index], chars_to_read);
    return R_OK;
}


static int string_putm(const void* buf, const Cell* p, int* err) {
    *err = 0;
    const char* src = (char*)buf;
    sb_append_str(p->port->data, src);
    return R_OK;
}


static int string_peek(const Cell* p, int* err) {
    *err = 0;
    return p->port->data->buffer[p->port->index];
}


static void string_close(Cell* p) {
    if (p->is_open) p->is_open = 0;
}


/*
 * Next 6: binary file-backed functions.
 *
 */
static int byte_gets_file(const Cell* p, int* err) {
    const int byte = getc(p->port->fh);
    if (byte == EOF) {
        if (ferror(p->port->fh)) {
            /* Save errno. */
            *err = errno;
            return R_ERR;
        }
        /* If not an error, it's a legitimate EOF. */
        return R_EOF;
    }
    return byte;
}


static int byte_puts_file(const int c, const Cell* p, int* err) {
    if (putc(c, p->port->fh) == EOF) {
        *err = errno;
        return READ_ERR;
    }
    return R_OK;
}


static int byte_getm_file(const int bytes_to_read, void* buf, const Cell* p, int* err) {
    uint8_t* dest = buf;
    for (int i = 0; i < bytes_to_read; i++) {
        const int byte = getc(p->port->fh);
        if (byte == EOF) {
            if (ferror(p->port->fh)) {
                /* Use strerror for the message. */
                *err = errno;
                return R_ERR;
            }
            /* If not an error, it's a legitimate EOF. */
            return R_EOF;
        }
        /* Add the byte to the bytevector. */
        dest[i] = byte;
    }
    return R_OK;
}


static int byte_putm_file(const void* buf, const Cell* p, int* err) {
    *err = 0;
    const char* src = (char*)buf;
    sb_append_str(p->port->data, src);
    return R_OK;
}


static int byte_peek_file(const Cell* p, int* err) {
    const int byte = getc(p->port->fh);

    if (byte == EOF) {
        if (ferror(p->port->fh)) {
            *err = errno;
            return R_ERR;
        }
        /* Legitimate EOF. */
        return R_EOF;
    }

    /* Push the byte back into the stream buffer. */
    if (ungetc(byte, p->port->fh) == EOF) {
        *err = errno;
        return READ_ERR;
    }
    return byte;
}


static void byte_close_file(Cell* p) {
    if (p->is_open) {
        fflush(p->port->fh);
        fclose(p->port->fh);
        p->is_open = 0;
    }
}


/*
 * Next 6: binary bytevector-backed functions.
 *
 */
static int byte_gets_byte(const Cell* p, int* err) {
    *err = 0;
    if (p->port->data->length == p->port->index) {
        return R_EOF;
    }

    const int ch = (unsigned char)p->port->data->buffer[p->port->index];
    p->port->index++;
    return ch;
}

static int byte_puts_byte(const int c, const Cell* p, int* err) {
    *err = 0;
    sb_append_char(p->port->data, (char)c);
    return R_OK;
}

static int byte_getm_byte(const int chars_to_read, void* buf, const Cell* p, int* err) {
    *err = 0;
    char* dest = buf;
    memcpy(dest, &p->port->data->buffer[p->port->index], chars_to_read);
    return R_OK;
}

static int byte_putm_byte(const void* buf, const Cell* p, int* err) {
    *err = 0;
    const char* src = (char*)buf;
    sb_append_str(p->port->data, src);
    return R_OK;
}

static int byte_peek_byte(const Cell* p, int* err) {
    *err = 0;
    return p->port->data->buffer[p->port->index];
}

static void byte_close_byte(Cell* p) {
    if (p->is_open) p->is_open = 0;
}


/* The PortInterface VTables map generic operations to
 * specific functions for the builtin I/O procedures. */

/* A textual port with file backing store. */
const PortInterface FileVTable = {
    .get_s = file_gets,
    .put_s = file_puts,
    .get_m = file_getm,
    .put_m = file_putm,
    .peek  = file_peek,
    .close = file_close
};

/* A textual port with in-memory backing store. */
const PortInterface StringVTable = {
    .get_s = string_gets,
    .put_s = string_puts,
    .get_m = string_getm,
    .put_m = string_putm,
    .peek  = string_peek,
    .close = string_close
};

/* A binary port with file backing store. */
const PortInterface ByteVTableFile = {
    .get_s = byte_gets_file,
    .put_s = byte_puts_file,
    .get_m = byte_getm_file,
    .put_m = byte_putm_file,
    .peek  = byte_peek_file,
    .close = byte_close_file
};

/* A binary port with in-memory backing store. */
const PortInterface ByteVTableByte = {
    .get_s = byte_gets_byte,
    .put_s = byte_puts_byte,
    .get_m = byte_getm_byte,
    .put_m = byte_putm_byte,
    .peek  = byte_peek_byte,
    .close = byte_close_byte
};

/*-------------------------------------------------------*
 *       Input/output and port builtin procedures        *
 * ------------------------------------------------------*/


/* (current-input-port)
 * Returns the current input port (stdin by default). */
Cell* builtin_current_input_port(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    return default_input_port;
}


/* (current-output-port)
 * Returns the current output port (stdout by default). */
Cell* builtin_current_output_port(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    return default_output_port;
}


/* (current-error-port)
 * Returns the current error port (stderr by default). */
Cell* builtin_current_error_port(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    return default_error_port;
}


/* (input-port? port)
 * Returns #t if the port argument is an input port, else #f. */
Cell* builtin_input_port_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "input-port?");
    if (err) return err;
    if (a->cell[0]->type != CELL_PORT || a->cell[0]->port->stream_t != INPUT_STREAM) {
        return False_Obj;
    }
    return True_Obj;
}


/* (output-port? port)
 * Returns #t if the port argument is an output port, else #f. */
Cell* builtin_output_port_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "output-port?");
    if (err) return err;
    if (a->cell[0]->type != CELL_PORT || a->cell[0]->port->stream_t != OUTPUT_STREAM) {
        return False_Obj;
    }
    return True_Obj;
}


/* (input-port-open? port)
 * Returns #t if the port argument is an open input port, else #f. */
Cell* builtin_input_port_open(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "input-port-open?");
    if (err) return err;
    if (a->cell[0]->type == CELL_PORT &&
        a->cell[0]->port->stream_t == INPUT_STREAM &&
        a->cell[0]->is_open == true) {
        return True_Obj;
        }
    return False_Obj;
}


/* (output-port-open? port)
 * Returns #t if the port argument is an open output port, else #f. */
Cell* builtin_output_port_open(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "output-port-open?");
    if (err) return err;
    if (a->cell[0]->type == CELL_PORT &&
        a->cell[0]->port->stream_t == OUTPUT_STREAM &&
        a->cell[0]->is_open == true) {
        return True_Obj;
    }
    return False_Obj;
}


/* (close-port port)
 * Closes the resource associated with 'port', rendering the port incapable of delivering or accepting data. This
 * procedure has no effect if the port is already closed. */
Cell* builtin_close_port(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "close-port");
    if (err) return err;
    if (a->cell[0]->type != CELL_PORT) {
        return make_cell_error(
            "close-port: arg1 is not a port",
            TYPE_ERR);
    }

    if (a->cell[0]->is_open == 1) {
        fflush(a->cell[0]->port->fh);
        fclose(a->cell[0]->port->fh);
        a->cell[0]->is_open = 0;
    }
    return True_Obj;
}


/* TODO:
 * close-input-port and close-output-port when ASYNC ports are implemented.
 * For now, both procedures are aliases to close-port. */


/* (read-line)
 * (read-line port)
 * Returns the next line of text available from the textual input port, updating the port to point to the following
 * character. If an end of line is read, a string containing all the text up to (but not including) the end of line
 * is returned, and the port is updated to point just past the end of line. If an end of file is encountered before any
 * end of line is read, but some characters have been read, a string containing those characters is returned. If an end
 * of file is encountered before any characters are read, an end-of-file object is returned. For the purpose of this
 * procedure, an end of line consists of either a linefeed character, a carriage return character, or a sequence of a
 * carriage return character followed by a linefeed character. Implementations may also recognize other end of line
 * characters or sequences. */
Cell* builtin_read_line(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1, "read-line");
    if (err) return err;
    err = check_arg_types(a, CELL_PORT, "read-line");
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_input_port(e, a);
    } else {
        port = a->cell[0];
    }

    if (port->is_open == 0 || port->port->stream_t != INPUT_STREAM)
        return make_cell_error(
            "read-line: port is not open for input",
            FILE_ERR);

    char *line = nullptr;
    size_t n = 0;
    const ssize_t len = getline(&line, &n, port->port->fh);
    if (len <= 0) { free(line); return make_cell_eof(); }
    /* Remove newline if present. */
    if (line[len-1] == '\n') line[len-1] = '\0';

    Cell* result = make_cell_string(line);
    free(line);
    return result;
}


/* (read-lines)
 * (read-lines port)
 * Reads from port, or the default input port until EOF, and returns a list of strings
 * delimited by '\n' from the source. */
Cell* builtin_read_lines(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1, "read-lines");
    if (err) return err;
    err = check_arg_types(a, CELL_PORT, "read-lines");
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_input_port(e, a);
    } else {
        port = a->cell[0];
    }

    if (port->is_open == 0 || port->port->stream_t != INPUT_STREAM)
        return make_cell_error(
            "read-lines: port is not open for input",
            FILE_ERR);

    Cell* result = make_cell_vector();
    size_t linecap = 0;
    char *line = nullptr;
    ssize_t len;
    while ((len = getline(&line, &linecap, port->port->fh)) > 0) {
        if (len <= 0) { break; }
        /* Remove newline if present. */
        if (line[len-1] == '\n') line[len-1] = '\0';

        Cell* s = make_cell_string(line);
        cell_add(result, s);
    }

    /* getline() malloc'ed the buffer; we didn't use the GC. */
    free(line);
    return builtin_vector_to_list(e, make_sexpr_len1(result));
}


/* (read-string k)
 * (read-string k port)
 * Reads the next k characters, or as many as are available before the end of file, from the textual input port into a
 * newly allocated string in left-to-right order and returns the string. If no characters are available before the end
 * of file, an end-of-file object is returned. */
Cell* builtin_read_string(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2, "read-string");
    if (err) return err;
    if (a->cell[0]->type != CELL_INTEGER) {
        return make_cell_error(
            "read-string: arg 1 must be exact positive integer",
            TYPE_ERR);
    }
    const int chars_to_read = (int)a->cell[0]->integer_v;
    if (chars_to_read <= 0) {
        return make_cell_error(
           "read-string: arg 1 must be exact positive integer",
           TYPE_ERR);
    }
    Cell* port;
    if (a->count == 1) {
        port = builtin_current_input_port(e, a);
    } else {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error(
                "read-string: arg 2 must be a port",
                TYPE_ERR);
        }
        port = a->cell[1];
    }

    /* Buffer size = the chars to read by potential 4 bytes each, plus 1 for \0. */
    char* buffer = GC_MALLOC_ATOMIC(chars_to_read * 4 + 1);
    int err_r;
    const int chars_read = port->port->vtable->get_m(chars_to_read, buffer, port, &err_r);

    if (chars_read == R_EOF) {
        return EOF_Obj;
    }
    if (chars_read == R_ERR) {
        return make_cell_error(
            fmt_err("read-string: %s", strerror(err_r)),
            OS_ERR);
    }

    return make_cell_string(buffer);
}


/* (read-char)
 * (read-char port)
 * Returns the next character available from the textual input port, updating the port to point to the following
 * character. If no more characters are available, an end-of-file object is returned. */
Cell* builtin_read_char(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1, "read-char");
    if (err) return err;
    err = check_arg_types(a, CELL_PORT,"read-char");
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_input_port(e, a);
    } else {
        port = a->cell[0];
    }

    if (port->is_open == 0 || port->port->stream_t != INPUT_STREAM)
        return make_cell_error(
            "read-char: port is not open for input",
            FILE_ERR);

    if (port->port->backend_t == BK_FILE_BINARY || port->port->backend_t == BK_BYTEVECTOR) {
        return make_cell_error(
            "read-char: port must be a text or string port",
            VALUE_ERR);
    }

    int err_r;
    const int wc = port->port->vtable->get_s(port, &err_r);
    if (wc == R_EOF) {
        return EOF_Obj;
    }
    if (wc == R_ERR) {
        return make_cell_error(
            fmt_err("read-char: %s", strerror(err_r)),
            READ_ERR);
    }
    return make_cell_char(wc);
}


/* (read-u8)
 * (read-u8 port)
 * Returns the next byte available from the binary input port, updating the port to point to the following byte. If no
 * more bytes are available, an end-of-file object is returned. */
Cell* builtin_read_u8(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1, "read-u8");
    if (err) return err;
    err = check_arg_types(a, CELL_PORT,"read-u8");
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_input_port(e, a);
    } else {
        port = a->cell[0];
    }

    if (port->is_open == 0 || port->port->stream_t != INPUT_STREAM)
        return make_cell_error(
            "read-u8: port is not open for input",
            FILE_ERR);

    int err_r;
    const int byte = port->port->vtable->get_s(port, &err_r);

    if (byte == R_EOF) {
        return EOF_Obj;
    }
    if (err_r == R_ERR) {
        return make_cell_error(
            fmt_err("read-u8: %s", strerror(err_r)),
            READ_ERR);
    }

    return make_cell_integer(byte);
}


/* (read-bytevector k)
 * (read-bytevector k port)
 * Reads the next k bytes, or as many as are available before the end of file, from the binary input port into a newly
 * allocated bytevector in left-to-right order and returns the bytevector. If no bytes are available before the end of
 * file, an end-of-file object is returned. */
Cell* builtin_read_bytevector(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2, "read-bytevector");
    if (err) return err;

    if (a->cell[0]->type != CELL_INTEGER) {
        return make_cell_error(
            "read-bytevector: arg 1 must be exact positive integer",
            TYPE_ERR);
    }
    const int bytes_to_read = (int)a->cell[0]->integer_v;
    if (bytes_to_read <= 0) {
        return make_cell_error(
           "read-bytevector: arg 1 must be exact positive integer",
           TYPE_ERR);
    }
    Cell* port;
    if (a->count == 1) {
        port = builtin_current_input_port(e, a);
    } else {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error(
                "read-bytevector: arg 2 must be a port",
                TYPE_ERR);
        }
        port = a->cell[1];
        /* Make sure port is an open input port! */
        if (port->is_open == 0 || port->port->stream_t != INPUT_STREAM)
            return make_cell_error(
                "read-u8: port is not open for input",
                FILE_ERR);
    }

    /* Check if there are any bytes to read. */
    Cell* test = builtin_peek_u8(e, make_sexpr_len1(port));
    if (test->type == CELL_EOF) { return test; }

    /* There are bytes. Read them until bytes_read == bytes_to_read,
     * or EOF, whichever comes first. */
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    Cell* bv = make_cell_bytevector(BV_U8);

    for (int i = 0; i < bytes_to_read; i++) {
        const int byte = getc(port->port->fh);
        if (byte == EOF) {
            if (ferror(port->port->fh)) {
                /* Use strerror for the message. */
                return make_cell_error(
                    fmt_err("read-u8: %s", strerror(errno)),
                    FILE_ERR);
            }
            /* If not an error, it's a legitimate EOF. Return the bytevector. */
            return bv;
        }
        /* Add the byte to the bytevector. */
        BV_OPS[BV_U8].append(bv, byte);
    }
    /* Read bytes_to_read bytes with no EOF; return the bytevector. */
    return bv;
}


Cell* builtin_read_bytevector_bang(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 4, "read-bytevector!");
    if (err) return err;

    /* Ensure arg1 is a u8 bytevector. */
    if (a->cell[0]->type != CELL_BYTEVECTOR) {
        return make_cell_error(
            "read-bytevector!: arg1 must be a u8 bytevector",
            TYPE_ERR);
    }
    Cell* bv = a->cell[0];
    if (bv->bv->type != BV_U8) {
        return make_cell_error(
            "read-bytevector!: arg1 must be a u8 bytevector",
            TYPE_ERR);
    }

    /* Ensure arg2 is an open input port, if supplied. */
    Cell* port;
    if (a->count == 1) {
        port = builtin_current_input_port(e, a);
    } else {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error(
                "read-bytevector!: arg2 must be a port",
                TYPE_ERR);
        }
        port = a->cell[1];
        if (port->port->stream_t != INPUT_STREAM || port->is_open != 1) {
            return make_cell_error(
                "read-bytevector!: port is not open for input",
                VALUE_ERR);
        }
    }

    /* Sanity check start/end args if supplied. */
    int start = 0;
    int end = bv->count;

    if (a->count > 2) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error(
                "read-bytevector!: arg3 must be an integer",
                TYPE_ERR);
        }
        start = (int)a->cell[2]->integer_v;
        if (start < 0) {
            return make_cell_error(
                "read-bytevector!: arg3 must be an exact, positive integer",
                VALUE_ERR);
        }
        if (a->count > 3) {
            if (a->cell[3]->type != CELL_INTEGER) {
                return make_cell_error(
                    "read-bytevector!: arg4 must be an integer",
                    TYPE_ERR);
            }
            end = (int)a->cell[3]->integer_v;
            if (end < 0) {
                return make_cell_error(
                    "read-bytevector!: arg4 must be an exact, positive integer",
                    VALUE_ERR);
            }
        }
    }

    if (end > bv->count) {
        return make_cell_error(
            "read-bytevector!: 'end' exceeds bytevector length",
            VALUE_ERR);
    }

    const int bytes_to_read = end - start;
    if (bytes_to_read > bv->count) {
        return make_cell_error(
            fmt_err("read-bytevector!: bytevector not large enough to store %d bytes", bytes_to_read),
            VALUE_ERR);
    }

    if (bytes_to_read == 0) return make_cell_integer(0);

    /* R7RS: Return EOF object if port is already at EOF. */
    const int c = getc(port->port->fh);
    if (c == EOF) {
        if (ferror(port->port->fh)) {
            return make_cell_error(
                fmt_err("read-bytevector!: %s", strerror(errno)),
                FILE_ERR);
        }
        return make_cell_eof();
    }
    ungetc(c, port->port->fh);

    /* Cast void* to uint8_t* to allow correct pointer arithmetic. */
    uint8_t* buffer_ptr = bv->bv->data;

    /* Read directly into the offset address. */
    const size_t n = fread(buffer_ptr + start, 1, (size_t)bytes_to_read, port->port->fh);

    if (n < (size_t)bytes_to_read && ferror(port->port->fh)) {
        return make_cell_error(
            fmt_err("read-bytevector!: %s", strerror(errno)),
            FILE_ERR);
    }

    return make_cell_integer((int)n);
}


/* (peek-char)
 * (peek-char port)
 * Returns the next character available from the textual input port, but without updating the port to point to the
 * following character. If no more characters are available, an end-of-file object is returned. */
Cell* builtin_peek_char(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1, "peek-char");
    if (err) return err;
    err = check_arg_types(a, CELL_PORT, "peek-char");
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_input_port(e, a);
    } else {
        port = a->cell[0];
    }

    if (port->is_open == 0 || port->port->stream_t != INPUT_STREAM)
        return make_cell_error(
            "peek-char: port is not open for input",
            FILE_ERR);


    int err_r;
    const int wc = port->port->vtable->peek(port, &err_r);

    if (wc == R_EOF) {
        return EOF_Obj;
    }
    if (wc == R_ERR) {
        return make_cell_error(
            fmt_err("peek-char: %s", strerror(err_r)),
            READ_ERR);
    }

    return make_cell_char(wc);
}


/* (peek-u8)
 * (peek-u8 port )
 * Returns the next byte available from the binary input port, but without updating the port to point to the following
 * byte. If no more bytes are available, an end-of-file object is returned. */
Cell* builtin_peek_u8(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1, "peek-u8");
    if (err) return err;
    err = check_arg_types(a, CELL_PORT,"peek-u8");
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_input_port(e, a);
    } else {
        port = a->cell[0];
    }

    if (port->is_open == 0 || port->port->stream_t != INPUT_STREAM)
        return make_cell_error(
            "peek-u8: port is not open for input",
            FILE_ERR);

    FILE* fh = port->port->fh;
    const int byte = getc(fh);

    if (byte == EOF) {
        if (ferror(fh)) {
            return make_cell_error(
                fmt_err("peek-u8: %s", strerror(errno)),
                FILE_ERR);
        }
        return make_cell_eof();
    }

    /* Push the byte back into the stream buffer. */
    if (ungetc(byte, fh) == EOF) {
        return make_cell_error(
            "peek-u8: internal pushback error",
            FILE_ERR);
    }

    return make_cell_integer(byte);
}


/* (write-char char )
 * (write-char char port )
 * Writes the character char (not an external representation of the character) to the given textual output port and
 * returns an unspecified value. */
Cell* builtin_write_char(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2, "write-char");
    if (err) return err;
    if (a->cell[0]->type != CELL_CHAR) {
        return make_cell_error(
            "write-char: arg1 must be a char",
            TYPE_ERR);
    }
    if (a->count == 2) {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error(
                "write-char: arg2 must be a port",
                TYPE_ERR);
        }
    }
    Cell* port;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        port = a->cell[1];
    }

    if (fputwc(a->cell[0]->char_v, port->port->fh) == WEOF) {
        return make_cell_error(
            fmt_err("write-char: %s", strerror(errno)),
            FILE_ERR);
    }
    return USP_Obj;
}


/* (write-string string)
 * (write-string string port)
 * (write-string string port start)
 * (write-string string port start end)
 * Writes the characters of string from start to end in left-to-right order to the textual output port. */
Cell* builtin_write_string(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 1, 4, "write-string");
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "write-string: arg1 must be a string",
            TYPE_ERR);
    }
    if (a->count >= 2) {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error(
                "write-string: arg2 must be a port",
                TYPE_ERR);
        }
    }

    int start = 0;
    int end = 0;
    int num_chars = a->cell[0]->char_count;

    if (a->count >= 3) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error(
                "write-string: arg3 must be an integer",
                TYPE_ERR);
        }
        start = (int)a->cell[2]->integer_v;
        if (a->count == 4) {
            if (a->cell[3]->type != CELL_INTEGER) {
                return make_cell_error(
                    "write-string: arg4 must be an integer",
                    TYPE_ERR);
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

    int* err_r = nullptr;
    const int rv = port->port->vtable->put_m(out_string, port, err_r);
    if (rv == R_ERR) {
        return make_cell_error(
            fmt_err("write-string: %s", strerror(*err_r)),
            FILE_ERR);
    }
    /* No meaningful return value. */
    return USP_Obj;
}


/* (write-u8 byte )
 * (write-u8 byte port )
 * Writes the byte to the given binary output port and returns an unspecified value. */
Cell* builtin_write_u8(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2, "write-u8");
    if (err) return err;

    /* Ensure the argument is an unsigned byte. */
    if (a->cell[0]->type != CELL_INTEGER ||
        a->cell[0]->integer_v > 255 ||
        a->cell[0]->integer_v < 0) {
        return make_cell_error(
            "write-u8: argument must be an octet (0-255)",
            TYPE_ERR);
        }

    if (a->count == 2) {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error(
                "write-u8: arg2 must be a port",
                TYPE_ERR);
        }
    }

    Cell* port;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        port = a->cell[1];
    }

    const int byte = (int)a->cell[0]->integer_v;

    /* For a single byte, putc does the same thing as
     * fwrite(&byte, 1, 1, fh) but more efficiently. */
    if (putc(byte, port->port->fh) == EOF) {
        return make_cell_error(
            fmt_err("write-u8: %s", strerror(errno)),
            FILE_ERR);
    }
    return USP_Obj;
}


/* (write-bytevector bytevector)
 * (write-bytevector bytevector port)
 * (write-bytevector bytevector port start)
 * (write-bytevector bytevector port start end)
 * Writes the bytes of bytevector from start to end in left-to-right order to the binary output port. */
Cell* builtin_write_bytevector(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 4, "write-bytevector");
    if (err) return err;
    if (a->cell[0]->type != CELL_BYTEVECTOR) {
        return make_cell_error(
            "write-bytevector: arg1 must be a bytevector",
            TYPE_ERR);
    }

    if (a->count == 2) {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error(
                "write-bytevector: arg2 must be a port",
                TYPE_ERR);
        }
    }

    int start = 0;
    int end = 0;
    int num_bytes = a->cell[0]->count;

    if (a->count >= 3) {
        if (a->cell[2]->type != CELL_INTEGER) {
            return make_cell_error(
                "write-bytevector: arg3 must be an integer",
                TYPE_ERR);
        }
        start = (int)a->cell[2]->integer_v;
        if (a->count == 4) {
            if (a->cell[3]->type != CELL_INTEGER) {
                return make_cell_error(
                    "write-bytevector: arg4 must be an integer",
                    TYPE_ERR);
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
        const int byte = (int)BV_OPS[bv->bv->type].get(bv, i);
        if (putc(byte, port->port->fh) == EOF) {
            return make_cell_error(fmt_err(
                "write-bytevector: %s", strerror(errno)),
                FILE_ERR);
        }
    }
    return USP_Obj;
}


/* (newline)
 * (newline port)
 * Writes an end of line to textual output port. Exactly how this is done differs from one operating system to another.
 * Returns an unspecified value. */
Cell* builtin_newline(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1, "newline");
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_output_port(e, a);
    } else {
        port = a->cell[0];
    }
    if (fputs("\n", port->port->fh) == EOF) {
        return make_cell_error(
            fmt_err("newline: %s", strerror(errno)),
            FILE_ERR);
    }
    /* No meaningful return value. */
    return USP_Obj;
}


/* (eof-object)
 * Returns an end-of-file object, not necessarily unique. */
Cell* builtin_eof(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "eof-object");
    if (err) return err;
    return EOF_Obj;
}


/* (read-error? obj)
 * Error type predicate. Returns #t if obj is an object raised by the read procedure. */
Cell* builtin_read_error(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "read-error?");
    if (err) return err;

    const Cell* obj = a->cell[0];
    if (obj->type != CELL_ERROR) {
        return False_Obj;
    }

    if (obj->err_t != READ_ERR) {
        return False_Obj;
    }

    return True_Obj;
}


/* (file-error? obj)
 * Error type predicate. Returns #t if obj is an object raised because of a file open/close error. */
Cell* builtin_file_error(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "file-error?");
    if (err) return err;

    const Cell* obj = a->cell[0];
    if (obj->type != CELL_ERROR) {
        return False_Obj;
    }

    if (obj->err_t != FILE_ERR) {
        return False_Obj;
    }

    return True_Obj;
}


/* (flush-output-port)
 * (flush-output-port port )
 * Flushes any buffered output from the buffer of output-port to the underlying file or device and returns an
 * unspecified value. */
Cell* builtin_flush_output_port(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1, "flush-output-port");
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_output_port(e, a);
    } else {
        err = check_arg_types(a, CELL_PORT, "flush-output-port");
        if (err) return err;
        port = a->cell[0];
    }

    int es;
    if ((es = fflush(port->port->fh)) != 0) {
        return make_cell_error(strerror(es), FILE_ERR);
    }
    return USP_Obj;
}


#if defined(__GLIBC__)
/* glibc internal check */
#define HAS_BUFFERED_DATA(fp) ((fp)->_IO_read_ptr < (fp)->_IO_read_end)
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
/* BSD/Darwin internal check */
#define HAS_BUFFERED_DATA(fp) ((fp)->_r > 0)
#else
/* Fallback: We don't know how to check the buffer for this LibC.
 * If we can't see the buffer, at least EOF counts as 'ready'. */
#define HAS_BUFFERED_DATA(fp) (feof(fp))
#endif


/* A simple function to check if a character is ready on a FILE* stream.
 * This directly implements the logic for char-ready? and u8-ready? */
static int is_stream_ready(FILE *fp)
{
    /* Check the C library buffer first */
    if (HAS_BUFFERED_DATA(fp)) {
        return 1;
    }

    /* If buffer is empty, check the OS kernel via select() */
    /* Get the underlying file descriptor. */
    const int fd = fileno(fp);
    if (fd < 0) {
        return -1;
    }

    fd_set readfds;
    struct timeval timeout;

    /* Clear the set and add our file descriptor to it. */
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    /* Set the timeout to 0, so select() returns immediately. */
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    /* Monitor the file descriptor for readiness.
     * The first argument is the highest-numbered file descriptor plus 1. */
    const int result = select(fd + 1, &readfds, nullptr, nullptr, &timeout);

    if (result == -1) {
        return -1;
    }

    /* result will be 1 if data is ready, 0 otherwise.
     * FD_ISSET is technically the most correct check. */
    return result > 0 && FD_ISSET(fd, &readfds);
}


/* (char-ready?)
 * (char-ready? port)
 * Returns #t if a character is ready on the textual input port and returns #f otherwise. If char-ready returns #t then
 * the next read-char operation on the given port is guaranteed not to hang. If the port is at end of file then
 * char-ready? returns #t. */
Cell* builtin_char_ready(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1, "char-ready?");
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_input_port(e, a);
    } else {
        err = check_arg_types(a, CELL_PORT, "char-ready?");
        if (err) return err;
        port = a->cell[0];
    }

    const int result = is_stream_ready(port->port->fh);
    if (result == -1) {
        return make_cell_error(
            "char-ready?: bad file descriptor",
            FILE_ERR);
    }
    return result ? True_Obj : False_Obj;
}


/* (u8-ready?)
 * (u8-ready? port )
 * Returns #t if a byte is ready on the binary input port and returns #f otherwise. If u8-ready? returns #t then the
 * next read-u8 operation on the given port is guaranteed not to hang. If the port is at end of file then u8-ready?
 * returns #t. */
Cell* builtin_u8_ready(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1, "u8-ready?");
    if (err) return err;

    Cell* port;
    if (a->count == 0) {
        port = builtin_current_input_port(e, a);
    } else {
        err = check_arg_types(a, CELL_PORT, "u8-ready?");
        if (err) return err;
        port = a->cell[0];
    }

    const int result = is_stream_ready(port->port->fh);
    if (result == -1) {
        return make_cell_error(
            "u8-ready?: bad file descriptor",
            FILE_ERR);
    }
    return result ? True_Obj : False_Obj;
}


/* (display obj )
 * (display obj port )
 * Writes a representation of obj to the given textual output port. Strings that appear in the written representation are
 * output as if by write-string instead of by write. Symbols are not escaped. Character objects appear in the
 * representation as if written by write-char instead of by write.
 *
 * The display representation of other objects is unspecified. However, display must not loop forever on
 * self-referencing pairs, vectors, or records. Thus, if the normal write representation is used, datum labels are
 * needed to represent cycles as in write.
 *
 * Implementations may support extended syntax to represent record types or other types that do not have datum
 * representations. The display procedure returns an unspecified value. */
Cell* builtin_display(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2, "display");
    if (err) return err;

    Cell* port;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error(
                "display: arg2 must be a port",
                TYPE_ERR);
        }
        port = a->cell[1];
    }
    const Cell* val = a->cell[0];
    fprintf(port->port->fh, "%s", cell_to_string(val, MODE_DISPLAY));
    return USP_Obj;
}


/* (println obj )
 * (println obj port )
 * Identical to 'display', but adds a newline for convenience. */
Cell* builtin_println(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2, "println");
    if (err) return err;

    Cell* port;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error(
                "println: arg2 must be a port",
                TYPE_ERR);
        }
        port = a->cell[1];
    }
    const Cell* val = a->cell[0];
    fprintf(port->port->fh, "%s\n", cell_to_string(val, MODE_DISPLAY));
    return USP_Obj;
}


/* (write obj)
 * (write obj port)
 * Writes a representation of obj to the given textual output port. Strings that appear in the written representation
 * are enclosed in quotation marks, and within those strings backslash and quotation mark characters are escaped by
 * backslashes. Symbols that contain non-ASCII characters are escaped with vertical lines. Character objects are written
 * using the #\ notation.
 *
 * If obj contains cycles which would cause an infinite loop using the normal written representation, then at least the
 * objects that form part of the cycle must be represented using datum labels as described in section 2.4. Datum labels
 * must not be used if there are no cycles.
 *
 * Implementations may support extended syntax to represent record types or other types that do not have datum
 * representations. The write procedure returns an unspecified value. */
/* TODO: does not handle circular objects/datum labels */
Cell* builtin_write(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2, "write");
    if (err) return err;

    Cell* port;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error(
                "write: arg1 must be a port",
                TYPE_ERR);
        }
        port = a->cell[1];
    }
    const Cell* val = a->cell[0];
    fprintf(port->port->fh, "%s", cell_to_string(val, MODE_WRITE));
    return USP_Obj;
}


/* (writeln obj)
 * (writeln obj port)
 * Identical to 'write', but add the newline for convenience. */
Cell* builtin_writeln(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2, "writeln");
    if (err) return err;

    Cell* port;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        /* TODO: ensure it is an output text port. */
        if (a->cell[1]->type != CELL_PORT) {
            return make_cell_error(
                "writeln: must be a port",
                TYPE_ERR);
        }
        port = a->cell[1];
    }
    const Cell* val = a->cell[0];
    fprintf(port->port->fh, "%s\n", cell_to_string(val, MODE_WRITE));
    return USP_Obj;
}


/* (open-input-file string)
 * Takes a string for an existing file and returns an input port that is capable of delivering text data from the file.
 * If the file does not exist or cannot be opened, an error that satisfies file-error? is signaled. */
Cell* builtin_open_input_file(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "open-input-file");
    if (err) { return err; }
    err = CHECK_ARITY_EXACT(a, 1, "open-input-file");
    if (err) { return err; }

    const char* filename = a->cell[0]->str;
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return make_cell_error(
            fmt_err("open-input-file", strerror(errno)),
            FILE_ERR);
    }
    char *actual_path = GC_MALLOC(PATH_MAX);
    const char *ptr = realpath(filename, actual_path);
    if (ptr == NULL) {
        fclose(fp);
        return make_cell_error(
            fmt_err("open-input-file", strerror(errno)),
            FILE_ERR);
    }

    /* Must be a text file port. */
    Cell* p = make_cell_port(ptr, fp, INPUT_STREAM, BK_FILE_TEXT);
    return p;
}


/* (open-bin-input-file string)
 * Takes a string for an existing file and returns an input port that is capable of delivering binary data from the file. If
 * the file does not exist or cannot be opened, an error that satisfies file-error? is signaled. */
Cell* builtin_open_bin_input_file(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING, "open-bin-input-file");
    if (err) { return err; }
    err = CHECK_ARITY_EXACT(a, 1, "open-bin-input-file");
    if (err) { return err; }

    const char* filename = a->cell[0]->str;
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return make_cell_error(
            fmt_err("open-bin-input-file", strerror(errno)),
            FILE_ERR);
    }
    char *actual_path = GC_MALLOC(PATH_MAX);
    const char *ptr = realpath(filename, actual_path);
    if (ptr == NULL) {
        fclose(fp);
        return make_cell_error(
            fmt_err("open-bin-input-file", strerror(errno)),
            FILE_ERR);
    }

    /* Must be a binary file port. */
    Cell* p = make_cell_port(ptr, fp, INPUT_STREAM, BK_FILE_BINARY);
    return p;
}


/* (open-output-file string )
 * Takes a string naming an output file to be opened and returns an output port that is capable of writing data to the
 * file by that name. If a file with the given name does not exist, it will be created. If the file already exists, the
 * file will be appended to. If the file cannot be opened, an error that satisfies file-error? is signaled. */
Cell* builtin_open_output_file(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING,"open-output-file");
    if (err) { return err; }
    err = CHECK_ARITY_RANGE(a, 1, 2, "open-output-file");
    if (err) { return err; }

    const char *mode = "a";
    const char* filename = a->cell[0]->str;
    if (a->count == 2 && a->cell[1]->type == CELL_STRING) {
        mode = a->cell[1]->str;
    }
    FILE *fp = fopen(filename, mode);
    if (!fp) {
        return make_cell_error(
            fmt_err("open-output-file", strerror(errno)),
            FILE_ERR);
    }
    char *actual_path = GC_MALLOC(PATH_MAX);
    const char *ptr = realpath(filename, actual_path);
    if (ptr == NULL) {
        fclose(fp);
        return make_cell_error(
            fmt_err("open-output-file", strerror(errno)),
            FILE_ERR);
    }

    /* Must be a file port. */
    Cell* p = make_cell_port(filename, fp, OUTPUT_STREAM, BK_FILE_TEXT);
    return p;
}


/* (open-bin-output-file string )
 * Takes a string naming an output file to be opened and returns an output port that is capable of writing binary data to the
 * file by that name. If a file with the given name does not exist, it will be created. If the file already exists, the
 * file will be appended to. If the file cannot be opened, an error that satisfies file-error? is signaled. */
Cell* builtin_open_bin_output_file(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING,"open-bin-output-file");
    if (err) { return err; }
    err = CHECK_ARITY_RANGE(a, 1, 2, "open-bin-output-file");
    if (err) { return err; }

    const char *mode = "a";
    const char* filename = a->cell[0]->str;
    if (a->count == 2 && a->cell[1]->type == CELL_STRING) {
        mode = a->cell[1]->str;
    }
    FILE *fp = fopen(filename, mode);
    if (!fp) {
        return make_cell_error(
            fmt_err("open-bin-output-file", strerror(errno)),
            FILE_ERR);
    }
    char *actual_path = GC_MALLOC(PATH_MAX);
    const char *ptr = realpath(filename, actual_path);
    if (ptr == NULL) {
        fclose(fp);
        return make_cell_error(
            fmt_err("open-bin-output-file", strerror(errno)),
            FILE_ERR);
    }

    /* Must be a binary file port. */
    Cell* p = make_cell_port(filename, fp, OUTPUT_STREAM, BK_FILE_BINARY);
    return p;
}


/* (open-and-trunc-output-file string )
 * Takes a string naming an output file to be opened and returns an output port that is capable of writing data to the
 * file by that name. If a file with the given name does not exist, it will be created. If the file does exist, it will
 * be truncated to length 0, and overwritten.  If the file cannot be opened, an error that satisfies file-error? is
 * signaled. */
Cell* builtin_open_and_trunc_output_file(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_STRING,"open-and-trunc-output-file");
    if (err) { return err; }
    err = CHECK_ARITY_RANGE(a, 1, 2, "open-and-trunc-output-file");
    if (err) { return err; }

    const char *mode = "w";
    const char* filename = a->cell[0]->str;
    if (a->count == 2 && a->cell[1]->type == CELL_STRING) {
        mode = a->cell[1]->str;
    }
    FILE *fp = fopen(filename, mode);
    if (!fp) {
        return make_cell_error(
            fmt_err("open-and-trunc-output-file", strerror(errno)),
            FILE_ERR);
    }
    char *actual_path = GC_MALLOC(PATH_MAX);
    const char *ptr = realpath(filename, actual_path);
    if (ptr == NULL) {
        fclose(fp);
        return make_cell_error(
            fmt_err("open-and-trunc-output-file", strerror(errno)),
            FILE_ERR);
    }

    /* Must be a file port. */
    Cell* p = make_cell_port(filename, fp, OUTPUT_STREAM, BK_FILE_TEXT);
    return p;
}

// Cell* builtin_open_output_string(const Lex* e, const Cell* a) {
//     (void)e;
//     Cell* err = CHECK_ARITY_EXACT(a, 0, "open-output-string");
//     if (err) { return err; }
//
//     Cell* p = make_cell_data_port(STRING_PORT, OUTPUT_PORT);
//
//
// }
//
//
// Cell* builtin_open_output_bytevector(const Lex* e, const Cell* a) {
//
// }

/* TODO: implement (call-with-port port proc) */


/* (call-with-input-file string proc)
 * This procedure obtains an input port obtained by opening the named file for input as if by open-input-file. The port
 * and proc are then passed to a procedure equivalent to call-with-port. It is an error if proc does not accept one
 * argument. */
Cell* builtin_call_with_input_file(const Lex* e, const Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2, "call-with-input-file");
    if (err) { return err; }

    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "call-with-input-file: arg1 must be a string",
            TYPE_ERR);
    }
    const char* path = a->cell[0]->str;

    if (a->cell[1]->type != CELL_PROC) {
        return make_cell_error(
            "call-with-input-file: arg2 must be a proc",
            TYPE_ERR);
    }
    const Cell* proc = a->cell[1];

    /* Check arity of lambda. */
    if (!proc->is_builtin) {
        if (check_lambda_arity(proc, 1) != 1) {
            return make_cell_error(
                "call-with-input-file: lambda must take exactly one arg",
                ARITY_ERR);
        }
    }

    /* Open the port for reading. */
    FILE* fp = fopen(path, "r");
    if (!fp) {
        return make_cell_error(strerror(errno), FILE_ERR);
    }

    const Cell* p = make_cell_port(path, fp, INPUT_STREAM, BK_FILE_TEXT);

    Cell* clos = make_sexpr_len2(proc, p);
    Cell* result = coz_eval((Lex*)e, clos);

    builtin_close_port((Lex*)e, make_sexpr_len1(p));
    return result;
}


/* (call-with-output-file string proc)
 * This procedure obtains an output port obtained by opening the named file for output as if by open-output-file. The
 * port and proc are then passed to a procedure equivalent to call-with-port. It is an error if proc does not accept one
 * argument. */
Cell* builtin_call_with_output_file(const Lex* e, const Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2, "call-with-output-file");
    if (err) { return err; }

    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "call-with-output-file: arg1 must be a string",
            TYPE_ERR);
    }
    const char* path = a->cell[0]->str;

    if (a->cell[1]->type != CELL_PROC) {
        return make_cell_error(
            "call-with-output-file: arg2 must be a proc",
            TYPE_ERR);
    }
    const Cell* proc = a->cell[1];

    /* Check arity of lambda. */
    if (!proc->is_builtin) {
        if (check_lambda_arity(proc, 1) != 1) {
            return make_cell_error(
                "call-with-input-file: lambda must take exactly one arg",
                ARITY_ERR);
        }
    }

    /* Open the port for writing */
    FILE* fp = fopen(path, "w");
    if (!fp) {
        return make_cell_error(strerror(errno), FILE_ERR);
    }

    const Cell* p = make_cell_port(path, fp, OUTPUT_STREAM, BK_FILE_TEXT);

    Cell* clos = make_sexpr_len2(proc, p);
    Cell* result = coz_eval((Lex*)e, clos);

    builtin_close_port((Lex*)e, make_sexpr_len1(p));
    return result;
}


/* (with-input-from-file string thunk)
 * The file is opened for input as if by open-input-file, and the new port is made to be the value returned by
 * current-input-port (as used by (read), (write obj), and so forth). The thunk is then called with no arguments. When
 * the thunk returns, the port is closed and the previous default is restored. It is an error if thunk does not accept
 * zero arguments. The procedure returns the value yielded by thunk. */
Cell* builtin_with_input_from_file(const Lex* e, const Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2, "with-input-from-file");
    if (err) { return err; }

    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "with-input-from-file: arg1 must be a string",
            TYPE_ERR);
    }
    const char* path = a->cell[0]->str;

    if (a->cell[1]->type != CELL_PROC) {
        return make_cell_error(
            "with-input-from-file: arg2 must be a proc",
            TYPE_ERR);
    }
    const Cell* proc = a->cell[1];

    /* Check arity of lambda. */
    if (!proc->is_builtin) {
        if (check_lambda_arity(proc, 0) != 0) {
            return make_cell_error(
                "call-with-input-file: lambda must not take args",
                ARITY_ERR);
        }
    }

    /* Save stdin port to local var. */
    Cell* std_input_port = default_input_port;

    /* Open the port for reading, and bind to default input. */
    FILE* fp = fopen(path, "r");
    if (!fp) {
        return make_cell_error(strerror(errno), FILE_ERR);
    }

    default_input_port = make_cell_port(path, fp, INPUT_STREAM, BK_FILE_TEXT);

    /* Pass the thunk to eval. */
    Cell* clos = make_sexpr_len1(proc);
    Cell* result = coz_eval((Lex*)e, clos);

    /* Reset original ports, and return result. */
    builtin_close_port((Lex*)e, default_input_port);
    default_input_port = std_input_port;
    return result;
}


/* (with-output-from-file string thunk)
 * The file is opened for output as if by open-output-file, and the new port is made to be the value returned by
 * current-output-port (as used by (read), (write obj), and so forth). The thunk is then called with no arguments. When
 * the thunk returns, the port is closed and the previous default is restored. It is an error if thunk does not accept
 * zero arguments. The procedure returns the value yielded by thunk. */
Cell* builtin_with_output_to_file(const Lex* e, const Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2, "with-output-to-file");
    if (err) { return err; }

    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "with-output-to-file: arg1 must be a string",
            TYPE_ERR);
    }
    const char* path = a->cell[0]->str;

    if (a->cell[1]->type != CELL_PROC) {
        return make_cell_error(
            "with-output-to-file: arg2 must be a proc",
            TYPE_ERR);
    }
    const Cell* proc = a->cell[1];

    /* Check arity of lambda. */
    if (!proc->is_builtin) {
        if (check_lambda_arity(proc, 0) != 0) {
            return make_cell_error(
                "call-with-input-file: lambda must not take args",
                ARITY_ERR);
        }
    }

    /* Save stdout port to local var. */
    Cell* std_output_port = default_output_port;

    /* Open the port for writing, and bind to default output. */
    FILE* fp = fopen(path, "w");
    if (!fp) {
        return make_cell_error(strerror(errno), FILE_ERR);
    }

    default_output_port = make_cell_port(path, fp, OUTPUT_STREAM, BK_FILE_TEXT);

    /* Pass the thunk to eval. */
    Cell* clos = make_sexpr_len1(proc);
    Cell* result = coz_eval((Lex*)e, clos);

    /* Reset original ports, and return result. */
    builtin_close_port((Lex*)e, default_output_port);
    default_output_port = std_output_port;
    return result;
}
