/*
 * 'symbols.c'
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

#include "symbols.h"
#include <gc/gc.h>
#include <string.h>
#include <stdlib.h>


/* Declare the symbol table */
Sym_T* symbol_table = nullptr;

#define INITIAL_CAPACITY 8

/* Initialize the symbol table, and return a pointer to it. */
Sym_T* sym_table_initialize(void) {
    Sym_T* table = GC_MALLOC(sizeof(Sym_T));
    table->count = 0;
    table->capacity = INITIAL_CAPACITY;
    table->syms = GC_MALLOC(sizeof(char*) * table->capacity);
    table->vals = GC_MALLOC(sizeof(Cell*) * table->capacity);
    if (!table->syms || !table->vals) {
        fprintf(stderr, "ENOMEM: sym_table_initialize failed\n");
        exit(EXIT_FAILURE);
    }
    return table;
}

/* Retrieve a Cell* value from the symbol table */
Cell* symbol_table_lookup(const Sym_T* table, const char* sym) {
    if (!table || !sym) return nullptr;

    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->syms[i], sym) == 0) {
            return table->vals[i];
        }
    }
    /* Not found, return NULL */
    return nullptr;
}

/* Place a symbol Cell* value into the symbol table */
const char* symbol_table_put(Sym_T* table, const char* sym, const Cell* v) {
    /* Check if the table is full, and needs reallocation. */
    if (table->count == table->capacity) {
        table->capacity *= 2; /* Double the capacity */
        table->syms = GC_REALLOC(table->syms, sizeof(char*) * table->capacity);
        table->vals = GC_REALLOC(table->vals, sizeof(Cell*) * table->capacity);
        if (!table->syms || !table->vals) {
            fprintf(stderr, "ENOMEM: symbol_table_put failed\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Add the new symbol. */
    const char* stored_sym = GC_strdup(sym);
    table->syms[table->count] = GC_strdup(stored_sym);
    table->vals[table->count] = (Cell*)v;
    table->count++;
    /* Return the interned string for make_cell_symbol to use */
    return stored_sym;
}
