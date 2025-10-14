/*
 * 'symbols.h'
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

#ifndef COZENAGE_SYMBOLS_H
#define COZENAGE_SYMBOLS_H

#include "hash.h"

typedef enum {
    SF_ID_DEFINE = 1,
    SF_ID_QUOTE,
    SF_ID_LAMBDA,
    SF_ID_IF,
    SF_ID_WHEN,
    SF_ID_UNLESS,
    SF_ID_COND,
    SF_ID_IMPORT,
    SF_ID_LET,
    SF_ID_LET_STAR,
    SF_ID_LETREC,
    SF_ID_SET_BANG,
    SF_ID_BEGIN,
    SF_ID_AND,
    SF_ID_OR,
    /* 'else' isn't mapped to a function !!!
     *  If adding more special forms - keep this
     * 'else' last in the enum or things will break */
    SF_ID_ELSE
} SpecialFormID;

/* Declarations of Global Symbols for Special Forms */
extern Cell* G_define_sym;
extern Cell* G_quote_sym;
extern Cell* G_lambda_sym;
extern Cell* G_if_sym;
extern Cell* G_when_sym;
extern Cell* G_unless_sym;
extern Cell* G_cond_sym;
extern Cell* G_import_sym;
extern Cell* G_let_sym;
extern Cell* G_let_star_sym;
extern Cell* G_letrec_sym;
extern Cell* G_set_bang_sym;
extern Cell* G_begin_sym;
extern Cell* G_and_sym;
extern Cell* G_or_sym;
extern Cell* G_else_sym;

/* The global symbol table */
extern ht_table* symbol_table;

/* SF initialization function */
void init_special_forms(void);

#endif //COZENAGE_SYMBOLS_H
