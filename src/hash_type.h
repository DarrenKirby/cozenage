/*
 * 'src/hash_type.h'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2026 Darren Kirby <darren@dragonbyte.ca>
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


#ifndef COZENAGE_HASH_TYPE_H
#define COZENAGE_HASH_TYPE_H

#include  <stdio.h>


/* Forward declare Cell. */
typedef struct Cell Cell;

/* Constants for hash function. */
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

/* Hash table item. */
typedef struct {
    Cell* key;     /* NULL = empty, TOMBSTONE = deleted */
    Cell* value;   /* value is ignored for sets */
} ght_item;


/* Hash table structure */
typedef struct Ght_Table{
    ght_item* items;
    size_t capacity;   /* must be power of two */
    size_t count;
} ght_table;


/* Hash table iterator: create with ght_iterator, iterate with ght_next. */
typedef struct {
    Cell* key;        /* Current key. */
    Cell* value;      /* Current value. */

    /* Don't use these fields directly. */
    ght_table* _table; /* Reference to hash table being iterated. */
    size_t _index;     /* Current index into ht._entries. */
} ghti;


ght_table* ght_create(size_t initial_capacity);
void ght_destroy(ght_table* table);
Cell* ght_get(const ght_table* table, const Cell* key);
bool ght_set(ght_table* table, Cell* key, Cell* value);
bool ght_delete(ght_table* table, const Cell* key);
size_t ght_length(const ght_table* table);
ghti ght_iterator(ght_table* table);
bool ght_next(ghti* it);
bool cell_is_hashable(const Cell* c);

#endif //COZENAGE_HASH_TYPE_H