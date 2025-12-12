/*
 * 'cell.h'
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

#ifndef COZENAGE_CELL_H
#define COZENAGE_CELL_H

#include "environment.h"

#include <stdio.h>
#include <unicode/umachine.h>
#include <gmp.h>


/* enum for error types. */
typedef enum : uint8_t  {
    GEN_ERR,
    FILE_ERR,
    READ_ERR,
    SYNTAX_ERR,
    ARITY_ERR,
    TYPE_ERR,
    INDEX_ERR,
    VALUE_ERR,
} err_t;

/* enums for port types. */
typedef enum : uint8_t  {
    INPUT_PORT,
    OUTPUT_PORT,
    ASYNC_PORT
} port_t;

typedef enum : uint8_t  {
    FILE_PORT,
    STRING_PORT,
    BV_PORT
} stream_t;

/* Cell_t type enum. */
typedef enum  : uint32_t {
    /* Primitive Scheme types. */
    CELL_INTEGER    = 1 << 0,   /* integer */
    CELL_RATIONAL   = 1 << 1,   /* rational */
    CELL_REAL       = 1 << 2,   /* real */
    CELL_COMPLEX    = 1 << 3,   /* complex number */

    CELL_BOOLEAN    = 1 << 4,   /* #t / #f */
    CELL_CHAR       = 1 << 5,   /* character */
    CELL_STRING     = 1 << 6,   /* string */
    CELL_SYMBOL     = 1 << 7,   /* symbol */

    CELL_PAIR       = 1 << 8,   /* cons cell */
    CELL_NIL        = 1 << 9,   /* '() empty list */
    CELL_VECTOR     = 1 << 10,  /* vector */
    CELL_BYTEVECTOR = 1 << 11,  /* byte vector */

    CELL_EOF        = 1 << 12,  /* EOF object */
    CELL_PROC       = 1 << 13,  /* procedure */
    CELL_PORT       = 1 << 14,  /* port */
    /* Internal-use types. */
    CELL_ERROR      = 1 << 15,  /* error */
    CELL_SEXPR      = 1 << 16,  /* an array of values */
    CELL_TCS        = 1 << 17,  /* Tail Call Sentinel object */
    CELL_MRV        = 1 << 18,  /* Multiple Return Value sentinel object */
    CELL_TRAMPOLINE = 1 << 19,  /* Trampoline object - returned from
                                   first-class procedures to signal a
                                   tail-call */
    CELL_UNSPEC     = 1 << 20,   /* Unspecified object */
    /* bigint/rat/float */
    CELL_BIGINT     = 1 << 21,   /* Arbitrary size/precision integer */
    CELL_BIGRAT     = 1 << 22,   /* Arbitrary size/precision rational */
    CELL_BIGFLOAT   = 1 << 23,   /* Arbitrary size/precision float */
} Cell_t;

/* Bytevector types. */
typedef enum : uint8_t {
    BV_U8,
    BV_S8,
    BV_U16,
    BV_S16,
    BV_U32,
    BV_S32
} bv_t;

/* Anonymous and named lambdas. */
typedef struct Lambda {
    char* l_name;     /* Name of builtin and named lambda procedures. */
    Cell* formals;    /* Must be symbols. */
    Cell* body;       /* S-expression for lambda. */
    Lex* env;         /* Closure environment. */
 } lambda;


 /* Ports */
typedef struct Port {
    char* path;       /* File path of associated fh (or string for string port). */
    FILE* fh;         /* The file handle. */
    int port_t;       /* Input or output. */
    uint8_t stream_t; /* Stream type */
} port;

typedef struct ByteV {
    uint16_t capacity;
    bv_t type;
    void* data;
} byte_v;

/* Definition of the Cell struct/tagged union. */
typedef struct Cell {
    Cell_t type;        /* type of data the Cell holds. */

    union {
        struct {
            int count;   /* length of compound type. (n-bytes for strings)*/
        };

        struct {
            int len;     /* length of proper list (-1 for improper). */
        };

        struct {
            int err_t;    /* error type. */
        };

        struct {
            bool exact;    /* exact/inexact flag for numerics. */
        };

        struct {
            bool is_open;  /* port open/closed status. */
        };

        struct {
            bool is_builtin;  /* proc object: builtin, or user-defined lambda. */
        };
    };

    union {
        struct {
            char* f_name;      /* name of builtin and named lambda procedure. */
            Cell* (*builtin)(const Lex*, const Cell*); /* builtin procedure. */
        };

        /* Pairs */
        struct {
            Cell* car;        /* first member */
            Cell* cdr;        /* second member */
        };

        /* Rationals */
        struct {
            long int num;     /* numerator */
            long int den;     /* denominator */
        };

        /* Complex numbers */
        struct {
            Cell* real;       /* real part */
            Cell* imag;       /* imaginary part */
        };

        /* Symbols */
        struct {
            char* sym;        /* Symbol text */
            int sf_id;        /* Special form id */
        };

        /* Strings */
        struct {
            char* str;           /* The string data */
            int32_t char_count;  /* Number of codepoints */
            bool ascii;          /* Just ASCII or Unicode? */
        };

        /* Single-field types */
        Cell** cell;              /* for compound types (sexpr, vector) */
        char* error_v;            /* error string */
        long double real_v;       /* reals */
        int64_t integer_v;        /* integers */
        UChar32 char_v;           /* character literal */
        bool boolean_v;           /* boolean */
        lambda* lambda;           /* Pointer to lambda struct */
        port* port;               /* Pointer to port struct */
        byte_v* bv;               /* Pointer to bytevector struct */
        mpz_t* bi;                /* -> CELL_BIGINT integer */
        mpf_t* bf;                /* -> CELL_BIGFLOAT float */
    };
} Cell;


/* Declare the global singletons */
extern Cell* Nil_Obj;
extern Cell* TCS_Obj;
extern Cell* EOF_Obj;
extern Cell* True_Obj;
extern Cell* False_Obj;
extern Cell* USP_Obj;
extern Cell* default_input_port;
extern Cell* default_output_port;
extern Cell* default_error_port;
void init_default_ports(void);
void init_global_singletons(void);


Cell* make_cell_nil(void);
Cell* make_cell_boolean(int the_boolean);
Cell* make_cell_eof(void);
Cell* make_cell_tcs(void);
Cell* make_cell_usp(void);
Cell* make_cell_real(long double the_real);
Cell* make_cell_integer(long long the_integer);
Cell* make_cell_rational(long int numerator, long int denominator, bool simplify);
Cell* make_cell_complex(Cell* real_part, Cell *imag_part);
Cell* make_cell_char(UChar32 the_char);
Cell* make_cell_vector(void);
Cell* make_cell_bytevector(bv_t t);
Cell* make_cell_symbol(const char* the_symbol);
Cell* make_cell_string(const char* the_string);
Cell* make_cell_sexpr(void);
Cell* make_cell_mrv(void);
Cell* make_cell_bigint(const char* s, const Cell* a, uint8_t base);
Cell* make_cell_bigfloat(const char* s);
Cell* make_cell_pair(Cell* car, Cell* cdr);
Cell* make_cell_error(const char* error_string, err_t error_type);
Cell* make_cell_port(const char* path, FILE* fh, int io_t, int stream_t);
Cell* cell_add(Cell* v, Cell* x);
Cell* cell_copy(const Cell* v);
Cell* cell_pop(Cell* v, int i);
Cell* cell_take(Cell* v, int i);
Cell* make_cell_bytevector_u8(void);
Cell* byte_add(Cell* bv, int64_t value);

#endif //COZENAGE_CELL_H
