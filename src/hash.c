/*
 * 'hash.c'
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

#include "hash.h"
#include "types.h"

#include <string.h>
#include <stdlib.h>
#include <gc/gc.h>


/* Note:
 * This hash table implementation is largely based on the work of Ben Hoyt[0] and James Routely[1].
 * I have essentially taken the design features I like from each of their implementations and
 * modified the result to fit the intended purpose in this interpreter. Big thanks to both of them!
 *
 * [0] https://benhoyt.com/writings/hash-table-in-c/
 * [1] https://github.com/jamesroutley/write-a-hash-table
 */

/* Mark a deleted item with sentinel 'tombstone' value so that the get
 * collision chain still works, but set can use the free slot. */
static ht_item HT_DELETED_ITEM = { (char*) -1, nullptr };

/* Initialize a hash table. Initial capacity is directly provided
 * by the caller, so that hash tables for different purposes can be
 * initialized to a sane size, but the argument must be a power of 2. */
ht_table* ht_create(const int initial_capacity)
{
    /* Allocate space for hash table struct. */
    ht_table* table = GC_MALLOC(sizeof(ht_table));
    if (table == NULL) {
        fprintf(stderr, "ENOMEM: malloc failed in ht_create\n");
        exit(EXIT_FAILURE);
    }
    table->count = 0;
    table->capacity = initial_capacity;

    /* Allocate space for entry buckets. */
    table->items = GC_MALLOC(table->capacity * sizeof(ht_item));
    if (table->items == NULL) {
        fprintf(stderr, "ENOMEM: malloc failed in ht_create\n");
        exit(EXIT_FAILURE);
    }
    return table;
}

/* Completely free table items and the table itself */
void ht_destroy(ht_table* table)
{
    /* First free allocated keys. */
    for (size_t i = 0; i < table->capacity; i++) {
        /* Skip nulls and tombstones */
        if (table->items[i].key != NULL && table->items[i].key != HT_DELETED_ITEM.key) {
            GC_free(table->items[i].key);
        }
    }
    /* Then free entries array and table itself. */
    GC_free(table->items);
    GC_free(table);
}

/* Return 64-bit FNV-1a hash for key (NUL-terminated). See description:
 * https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function */
static uint64_t get_hash_key(const char* key)
{
    uint64_t hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (uint64_t)(unsigned char)*p;
        hash *= FNV_PRIME;
    }
    return hash;
}

/* Given a hash table and key, return a pointer to the object or null */
Cell* ht_get(const ht_table* table, const char* key)
{
    /* AND hash with capacity-1 to ensure it's within entries array. */
    const uint64_t hash = get_hash_key(key);
    size_t index = hash & (uint64_t)(table->capacity - 1);

    /* Loop till we find an empty entry. */
    while (table->items[index].key != NULL) {
        /* Keep iterating if slot marked as deleted */
        if (table->items[index].key != HT_DELETED_ITEM.key) {
            if (strcmp(key, table->items[index].key) == 0) {
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
static const char* ht_set_item(ht_item* slot, const size_t capacity,
        const char* key, Cell* value, size_t* p_length)
{
    /* AND hash with capacity-1 to ensure it's within slot array. */
    const uint64_t hash = get_hash_key(key);
    size_t index = hash & (uint64_t)(capacity - 1);

    /* Loop till we find an empty or deleted entry. */
    while (slot[index].key != NULL && slot[index].key != HT_DELETED_ITEM.key) {
        if (strcmp(key, slot[index].key) == 0) {
            /* Found key (it already exists), update value. */
            slot[index].value = value;
            return slot[index].key;
        }
        /* Key wasn't in this slot, move to next (linear probing). */
        index++;
        if (index >= capacity) {
            /* At end of slot array, wrap around. */
            index = 0;
        }
    }
    /* Didn't find the key, allocate+copy if needed, then insert it. */
    if (p_length != NULL) {
        key = GC_strdup(key);
        if (key == NULL) {
            return nullptr;
        }
        (*p_length)++;
    }
    slot[index].key = (char*)key;
    slot[index].value = value;
    return key;
}

/* Expand hash table to twice its current size. */
static bool ht_resize(ht_table* table)
{
    /* Allocate new entries array. */
    const size_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity) {
        return false;  /* overflow (capacity would be too big). */
    }
    ht_item* new_items = GC_malloc(new_capacity * sizeof(ht_item));
    if (new_items == NULL) {
        return false;
    }
    /* Iterate items, move all non-empty items to new table's items. */
    for (size_t i = 0; i < table->capacity; i++) {
        const ht_item item = table->items[i];
        /* Ensure we only move real entries, not empty or deleted ones. */
        if (item.key != nullptr && item.key != HT_DELETED_ITEM.key) {
            ht_set_item(new_items, new_capacity, item.key,
                         item.value, nullptr);
        }
    }
    /* Free old items array and update this table's details. */
    GC_free(table->items);
    table->items = new_items;
    table->capacity = new_capacity;
    return true;
}

const char* ht_set(ht_table* table, const char* key, Cell* value)
{
    if (value == NULL) {
        return nullptr;
    }
    /* Resize table if load >= 70 (0.7) */
    const size_t load = table->count * 100 / table->capacity;
    if (load >= 70) {
        if (!ht_resize(table)) {
            return nullptr;
        }
    }
    /* Set entry and update count. */
    return ht_set_item(table->items, table->capacity, key, value,
                        &table->count);
}

void ht_delete(ht_table* table, const char* key)
{
    const uint64_t hash = get_hash_key(key);
    size_t index = hash & (uint64_t)(table->capacity - 1);

    while (table->items[index].key != NULL) {
        if (table->items[index].key != HT_DELETED_ITEM.key) {
            if (strcmp(key, table->items[index].key) == 0) {
                /* Found item. Free its key (which we own)
                 * and mark the slot as deleted. */
                GC_free(table->items[index].key);
                table->items[index] = HT_DELETED_ITEM;
                table->count--;
                return; /* Item deleted, we are done. */
            }
        }
        /* Move to next slot */
        index++;
        if (index >= table->capacity) {
            index = 0;
        }
    }
}

size_t ht_length(const ht_table* table)
{
    return table->count;
}

/* This iterator (written by Ben Hoyt) is not needed yet, but may be useful
 * when implementing a hash/map/dict scheme type */
hti ht_iterator(ht_table* table)
{
    hti it;
    it._table = table;
    it._index = 0;
    return it;
}

bool ht_next(hti* it)
{
    /* Loop till we've hit end of items array. */
    const ht_table* table = it->_table;
    while (it->_index < table->capacity) {
        const size_t i = it->_index;
        it->_index++;
        if (table->items[i].key != NULL && table->items[i].key != HT_DELETED_ITEM.key) {
            /* Found next non-empty item, update iterator key and value. */
            const ht_item entry = table->items[i];
            it->key = entry.key;
            it->value = entry.value;
            return true;
        }
    }
    return false;
}
