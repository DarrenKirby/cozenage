/*
 * 'src/cell.h'
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


/* This file defines the Cell structure/tagged union which is used to
 * represent all 'Cozenage values' internally and externally. It also
 * defines associated structs for Cell values that require additional
 * fields such as lambdas, bytevectors, ports, and bigints. It defines
 * various type enums which are used by the Cell and other derived structs,
 * and it exports declarations for global singleton objects and the Cell
 * constructor functions which are defined in cell.c
 */

#ifndef COZENAGE_CELL_H
#define COZENAGE_CELL_H

#include "environment.h"
#include "buffer.h"

#include <stdio.h>
#include <unicode/umachine.h>
#include <gmp.h>


/* Cell_t type enum. */
typedef enum Cell_t : uint32_t {
    CELL_INTEGER    = 1 << 0,   /* An integer value. */
    CELL_RATIONAL   = 1 << 1,   /* A rational number. */
    CELL_REAL       = 1 << 2,   /* A real number. */
    CELL_COMPLEX    = 1 << 3,   /* A complex number. */

    CELL_BOOLEAN    = 1 << 4,   /* Boolean #t or #f singleton objects.*/
    CELL_CHAR       = 1 << 5,   /* A character. */
    CELL_STRING     = 1 << 6,   /* A string. */
    CELL_SYMBOL     = 1 << 7,   /* A symbol. */

    CELL_PAIR       = 1 << 8,   /* A cons cell. */
    CELL_NIL        = 1 << 9,   /* An empty list. */
    CELL_VECTOR     = 1 << 10,  /* A vector. */
    CELL_BYTEVECTOR = 1 << 11,  /* A byte vector. */

    CELL_EOF        = 1 << 12,  /* The singleton EOF object. */
    CELL_PROC       = 1 << 13,  /* A procedure object. Lambda or builtin. */
    CELL_PORT       = 1 << 14,  /* A port object. */
    CELL_ERROR      = 1 << 15,  /* An error object. */

    CELL_SEXPR      = 1 << 16,  /* An array of values, used internally. */
    CELL_TCS        = 1 << 17,  /* Tail Call Sentinel object. */
    //CELL_TRAMPOLINE = 1 << 18,  /* Return value to signal a builtin procedure tail-call. */
    CELL_UNSPEC     = 1 << 18,  /* Unspecified object. */

    CELL_BIGINT     = 1 << 19,  /* Arbitrary size/precision integer. */
    CELL_BIGRAT     = 1 << 20,  /* TODO: Arbitrary size/precision rational. */
    CELL_BIGFLOAT   = 1 << 21,  /* TODO: Arbitrary size/precision float. */
    CELL_PROMISE    = 1 << 22,  /* For delayed evaluation/streams. */

    CELL_STREAM     = 1 << 23,  /* A stream datatype for lazy evaluation. */
    CELL_MACRO      = 1 << 24   /* A non-hygienic 'defmacro' macro. */
} Cell_t;


/* LAMBDA
 * Anonymous and named lambda struct. */
typedef struct Lambda {
    char* l_name;     /* Name of builtin and named lambda procedures. */
    Cell* formals;    /* Must be symbols. */
    Cell* body;       /* S-expression for lambda. */
    Lex* env;         /* Closure environment. */
 } lambda;


/* PROMISE - used for delayed evaluation and streams. */
/* Delayed evaluation CELL_PROMISE. */
typedef enum P_Status_t : uint8_t {
    READY,     /* Unevaluated state. */
    LAZY,      /* Used by delay-force to trigger trampoline evaluation. */
    RUNNING,   /* Used to detect re-entrant promises. */
    DONE       /* An evaluated and cached value. */
} p_status_t;

/* Promise struct. */
typedef struct Promise {
    Cell* expr;         /* Either an unevaluated expression, or a final value. */
    p_status_t status;  /* State flag. */
    Lex* env;           /* Enclosing environment. */
} promise;


/* BYTEVECTORS - type enums and struct. */
/* Bytevector types. */
typedef enum BV_t : uint8_t {
    BV_U8,
    BV_S8,
    BV_U16,
    BV_S16,
    BV_U32,
    BV_S32,
    BV_U64,
    BV_S64,
    BV_F32, /* Not implemented yet. */
    BV_F64  /* Not implemented yet. */
} bv_t;

/* Bytevector struct. */
typedef struct ByteV {
    uint16_t capacity;
    bv_t type;
    void* data;
} byte_v;


 /* PORTS - type enums and struct.
  * Track whether the port is for input, output, or async,
  * and whether it operates on a file, string, or bytevector.
  */

/* enums for port types. */
typedef enum Stream_t : uint8_t  {
    INPUT_STREAM,
    OUTPUT_STREAM,
    ASYNC_STREAM
} stream_t;

typedef enum Backend_t : uint8_t  {
    BK_FILE_TEXT,
    BK_FILE_BINARY,
    BK_STRING,
    BK_BYTEVECTOR
} backend_t;

typedef struct PortInterface {
    ssize_t (*write)(const void* buf, size_t len, const Cell* port, int* err);
    ssize_t (*read)(void* buf, size_t len, const Cell* port, int* err);
    long (*tell)(const Cell* port, int* err);
    int (*seek)(const Cell* port, long offset, int* err);
    ssize_t (*getdelim)(char **lineptr, size_t *n,
                        int delim, const Cell *port, int *err);
    void (*close)(Cell *port);
} PortInterface;

/* Port data struct. */
typedef struct Port_d {
    char* path;           /* File path of associated fh. Set to null for data ports. */
    union {
        FILE* fh;         /* The associated file handle for a file port. */
        str_buf_t* data;  /* The data store for string and bv ports. */
    };
    const PortInterface *vtable;
    uint8_t backend_t;    /* The backing store (text file/bin file/string/bytevector). */
    uint8_t stream_t;     /* Stream type (input/output/async) */
    uint32_t index;       /* read/write pointer. */
} port_d;


/* ERROR - enum for error types. */
typedef enum Err_t : uint8_t  {
    GEN_ERR,    /* General, unspecified error. */
    FILE_ERR,   /* Error opening or closing a file or memory-backed port. */
    READ_ERR,   /* Error reading from a port. */
    WRITE_ERR,  /* Error writing to a port. */
    SYNTAX_ERR, /* Syntax error - generally only called from the parser or transformer. */
    ARITY_ERR,  /* Arity error - wrong number of args passed to procedure. */
    TYPE_ERR,   /* Type error - wrong type of arg passed to procedure. */
    INDEX_ERR,  /* Index error - invalid index passed for compound type. */
    VALUE_ERR,  /* Value error - Invalid value of correct type. */
    OS_ERR      /* OS error - mainly used in system library to report failed syscalls. */
} err_t;


/* Definition of the Cell struct/tagged union.
 *
 * This object represents all Scheme values, as well as some internal types.
 * Most Scheme values are stored directly, however, ports, lambdas, bytevectors,
 * and promises are represented by a pointer to their external structs to keep
 * the size at a reasonable 24 bytes. */
typedef struct Cell {
    Cell_t type;         /* type of data the Cell holds. */

    /* Union of type-specific flags and atomic values. */
    union {
        int count;       /* length of compound type. (n-bytes for strings)*/
        int len;         /* length of proper list (-1 for improper). */
        int err_t;       /* error type. */
        bool exact;      /* exact/inexact flag for numerics. */
        bool is_open;    /* port open/closed status. */
        bool is_builtin; /* proc object: builtin, or user-defined lambda. */
    };

    /* Union of type-specific data storage. */
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

        /* Streams */
        struct {
            Cell* head;        /* first member */
            Cell* tail;        /* second member */
        };

        /* Single-field types */
        Cell** cell;              /* for compound types (sexpr, vector) */
        char* error_v;            /* error string */
        long double real_v;       /* reals */
        int64_t integer_v;        /* integers */
        UChar32 char_v;           /* character literal */
        bool boolean_v;           /* boolean */

        /* Pointers to outside structs. */
        lambda* lambda;           /* -> lambda/defmacro struct. */
        port_d* port;             /* -> port struct */
        byte_v* bv;               /* -> bytevector struct */
        promise* promise;         /* -> promise struct */
        mpz_t* bi;                /* -> GMP integer */
        mpf_t* bf;                /* -> CELL_BIGFLOAT float */
    };
} Cell;


/* Declare the global singletons. */
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
Cell* make_cell_bytevector(bv_t t, size_t initial_size);
Cell* make_cell_symbol(const char* the_symbol);
Cell* make_cell_string(const char* the_string);
Cell* make_cell_sexpr(void);
Cell* make_cell_bigint(const char* s, const Cell* a, uint8_t base);
Cell* make_cell_bigfloat(const char* s);
Cell* make_cell_pair(Cell* car, Cell* cdr);
Cell* make_cell_error(const char* error_string, err_t error_type);
Cell* make_cell_file_port(const char* path, FILE* fh, stream_t stream, backend_t backend);
Cell* make_cell_memory_port(stream_t stream, backend_t backend);
Cell* make_cell_promise(Cell* expr, Lex* env);
Cell* make_cell_stream(Cell* head, Cell* tail_promise);
Cell* cell_add(Cell* v, Cell* x);
Cell* cell_copy(const Cell* v);
Cell* make_cell_bytevector_u8(void);
Cell* byte_add(Cell* bv, int64_t value);

#endif //COZENAGE_CELL_H
