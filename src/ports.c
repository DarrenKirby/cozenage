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
    if (a->cell[0]->type != VAL_PORT || a->cell[0]->port_t != INPUT_PORT) {
        return make_val_bool(0);
    }
    return make_val_bool(1);
}

Cell* builtin_output_port_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_PORT || a->cell[0]->port_t != OUTPUT_PORT) {
        return make_val_bool(0);
    }
    return make_val_bool(1);
}

Cell* builtin_text_port_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_PORT || a->cell[0]->stream_t != TEXT_PORT) {
        return make_val_bool(0);
    }
    return make_val_bool(1);
}

Cell* builtin_binary_port_pred(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_PORT || a->cell[0]->stream_t != BINARY_PORT) {
        return make_val_bool(0);
    }
    return make_val_bool(1);
}

Cell* builtin_input_port_open(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type == VAL_PORT ||
        a->cell[0]->port_t != INPUT_PORT ||
        a->cell[0]->is_open == true) {
        return make_val_bool(1);
        }
    return make_val_bool(0);
}

Cell* builtin_output_port_open(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type == VAL_PORT ||
        a->cell[0]->port_t != OUTPUT_PORT ||
        a->cell[0]->is_open == true) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

Cell* builtin_close_port(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != VAL_PORT) {
        return make_val_err("arg1 is not a port", GEN_ERR);
    }

    if (a->cell[0]->is_open == 1) {
        fclose(a->cell[0]->fh);
        a->cell[0]->is_open = 0;
    }
    return make_val_bool(1);
}

Cell* builtin_read_line(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1);
    if (err) return err;
    err = check_arg_types(a, VAL_PORT);
    if (err) return err;

    Cell* port = NULL;
    if (a->count == 0) {
        port = builtin_current_input_port(e, a);
    } else {
        port = a->cell[0];
    }

    if (port->is_open == 0 || port->port_t != INPUT_PORT)
        return make_val_err("port is not open for input", GEN_ERR);

    char *line = NULL;
    size_t n = 0;
    const ssize_t len = getline(&line, &n, port->fh);
    if (len <= 0) { free(line); return make_val_eof(); }
    /* remove newline if present */
    if (line[len-1] == '\n') line[len-1] = '\0';

    Cell* result = make_val_str(line);
    free(line);
    return result;
}

Cell* builtin_write_string(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 4);
    if (err) return err;
    if (a->cell[0]->type != VAL_STR) {
        return make_val_err("arg1 must be a string", GEN_ERR);
    }
    if (a->count >= 2) {
        if (a->cell[1]->type != VAL_PORT) {
            return make_val_err("arg2 must be a port", GEN_ERR);
        }
    }
    Cell* port = NULL;
    if (a->count == 1) {
        port = builtin_current_output_port(e, a);
    } else {
        port = a->cell[1];
    }
    if (fputs(a->cell[0]->str, port->fh) == EOF) {
        return make_val_err(strerror(errno), FILE_ERR);
    }
    /* No meaningful return value */
    return NULL;
}

Cell* builtin_newline(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1);
    if (err) return err;

    Cell* port = NULL;
    if (a->count == 0) {
        port = builtin_current_output_port(e, a);
    } else {
        port = a->cell[0];
    }
    if (fputs("\n", port->fh) == EOF) {
        return make_val_err(strerror(errno), FILE_ERR);
    }
    /* No meaningful return value */
    return NULL;
}
