/*
 * 'cell.c'
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

#include "cell.h"
#include "types.h"
#include "symbols.h"
#include "hash.h"
#include <gc/gc.h>
#include <stdlib.h>
#include <string.h>


/* Define the global nil */
Cell* Nil_Obj = nullptr;

/* Define global #t and #f */
Cell* True_Obj = nullptr;
Cell* False_Obj = nullptr;

/* Define global EOF object */
Cell* EOF_Obj = nullptr;

/* Default ports */
Cell* default_input_port  = nullptr;
Cell* default_output_port = nullptr;
Cell* default_error_port  = nullptr;

void init_default_ports(void) {
    default_input_port  = make_cell_port("stdin",  stdin,  INPUT_PORT, TEXT_PORT);
    default_output_port = make_cell_port("stdout", stdout, OUTPUT_PORT, TEXT_PORT);
    default_error_port  = make_cell_port("stderr", stderr, OUTPUT_PORT, TEXT_PORT);
}

/*-------------------------------------------*
 *       Global singleton constructors       *
 *                                           *
 *  These constructors should be considered  *
 *  'private', and never directly accessed.  *
 * ------------------------------------------*/

static Cell* make_cell_nil__(void) {
    Cell* nil_obj = GC_MALLOC_ATOMIC_UNCOLLECTABLE(sizeof(Cell));
    nil_obj->type = CELL_NIL;
    return nil_obj;
}

static Cell* make_cell_boolean__(const int the_boolean) {
    Cell* v = GC_MALLOC_ATOMIC_UNCOLLECTABLE(sizeof(Cell));
    v->type = CELL_BOOLEAN;
    v->boolean_v = the_boolean;
    return v;
}

static Cell* make_cell_eof__(void) {
    Cell* eof_obj = GC_MALLOC_ATOMIC_UNCOLLECTABLE(sizeof(Cell));
    eof_obj->type = CELL_EOF;
    return eof_obj;
}

void init_global_singletons(void) {
    Nil_Obj = make_cell_nil__();
    True_Obj = make_cell_boolean__(1);
    False_Obj = make_cell_boolean__(0);
    EOF_Obj = make_cell_eof__();
}

/*------------------------------------*
 *       Cell type constructors       *
 * -----------------------------------*/

/* Thin wrapper that returns the singleton nil object */
Cell* make_cell_nil(void) {
    return Nil_Obj;
}

/* Thin wrapper that returns the singleton #t or #f object */
Cell* make_cell_boolean(const int the_boolean) {
    return the_boolean ? True_Obj : False_Obj;
}

/* Thin wrapper that returns the singleton EOF object */
Cell* make_cell_eof(void) {
    return EOF_Obj;
}

Cell* make_cell_real(const long double the_real) {
    Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_REAL;
    v->exact = false;
    v->real_v = the_real;
    return v;
}

Cell* make_cell_integer(const long long int the_integer) {
    Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_INTEGER;
    v->exact = true;
    v->integer_v = the_integer;
    return v;
}

Cell* make_cell_rational(const long int numerator, const long int denominator,
                         const bool simplify) {
    Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_RATIONAL;
    v->exact = true;
    v->num = numerator;
    v->den = denominator;
    if (simplify) {
        return simplify_rational(v);
    }
    return v;
}

Cell* make_cell_complex(Cell* real_part, Cell *imag_part) {
    if (real_part->type == CELL_COMPLEX|| imag_part->type == CELL_COMPLEX) {
        return make_cell_error("Cannot have complex real or imaginary parts.", GEN_ERR);
    }
    Cell* v = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_COMPLEX;
    v->real = real_part;
    v->imag = imag_part;
    v->exact = real_part->exact && imag_part->exact;
    return v;
}

Cell* make_cell_symbol(const char* the_symbol) {
    /* Lookup in symbol table first */
    Cell* v = ht_get(symbol_table, the_symbol);
    if (v) {
        return v;
    }
    /* Not found, so construct the cell, place in the table, then return it */
    v = GC_MALLOC_ATOMIC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_SYMBOL;
    v->quoted = false;
    const char* canonical_name = ht_set(symbol_table, the_symbol, v);
    v->sym = (char*)canonical_name;
    return v;
}

Cell* make_cell_string(const char* the_string) {
    Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_STRING;
    v->str = GC_strdup(the_string);
    return v;
}

Cell* make_cell_sexpr(void) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_SEXPR;
    v->count = 0;
    v->cell = nullptr;
    return v;
}

Cell* make_cell_char(const UChar32 the_char) {
    Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_CHAR;
    v->char_v = the_char;
    return v;
}

Cell* make_cell_pair(Cell* car, Cell* cdr) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_PAIR;
    v->car = car;
    v->cdr = cdr;
    v->len = -1;
    return v;
}

Cell* make_cell_vector(void) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_VECTOR;
    v->cell = nullptr;
    v->count = 0;
    return v;
}

Cell* make_cell_bytevector(void) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_BYTEVECTOR;
    v->cell = nullptr;
    v->count = 0;
    return v;
}

Cell* make_cell_error(const char* error_string, const err_t error_type) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_ERROR;
    v->err_t = error_type;
    v->error_v = GC_strdup(error_string);
    return v;
}

Cell* make_cell_port(const char* path, FILE* fh, const int io_t, const int stream_t) {
    Cell* v = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->is_open = true;
    v->type = CELL_PORT;
    v->path = GC_strdup(path);
    v->port_t = io_t;
    v->stream_t = stream_t;
    v->fh = fh;
    return v;
}

/*------------------------------------------------*
 *    Cell accessors, destructors, and helpers    *
 * -----------------------------------------------*/

Cell* cell_add(Cell* v, Cell* x) {
    v->count++;
    v->cell = GC_REALLOC(v->cell, sizeof(Cell*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}

Cell* cell_pop(Cell* v, const int i) {
    if (i < 0 || i >= v->count) return nullptr; /* defensive */

    /* Grab item */
    Cell* x = v->cell[i];

    /* Shift the memory after the item at "i" over the top */
    if (i < v->count - 1) {
        memmove(&v->cell[i], &v->cell[i+1],
                sizeof(Cell*) * (v->count - i - 1));
    }

    /* Decrease the count of items */
    v->count--;

    /* If there are no elements left, free the array and set to NULL.
       Do NOT call GC_REALLOC(..., 0). */
    if (v->count == 0) {
        v->cell = nullptr;
    } else {
        /* Try to shrink the allocation; keep old pointer on OOM */
        Cell** tmp = GC_REALLOC(v->cell, sizeof(Cell*) * v->count);
        if (tmp) {
            v->cell = tmp;
        } /* else: on OOM we keep the old block (safe) */
    }
    return x;
}

/* Take an element out and delete the rest */
Cell* cell_take(Cell* v, const int i) {
    Cell* x = cell_pop(v, i);
    return x;
}

/* Recursively deep-copy all components of a Cell */
Cell* cell_copy(const Cell* v) {
    if (!v) return nullptr;

    Cell* copy = GC_MALLOC(sizeof(Cell));
    if (!copy) {
        fprintf(stderr, "ENOMEM: cell_copy failed\n");
        exit(EXIT_FAILURE);
    }

    copy->type = v->type;

    switch (v->type) {
    case CELL_INTEGER:
        copy->integer_v = v->integer_v;
        copy->exact = v->exact;
        break;
    case CELL_REAL:
        copy->real_v = v->real_v;
        copy->exact = v->exact;
        break;
    case CELL_BOOLEAN:
        /* Not sure why we would ever copy a boolean Cell, but just in case
         we will just return the global #t or #f singleton */
        copy = make_cell_boolean(v->boolean_v);
        break;
    case CELL_CHAR:
        copy->char_v = v->char_v;
        break;
    case CELL_SYMBOL:
        /* Symbols are interned, just grab the pointer */
        copy = (Cell*)v;
        break;
    case CELL_STRING:
        copy->str = GC_strdup(v->str);
        break;
    case CELL_ERROR:
        copy->error_v = GC_strdup(v->error_v);
        copy->err_t = v->err_t;
        break;
    case CELL_PROC:
        if (v->is_builtin) {
            copy->is_builtin = true;
            copy->builtin = v->builtin;
            copy->f_name    = GC_strdup(v->f_name);
        } else {
            copy->is_builtin = false;
            copy->l_name    = v->l_name ? GC_strdup(v->l_name) : nullptr;
            copy->formals = cell_copy(v->formals) ;
            copy->body    = cell_copy(v->body);
            copy->env     = v->env;   /* DO NOT copy environments; share the pointer */
        }
        break;
    case CELL_SEXPR:
    case CELL_VECTOR:
    case CELL_BYTEVECTOR:
        copy->count = v->count;
        if (v->count) {
            copy->cell = GC_MALLOC(sizeof(Cell*) * v->count);
        } else {
            copy->cell = nullptr;
        }
        for (int i = 0; i < v->count; i++) {
            copy->cell[i] = cell_copy(v->cell[i]);
        }
        break;
    case CELL_NIL:
        /* return the singleton instead of allocating */
        return make_cell_nil();
    case CELL_PAIR: {
        copy->car = cell_copy(v->car);
        copy->cdr = cell_copy(v->cdr);
        copy->len = v->len;
        break;
        }
    case CELL_RATIONAL: {
        copy->exact = v->exact;
        copy->num = v->num;
        copy->den = v->den;
        break;
    }
    case CELL_COMPLEX: {
        copy->exact = v->exact;
        copy->real = cell_copy(v->real);
        copy->imag = cell_copy(v->imag);
        break;
        }
    case CELL_PORT: {
        copy->is_open = v->is_open;
        copy->port_t = v->port_t;
        copy->stream_t = v->stream_t;
        copy->fh = v->fh;
        copy->path = GC_strdup(v->path);
        break;
    }
    case CELL_CONT:
        /* shallow copy (all fields remain zeroed) */
        break;

    default:
        fprintf(stderr, "cell_copy: unknown type %d\n", v->type);
        return nullptr;
    }
    return copy;
}
