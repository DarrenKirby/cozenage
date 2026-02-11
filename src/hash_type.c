/*
 * 'src/hash_type.c'
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

#include "hash_type.h"
#include "cell.h"
#include "hash.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include  <gc/gc.h>

#define GHT_TOMBSTONE ((Cell*)0x1)


/* Thomas Wang's fast, avalanching, 64-bit integer hash function. */
uint64_t hash_int_key(uint64_t key)
{
    key = (~key) + (key << 21);
    key = key ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8);
    key = key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4);
    key = key ^ (key >> 28);
    key = key + (key << 31);
    return key;
}


uint64_t hash_real_key(const long double key)
{
    /* Ensure 0.0 and −0.0 hash to the same value. */
    if (key == 0.0L) {
        return hash_int_key(0);
    }
    /* Use a fixed hash for NaN. */
    if (isnan(key)) {
        return 0x9e3779b97f4a7c15ULL;
    }

    int exp;
    const long double mant = frexpl(key, &exp);

    const uint64_t h = hash_int_key((uint64_t)exp);
    uint64_t m;
    memcpy(&m, &mant, sizeof(uint64_t));
    return h ^ hash_int_key(m);
}


uint64_t hash_cell(const Cell* c)
{
    uint64_t h = 0;

    switch (c->type) {
        case CELL_STRING:
            h = hash_string_key(c->str);
            break;
        case CELL_SYMBOL:
            h = hash_string_key(c->sym);
            break;
        case CELL_INTEGER:
            h = hash_int_key((uint64_t)c->integer_v);
            break;
        case CELL_RATIONAL:
            h = hash_int_key(c->num);
            h ^= hash_int_key(c->den);
            break;
        case CELL_REAL:
            h = hash_real_key(c->real_v);
            break;
        case CELL_COMPLEX:
            h = hash_cell(c->real);
            h ^= hash_cell(c->imag);
            break;
        case CELL_BOOLEAN:
            h = hash_int_key(c->boolean_v);
            break;
        case CELL_CHAR:
            h = hash_int_key((uint64_t)c->char_v);
            break;
            /* Types are checked long before we get here,
             * so this _should_ never run. */
        default:
            fprintf(stderr, "Cannot hash type %d\n", c->type);
            exit(EXIT_FAILURE);
    }

    /* Mix in the type tag enum val, so string "hello"
     * and symbol 'hello return different hashes. */
    h ^= c->type;
    h *= FNV_PRIME;
    /* Debug print for hashes. */
    //printf("Hashed '%s' as : %llx\n", cell_to_string(c, MODE_REPL), h);
    return h;
}


bool equal_cell(const Cell* a, const Cell* b)
{
    if (a->type != b->type) {
        return false;
    }
    switch (a->type) {
        case CELL_STRING:
            if (a->count != b->count || a->char_count != b->char_count) {
                return false;
            }
            return memcmp(a->str, b->str, a->count) == 0 ? true : false;
        case CELL_SYMBOL:
            return a == b ? true : false;
        case CELL_INTEGER:
            return a->integer_v == b->integer_v ? true : false;
        case CELL_RATIONAL:
            return (a->num == b->num) && (a->den == b->den) ? true : false;
        case CELL_REAL:
            return a->real_v == b->real_v ? true : false;
        case CELL_COMPLEX:
            return equal_cell(a->real, b->real) && equal_cell(a->imag, b->imag);
        case CELL_BOOLEAN:
            return a->boolean_v == b->boolean_v ? true : false;
        case CELL_CHAR:
            return a->char_v == b->char_v ? true : false;
        default:
            return false;
    }
}


/* This is used by the user-level map/set
 * procedures to ensure sane key values. */
bool cell_is_hashable(const Cell* c)
{
    switch (c->type) {
        case CELL_STRING:
        case CELL_SYMBOL:
        case CELL_INTEGER:
        case CELL_RATIONAL:
        case CELL_REAL:
        case CELL_COMPLEX:
        case CELL_BOOLEAN:
        case CELL_CHAR:
            return true;
        default:
            return false;
    }
}


static ght_item GHT_DELETED_ITEM = { GHT_TOMBSTONE, NULL };


ght_table* ght_create(const size_t initial_capacity)
{
    /* Allocate space for hash table struct. */
    ght_table* table = GC_MALLOC(sizeof(ght_table));
    if (table == NULL) {
        fprintf(stderr, "ENOMEM: malloc failed in ght_create\n");
        exit(EXIT_FAILURE);
    }
    table->count = 0;
    table->capacity = initial_capacity;

    /* Allocate space for entry buckets, and zero them out. */
    table->items = GC_MALLOC(table->capacity * sizeof(ght_item));
    memset(table->items, 0, table->capacity * sizeof(ght_item));
    if (table->items == NULL) {
        fprintf(stderr, "ENOMEM: malloc failed in ght_create\n");
        exit(EXIT_FAILURE);
    }
    return table;
}


void ght_destroy(ght_table* table)
{
    GC_free(table->items);
    GC_free(table);
}


Cell* ght_get(const ght_table* table, const Cell* key)
{
    /* Note: proper type checking will be done at the
     * user-procedure level, so all calls here
     * will be type-checked and safe. */

    /* AND hash with capacity-1 to ensure it's within entries array. */
    const uint64_t hash = hash_cell(key);
    size_t index = hash & (uint64_t)(table->capacity - 1);

    /* Loop till we find an empty entry. */
    while (table->items[index].key != NULL) {
        /* Keep iterating if slot marked as deleted. */
        if (table->items[index].key != GHT_DELETED_ITEM.key) {
            if (equal_cell(table->items[index].key, key)) {
                /* Found key, return value. */
                return table->items[index].value;
            }
        }
        /* Key wasn't in this slot, move to next (linear probing). */
        index++;
        if (index >= table->capacity) {
            /* At end of entries array, wrap around. */
            index = 0;
        }
    }
    return nullptr;
}


/* Internal function to populate a slot with an item */
bool ght_set_item(ght_item* slot, const size_t capacity,
        Cell* key, Cell* value, size_t* p_length)
{
    if (!key || !value || !slot) {
        return false;
    }
    /* AND hash with capacity-1 to ensure it's within slot array. */
    const uint64_t hash = hash_cell(key);
    size_t index = hash & (uint64_t)(capacity - 1);

    /* Loop till we find an empty or deleted entry. */
    while (slot[index].key != NULL && slot[index].key != GHT_DELETED_ITEM.key) {
        if (equal_cell(key, slot[index].key)) {
            /* Found key (it already exists), update value. */
            slot[index].value = value;
            return true;
        }
        /* Key wasn't in this slot, move to next (linear probing). */
        index++;
        if (index >= capacity) {
            /* At end of slot array, wrap around. */
            index = 0;
        }
    }
    /* Didn't find the key, just insert it. */
    if (p_length != NULL) {
        (*p_length)++;
    }
    slot[index].key = key;
    slot[index].value = value;
    return true;
}


/* Expand hash table to twice its current size. */
static bool ght_resize(ght_table* table)
{
    /* Allocate new entries array. */
    const size_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity) {
        return false;  /* Overflow (capacity would be too big). */
    }
    ght_item* new_items = GC_malloc(new_capacity * sizeof(ght_item));
    if (new_items == NULL) {
        return false;
    }
    /* Iterate items, move all non-empty items to new table's items. */
    for (size_t i = 0; i < table->capacity; i++) {
        const ght_item item = table->items[i];
        /* Ensure we only move real entries, not empty or deleted ones. */
        if (item.key != nullptr && item.key != GHT_DELETED_ITEM.key) {
            ght_set_item(new_items, new_capacity, item.key,
                         item.value, nullptr);
        }
    }
    /* Free old items array and update this table's details. */
    GC_free(table->items);
    table->items = new_items;
    table->capacity = new_capacity;
    return true;
}


bool ght_set(ght_table* table, Cell* key, Cell* value)
{
    /* Resize table if load >= 0.7. */
    const size_t load = table->count * 100 / table->capacity;
    if (load >= 70) {
        if (!ght_resize(table)) {
            return false;
        }
    }
    /* Set entry and update count. */
    return ght_set_item(table->items, table->capacity, key, value, &table->count);
}


bool ght_delete(ght_table* table, const Cell* key)
{
    const uint64_t hash = hash_cell(key);
    size_t index = hash & (uint64_t)(table->capacity - 1);

    while (table->items[index].key != NULL) {
        if (table->items[index].key != GHT_DELETED_ITEM.key) {
            if (equal_cell(key, table->items[index].key)) {
                /* Found item. Do not free the key,
                 * just mark the slot as deleted. */
                table->items[index] = GHT_DELETED_ITEM;
                table->count--;
                return true;
            }
        }
        /* Move to next slot. */
        index++;
        if (index >= table->capacity) {
            index = 0;
        }
    }
    return false;
}


size_t ght_length(const ght_table* table)
{
    return table->count;
}


ghti ght_iterator(ght_table* table)
{
    ghti it;
    /* Just to shut up the linter. */
    it.key = nullptr;
    it.value = nullptr;

    it._table = table;
    it._index = 0;
    return it;
}


bool ght_next(ghti* it)
{
    /* Loop till we've hit end of items array. */
    const ght_table* table = it->_table;
    while (it->_index < table->capacity) {
        const size_t i = it->_index;
        it->_index++;
        if (table->items[i].key != NULL && table->items[i].key != GHT_DELETED_ITEM.key) {
            /* Found next non-empty item, update iterator key and value. */
            const ght_item entry = table->items[i];
            it->key = entry.key;
            it->value = entry.value;
            return true;
        }
    }
    return false;
}