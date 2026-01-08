/*
 * 'src/pairs.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 - 2026 Darren Kirby <darren@dragonbyte.ca>
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

#include "pairs.h"
#include "eval.h"
#include "repr.h"
#include "types.h"
#include "comparators.h"


/* Helpers */

static Cell* sexp_cdr(const Cell* s) {
    Cell* result = make_cell_sexpr();
    for (int i = 1; i < s->count; i++) {
        cell_add(result, s->cell[i]);
    }
    return result;
}


inline Cell* car__(const Cell* list)
{
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const int mask = CELL_PAIR|CELL_SEXPR;
    if (!(list->type & mask)) {
        return make_cell_error(
        fmt_err("car: got %s, expected %s",
             cell_type_name(list->type),
             cell_mask_types(CELL_PAIR)),
            TYPE_ERR);
    }
    if (list->type == CELL_PAIR) {
        return list->car;
    }
    return list->cell[0];
}


inline Cell* cdr__(const Cell* list)
{
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const int mask = CELL_PAIR|CELL_SEXPR;
    if (!(list->type & mask)) {
        return make_cell_error(
        fmt_err("car: got %s, expected %s",
             cell_type_name(list->type),
             cell_mask_types(CELL_PAIR)),
            TYPE_ERR);
    }
    if (list->type == CELL_PAIR) {
        return list->cdr;
    }
    return sexp_cdr((Cell*)list);
}


/* ----------------------------------------------------------*
 *     pair/list constructors, selectors, and procedures     *
 * ----------------------------------------------------------*/


/* (cons obj1 obj2)
 * Returns a newly allocated pair whose car is obj1 and whose cdr is obj2. The pair is guaranteed
 * to be different (in the sense of eqv?) from every existing object. */
Cell* builtin_cons(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "cons");
    if (err) return err;
    return make_cell_pair(a->cell[0], a->cell[1]);
}


/* (car pair)
 * Returns the contents of the car field of pair. */
Cell* builtin_car(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "car");
    if (err) return err;
    return car__(a->cell[0]);
}


/* (cdr pair)
 * Returns the contents of the cdr field of pair. */
Cell* builtin_cdr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdr");
    if (err) return err;
    return cdr__(a->cell[0]);
}


/* These procedures are compositions of car and cdr as follows:
 *    (define (caar x) (car (car x)))
 *    (define (cadr x) (car (cdr x)))
 *    (define (cdar x) (cdr (car x)))
 *    (define (cddr x) (cdr (cdr x)))
 *    */
Cell* builtin_caar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caar");
    if (err) { return err; }
    return car__(car__(a->cell[0]));
}


Cell* builtin_cadr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cadr");
    if (err) { return err; }
    return car__(cdr__(a->cell[0]));
}


Cell* builtin_cdar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdar");
    if (err) { return err; }
    return cdr__(car__(a->cell[0]));
}


Cell* builtin_cddr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cddr");
    if (err) { return err; }
    return cdr__(cdr__(a->cell[0]));
}


/* (list obj ... )
 * Returns a newly allocated list of its arguments. */
Cell* builtin_list(const Lex* e, const Cell* a)
{
    (void)e;
    /* Start with nil. */
    Cell* result = make_cell_nil();

    const int len = a->count;
    /* Build backwards so it comes out in the right order. */
    for (int i = len - 1; i >= 0; i--) {
        result = make_cell_pair(a->cell[i], result);
        result->len = len - i;
    }
    return result;
}


/* (set-car! pair obj)
 * Stores obj in the car field of pair. */
Cell* builtin_set_car(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-car!");
    if (err) return err;
    if (a->cell[0]->type != CELL_PAIR) {
        return make_cell_error(
            "set-car!: arg 1 must be a pair",
            TYPE_ERR);
    }
    a->cell[0]->car = a->cell[1];
    return USP_Obj;
}


/* (set-cdr! pair obj)
 * Stores obj in the cdr field of pair. */
Cell* builtin_set_cdr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set-cdr!");
    if (err) return err;

    Cell* pair = a->cell[0];
    if (pair->type != CELL_PAIR) {
        return make_cell_error(
            "set-cdr!: arg 1 must be a pair",
            TYPE_ERR);
    }

    pair->cdr = a->cell[1];

    /* INVALIDATION:
       We no longer know the length of this pair or any pair that points to it.
       By setting this to -1, we force builtin_list_length to recount. */
    pair->len = -1;

    return USP_Obj;
}


/* (length list)
 * Returns the length of list. */
Cell* builtin_list_length(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "length");
    if (err) return err;

    const Cell* list = a->cell[0];
    if (list->type == CELL_NIL) return make_cell_integer(0);
    if (list->type != CELL_PAIR) {
        return make_cell_error(
            "length: arg must be a list",
            TYPE_ERR);
    }

    /* FAST PATH: The cache is valid and positive. */
    if (list->len > 0) {
        return make_cell_integer(list->len);
    }

    /* SLOW PATH: Recount and detect cycles. */
    int32_t count = 0;
    const Cell* slow = list;
    const Cell* fast = list;

    while (list->type == CELL_PAIR) {
        count++;
        list = list->cdr;

        /* Tortoise and Hare Cycle Detection. */
        if (fast->type == CELL_PAIR && fast->cdr->type == CELL_PAIR) {
            fast = fast->cdr->cdr;
            slow = slow->cdr;
            if (fast == slow) return make_cell_error(
                "length: circular list",
                VALUE_ERR);
        }

        if (list->type == CELL_NIL) {
            /* Found the end! Cache the result in the head for next time. */
            a->cell[0]->len = count;
            return make_cell_integer(count);
        }
    }

    return make_cell_error(
        "length: improper list",
        TYPE_ERR);
}


/* (list-ref list k)
 * Returns the kth element of list. (This is the same as the car of (list-tail list k).) */
Cell* builtin_list_ref(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "list-ref");
    if (err) return err;

    Cell* list = a->cell[0];
    if (list->type != CELL_PAIR) {
        return make_cell_error(
            "list-ref: arg 1 must be a pair",
            TYPE_ERR);
    }

    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error(
            "list-ref: arg 2 must be an integer",
            TYPE_ERR);
    }

    const int32_t idx = (int32_t)a->cell[1]->integer_v;
    if (idx < 0) {
        return make_cell_error(
            "list-ref: index must be non-negative",
            INDEX_ERR);
    }

    /* Fast Path: Bounds check using metadata (if available). */
    if (list->len > 0 && idx >= list->len) {
        return make_cell_error(
            "list-ref: index out of range",
            INDEX_ERR);
    }

    /* Walk the list. */
    const Cell* curr = list;
    int32_t count = idx;

    while (count > 0) {
        curr = curr->cdr;
        count--;

        /* If we hit something that isn't a pair before count reaches 0, it's an error. */
        if (curr->type != CELL_PAIR) {
            return make_cell_error("list-ref: index out of range or improper list", INDEX_ERR);
        }
    }

    return curr->car;
}


/* (append list ...)
 * The last argument, if there is one, can be of any type. Returns a list consisting of the elements
 * of the first list followed by the elements of the other lists. If there are no arguments, the
 * empty list is returned. If there is exactly one argument, it is returned. Otherwise, the resulting
 * list is always newly allocated, except that it shares structure with the last argument. An
 * improper list results if the last argument is not a proper list. */
Cell* builtin_list_append(const Lex* e, const Cell* a)
{
    (void)e;
    /* Base case: (append) -> '(). */
    if (a->count == 0) {
        return make_cell_nil();
    }
    /* Base case: (append x) -> x. */
    if (a->count == 1) {
        return a->cell[0];
    }

    /* Validate args and calculate total length of copied lists. */
    long long total_copied_len = 0;
    for (int i = 0; i < a->count - 1; i++) {
        const Cell* current_list = a->cell[i];
        if (current_list->type == CELL_NIL) {
            continue; /* This is a proper, empty list. */
        }
        /* All but the last argument must be a list. */
        if (current_list->type != CELL_PAIR) {
            return make_cell_error(
                fmt_err("append: arg%d is not a list", i+1),
                TYPE_ERR);
        }

        /* Now, walk the list to ensure it's a *proper* list */
        const Cell* p = current_list;
        while (p->type == CELL_PAIR) {
            p = p->cdr;
        }
        if (p->type != CELL_NIL) {
            return make_cell_error(
                fmt_err("append: arg%d is not a proper list", i+1),
                TYPE_ERR);
        }

        /* If we get here, the list is proper. Add its length. */
        total_copied_len += current_list->len;
    }

    /* Determine the final list's properties based on the last argument. */
    const Cell* last_arg = a->cell[a->count - 1];
    long long final_total_len = -1; /* Use -1 to signify an improper list. */

    if (last_arg->type == CELL_NIL) {
        final_total_len = total_copied_len;
    } else if (last_arg->type == CELL_PAIR) {
        /* If the last arg has a valid length, the result will be a proper list. */
        if (last_arg->len != -1) {
             final_total_len = total_copied_len + last_arg->len;
        }
    }

    /* Build the new list structure. */
    Cell* result_head = make_cell_nil();
    Cell* result_tail = nullptr;
    long long len_countdown = final_total_len;

    for (int i = 0; i < a->count - 1; i++) {
        const Cell* p = a->cell[i];
        while (p->type == CELL_PAIR) {
            /* Create a new pair with a copy of the element. */
            Cell* new_pair = make_cell_pair(p->car, make_cell_nil());

            /* Assign the correct length based on our pre-calculated total. */
            if (final_total_len != -1) {
                new_pair->len = (int)len_countdown--;
            } else {
                new_pair->len = -1;
            }

            /* Append the new pair to our result list. */
            if (result_tail == NULL) {
                result_head = result_tail = new_pair;
            } else {
                result_tail->cdr = new_pair;
                result_tail = new_pair;
            }
            p = p->cdr;
        }
    }

    /* Finalize: Link the last argument and return */
    if (result_tail == NULL) {
        /* This happens if all arguments before the last were '(). */
        return (Cell*)last_arg;
    }
    /* Splice the last argument onto the end of our newly created list. */
    result_tail->cdr = (Cell*)last_arg;
    return result_head;
}


/* (reverse list)
 * Returns a newly allocated list consisting of the elements of list in reverse order. */
Cell* builtin_list_reverse(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "list-reverse");
    if (err) return err;
    err = check_arg_types(a, CELL_PAIR|CELL_NIL, "reverse");
    if (err) { return err; }

    const Cell* original_list = a->cell[0];
    if (original_list->type == CELL_NIL) {
        return make_cell_nil();
    }

    Cell* reversed_list = make_cell_nil();
    const Cell* current = original_list;
    int length = 0;

    while (current->type == CELL_PAIR) {
        reversed_list = make_cell_pair(current->car, reversed_list);
        length++;
        current = current->cdr;
    }

    if (current->type != CELL_NIL) {
        return make_cell_error(
            "reverse: cannot reverse improper list",
            TYPE_ERR);
    }

    /* Set the length on the final result. */
    if (reversed_list->type != CELL_NIL) {
        reversed_list->len = length;
    }
    return reversed_list;
}


/* (list-tail list k)
 * Returns the sublist of list obtained by omitting the first k elements */
Cell* builtin_list_tail(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "list-tail");
    if (err) return err;

    Cell* lst = a->cell[0];
    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error(
            "list-tail: arg 2 must be an integer",
            TYPE_ERR);
    }

    const int32_t k = (int32_t)a->cell[1]->integer_v;
    if (k < 0) {
        return make_cell_error(
            "list-tail: index must be non-negative",
            VALUE_ERR);
    }

    /* Fast Path: If we have a cached length, use it to fail fast. */
    if (lst->type == CELL_PAIR && lst->len > 0 && k > lst->len) {
        return make_cell_error(
            "list-tail: index out of range",
            INDEX_ERR);
    }

    /* Traverse the list. */
    Cell* p = lst;
    for (int32_t i = 0; i < k; i++) {
        if (p->type != CELL_PAIR) {
            /* If we aren't at a pair and still need to move forward, it's an error. */
            return make_cell_error(
                "list-tail: index out of range",
                INDEX_ERR);
        }
        p = p->cdr;
    }

    /* If k was 0, p is still the original lst (correct for any type).
       Otherwise, p is the k-th cdr. */
    return p;
}


/* (make-list k)
 * (make-list k fill)
 * Returns a newly allocated list of k elements. If a second argument is given, then each element is
 * initialized to fill. Otherwise, the initial contents of each element is set to 0. */
Cell* builtin_make_list(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 1, 2, "make-list");
    if (err) return err;
    if (a->cell[0]->type != CELL_INTEGER && a->cell[0]->integer_v < 1) {
        return make_cell_error(
            "make-list: arg 1 must be a positive integer",
            VALUE_ERR);
    }
    Cell* fill;
    if (a->count== 2) {
        fill = a->cell[1];
    } else {
        fill = make_cell_integer(0);
    }
    /* Start with nil. */
    Cell* result = make_cell_nil();

    const int len = (int)a->cell[0]->integer_v;
    /* Build backwards so it comes out in the right order. */
    for (int i = len - 1; i >= 0; i--) {
        result = make_cell_pair(fill, result);
        result->len = len - i;
    }
    return result;
}


/* (list-set! list k obj)
 * The list-set! procedure stores obj in element k of list. */
Cell* builtin_list_set(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 3, "list-set!");
    if (err) return err;
    if (a->cell[0]->type != CELL_PAIR) {
        return make_cell_error(
            "list-set!: arg 1 must be a list",
            TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_INTEGER && a->cell[1]->integer_v < 0) {
        return make_cell_error(
            "list-set!: arg 2 must be a valid list index",
            VALUE_ERR);
    }
    Cell* p = a->cell[0];
    const int len = (int)a->cell[1]->integer_v;

    if (a->cell[0]->len <= len) {
        return make_cell_error(
            "list-set!: list index out of range",
            INDEX_ERR);
    }

    for (int i = 0; i < len; i++) {
        p = p->cdr;
    }
    /* Now p is pointing at the pair to mutate. */
    p->car = a->cell[2];
    /* No meaningful return value. */
    return USP_Obj;
}


/* (memq obj list)
 * Return the first sublist of list whose car is obj, where the sublists of list are the non-empty
 * lists returned by (list-tail list k) for k less than the length of list. If obj does not occur in
 * list, then #f (not the empty list) is returned. Uses eq? to compare obj with the elements of list
 */
Cell* builtin_memq(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "memq");
    if (err) return err;

    const Cell* key = a->cell[0];
    Cell* list = a->cell[1];

    while (list != NULL && list->type == CELL_PAIR) {
        if (list->car == key) {
            return list;
        }
        list = list->cdr;
    }
    return False_Obj;
}


/* (memv obj list)
 * Return the first sublist of list whose car is obj, where the sublists of list are the non-empty
 * lists returned by (list-tail list k) for k less than the length of list. If obj does not occur in
 * list, then #f (not the empty list) is returned. Uses eqv? to compare obj with the elements of list
 */
Cell* builtin_memv(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "memv");
    if (err) return err;

    const Cell* key = a->cell[0];
    Cell* list = a->cell[1];

    /* Iterate until we hit the end of the list (Empty/False_Obj). */
    while (list != NULL && list->type == CELL_PAIR) {
        /* Prepare args for eqv? : (key element). */
        Cell* eqv_args = make_cell_sexpr();
        cell_add(eqv_args, (Cell*)key);
        cell_add(eqv_args, list->car);

        const Cell* result = builtin_eqv(e, eqv_args);

        /* Only False_Obj is false. */
        if (result != False_Obj && result->boolean_v == 1) {
            return list;
        }

        list = list->cdr;
    }
    return False_Obj;
}


/* (member obj list)
 * (member obj list compare)
 * Return the first sublist of list whose car is obj, where the sublists of list are the non-empty
 * lists returned by (list-tail list k) for k less than the length of list. If obj does not occur in
 * list, then #f (not the empty list) is returned. Uses equal? to compare obj with the elements of
 * list unless an optional third arg, a comparison procedure, is provided */
Cell* builtin_member(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3, "member");
    if (err) return err;

    const Cell* key = a->cell[0];
    Cell* list = a->cell[1];
    Cell* predicate = a->count == 3 ? a->cell[2] : USP_Obj;

    while (list != NULL && list->type == CELL_PAIR) {
        Cell* result;
        if (predicate == USP_Obj) {
            /* Default to equal? */
            result = builtin_equal(e, make_sexpr_len2(list->car, (Cell*)key));
        } else {
            /* Use custom predicate: (proc element key). */
            Cell* args = make_cell_sexpr();
            cell_add(args, predicate);
            cell_add(args, (Cell*)key);
            cell_add(args, list->car); /* Current element. */
            result = coz_eval((Lex*)e, args);
        }

        if (result != False_Obj && result->boolean_v == 1) {
            return list;
        }
        list = list->cdr;
    }
    return False_Obj;
}


/* (assq obj alist )
 * Find the first pair in alist whose car field is obj, and returns that pair. If no pair in alist has obj as its car,
 * then #f is returned. Uses eq? for the comparison. */
Cell* builtin_assq(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "assq");
    if (err) return err;
    if (a->cell[1]->type != CELL_PAIR) {
        return make_cell_error(
            "assq: arg 2 must be a pair",
            TYPE_ERR);
    }

    const Cell* p = a->cell[1];
    const Cell* obj = a->cell[0];
    while (p->type != CELL_NIL) {
        /* eq? == direct pointer equality */
        if (p->car->type != CELL_PAIR) {
            return make_cell_error(
                "assq: arg 2 must be an association list",
                VALUE_ERR);
        }
        if (p->car->car == obj) {
            return p->car;
        }
        p = p->cdr;
    }
    return False_Obj;
}


/* (assv obj alist )
 * Find the first pair in alist whose car field is obj, and returns that pair. If no pair in alist has obj as its car,
 * then #f is returned. Uses eqv? for the comparison. */
Cell* builtin_assv(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "assv");
    if (err) return err;
    if (a->cell[1]->type != CELL_PAIR) {
        return make_cell_error(
            "assv: arg 2 must be a pair",
            TYPE_ERR);
    }

    const Cell* p = a->cell[1];
    const Cell* obj = a->cell[0];
    while (p->type != CELL_NIL) {
        if (p->car->type != CELL_PAIR) {
            return make_cell_error(
                "assq: arg 2 must be an association list",
                VALUE_ERR);
        }
        const Cell* p_test = builtin_eqv(e, make_sexpr_len2(p->car->car, obj));
        if (p_test->boolean_v == 1) {
            return p->car;
        }
        p = p->cdr;
    }
    return False_Obj;
}


/* (assoc obj alist )
 * (assoc obj alist compare)
 * Find the first pair in alist whose car field is obj, and returns that pair. If no pair in alist has obj as its car,
 * then #f is returned. Uses eq? for the comparison, unless an optional procedure is passed as the third arg. */
Cell* builtin_assoc(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3, "assoc");
    if (err) return err;
    if (a->cell[1]->type != CELL_PAIR) {
        return make_cell_error(
            "assoc: arg 2 must be a pair",
            TYPE_ERR);
    }
    if (a->count == 3) {
        if (a->cell[2]->type != CELL_PROC) {
            return make_cell_error(
                "assoc: arg 3 must be a procedure",
                TYPE_ERR);
        }
    }

    const Cell* p = a->cell[1];
    const Cell* obj = a->cell[0];
    while (p->type != CELL_NIL) {
        if (p->car->type != CELL_PAIR) {
            return make_cell_error(
                "assoc: arg 2 must be an association list",
                VALUE_ERR);
        }
        if (a->count == 2) {
            const Cell* p_test = builtin_equal(e, make_sexpr_len2(p->car->car, obj));
            if (p_test->boolean_v == 1) {
                return p->car;
            }
        } else {
            Cell* args = make_cell_sexpr();
            cell_add(args, a->cell[2]);
            cell_add(args, p->car->car);
            cell_add(args, (Cell*)obj);
            const Cell *p_test = coz_eval((Lex *) e, args);
            if (p_test->boolean_v == 1) {
                return p->car;
            }
        }
        p = p->cdr;
    }
    return False_Obj;
}


/* (list-copy obj )
* Returns a newly allocated copy of the given obj if it is a list. Only the pairs themselves are
* copied; the cars of the result are the same (in the sense of eqv?) as the cars of list. If obj is
* an improper list, so is the result, and the final cdr's are the same in the sense of eqv?. An obj
* which is not a list is returned unchanged. It is an error if obj is a circular list. */
Cell* builtin_list_copy(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "list-copy");
    if (err) return err;
    /* Non-pairs get returned unaltered */
    if (a->cell[0]->type != CELL_PAIR) {
        return a->cell[0];
    }

    const Cell* old_list = a->cell[0];
    Cell* new_list_head = make_cell_pair(old_list->car, nullptr);

    /* Runner pointers for iteration */
    Cell* new_p = new_list_head;
    const Cell* old_p = old_list;
    while (old_p->cdr->type != CELL_NIL && old_p->cdr->type == CELL_PAIR) {
        /* Advance the old pointer */
        old_p = old_p->cdr;
        /* Create a new cell and link it */
        new_p->cdr = make_cell_pair(old_p->car, nullptr);
        /* Advance the new pointer */
        new_p = new_p->cdr;
    }

    /* Now, handle the final cdr.
     * This correctly handles both proper and improper lists. */
    new_p->cdr = old_p->cdr;
    /* Return the original head pointer */
    return new_list_head;
}

/* ----------------------------------------------------------*
 *                 List iteration procedures                 *
 * ----------------------------------------------------------*/


Cell* builtin_filter(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "filter");
    if (err) return err;
    if (a->cell[0]->type != CELL_PROC) {
        return make_cell_error(
            "filter: arg 1 must be a procedure",
            TYPE_ERR);
    }
    /* Empty list arg :: empty list result */
    if (a->cell[1]->type == CELL_NIL) {
        return make_cell_nil();
    }
    if (a->cell[1]->type != CELL_PAIR || a->cell[1]->len < 1 ) {
        return make_cell_error(
            "filter: arg 2 must be a proper list",
            TYPE_ERR);
    }

    const Cell* proc = a->cell[0];
    Cell* result = make_cell_nil();
    const Cell* val = a->cell[1];
    for (int i = 0; i < a->cell[1]->len; i++) {
        Cell* pred_outcome;
        /* If proc is a builtin, run it directly.
         * Otherwise, send it to apply. */
        if (proc->is_builtin) {
            Cell* (*func)(const Lex *, const Cell *) = proc->builtin;
            pred_outcome = func(e, make_sexpr_len1(val->car));
        } else {
            pred_outcome = coz_apply_and_get_val(proc, make_sexpr_len1(val->car), (Lex*)e);
        }
        if (pred_outcome->type == CELL_ERROR) {
            return pred_outcome;
        }
        /* Continue if pred isn't true/truthy. */
        if (pred_outcome->type == CELL_BOOLEAN && pred_outcome->boolean_v == 0) {
            val = val->cdr;
            continue;
        }
        /* Otherwise write it to the result list. */
        result = make_cell_pair(val->car, result);
        val = val->cdr;
    }
    return builtin_list_reverse(e, make_sexpr_len1(result));
}


Cell* builtin_foldl(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 3, "foldl");
    if (err) return err;
    if (a->cell[0]->type != CELL_PROC) {
        return make_cell_error(
            "foldl: arg 1 must be a procedure",
            TYPE_ERR);
    }

    int shortest_list_length = INT32_MAX;
    for (int i = 2; i < a->count; i++) {
        /* If any of the list args is empty, return the accumulator. */
        if (a->cell[i]->type == CELL_NIL) {
            return a->cell[1];
        }

        if (a->cell[i]->type != CELL_PAIR || a->cell[i]->len == -1) {
            return make_cell_error(
                fmt_err("foldl: arg %d must be a proper list", i+1),
                TYPE_ERR);
        }
        if (a->cell[i]->len < shortest_list_length) {
            shortest_list_length = a->cell[i]->len;
        }
    }

    const int shortest_len = shortest_list_length;
    const int num_lists = a->count - 2;
    const Cell* proc = a->cell[0];
    Cell* init = a->cell[1];

    for (int i = 0; i < shortest_len; i++) {
        Cell* arg_list = make_cell_sexpr();
        /* Add the acc arg. */
        cell_add(arg_list, init);
        for (int j = 2; j < 2 + num_lists; j++) {
            /* Add the i-th element of the list argument(s). */
            const Cell* current_list = a->cell[j];
            Cell* nth_item = list_get_nth_cell_ptr(current_list, i);
            cell_add(arg_list, nth_item);
        }

        Cell* tmp_result;
        /* If the procedure is a builtin - grab a pointer to it and call it directly
         * otherwise - it is a lambda and needs to be evaluated and applied to the args. */
        if (proc->is_builtin) {
            Cell* (*func)(const Lex *, const Cell *) = proc->builtin;
            tmp_result = func(e, arg_list);
        } else {
            tmp_result = coz_apply_and_get_val(proc, arg_list, (Lex*)e);
        }
        if (tmp_result->type == CELL_ERROR) {
            /* Propagate any evaluation errors. */
            return tmp_result;
        }
        /* assign the result to the accumulator. */
        init = tmp_result;
    }
    /* Return the accumulator after all list args are evaluated. */
    return init;
}


Cell* builtin_foldr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 3, "foldr");
    if (err) return err;
    if (a->cell[0]->type != CELL_PROC) {
        return make_cell_error(
            "foldr: arg 1 must be a procedure",
            TYPE_ERR);
    }

    int shortest_list_length = INT32_MAX;
    for (int i = 2; i < a->count; i++) {
        /* If any of the list args is empty, return the accumulator. */
        if (a->cell[i]->type == CELL_NIL) {
            return a->cell[1];
        }

        if (a->cell[i]->type != CELL_PAIR || a->cell[i]->len == -1) {
            return make_cell_error(
                fmt_err("foldr: arg %d must be a proper list", i+1),
                TYPE_ERR);
        }
        if (a->cell[i]->len < shortest_list_length) {
            shortest_list_length = a->cell[i]->len;
        }
    }

    const int shortest_len = shortest_list_length;
    const int num_lists = a->count - 2;
    const Cell* proc = a->cell[0];
    Cell* init = a->cell[1];

    /* Grab elements from the end of the lists */
    for (int i = shortest_len - 1; i >= 0; i--) {
        Cell* arg_list = make_cell_sexpr();
        for (int j = 2; j < 2 + num_lists; j++) {
            /* Add the i-th element of the list argument(s). */
            const Cell* current_list = a->cell[j];
            Cell* nth_item = list_get_nth_cell_ptr(current_list, i);
            cell_add(arg_list, nth_item);
        }
        /* Add the acc arg LAST for foldr. */
        cell_add(arg_list, init);

        Cell* tmp_result;
        /* If the procedure is a builtin - grab a pointer to it and call it directly
         * otherwise - it is a lambda and needs to be evaluated and applied to the args. */
        if (proc->is_builtin) {
            Cell* (*func)(const Lex *, const Cell *) = proc->builtin;
            tmp_result = func(e, arg_list);
        } else {
            tmp_result = coz_apply_and_get_val(proc, arg_list, (Lex*)e);
        }
        if (tmp_result->type == CELL_ERROR) {
            /* Propagate any evaluation errors. */
            return tmp_result;
        }
        /* assign the result to the accumulator. */
        init = tmp_result;
    }
    /* Return the accumulator after all list args are evaluated. */
    return init;
}


Cell* builtin_zip(const Lex* e, const Cell* a)
{
    (void)e;
    /* no args case. */
    if (a->count == 0) {
        return Nil_Obj;
    }

    int shortest_list_length = INT32_MAX;
    for (int i = 0; i < a->count; i++) {
        /* If any of the list args is empty, return an empty list. */
        if (a->cell[i]->type == CELL_NIL) {
            return make_cell_nil();
        }

        if (a->cell[i]->type != CELL_PAIR || a->cell[i]->len == -1) {
            return make_cell_error(
                fmt_err("zip: arg %d must be a proper list", i+1),
                TYPE_ERR);
        }
        if (a->cell[i]->len < shortest_list_length) {
            shortest_list_length = a->cell[i]->len;
        }
    }

    const int shortest_len = shortest_list_length;
    const int num_lists = a->count;
    Cell* outer_list = make_cell_nil();
    for (int i = 0; i < shortest_len; i++) {
        Cell* inner_list = make_cell_nil();

        /* Grab vals starting from the last list, so that after the
         * 'reversed' list is constructed, order is correct. */
        for (int j = num_lists-1 ; j >= 0; j--) {
            const Cell* current_list = a->cell[j];
            Cell* nth_item = list_get_nth_cell_ptr(current_list, i);
            inner_list = make_cell_pair(nth_item, inner_list);
            /* len is the number of lists. */
            inner_list->len = num_lists;
        }
        outer_list = make_cell_pair(inner_list, outer_list);
    }
    outer_list->len = num_lists;
    return builtin_list_reverse(e, make_sexpr_len1(outer_list));
}


/* (count pred list)
 * Returns the count of objects in list for which pred returns true. */
Cell* builtin_count(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "count");
    if (err) return err;
    if (a->cell[0]->type != CELL_PROC) {
        return make_cell_error(
            "count: arg 1 must be a predicate procedure",
            TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_PAIR || a->cell[0]->len == -1) {
        return make_cell_error(
            "count: arg 2 must be a list",
            TYPE_ERR);
    }
    /* The arg list. */
    const Cell* l = a->cell[1];
    /* The predicate. */
    const Cell* p = a->cell[0];
    int64_t count = 0;
    while (l != Nil_Obj) {
        Cell* tmp_result;
        /* If the procedure is a builtin - grab a pointer to it and call it directly
         * otherwise - it is a lambda and needs to be evaluated and applied to the args. */
        if (p->is_builtin) {
            Cell* (*func)(const Lex *, const Cell *) = p->builtin;
            tmp_result = func(e, make_sexpr_len1(l->car));
        } else {
            tmp_result = coz_apply_and_get_val(p, make_sexpr_len1(l->car), (Lex*)e);
        }
        if (tmp_result->type == CELL_ERROR) {
            /* Propagate any evaluation errors. */
            return tmp_result;
        }
        /* Ensure the proc returned a boolean. */
        if (tmp_result->type != CELL_BOOLEAN) {
            return make_cell_error(
                "count: arg one must be a predicate procedure",
                TYPE_ERR);
        }
        /* If the result is #t, bump the counter. */
        if (tmp_result->boolean_v == 1) {
            count++;
        }
        /* Adjust pointer to next value in the list. */
        l = l->cdr;
    }
    return make_cell_integer(count);
}


/* (count-equal obj list)
 * Returns the number of occurrences of obj in list.
 * Uses equal? for the comparison. */
Cell* builtin_count_equal(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2, "count-equal");
    if (err) return err;
    if (a->cell[1]->type != CELL_PAIR || a->cell[0]->len == -1) {
        return make_cell_error("count-equal: arg 2 must be a list", TYPE_ERR);
    }
    /* The haystack. */
    const Cell* l = a->cell[1];
    /* The needle. */
    const Cell* s = a->cell[0];
    int64_t count = 0;
    while (l != Nil_Obj) {
        /* (if (equal? s (car l)). */
        if (builtin_equal(e, make_sexpr_len2(s, l->car))->boolean_v) {
            count++;
        }
        l = l->cdr;
    }
    return make_cell_integer(count);
}
