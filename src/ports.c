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
#include <stdlib.h>
#include <string.h>
#include <errno.h>


/*-------------------------------------------------------*
 *                Input/output and ports                 *
 * ------------------------------------------------------*/

Cell* builtin_current_input_port(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    return default_input_port;
}

Cell* builtin_current_output_port(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    return default_output_port;
}

Cell* builtin_current_error_port(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    return default_error_port;
}

Cell* builtin_input_port_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_PORT || a->cell[0]->port->port_t != INPUT_PORT) {
        return make_cell_boolean(0);
    }
    return make_cell_boolean(1);
}

Cell* builtin_output_port_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_PORT || a->cell[0]->port->port_t != OUTPUT_PORT) {
        return make_cell_boolean(0);
    }
    return make_cell_boolean(1);
}

Cell* builtin_text_port_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_PORT || a->cell[0]->port->stream_t != TEXT_PORT) {
        return make_cell_boolean(0);
    }
    return make_cell_boolean(1);
}

Cell* builtin_binary_port_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_PORT || a->cell[0]->port->stream_t != BINARY_PORT) {
        return make_cell_boolean(0);
    }
    return make_cell_boolean(1);
}

Cell* builtin_input_port_open(const Lex* e, const Cell* a) {
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

Cell* builtin_output_port_open(const Lex* e, const Cell* a) {
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

Cell* builtin_close_port(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_PORT) {
        return make_cell_error("arg1 is not a port", TYPE_ERR);
    }

    if (a->cell[0]->is_open == 1) {
        fclose(a->cell[0]->port->fh);
        a->cell[0]->is_open = 0;
    }
    return make_cell_boolean(1);
}

Cell* builtin_read_line(const Lex* e, const Cell* a) {
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
        return make_cell_error("port is not open for input", GEN_ERR);

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

Cell* builtin_read_char(const Lex* e, const Cell* a) {
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
        return make_cell_error("port is not open for input", GEN_ERR);

    /* TODO: need to use feof() to tell EOF from error? */
    const int c = fgetc(port->port->fh);
    if (c == EOF) {
        return make_cell_eof();
    }
    return make_cell_char(c);
}

Cell* builtin_peek_char(const Lex* e, const Cell* a) {
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
        return make_cell_error("port is not open for input", GEN_ERR);

    /* TODO: need to use feof() to tell EOF from error? */
    const int c = fgetc(port->port->fh);
    const int pb_c = ungetc(c, port->port->fh);
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

Cell* builtin_write_string(const Lex* e, const Cell* a) {
    (void)e;
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
    Cell* port;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        port = a->cell[1];
    }
    if (fputs(a->cell[0]->str, port->port->fh) == EOF) {
        return make_cell_error(strerror(errno), FILE_ERR);
    }
    /* No meaningful return value */
    return nullptr;
}

Cell* builtin_newline(const Lex* e, const Cell* a) {
    (void)e;
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

Cell* builtin_eof(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 0);
    if (err) return err;

    return make_cell_eof();
}
