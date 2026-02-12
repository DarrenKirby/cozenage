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
#include "types.h"
#include "hash_type.h"
#include "repr.h"


/* (set-add set obj ...)
 * Adds an arbitrary number of objects to set. obj may be any hashable type, or compound types list and vector, in which
 * case the members of list or vector objects will be added to set. Raises type error if any object, or any member of a
 * list or vector argument is non-hashable. */
Cell* builtin_set_add(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 2, "set-add");
    if (err) return err;

    Cell* set = a->cell[0];
    if (set->type != CELL_SET) {
        return make_cell_error(
            "set-add: arg1 must be a set",
            TYPE_ERR);
    }

    for (int i = 1; i < a->count; i++) {
        Cell* arg = a->cell[i];
        if (arg->type == CELL_VECTOR) {
            for (int j = 0; j < arg->count; j++) {
                Cell* varg = arg->cell[j];
                if (!cell_is_hashable(varg)) {
                    return make_cell_error(
                        fmt_err("set-add: arg type %s is not a hashable", cell_type_name(varg->type)),
                        TYPE_ERR);
                }
                ght_set(set->table, varg, True_Obj);
            }
        } else if (arg->type == CELL_PAIR) {
            Cell* p = arg;
            while (p->cdr) {
                if (!cell_is_hashable(p->car)) {
                    return make_cell_error(
                        fmt_err("set-add: arg type %s is not a hashable", cell_type_name(p->car->type)),
                        TYPE_ERR);
                }
                ght_set(set->table, p->car, True_Obj);
                p = p->cdr;
            }

        } else {
            if (!cell_is_hashable(arg)) {
                return make_cell_error(
                    fmt_err("set-add: arg type %s is not a hashable", cell_type_name(arg->type)),
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
Cell* builtin_set_remove(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3, "set-remove");
    if (err) return err;

    Cell* set = a->cell[0];
    if (set->type != CELL_SET) {
        return make_cell_error(
            "set-remove: arg 1 must be a set",
            TYPE_ERR);
    }

    const bool removed = ght_delete(set->table, a->cell[1]);

    if (a->count == 2 && !removed) {
        return make_cell_error(
            "set-remove: arg 3 not member of set",
            INDEX_ERR);
    }
    if (a->count == 3 && a->cell[2]->type != CELL_SYMBOL) {
        return make_cell_error(
            "set-remove: arg 3 must be a symbol",
            INDEX_ERR);
    }
    return set;
}


/* (set-member? set obj)
 * Returns #true if obj is a member of set (ie: obj ∈ set), else #false. */
Cell* builtin_set_member(const Lex* e, const Cell* a) {
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


/* (set-union set1 set2)
 * Returns the union of set1 and set2 (ie: set1 ∪ set2) as a newly allocated set object. */
Cell* builtin_set_union(const Lex* e, const Cell* a) {
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
Cell* builtin_set_union_bang(const Lex* e, const Cell* a) {
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
Cell* builtin_set_intersection(const Lex* e, const Cell* a) {
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
Cell* builtin_set_intersection_bang(const Lex* e, const Cell* a) {
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


Cell* builtin_set_difference(const Lex* e, const Cell* a) {
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


Cell* builtin_set_difference_bang(const Lex* e, const Cell* a) {
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
