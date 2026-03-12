/*
 * 'src/hashmaps.c'
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

#include "hashmaps.h"
#include "sets.h"
#include "types.h"
#include "eval.h"


/* (hash obj1 obj2 ...)
 * Returns a newly allocated hash object made from key -> value pairs supplied as args. */
Cell* builtin_hash(const Lex* e, const Cell* a)
{
    (void)e;
    if (a->count == 0) {
        /* No args: return an empty hash. */
        return make_cell_hash(nullptr);
    }

    if (a->count % 2 != 0) {
        return make_cell_error(
            "hash: requires even number of args",
            VALUE_ERR);
    }
    for (int i = 0; i < a->count; i += 2) {
        if (!cell_is_hashable(a->cell[i])) {
            return make_cell_error(
            fmt_err("hash: arg type '%s' is not a hashable",
                     cell_type_name(a->cell[i]->type)),
                     TYPE_ERR);
        }
    }
    /* Confirmed args are OK, just feed right to hash constructor. */
    return make_cell_hash(a);
}


/* (hash-copy hash)
 * Copies the structure of hash into a newly allocated hash object. Note that this is not a deep-copy. The keys and
 * values themselves will not be copied, just the pointers to them. */
Cell* builtin_hash_copy(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "hash-copy");
    if (err) return err;

    const Cell* hash = a->cell[0];
    if (hash->type != CELL_HASH) {
        return make_cell_error(
            "hash-copy: arg must be a hash",
            TYPE_ERR);
    }
    return copy_hash_table(hash);
}


/* (hash-clear! hash)
 * Returns the same hash object with all keys and values cleared. */
Cell* builtin_hash_clear(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "hash-clear!");
    if (err) return err;

    Cell* hash = a->cell[0];
    if (hash->type != CELL_HASH) {
        return make_cell_error(
            "hash-clear!: arg must be a hash",
            TYPE_ERR);
    }
    return clear_hash_table(hash);
}


/* (hash-get hash key)
 * (hash-get hash key default_val)
 * Returns the value indexed by key in hash, or else raises an index error. If an optional third argument is passed,
 * then that value will be returned if the key does not exist in hash. */
Cell* builtin_hash_get(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3, "hash-get");
    if (err) return err;

    const Cell* hash = a->cell[0];
    const Cell* obj = a->cell[1];

    if (hash->type != CELL_HASH) {
        return make_cell_error(
            "hash-get: arg1 must be a hash",
            TYPE_ERR);
    }
    if (!cell_is_hashable(obj)) {
        return make_cell_error(
            fmt_err("hash-get: arg type '%s' is not a hashable",
                cell_type_name(obj->type)),
            TYPE_ERR);
    }

    Cell* val = ght_get(hash->table, obj);
    if (!val) {
        if (a->count == 3) {
            return a->cell[2];
        }
        return make_cell_error(
            "hash-get: object not found in hash",
            INDEX_ERR);
    }
    return val;
}


/* (hash-add! hash obj1 obj2)
 * Adds obj1 as key, and obj2 as value to the hash specified by hash. Raises type error if obj1 is not hashable. */
Cell* builtin_hash_add(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 3, "hash-add!");
    if (err) return err;

    Cell* hash = a->cell[0];
    if (hash->type != CELL_HASH) {
        return make_cell_error(
            "hash-add!: arg1 must be a hash",
            TYPE_ERR);
    }

    if (!cell_is_hashable(a->cell[1])) {
        return make_cell_error(
        fmt_err("hash-add!: arg type '%s' is not hashable",
                 cell_type_name(a->cell[1]->type)),
                 TYPE_ERR);
    }

    if (!ght_set(hash->table, a->cell[1], a->cell[2])) {
        return make_cell_error(
            fmt_err("hash-add!: failed to add new items to hash"),
            GEN_ERR);
    }
    return hash;
}


/* (hash-remove! hash obj)
 * (hash-remove! hash obj sym)
 * Removes the key obj and associated value from hash, and returns the mutated hash. Raises an index error if object is not
 * a member of hash. If an optional symbol (any symbol) is passed as third arg, the procedure will not raise the
 * index error, but rather just silently return. */
Cell* builtin_hash_remove(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3, "hash-remove!");
    if (err) return err;

    Cell* hash = a->cell[0];
    if (hash->type != CELL_HASH) {
        return make_cell_error(
            "hash-remove!: arg1 must be a hash",
            TYPE_ERR);
    }
    if (!cell_is_hashable(a->cell[1])) {
        return make_cell_error(
            "hash-remove!: arg2 is not hashable (therefore will not be in hash)",
            TYPE_ERR);
    }

    const bool removed = ght_delete(hash->table, a->cell[1]);

    if (a->count == 2 && !removed) {
        return make_cell_error(
            "hash-remove!: arg 2 not member of hash",
            INDEX_ERR);
    }
    if (a->count == 3 && a->cell[2]->type != CELL_SYMBOL) {
        return make_cell_error(
            "hash-remove!: arg 3 must be a symbol",
            INDEX_ERR);
    }
    return hash;
}


/* (hash-keys hash)
 * Returns a list containing all keys in hash. */
Cell* builtin_hash_keys(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "hash-keys");
    if (err) return err;
    const Cell* hash = a->cell[0];
    if (hash->type != CELL_HASH) {
        return make_cell_error(
            "hash-keys: arg must be a hash",
            TYPE_ERR);
    }

    Cell* r = make_cell_sexpr();
    ghti it = ght_iterator(hash->table);
    while (ght_next(&it)) {
        cell_add(r, it.key);
    }

    return make_list_from_sexpr(r);
}


/* (hash-values hash)
 * Returns a list containing all values in hash. */
Cell* builtin_hash_values(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "hash-values");
    if (err) return err;
    const Cell* hash = a->cell[0];
    if (hash->type != CELL_HASH) {
        return make_cell_error(
            "hash-values: arg must be a hash",
            TYPE_ERR);
    }

    Cell* r = make_cell_sexpr();
    ghti it = ght_iterator(hash->table);
    while (ght_next(&it)) {
        cell_add(r, it.value);
    }

    return make_list_from_sexpr(r);
}


/* (hash->alist hash)
 * Returns an alist containing all keys and values found in hash, ie: it returns a list where each car field is a
 * (key . value) dotted pair. */
Cell* builtin_hash_to_alist(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "hash->alist");
    if (err) return err;
    const Cell* hash = a->cell[0];
    if (hash->type != CELL_HASH) {
        return make_cell_error(
            "hash->alist: arg must be a hash",
            TYPE_ERR);
    }

    Cell* r = make_cell_sexpr();
    ghti it = ght_iterator(hash->table);
    while (ght_next(&it)) {
        Cell* p = make_cell_pair(it.key, it.value);
        cell_add(r, p);
    }

    return make_list_from_sexpr(r);
}


/* (alist->hash alist)
 * Returns a newly allocated hash object where all key -> value pairs are derived from association list car fields.
 * Raises type error if alist car fields are not dotted pairs. */
Cell* builtin_alist_to_hash(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "alist->hash");
    if (err) return err;
    const Cell* alist = a->cell[0];
    if (alist->type != CELL_PAIR || a->cell[0]->count == -1) {
        return make_cell_error(
            "alist->hash: arg must be an association list",
            TYPE_ERR);
    }

    Cell* r = make_cell_sexpr();
    while (alist->cdr) {
        /* This also enforces an even number of objects fed to make_cell_hash(). */
        if (alist->car->type != CELL_PAIR) {
            return make_cell_error(
                "alist->hash: car field of list is not a dotted pair",
                TYPE_ERR);
        }
        cell_add(r, alist->car->car);
        cell_add(r, alist->car->cdr);
        alist = alist->cdr;
    }
    return make_cell_hash(r);
}


/* (hash-keys-map proc hash)
 * Returns a list containing the results of applying proc to each key in hash. The order of the procedure applications is
 * indeterminate. The proc must accept exactly one arg, which will be each key from the hash. */
Cell* builtin_hash_keys_map(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "hash-keys-map");
    if (err) return err;

    const Cell* proc = a->cell[0];
    const Cell* hash = a->cell[1];

    if (proc->type != CELL_PROC) {
        return make_cell_error(
            "hash-keys-map: arg1 must be a procedure",
            TYPE_ERR);
    }
    if (hash->type != CELL_HASH) {
        return make_cell_error(
            "hash-keys-map: arg2 must be a hash",
            TYPE_ERR);
    }
    return map_logic(proc, hash, e, true, true);
}


/* (hash-keys-foreach proc hash)
 * Applies proc to each key in hash. Unlike hash-keys-map, this procedure is used for side effects. Returns #void. The
 * order of the procedure applications is indeterminate. The proc must accept exactly one arg, which will be each key
 * from the hash. */
Cell* builtin_hash_keys_foreach(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "hash-keys-foreach");
    if (err) return err;

    const Cell* proc = a->cell[0];
    const Cell* hash = a->cell[1];

    if (proc->type != CELL_PROC) {
        return make_cell_error(
            "hash-keys-foreach: arg1 must be a procedure",
            TYPE_ERR);
    }
    if (hash->type != CELL_HASH) {
        return make_cell_error(
            "hash-keys-foreach: arg2 must be a hash",
            TYPE_ERR);
    }
    return map_logic(proc, hash, e, true, false);
}


/* (hash-values-map proc hash)
 * Returns a list containing the results of applying proc to each value in hash. The order of the procedure applications
 * is indeterminate. The proc must accept exactly one arg, which will be each value from the hash. */
Cell* builtin_hash_values_map(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "hash-values-map");
    if (err) return err;

    const Cell* proc = a->cell[0];
    const Cell* hash = a->cell[1];

    if (proc->type != CELL_PROC) {
        return make_cell_error(
            "hash-values-map: arg1 must be a procedure",
            TYPE_ERR);
    }
    if (hash->type != CELL_HASH) {
        return make_cell_error(
            "hash-values-map: arg2 must be a hash",
            TYPE_ERR);
    }
    return map_logic(proc, hash, e, false, true);
}


/* (hash-values-foreach proc hash)
 * Applies proc to each value in hash. Unlike hash-values-map, this procedure is used for side effects. Returns #void. The
 * order of the procedure applications is indeterminate. The proc must accept exactly one arg, which will be each value
 * from the hash. */
Cell* builtin_hash_values_foreach(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "hash-values-foreach");
    if (err) return err;

    const Cell* proc = a->cell[0];
    const Cell* hash = a->cell[1];

    if (proc->type != CELL_PROC) {
        return make_cell_error(
            "hash-values-foreach: arg1 must be a procedure",
            TYPE_ERR);
    }
    if (hash->type != CELL_HASH) {
        return make_cell_error(
            "hash-values-foreach: arg2 must be a hash",
            TYPE_ERR);
    }
    return map_logic(proc, hash, e, false, false);
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


/* (hash-items-map proc hash)
 * Returns a list containing the results of applying proc to each item in hash. The order of the procedure applications
 * is indeterminate. The proc must accept exactly two args, which will be each key/value pair from the hash. */
Cell* builtin_hash_items_map(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "hash-items-map");
    if (err) return err;

    const Cell* proc = a->cell[0];
    const Cell* hash = a->cell[1];

    if (proc->type != CELL_PROC) {
        return make_cell_error(
            "hash-items-map: arg1 must be a procedure",
            TYPE_ERR);
    }
    if (hash->type != CELL_HASH) {
        return make_cell_error(
            "hash-items-map: arg2 must be a hash",
            TYPE_ERR);
    }
    return map_foreach(proc, hash, e, true);
}


/* (hash-items-foreach proc hash)
 * Applies proc to each item in hash. Unlike hash-items-map, this procedure is used for side effects. Returns #void. The
 * order of the procedure applications is indeterminate. The proc must accept exactly two args, which will be each
 * key/value pair from the hash. */
Cell* builtin_hash_items_foreach(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "hash-items-foreach");
    if (err) return err;

    const Cell* proc = a->cell[0];
    const Cell* hash = a->cell[1];

    if (proc->type != CELL_PROC) {
        return make_cell_error(
            "hash-items-foreach: arg1 must be a procedure",
            TYPE_ERR);
    }
    if (hash->type != CELL_HASH) {
        return make_cell_error(
            "hash-items-foreach: arg2 must be a hash",
            TYPE_ERR);
    }
    return map_foreach(proc, hash, e, false);
}
