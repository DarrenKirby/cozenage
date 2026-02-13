/*
 * 'src/sets.c'
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

#include "sets.h"
#include "eval.h"
#include "types.h"
#include "hash_type.h"
#include "repr.h"

#include <gc/gc.h>
#include <string.h>


/* Helper for fast copy of hash table structure. */
Cell* copy_hash_table(const Cell* t) {
    const uint32_t type = t->type;

    const size_t cap = t->table->capacity;
    Cell* r = GC_MALLOC(sizeof(Cell));
    r->type = type;
    r->table = GC_MALLOC(sizeof(ght_table));
    r->table->count = t->table->count;
    r->count = t->count;
    r->table->capacity = t->table->capacity;
    r->table->items = GC_MALLOC(sizeof(ght_item) * cap);

    memcpy(r->table->items, t->table->items,
        sizeof(ght_item) * t->table->capacity);

    return r;
}


/* (set obj ...)
* Returns a newly allocated set whose elements contain the given arguments.
* It is analogous to list. Raises type error if any of the objects are not hashable. */
Cell* builtin_set(const Lex* e, const Cell* a)
{
    (void)e;
    if (a->count == 0) {
        return make_cell_set(nullptr);
    }

    Cell* tmp = make_cell_sexpr();
    for (int i = 0; i < a->count; i++) {
        if (!cell_is_hashable(a->cell[i])) {
            return make_cell_error(
            fmt_err("set: arg type '%s' is not a hashable",
            cell_type_name(a->cell[i]->type)),
            TYPE_ERR);
        }
        cell_add(tmp, a->cell[i]);
    }
    return make_cell_set(tmp);
}


/* (set-copy set)
 * Copies the structure of set into a newly allocated set object. Note that this is not a deep-copy. The keys
 * themselves will not be copied, just the pointers to them. */
Cell* builtin_set_copy(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "set-copy");
    if (err) return err;
    const Cell* set = a->cell[0];
    if (set->type != CELL_SET) {
        return make_cell_error(
            "set-copy: arg must be a set",
            TYPE_ERR);
    }

    return copy_hash_table(set);
}


/* (set-add set obj ...)
 * Adds an arbitrary number of objects to set. obj may be any hashable type, or compound types list and vector, in which
 * case the members of list or vector objects will be added to set. Raises type error if any object, or any member of a
 * list or vector argument is non-hashable. */
Cell* builtin_set_add(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 2, "set-add!");
    if (err) return err;

    Cell* set = a->cell[0];
    if (set->type != CELL_SET) {
        return make_cell_error(
            "set-add!: arg1 must be a set",
            TYPE_ERR);
    }

    for (int i = 1; i < a->count; i++) {
        Cell* arg = a->cell[i];
        if (arg->type == CELL_VECTOR) {
            for (int j = 0; j < arg->count; j++) {
                Cell* varg = arg->cell[j];
                if (!cell_is_hashable(varg)) {
                    return make_cell_error(
                        fmt_err("set-add!: arg type '%s' is not a hashable",
                            cell_type_name(varg->type)),
                        TYPE_ERR);
                }
                ght_set(set->table, varg, True_Obj);
            }
        } else if (arg->type == CELL_PAIR) {
            Cell* p = arg;
            while (p->cdr) {
                if (!cell_is_hashable(p->car)) {
                    return make_cell_error(
                        fmt_err("set-add!: arg type '%s' is not a hashable",
                            cell_type_name(p->car->type)),
                        TYPE_ERR);
                }
                ght_set(set->table, p->car, True_Obj);
                p = p->cdr;
            }

        } else {
            if (!cell_is_hashable(arg)) {
                return make_cell_error(
                    fmt_err("set-add!: arg type '%s' is not a hashable",
                        cell_type_name(arg->type)),
                    TYPE_ERR);
            }
            ght_set(set->table, arg, True_Obj);
        }
    }
    return set;
}


/* (set-remove set obj)
 * (set-remove set obj sym)
 * Removes obj from set, and returns the mutated set. Raises an index error if object is not a member of set. If an
 * optional symbol (any symbol) is passed in fourth position, the procedure will not raise the index error, but rather
 * silently return. */
Cell* builtin_set_remove(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3, "set-remove!");
    if (err) return err;

    Cell* set = a->cell[0];
    if (set->type != CELL_SET) {
        return make_cell_error(
            "set-remove!: arg 1 must be a set",
            TYPE_ERR);
    }

    const bool removed = ght_delete(set->table, a->cell[1]);

    if (a->count == 2 && !removed) {
        return make_cell_error(
            "set-remove!: arg 2 not member of set",
            INDEX_ERR);
    }
    if (a->count == 3 && a->cell[2]->type != CELL_SYMBOL) {
        return make_cell_error(
            "set-remove!: arg 3 must be a symbol",
            INDEX_ERR);
    }
    return set;
}


/* (set-member? set obj)
 * Returns #true if obj is a member of set (ie: obj ∈ set), else #false. */
Cell* builtin_set_member(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-member?");
    if (err) return err;
    const Cell* set = a->cell[0];
    if (set->type != CELL_SET) {
        return make_cell_error(
            "set-member?: arg 1 must be a set",
            TYPE_ERR);
    }
    if (!ght_get(set->table, a->cell[1])) {
        return False_Obj;
    }
    return True_Obj;
}


/* (set-disjoint? set1 set2)
 * Returns #true if the two sets are completely disjoint, that is, if they do not share any members. Otherwise,
 * returns #false. */
Cell* builtin_set_is_disjoint(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-disjoint?");
    if (err) return err;
    err = check_arg_types(a, CELL_SET, "set-disjoint?");
    if (err) return err;

    const Cell* sa = a->cell[0];
    const Cell* sb = a->cell[1];

    ghti it = ght_iterator(sa->table);
    while (ght_next(&it)) {
        if (ght_get(sb->table, it.key)) {
            return False_Obj;
        }
    }
    return True_Obj;
}


/* (set-subset? set1 set2)
 * Returns #true if all members of set1 are also contained in set2, else, returns #false. */
Cell* builtin_set_is_subset(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-subset?");
    if (err) return err;
    err = check_arg_types(a, CELL_SET, "set-subset?");
    if (err) return err;

    const Cell* sa = a->cell[0];
    const Cell* sb = a->cell[1];

    ghti it = ght_iterator(sa->table);
    while (ght_next(&it)) {
        if (!ght_get(sb->table, it.key)) {
            return False_Obj;
        }
    }
    return True_Obj;
}


/* (set-is-superset? set1 set2)
 * Returns #true if set1 contains all members of set2, else, returns #false. */
Cell* builtin_set_is_superset(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-superset?");
    if (err) return err;
    err = check_arg_types(a, CELL_SET, "set-superset?");
    if (err) return err;

    const Cell* sa = a->cell[0];
    const Cell* sb = a->cell[1];

    ghti it = ght_iterator(sb->table);
    while (ght_next(&it)) {
        if (!ght_get(sa->table, it.key)) {
            return False_Obj;
        }
    }
    return True_Obj;
}


/* (set-union set1 set2)
 * Returns the union of set1 and set2 (ie: set1 ∪ set2) as a newly allocated set object. */
Cell* builtin_set_union(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-union");
    if (err) return err;
    err = check_arg_types(a, CELL_SET, "set-union");
    if (err) return err;

    const Cell* sa = a->cell[0];
    const Cell* sb = a->cell[1];

    Cell* ns = make_cell_sexpr();

    ghti it = ght_iterator(sa->table);
    while (ght_next(&it)) {
        cell_add(ns, it.key);
    }
    it = ght_iterator(sb->table);
    while (ght_next(&it)) {
        cell_add(ns, it.key);
    }
    return make_cell_set(ns);
}


/* (set-union! set1 set2)
 * Returns set1 mutated to the union of set1 and set2 (ie: set1 ∪ set2). */
Cell* builtin_set_union_bang(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-union!");
    if (err) return err;
    err = check_arg_types(a, CELL_SET, "set-union!");
    if (err) return err;

    Cell* sa = a->cell[0];
    const Cell* sb = a->cell[1];

    ghti it = ght_iterator(sb->table);
    while (ght_next(&it)) {
        ght_set(sa->table, it.key, True_Obj);
    }
    return sa;
}


/* (set-intersection set1 set2)
 * Returns the intersection of set1 and set2 (ie: set1 ∩ set2) as a newly allocated set object. */
Cell* builtin_set_intersection(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-intersection");
    if (err) return err;
    err = check_arg_types(a, CELL_SET, "set-intersection");
    if (err) return err;

    const Cell* sa = a->cell[0];
    const Cell* sb = a->cell[1];
    Cell* ns = make_cell_sexpr();

    ghti it = ght_iterator(sa->table);
    while (ght_next(&it)) {
        if (ght_get(sb->table, it.key)) {
            cell_add(ns, it.key);
        }
    }
    return make_cell_set(ns);
}


/* (set-intersection set1 set2)
 * Returns set1 mutated to the intersection of set1 and set2 (ie: set1 ∩ set2). */
Cell* builtin_set_intersection_bang(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-intersection!");
    if (err) return err;
    err = check_arg_types(a, CELL_SET, "set-intersection!");
    if (err) return err;

    Cell* sa = a->cell[0];
    const Cell* sb = a->cell[1];

    ghti it = ght_iterator(sa->table);
    while (ght_next(&it)) {
        if (!ght_get(sb->table, it.key)) {
            ght_delete(sa->table, it.key);
        }
    }
    return sa;
}


/* (set-difference set1 set2)
 * Returns the difference of set1 and set2 (ie: set1 - set2) as a newly allocated set object. */
Cell* builtin_set_difference(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-difference");
    if (err) return err;
    err = check_arg_types(a, CELL_SET, "set-difference");
    if (err) return err;

    const Cell* sa = a->cell[0];
    const Cell* sb = a->cell[1];
    Cell* ns = make_cell_sexpr();

    ghti it = ght_iterator(sa->table);
    while (ght_next(&it)) {
        if (!ght_get(sb->table, it.key)) {
            cell_add(ns, it.key);
        }
    }
    return make_cell_set(ns);
}


/* (set-difference set1 set2)
 * Returns set1 mutated to the difference of set1 and set2 (ie: set1 - set2). */
Cell* builtin_set_difference_bang(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-difference!");
    if (err) return err;
    err = check_arg_types(a, CELL_SET, "set-difference!");
    if (err) return err;

    Cell* sa = a->cell[0];
    const Cell* sb = a->cell[1];

    ghti it = ght_iterator(sa->table);
    while (ght_next(&it)) {
        if (ght_get(sb->table, it.key)) {
            ght_delete(sa->table, it.key);
        }
    }
    return sa;
}


/* (set-sym-difference set1 set2)
 * Returns the symmetric difference of set1 and set2 (ie: set1 Δ set2) as a newly allocated set object. */
Cell* builtin_set_sym_difference(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-sym-difference");
    if (err) return err;
    err = check_arg_types(a, CELL_SET, "set-sym-difference");
    if (err) return err;

    const Cell* sa = a->cell[0];
    const Cell* sb = a->cell[1];
    Cell* ns = make_cell_sexpr();

    ghti it = ght_iterator(sa->table);
    while (ght_next(&it)) {
        if (!ght_get(sb->table, it.key)) {
            cell_add(ns, it.key);
        }
    }
    it = ght_iterator(sb->table);
    while (ght_next(&it)) {
        if (!ght_get(sa->table, it.key)) {
            cell_add(ns, it.key);
        }
    }
    return make_cell_set(ns);
}


/* (set-sym-difference set1 set2)
 * Returns set1 mutated to the symmetric difference of set1 and set2 (ie: set1 Δ set2). */
Cell* builtin_set_sym_difference_bang(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-sym-difference!");
    if (err) return err;
    err = check_arg_types(a, CELL_SET, "set-sym-difference!");
    if (err) return err;

    Cell* sa = a->cell[0];
    const Cell* sb = a->cell[1];

    ghti it = ght_iterator(sa->table);
    while (ght_next(&it)) {
        if (ght_get(sb->table, it.key)) {
            ght_delete(sa->table, it.key);
        }
    }
    it = ght_iterator(sb->table);
    while (ght_next(&it)) {
        if (ght_get(sa->table, it.key)) {
            ght_delete(sa->table, it.key);
        }
    }
    return sa;
}


/* (set->map proc set)
 * Returns a list containing the results of applying proc to each key in set. The order of the procedure applications is
 * indeterminate. */
Cell* builtin_set_map(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-map");
    if (err) return err;

    const Cell* proc = a->cell[0];
    const Cell* set = a->cell[1];

    if (proc->type != CELL_PROC) {
        return make_cell_error(
            "set-map: arg1 must be a procedure",
            TYPE_ERR);
    }
    if (set->type != CELL_SET) {
        return make_cell_error(
            "set-map: arg2 must be a set",
            TYPE_ERR);
    }

    Cell* result = make_cell_sexpr();
    ghti it = ght_iterator(set->table);
    while (ght_next(&it)) {
        /* Apply procedure. */
        Cell* val;

        if (proc->is_builtin) {
            val = proc->builtin(e, make_sexpr_len1(it.key));
        } else {
            val = coz_apply_and_get_val(proc, make_sexpr_len1(it.key), (Lex*)e);
        }

        if (val && val->type == CELL_ERROR) return val;
        /* Ignore unspecified results. */
        if (val == USP_Obj) continue;
        cell_add(result, val);
    }
    return make_list_from_sexpr(result);
}


/* (set-foreach proc set)
 * Applies proc to each key in set. Unlike set-map, this procedure is used for side effects. Returns #void. The order of
 * the procedure applications is indeterminate. */
Cell* builtin_set_foreach(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-foreach");
    if (err) return err;

    const Cell* proc = a->cell[0];
    const Cell* set = a->cell[1];

    if (proc->type != CELL_PROC) {
        return make_cell_error(
            "set-foreach: arg1 must be a procedure",
            TYPE_ERR);
    }
    if (set->type != CELL_SET) {
        return make_cell_error(
            "set-foreach: arg2 must be a set",
            TYPE_ERR);
    }

    ghti it = ght_iterator(set->table);
    while (ght_next(&it)) {
        Cell* val;
        if (proc->is_builtin) {
            val = proc->builtin(e, make_sexpr_len1(it.key));
        } else {
            val = coz_apply_and_get_val(proc, make_sexpr_len1(it.key), (Lex*)e);
        }

        /* If the procedure returns an error, stop and propagate it. */
        if (val && val->type == CELL_ERROR) return val;
    }

    return USP_Obj;
}


/* (list->set list)
 * Returns a newly-allocated set which contains all the (non-duplicated) members of list. */
Cell* builtin_list_to_set(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "list->set");
    if (err) return err;
    if (a->cell[0]->type != CELL_PAIR || a->cell[0]->count == -1) {
        make_cell_error(
            "list->set: arg must be a proper list",
            TYPE_ERR);
    }
    return make_cell_set(make_sexpr_from_list(a->cell[0], false));
}


/* (set->list set)
 * Returns a newly-allocated list containing all the member objects of set. */
Cell* builtin_set_to_list(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "set->list");
    if (err) return err;
    const Cell* set = a->cell[0];
    if (set->type != CELL_SET) {
        return make_cell_error(
            "set->list: arg must be a set",
            TYPE_ERR);
    }
    Cell* result = make_cell_sexpr();
    ghti it = ght_iterator(set->table);
    while (ght_next(&it)) {
        cell_add(result, it.key);
    }
    return make_list_from_sexpr(result);
}
