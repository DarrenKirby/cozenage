/*
 * 'hash.h'
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

#ifndef COZENAGE_HASH_H
#define COZENAGE_HASH_H

#include <stdio.h>

/* Forward declare Cell */
typedef struct Cell Cell;

/* Constants for hash function */
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

/* Hash table item */
typedef struct {
    char* key;  /* key is NULL if this slot is empty */
    Cell* value;
} ht_item;

/* Hash table structure */
typedef struct hash_table {
    ht_item* items;     /* items array */
    size_t capacity;    /* size of items array */
    size_t count;       /* number of items in hash table */
} ht_table;

ht_table* ht_create(int initial_capacity);
void ht_destroy(ht_table* table);
Cell* ht_get(const ht_table* table, const char* key);
const char* ht_set(ht_table* table, const char* key, Cell* value);
void ht_delete(ht_table* table, const char* key);
size_t ht_length(const ht_table* table);

#endif //COZENAGE_HASH_H

/* Hash table iterator: create with ht_iterator, iterate with ht_next. */
/* Don't need this until implementing a hash/map/dict Scheme type */
// typedef struct {
//     const char* key;  // current key
//     Cell* value;      // current value
//
//     // Don't use these fields directly.
//     ht_table* _table;       // reference to hash table being iterated
//     size_t _index;    // current index into ht._entries
// } hti;
