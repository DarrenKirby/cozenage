/*
 * 'src/symbols.h'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 Darren Kirby <darren@dragonbyte.ca>
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

#ifndef COZENAGE_SYMBOLS_H
#define COZENAGE_SYMBOLS_H

#include "hash.h"
#include "environment.h"

#include <stdint.h>


typedef enum : int8_t {
    /* These are the SFs that are implemented primitively. */
    SF_ID_DEFINE = 1,
    SF_ID_QUOTE,
    SF_ID_LAMBDA,
    SF_ID_IF,
    SF_ID_IMPORT,
    SF_ID_LET,
    SF_ID_LETREC,
    SF_ID_SET_BANG,
    SF_ID_BEGIN,
    SF_ID_AND,
    SF_ID_DELAY,
    SF_ID_DELAY_FORCE,
    SF_ID_STREAM,
    SF_ID_DEFMACRO,
    SF_ID_DEBUG,
    /* These are the SFs implemented as transforms. */
    SF_ID_LET_STAR = 50,
    SF_ID_OR,
    SF_ID_WHEN,
    SF_ID_UNLESS,
    SF_ID_COND,
    SF_ID_ELSE,
    SF_ID_CASE,
    SF_ID_LETREC_STAR,
    SF_ID_DO
} SpecialFormID;


/* Declarations of Global Symbols for Special Forms */
extern Cell* G_define_sym;
extern Cell* G_quote_sym;
extern Cell* G_lambda_sym;
extern Cell* G_if_sym;
extern Cell* G_when_sym;
extern Cell* G_unless_sym;
extern Cell* G_cond_sym;
extern Cell* G_case_sym;
extern Cell* G_import_sym;
extern Cell* G_let_sym;
extern Cell* G_let_star_sym;
extern Cell* G_letrec_star_sym;
extern Cell* G_letrec_sym;
extern Cell* G_set_bang_sym;
extern Cell* G_begin_sym;
extern Cell* G_and_sym;
extern Cell* G_or_sym;
extern Cell* G_do_sym;
extern Cell* G_arrow_sym;
extern Cell* G_else_sym;
extern Cell* G_defmacro_sym;
extern Cell* G_debug_sym;
extern Cell* G_quasiquote_sym;
extern Cell* G_unquote_sym;
extern Cell* G_unquote_splicing_sym;


/* The global symbol table. */
extern ht_table* symbol_table;

/* SF initialization function. */
void init_special_forms(void);

/* Builtin symbol procedures. */
Cell* builtin_string_to_symbol(const Lex* e, const Cell* a);
Cell* builtin_symbol_to_string(const Lex* e, const Cell* a);
Cell* builtin_symbol_equal_pred(const Lex* e, const Cell* a);
Cell* builtin_features(const Lex* e, const Cell* a);

#endif //COZENAGE_SYMBOLS_H
