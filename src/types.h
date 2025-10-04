/*
 * 'src/types.h'
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

#ifndef COZENAGE_TYPES_H
#define COZENAGE_TYPES_H

#include "parser.h"
#include "environment.h"
#include <stdio.h>
#include <unicode/umachine.h>


/* Convenience macros for readability */
#define CHECK_ARITY_EXACT(a, n) \
check_arg_arity((a), (n), -1, -1)

#define CHECK_ARITY_MIN(a, n) \
check_arg_arity((a), -1, (n), -1)

#define CHECK_ARITY_MAX(a, n) \
check_arg_arity((a), -1, -1, (n))

#define CHECK_ARITY_RANGE(a, lo, hi) \
check_arg_arity((a), -1, (lo), (hi))

/* enum for error types */
typedef enum {
    GEN_ERR,
    FILE_ERR,
    READ_ERR,
    SYNTAX_ERR,
    ARITY_ERR,
    TYPE_ERR,
    INDEX_ERR,
    VALUE_ERR,
} err_t;

/* enums for port types */
typedef enum {
    INPUT_PORT,
    OUTPUT_PORT
} port_t;

typedef enum {
    TEXT_PORT,
    BINARY_PORT
} stream_t;

/* Cell_t type enum */
typedef enum {
    VAL_INT     = 1 << 0,   /* integer */
    VAL_RAT     = 1 << 1,   /* rational */
    VAL_REAL    = 1 << 2,   /* real */
    VAL_COMPLEX = 1 << 3,   /* complex number */

    VAL_BOOL    = 1 << 4,   /* #t / #f */
    VAL_CHAR    = 1 << 5,   /* character */
    VAL_STR     = 1 << 6,   /* string */
    VAL_SYM     = 1 << 7,   /* symbol */

    VAL_PAIR    = 1 << 8,   /* cons cell */
    VAL_NIL     = 1 << 9,   /* '() empty list */
    VAL_VEC     = 1 << 10,  /* vector */
    VAL_BYTEVEC = 1 << 11,  /* byte vector */

    VAL_SEXPR   = 1 << 12,  /* an array of values, used internally */
    VAL_PROC    = 1 << 13,  /* procedure */
    VAL_PORT    = 1 << 14,  /* port */
    VAL_CONT    = 1 << 15,  /* continuation (maybe) */

    VAL_ERR     = 1 << 16,   /* error */
    VAL_EOF     = 1 << 17    /* EOF object */
} Cell_t;

/* Definition of the Cell struct/tagged union */
typedef struct Cell {
    Cell_t type;        /* type of data the Cell holds */

    union {
        struct {
            int count;   /* length of compound type */
        };
        struct {
            int len;     /* length of proper list (-1 for improper) */
        };
        struct {
            int err_t;    /* error type */
        };
        struct {
            bool exact;    /* exact/inexact flag for numerics*/
        };
        struct {
            bool is_open;  /* port open/closed status */
        };
        struct {
            bool quoted;   /* whether a symbol has been quoted or not */
        };
        struct {
            bool is_builtin;
        };
    };

    union {
        /* Anonymous and named lambdas */
        struct {
            char* l_name;     /* name of builtin and named lambda procedures */
            Cell* formals;    /* non-NULL → user-defined lambda */
            Cell* body;       /* S-expression for lambda */
            Lex* env;         /* closure environment */
        };
        /* Ports */
        struct {
            char* path;       /* file path of associated fh */
            FILE* fh;         /* the file handle */
            int port_t;       /* input or output */
            int stream_t;     /* binary or textual */
        };
        /* builtin procedures */
        struct {
            char* f_name;      /* name of builtin and named lambda procedure */
            Cell* (*builtin)(const Lex*, const Cell*); /* builtin procedure */
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
        /* Single-field types */
        Cell** cell;          /* for compound types (sexpr, vector, bytevector) */
        char* sym;            /* symbols */
        char* str;            /* strings */
        char* err;            /* error string */
        long double r_val;    /* reals */
        long long int i_val;  /* integers */
        UChar32 c_val;        /* character literal */
        bool b_val;           /* boolean */
    };
} Cell;

/* For named chars */
typedef struct {
    const char* name;
    UChar32     codepoint;
} NamedChar;

extern Cell* val_nil;  /* declare the global singleton */
extern Cell* default_input_port;
extern Cell* default_output_port;
extern Cell* default_error_port;
void init_default_ports(void);
typedef Cell* (*BuiltinFn)(const Lex* e, const Cell* args);

Cell* make_val_real(long double n);
Cell* make_val_int(long long n);
Cell* make_val_rat(long int num, long int den, bool simplify);
Cell* make_val_complex(Cell* real, Cell *imag);
Cell* make_val_bool(int b);
Cell* make_val_char(UChar32 c);
Cell* make_val_vect(void);
Cell* make_val_bytevec(void);
Cell* make_val_sym(const char* s);
Cell* make_val_str(const char* s);
Cell* make_val_sexpr(void);
Cell* make_val_nil(void);
Cell* make_val_pair(Cell* car, Cell* cdr);
Cell* make_val_err(const char* m, err_t t);
Cell* make_val_port(const char* path, FILE* fh, int io_t, int stream_t);
Cell* make_val_eof(void);
Cell* cell_add(Cell* v, Cell* x);
Cell* cell_copy(const Cell* v);
Cell* cell_pop(Cell* v, int i);
Cell* cell_take(Cell* v, int i);
const char* cell_type_name(int t);
const char* cell_mask_types(int mask);
Cell* check_arg_types(const Cell* a, int mask);
Cell* check_arg_arity(const Cell* a, int exact, int min, int max);
void numeric_promote(Cell** lhs, Cell** rhs);
Cell* make_sexpr_len1(const Cell* a);
Cell* make_sexpr_len2(const Cell* a, const Cell* b);
Cell* make_sexpr_len4(const Cell* a, const Cell* b, const Cell* c, const Cell* d);
Cell* make_sexpr_from_list(Cell* v);
Cell* make_sexpr_from_array(int count, Cell** cells);
Cell* flatten_sexpr(const Cell* sexpr);
bool cell_is_real_zero(const Cell* c);
bool cell_is_integer(const Cell* c);
bool cell_is_real(const Cell* c);
bool cell_is_positive(const Cell* c);
bool cell_is_negative(const Cell* c);
bool cell_is_odd(const Cell* c);
bool cell_is_even(const Cell* c);
Cell* negate_numeric(const Cell* x);
Cell* simplify_rational(Cell* v);
void complex_apply(BuiltinFn fn, const Lex* e, Cell* result, const Cell* rhs);
long double cell_to_long_double(const Cell* c);
Cell* make_cell_from_double(long double d);
char* GC_strdup(const char* s);
char* GC_strndup(const char* s, size_t n);
int compare_named_chars(const void* key, const void* element);
const NamedChar* find_named_char(const char* name);
Cell* list_get_nth_cell_ptr(const Cell* list, long n);
char* convert_to_utf8(const UChar* ustr);
UChar* convert_to_utf16(const char* str);

#endif //COZENAGE_TYPES_H
