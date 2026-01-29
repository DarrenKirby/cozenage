/*
 * 'src/cell.c'
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

#include "cell.h"
#include "types.h"
#include "symbols.h"
#include "hash.h"
#include "bytevectors.h"
#include "ports.h"

#include <gc/gc.h>
#include <stdlib.h>
#include <string.h>


/* The global nil. */
Cell* Nil_Obj = nullptr;

/* Global #t and #f. */
Cell* True_Obj = nullptr;
Cell* False_Obj = nullptr;

/* Global End Of File object. */
Cell* EOF_Obj = nullptr;

/* Global tail call sentinel object. */
Cell* TCS_Obj = nullptr;

/* Global unspecified object. */
Cell* USP_Obj = nullptr;

/* Default ports. */
Cell* default_input_port  = nullptr;
Cell* default_output_port = nullptr;
Cell* default_error_port  = nullptr;


/* Initialize default input, output, and error ports. */
void init_default_ports(void)
{
    default_input_port  = make_cell_file_port("stdin",  stdin,  INPUT_STREAM, BK_FILE_TEXT);
    default_output_port = make_cell_file_port("stdout", stdout, OUTPUT_STREAM, BK_FILE_TEXT);
    default_error_port  = make_cell_file_port("stderr", stderr, OUTPUT_STREAM, BK_FILE_TEXT);
}


/*-------------------------------------------*
 *       Global singleton constructors       *
 *                                           *
 *  These constructors should be considered  *
 *  'private', and never directly accessed.  *
 * ------------------------------------------*/


static Cell* make_cell_nil__(void)
{
    Cell* nil_obj = GC_MALLOC_ATOMIC_UNCOLLECTABLE(sizeof(Cell));
    nil_obj->type = CELL_NIL;
    return nil_obj;
}


static Cell* make_cell_boolean__(const int the_boolean)
{
    Cell* v = GC_MALLOC_ATOMIC_UNCOLLECTABLE(sizeof(Cell));
    v->type = CELL_BOOLEAN;
    v->boolean_v = the_boolean;
    return v;
}


static Cell* make_cell_eof__(void)
{
    Cell* eof_obj = GC_MALLOC_ATOMIC_UNCOLLECTABLE(sizeof(Cell));
    eof_obj->type = CELL_EOF;
    return eof_obj;
}


static Cell* make_cell_tcs__(void)
{
    Cell* tcs_obj = GC_MALLOC_ATOMIC_UNCOLLECTABLE(sizeof(Cell));
    tcs_obj->type = CELL_TCS;
    return tcs_obj;
}


static Cell* make_cell_usp__(void)
{
    Cell* usp_obj = GC_MALLOC_ATOMIC_UNCOLLECTABLE(sizeof(Cell));
    usp_obj->type = CELL_UNSPEC;
    return usp_obj;
}


/* Initialize global singletons. */
void init_global_singletons(void)
{
    Nil_Obj = make_cell_nil__();
    True_Obj = make_cell_boolean__(1);
    False_Obj = make_cell_boolean__(0);
    EOF_Obj = make_cell_eof__();
    TCS_Obj = make_cell_tcs__();
    USP_Obj = make_cell_usp__();
}


/*------------------------------------*
 *       Cell type constructors       *
 * -----------------------------------*/


/* Thin wrapper that returns the singleton nil object. */
Cell* make_cell_nil(void)
{
    return Nil_Obj;
}


/* Thin wrapper that returns the singleton #t or #f object. */
Cell* make_cell_boolean(const int the_boolean)
{
    if (the_boolean == 1) return True_Obj;
    if (the_boolean == 0) return False_Obj;
    /* This would only be triggered from C code, so
     * raise an error and print diagnostic message. */
    fprintf(stderr, "Bad value passed to boolean constructor: make_cell_boolean()\n");
    fprintf(stderr, "Just return TrueObj or FalseObj\n");
    exit(EXIT_FAILURE);
}


/* Thin wrapper that returns the singleton EOF object. */
Cell* make_cell_eof(void)
{
    return EOF_Obj;
}


/* Thin wrapper that returns the singleton TCS object. */
Cell* make_cell_tcs(void)
{
    return TCS_Obj;
}


/* Thin wrapper that returns the singleton unspecified object. */
Cell* make_cell_usp(void)
{
    return USP_Obj;
}


/* Cell constructor for real-values numbers. */
Cell* make_cell_real(const long double the_real)
{
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


/* Cell constructor for integers < INT64_MAX. */
Cell* make_cell_integer(const long long int the_integer)
{
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


/* Cell constructor for rational numbers. Optionally handles reducing to the lowest terms. */
Cell* make_cell_rational(const long int numerator,
                         const long int denominator,
                         const bool simplify)
{
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


/* Cell constructor for complex numbers. */
Cell* make_cell_complex(Cell* real_part, Cell *imag_part)
{
    if (real_part->type == CELL_COMPLEX || imag_part->type == CELL_COMPLEX) {
        return make_cell_error(
            "Cannot have complex real or imaginary parts.",
            GEN_ERR);
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


/* Cell constructor for symbols. All symbols are first looked up in the intern hash. */
Cell* make_cell_symbol(const char* the_symbol)
{
    /* Lookup in symbol table first. */
    Cell* v = ht_get(symbol_table, the_symbol);
    if (v) {
        return v;
    }
    /* Not found, so construct the cell,
     * place in the table, then return it. */
    v = GC_MALLOC_ATOMIC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->sf_id = 0; /* Special form id zero by default. */
    v->type = CELL_SYMBOL;
    const char* canonical_name = ht_set(symbol_table, the_symbol, v);
    v->sym = (char*)canonical_name;
    return v;
}


/* Cell constructor for strings. Calculate and store byte length and char length, and set an ascii flag for faster
 * operations on pure-ascii strings. */
Cell* make_cell_string(const char* the_string)
{
    Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }

    const int32_t byte_len = (int)strlen(the_string);
    v->count = byte_len;

    /* Run the SWAR check. */
    if (is_pure_ascii(the_string, byte_len)) {
        v->ascii = 1;
        v->char_count = byte_len; /* For ASCII, bytes == chars. */
    } else {
        /* Scan string to count actual UTF-8 codepoints. */
        v->ascii = 0;
        v->char_count = string_length_utf8(the_string);
    }

    v->type = CELL_STRING;
    v->str = GC_strdup(the_string);
    return v;
}


/* Cell constructor for S-expressions. Not a user-type, but all builtin procedures expect the args to be wrapped
 * in one. */
Cell* make_cell_sexpr(void)
{
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


/* Cell constructor for chars. */
Cell* make_cell_char(const UChar32 the_char)
{
    Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_CHAR;
    v->char_v = the_char;
    return v;
}


/* Cell constructor for pairs and lists. */
Cell* make_cell_pair(Cell* car, Cell* cdr)
{
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


/* Cell constructor for vectors. */
Cell* make_cell_vector(void)
{
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


/* Cell constructor for bytevectors. */
Cell* make_cell_bytevector(const bv_t t)
{
    Cell* v = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }

    v->type = CELL_BYTEVECTOR;
    v->bv = GC_MALLOC(sizeof(byte_v));

    v->bv->type = t;
    v->bv->capacity = 8;
    v->count = 0;

    const size_t elem_size = BV_OPS[t].elem_size;
    v->bv->data = GC_MALLOC_ATOMIC(elem_size * v->bv->capacity);

    return v;
}


/* Cell constructor for error type. */
Cell* make_cell_error(const char* error_string, const err_t error_type)
{
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


/* Cell constructor for text or binary FILE-backed ports. */
Cell* make_cell_file_port(const char* path, FILE* fh, const stream_t stream, const backend_t backend)
{
    Cell* v = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->is_open = true;
    v->type = CELL_PORT;
    v->port = GC_MALLOC(sizeof(port_d));
    v->port->stream_t = stream;
    v->port->path = GC_strdup(path);
    v->port->backend_t = backend;
    v->port->fh = fh;
    v->port->vtable = &FileVTable;
    v->port->index = 0;
    return v;
}


/* Cell constructor for STRING and BYTEVECTOR memory-backed ports. */
Cell* make_cell_memory_port(const stream_t stream, const backend_t backend)
{
    Cell* v = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->is_open = true;
    v->type = CELL_PORT;
    v->port = GC_MALLOC(sizeof(port_d));
    v->port->stream_t = stream;
    v->port->path = nullptr;
    v->port->backend_t = backend;
    v->port->vtable = &MemoryVTable;
    /* Initialize the data store. */
    v->port->data = sb_new();
    v->port->index = 0;
    return v;
}


/* Cell constructor for bigints. */
Cell* make_cell_bigint(const char* s, const Cell* a,  const uint8_t base)
{
    Cell* v = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_BIGINT;
    v->exact = true;
    v->bi = GC_MALLOC(sizeof(mpz_t));
    if (s) {
        /* Set from string (from the parser). */
        const int status = mpz_init_set_str(*v->bi, s, base);
        if (status != 0) {
            return make_cell_error(
                "bigint construction failed!",
                GEN_ERR);
        }
    } else {
        /* Set from integer (type promotion). */
        mpz_init_set_si(*v->bi, a->integer_v);
    }
    return v;
}


/* Cell constructor for bigfloats. */
Cell* make_cell_bigfloat(const char* s)
{
    Cell* v = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_BIGINT;
    mpf_t n;
    mpf_init_set_str(n, s, 10);
    v->bf = &n;
    return v;
}


/* Cell constructor for promise type. */
Cell* make_cell_promise(Cell* expr, Lex* env)
{
    /* Allocate Cell. */
    Cell* v = GC_MALLOC(sizeof(Cell));
    if (!v) {
        fprintf(stderr, "ENOMEM: GC_MALLOC failed\n");
        exit(EXIT_FAILURE);
    }
    v->type = CELL_PROMISE;
    /* Allocate promise struct. */
    v->promise = GC_MALLOC(sizeof(promise));
    v->promise->expr = expr;
    /* Optimization - if expr is atomic, just set as DONE. */
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const int mask = CELL_BOOLEAN|CELL_CHAR|CELL_INTEGER|CELL_RATIONAL|
                     CELL_REAL|CELL_COMPLEX|CELL_STRING;
    if (expr->type & mask) {
        v->promise->status = DONE;
        v->promise->env = nullptr;
    } else {
        v->promise->status = READY;
        v->promise->env = env;
    }
    return v;
}


/* Cell constructor for stream type. */
Cell* make_cell_stream(Cell* head, Cell* tail_promise) {
    /* Safety check: Ensure the tail is actually a promise. */
    if (tail_promise->type != CELL_PROMISE) {
        return make_cell_error(
            "Stream tail must be a promise",
            TYPE_ERR);
    }

    Cell* v = GC_MALLOC(sizeof(Cell));
    v->type = CELL_STREAM;
    v->head = head;
    v->tail = tail_promise;
    return v;
}


/*------------------------------------------------*
 *    Cell accessors, destructors, and helpers    *
 * -----------------------------------------------*/


/* Add a cell to compound type S-expr or vector. */
Cell* cell_add(Cell* v, Cell* x)
{
    v->count++;
    v->cell = GC_REALLOC(v->cell, sizeof(Cell*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}


/* Adds a byte to a bytevector object. */
Cell* byte_add(Cell* bv, const int64_t value)
{
    BV_OPS[bv->bv->type].append(bv, value);
    return bv;
}


/* Recursively deep-copy all components of a Cell. Not every Cell type
 * is represented here, as not all Cell types would credibly need to
 * be copied. Deep-copying is expensive, and should be avoided if possible. */
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
        return v->boolean_v == 1 ? True_Obj : False_Obj;

    case CELL_CHAR:
        copy->char_v = v->char_v;
        break;

    case CELL_SYMBOL:
        /* Symbols are interned, just grab the pointer. */
        copy = (Cell*)v;
        break;

    case CELL_STRING:
        copy->str = GC_strdup(v->str);
        copy->count = v->count;
        copy->char_count = v->char_count;
        copy->ascii = v->ascii;
        break;

    case CELL_ERROR:
        copy->error_v = GC_strdup(v->error_v);
        copy->err_t = v->err_t;
        break;

    case CELL_PROC:
        if (v->is_builtin) {
            copy->is_builtin = true;
            copy->builtin = v->builtin;
            copy->f_name = GC_strdup(v->f_name);
        } else {
            copy->is_builtin = false;
            copy->lambda = GC_MALLOC(sizeof(lambda));
            copy->lambda->l_name = v->lambda->l_name ? GC_strdup(v->lambda->l_name) : nullptr;
            copy->lambda->formals = cell_copy(v->lambda->formals) ;
            copy->lambda->body = cell_copy(v->lambda->body);
            /* DO NOT copy environments; share the pointer. */
            copy->lambda->env = v->lambda->env;
        }
        break;

    case CELL_SEXPR:
    case CELL_VECTOR:
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
        /* FIXME: change fields to copy based on type. */
        copy->is_open = v->is_open;
        copy->port = GC_MALLOC(sizeof(port_d));
        copy->port->backend_t = v->port->backend_t;
        copy->port->stream_t = v->port->stream_t;
        copy->port->fh = v->port->fh;
        copy->port->path = GC_strdup(v->port->path);
        break;
    }

    case CELL_PROMISE: {
        copy->promise = GC_MALLOC(sizeof(promise));
        copy->promise->status = v->promise->status;
        copy->promise->expr = cell_copy(v->promise->expr);
        if (v->promise->env == nullptr) {
            copy->promise->env = nullptr;
        } else {
            /* DO NOT copy environments; share the pointer. */
            copy->promise->env = v->promise->env;
        }
        break;
    }

    case CELL_STREAM: {
        copy->head = cell_copy(v->head);
        copy->tail = cell_copy(v->tail);
        break;
    }

    case CELL_BIGINT:
        copy->bi = GC_MALLOC(sizeof(mpz_t));
        copy->bi = v->bi;
        break;

    /* Return the singleton objects instead of allocating for these types. */
    case CELL_NIL:
        return make_cell_nil();
    case CELL_TCS:
        return make_cell_tcs();
    case CELL_UNSPEC:
        return make_cell_usp();

    default:
        fprintf(stderr, "cell_copy: unknown type %d\n", v->type);
        return nullptr;
    }
    return copy;
}
