/*
 * 'src/maps.c'
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

#include "maps.h"
#include "sets.h"
#include "types.h"



Cell* builtin_make_map(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 2, "make-map");
    if (err) return err;

    if (a->count % 2 != 0) {
        return make_cell_error(
            "make-map: requires even number of args",
            VALUE_ERR);
    }
    for (int i = 0; i < a->count; i += 2) {
        if (!cell_is_hashable(a->cell[i])) {
            return make_cell_error(
            fmt_err("make-map: arg type '%s' is not a hashable",
                cell_type_name(a->cell[i]->type)),
            TYPE_ERR);
        }
    }
    /* Confirmed args are OK, just feed right to map constructor. */
    return make_cell_map(a);
}


/* (map-copy map)
 * Copies the structure of map into a newly allocated map object. Note that this is not a deep-copy. The keys and
 * values themselves will not be copied, just the pointers to them. */
Cell* builtin_map_copy(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "map-copy");
    if (err) return err;

    const Cell* map = a->cell[0];
    if (map->type != CELL_MAP) {
        return make_cell_error(
            "map-copy: arg must be a map",
            TYPE_ERR);
    }

    return copy_hash_table(map);
}

//Cell* builtin_map_clear(const Lex* e, const Cell* a) {}


/* (map-get map key)
 * (map-get map key default_val)
 * Returns the value indexed by key in map, or else raises an index error. If an optional third argument is passed,
 * then that value will be returned if the key does not exist in map. */
Cell* builtin_map_get(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3, "map-get");
    if (err) return err;

    const Cell* map = a->cell[0];
    const Cell* obj = a->cell[1];

    if (map->type != CELL_MAP) {
        return make_cell_error(
            "map-get: arg1 must be a map",
            TYPE_ERR);
    }
    if (!cell_is_hashable(obj)) {
        return make_cell_error(
            fmt_err("map-get: arg type '%s' is not a hashable",
                cell_type_name(obj->type)),
            TYPE_ERR);
    }

    Cell* val = ght_get(map->table, obj);
    if (!val) {
        if (a->count == 3) {
            return a->cell[2];
        }
        return make_cell_error("map-get: object not found in map", INDEX_ERR);
    }

    return val;
}


/* (map-add map obj1 obj2)
 * Adds obj1 as key, and obj2 as value to the map specified by map. Raises type error if obj1 is not hashable. */
Cell* builtin_map_add(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 3, "map-add!");
    if (err) return err;

    Cell* map = a->cell[0];
    if (map->type != CELL_MAP) {
        return make_cell_error(
            "map-add!: arg1 must be a map",
            TYPE_ERR);
    }

    if (!cell_is_hashable(a->cell[1])) {
        return make_cell_error(
            fmt_err("map-add!: arg type '%s' is not a hashable", cell_type_name(map->type)),
            TYPE_ERR);
    }

    if (!ght_set(map->table, a->cell[1], a->cell[2])) {
        return make_cell_error(
            fmt_err("map-add!: failed to add new items to map"),
            GEN_ERR);
    }
    return map;
}


/* (map-remove! set obj)
 * (map-remove! set obj sym)
 * Removes the key obj and associated value from map, and returns the mutated map. Raises an index error if object is not
 * a member of map. If an optional symbol (any symbol) is passed in fourth position, the procedure will not raise the
 * index error, but rather silently return. */
Cell* builtin_map_remove(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3, "map-remove!");
    if (err) return err;

    Cell* map = a->cell[0];
    if (map->type != CELL_MAP) {
        return make_cell_error(
            "map-remove!: arg1 must be a map",
            TYPE_ERR);
    }

    const bool removed = ght_delete(map->table, a->cell[1]);

    if (a->count == 2 && !removed) {
        return make_cell_error(
            "map-remove!: arg 2 not member of map",
            INDEX_ERR);
    }
    if (a->count == 3 && a->cell[2]->type != CELL_SYMBOL) {
        return make_cell_error(
            "map-remove!: arg 3 must be a symbol",
            INDEX_ERR);
    }
    return map;
}


//Cell* builtin_map_keys(const Lex* e, const Cell* a) {}

//Cell* builtin_map_values(const Lex* e, const Cell* a) {}

//Cell* builtin_map_to_alist(const Lex* e, const Cell* a) {}

//Cell* builtin_alist_to_map(const Lex* e, const Cell* a) {}

//Cell* builtin_map_keys_map(const Lex* e, const Cell* a) {}

//Cell* builtin_map_keys_foreach(const Lex* e, const Cell* a) {}

//Cell* builtin_map_values_map(const Lex* e, const Cell* a) {}

//Cell* builtin_map_values_foreach(const Lex* e, const Cell* a) {}

//Cell* builtin_map_items_map(const Lex* e, const Cell* a) {}

//Cell* builtin_map_items_foreach(const Lex* e, const Cell* a) {}
