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
#include "eval.h"


/* (make-map obj1 obj2 ...)
 * Returns a newly allocated map object made from key -> value pairs supplied as args. */
Cell* builtin_make_map(const Lex* e, const Cell* a)
{
    (void)e;
    if (a->count == 0) {
        /* No args: return an empty map. */
        return make_cell_hash(nullptr);
    }

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
    return make_cell_hash(a);
}


/* (map-copy map)
 * Copies the structure of map into a newly allocated map object. Note that this is not a deep-copy. The keys and
 * values themselves will not be copied, just the pointers to them. */
Cell* builtin_map_copy(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "map-copy");
    if (err) return err;

    const Cell* map = a->cell[0];
    if (map->type != CELL_HASH) {
        return make_cell_error(
            "map-copy: arg must be a map",
            TYPE_ERR);
    }
    return copy_hash_table(map);
}


/* (map-clear! map)
 * Returns the same map object with all keys and values cleared. */
Cell* builtin_map_clear(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "map-clear!");
    if (err) return err;

    Cell* map = a->cell[0];
    if (map->type != CELL_HASH) {
        return make_cell_error(
            "map-clear!: arg must be a map",
            TYPE_ERR);
    }
    return clear_hash_table(map);
}


/* (map-get map key)
 * (map-get map key default_val)
 * Returns the value indexed by key in map, or else raises an index error. If an optional third argument is passed,
 * then that value will be returned if the key does not exist in map. */
Cell* builtin_map_get(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3, "map-get");
    if (err) return err;

    const Cell* map = a->cell[0];
    const Cell* obj = a->cell[1];

    if (map->type != CELL_HASH) {
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
        return make_cell_error(
            "map-get: object not found in map",
            INDEX_ERR);
    }
    return val;
}


/* (map-add! map obj1 obj2)
 * Adds obj1 as key, and obj2 as value to the map specified by map. Raises type error if obj1 is not hashable. */
Cell* builtin_map_add(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 3, "map-add!");
    if (err) return err;

    Cell* map = a->cell[0];
    if (map->type != CELL_HASH) {
        return make_cell_error(
            "map-add!: arg1 must be a map",
            TYPE_ERR);
    }

    if (!cell_is_hashable(a->cell[1])) {
        return make_cell_error(
        fmt_err("map-add!: arg type '%s' is not hashable",
                 cell_type_name(a->cell[1]->type)),
                 TYPE_ERR);
    }

    if (!ght_set(map->table, a->cell[1], a->cell[2])) {
        return make_cell_error(
            fmt_err("map-add!: failed to add new items to map"),
            GEN_ERR);
    }
    return map;
}


/* (map-remove! map obj)
 * (map-remove! map obj sym)
 * Removes the key obj and associated value from map, and returns the mutated map. Raises an index error if object is not
 * a member of map. If an optional symbol (any symbol) is passed as third arg, the procedure will not raise the
 * index error, but rather just silently return. */
Cell* builtin_map_remove(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3, "map-remove!");
    if (err) return err;

    Cell* map = a->cell[0];
    if (map->type != CELL_HASH) {
        return make_cell_error(
            "map-remove!: arg1 must be a map",
            TYPE_ERR);
    }
    if (!cell_is_hashable(a->cell[1])) {
        return make_cell_error(
            "map-remove!: arg2 is not hashable (therefore will not be in map)",
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


/* (map-keys map)
 * Returns a list containing all keys in map. */
Cell* builtin_map_keys(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "map-keys");
    if (err) return err;
    const Cell* map = a->cell[0];
    if (map->type != CELL_HASH) {
        return make_cell_error(
            "map-keys: arg must be a map",
            TYPE_ERR);
    }

    Cell* r = make_cell_sexpr();
    ghti it = ght_iterator(map->table);
    while (ght_next(&it)) {
        cell_add(r, it.key);
    }

    return make_list_from_sexpr(r);
}


/* (map-values map)
 * Returns a list containing all values in map. */
Cell* builtin_map_values(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "map-values");
    if (err) return err;
    const Cell* map = a->cell[0];
    if (map->type != CELL_HASH) {
        return make_cell_error(
            "map-values: arg must be a map",
            TYPE_ERR);
    }

    Cell* r = make_cell_sexpr();
    ghti it = ght_iterator(map->table);
    while (ght_next(&it)) {
        cell_add(r, it.value);
    }

    return make_list_from_sexpr(r);
}


/* (map->alist map)
 * Returns an alist containing all keys and values found in map, ie: it returns a list where each car field is a
 * (key . value) dotted pair. */
Cell* builtin_map_to_alist(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "map->alist");
    if (err) return err;
    const Cell* map = a->cell[0];
    if (map->type != CELL_HASH) {
        return make_cell_error(
            "map->alist: arg must be a map",
            TYPE_ERR);
    }

    Cell* r = make_cell_sexpr();
    ghti it = ght_iterator(map->table);
    while (ght_next(&it)) {
        Cell* p = make_cell_pair(it.key, it.value);
        cell_add(r, p);
    }

    return make_list_from_sexpr(r);
}


/* (alist->map alist)
 * Returns a newly allocated map object where all key -> value pairs are derived from association list car fields.
 * Raises type error if alist car fields are not dotted pairs. */
Cell* builtin_alist_to_map(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "alist->map");
    if (err) return err;
    const Cell* alist = a->cell[0];
    if (alist->type != CELL_PAIR || a->cell[0]->count == -1) {
        return make_cell_error(
            "alist->map: arg must be an association list",
            TYPE_ERR);
    }

    Cell* r = make_cell_sexpr();
    while (alist->cdr) {
        /* This also enforces an even number of objects fed to make_cell_hash(). */
        if (alist->car->type != CELL_PAIR) {
            return make_cell_error(
                "alist->map: car field of list is not a dotted pair",
                TYPE_ERR);
        }
        cell_add(r, alist->car->car);
        cell_add(r, alist->car->cdr);
        alist = alist->cdr;
    }
    return make_cell_hash(r);
}


/* (map-keys-map proc map)
 * Returns a list containing the results of applying proc to each key in map. The order of the procedure applications is
 * indeterminate. The proc must accept exactly one arg, which will be each key from the map. */
Cell* builtin_map_keys_map(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "map-keys-map");
    if (err) return err;

    const Cell* proc = a->cell[0];
    const Cell* map = a->cell[1];

    if (proc->type != CELL_PROC) {
        return make_cell_error(
            "map-keys-map: arg1 must be a procedure",
            TYPE_ERR);
    }
    if (map->type != CELL_HASH) {
        return make_cell_error(
            "map-keys-map: arg2 must be a map",
            TYPE_ERR);
    }
    return map_logic(proc, map, e, true, true);
}


/* (map-keys-foreach proc map)
 * Applies proc to each key in map. Unlike map-keys-map, this procedure is used for side effects. Returns #void. The
 * order of the procedure applications is indeterminate. The proc must accept exactly one arg, which will be each key
 * from the map. */
Cell* builtin_map_keys_foreach(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "map-keys-foreach");
    if (err) return err;

    const Cell* proc = a->cell[0];
    const Cell* map = a->cell[1];

    if (proc->type != CELL_PROC) {
        return make_cell_error(
            "map-keys-foreach: arg1 must be a procedure",
            TYPE_ERR);
    }
    if (map->type != CELL_HASH) {
        return make_cell_error(
            "map-keys-foreach: arg2 must be a map",
            TYPE_ERR);
    }
    return map_logic(proc, map, e, true, false);
}


/* (map-values-map proc map)
 * Returns a list containing the results of applying proc to each value in map. The order of the procedure applications
 * is indeterminate. The proc must accept exactly one arg, which will be each value from the map. */
Cell* builtin_map_values_map(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "map-values-map");
    if (err) return err;

    const Cell* proc = a->cell[0];
    const Cell* map = a->cell[1];

    if (proc->type != CELL_PROC) {
        return make_cell_error(
            "map-values-map: arg1 must be a procedure",
            TYPE_ERR);
    }
    if (map->type != CELL_HASH) {
        return make_cell_error(
            "map-values-map: arg2 must be a map",
            TYPE_ERR);
    }
    return map_logic(proc, map, e, false, true);
}


/* (map-values-foreach proc map)
 * Applies proc to each value in map. Unlike map-values-map, this procedure is used for side effects. Returns #void. The
 * order of the procedure applications is indeterminate. The proc must accept exactly one arg, which will be each value
 * from the map. */
Cell* builtin_map_values_foreach(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "map-values-foreach");
    if (err) return err;

    const Cell* proc = a->cell[0];
    const Cell* map = a->cell[1];

    if (proc->type != CELL_PROC) {
        return make_cell_error(
            "map-values-foreach: arg1 must be a procedure",
            TYPE_ERR);
    }
    if (map->type != CELL_HASH) {
        return make_cell_error(
            "map-values-foreach: arg2 must be a map",
            TYPE_ERR);
    }
    return map_logic(proc, map, e, false, false);
}


/* Helper for map and foreach logic.
 * 'ret_res' is whether we are returning results (map -> true) or not (foreach -> false). */
static Cell* map_foreach(const Cell* proc, const Cell* h_table, const Lex* e, const bool ret_res)
{
    Cell* result = make_cell_sexpr();

    ghti it = ght_iterator(h_table->table);
    while (ght_next(&it)) {
        /* Apply procedure. */
        Cell* val;

        if (proc->is_builtin) {
            val = proc->builtin(e, make_sexpr_len2(it.key, it.value));
        } else {
            val = coz_apply_and_get_val(proc, make_sexpr_len2(it.key, it.value), e);
        }
        /* Propagate errors. */
        if (val && val->type == CELL_ERROR) return val;
        if (ret_res) {
            /* Ignore unspecified results. */
            if (val == USP_Obj) continue;
            cell_add(result, val);
        }
    }
    if (ret_res) {
        return make_list_from_sexpr(result);
    }
    return USP_Obj;
}


/* (map-items-map proc map)
 * Returns a list containing the results of applying proc to each item in map. The order of the procedure applications
 * is indeterminate. The proc must accept exactly two args, which will be each key/value pair from the map. */
Cell* builtin_map_items_map(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "map-items-map");
    if (err) return err;

    const Cell* proc = a->cell[0];
    const Cell* map = a->cell[1];

    if (proc->type != CELL_PROC) {
        return make_cell_error(
            "map-items-map: arg1 must be a procedure",
            TYPE_ERR);
    }
    if (map->type != CELL_HASH) {
        return make_cell_error(
            "map-items-map: arg2 must be a map",
            TYPE_ERR);
    }
    return map_foreach(proc, map, e, true);
}


/* (map-items-foreach proc map)
 * Applies proc to each item in map. Unlike map-items-map, this procedure is used for side effects. Returns #void. The
 * order of the procedure applications is indeterminate. The proc must accept exactly two args, which will be each
 * key/value pair from the map. */
Cell* builtin_map_items_foreach(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "map-items-foreach");
    if (err) return err;

    const Cell* proc = a->cell[0];
    const Cell* map = a->cell[1];

    if (proc->type != CELL_PROC) {
        return make_cell_error(
            "map-items-foreach: arg1 must be a procedure",
            TYPE_ERR);
    }
    if (map->type != CELL_HASH) {
        return make_cell_error(
            "map-items-foreach: arg2 must be a map",
            TYPE_ERR);
    }
    return map_foreach(proc, map, e, false);
}
