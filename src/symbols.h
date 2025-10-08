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

#include "cell.h"

/* Symbol interning lookup table */
/* Just parallel arrays for now. Will optimize this later */
typedef struct Symbol_table {
    int count;        /* How many symbols are currently stored */
    int capacity;     /* How many slots are allocated in memory */
    char** syms;      /* symbol names */
    Cell** vals;      /* values */
} Sym_T;

extern Sym_T* symbol_table;

Sym_T* sym_table_initialize(void);
Cell* symbol_table_lookup(const Sym_T* table, const char* sym);
const char* symbol_table_put(Sym_T* table, const char* sym, const Cell* v);

#endif //COZENAGE_SYMBOLS_H
