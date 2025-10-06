/*
 * 'pairs.c'
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

#include "pairs.h"
#include "eval.h"
#include "types.h"


/* Helpers */
inline Cell* car__(const Cell* list) {
    if (!(list->type & CELL_PAIR)) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "car: got %s, expected %s",
                 cell_type_name(list->type),
                 cell_mask_types(CELL_PAIR));
        return make_cell_error(buf, TYPE_ERR);
    }
    return list->car;
}

inline Cell* cdr__(const Cell* list) {
    if (!(list->type & CELL_PAIR)) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "cdr: got %s, expected %s",
                 cell_type_name(list->type),
                 cell_mask_types(CELL_PAIR));
        return make_cell_error(buf, TYPE_ERR);
    }
    return list->cdr;
}

/* ----------------------------------------------------------*
 *     pair/list constructors, selectors, and procedures     *
 * ----------------------------------------------------------*/

/* 'cons' -> CELL_PAIR - returns a pair made from two arguments */
Cell* builtin_cons(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    return make_cell_pair(a->cell[0], a->cell[1]);
}

/* 'car' -> ANY - returns the first member of a pair */
Cell* builtin_car(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return car__(a->cell[0]);
}

/* 'cdr' -> ANY - returns the second member of a pair */
Cell* builtin_cdr(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return cdr__(a->cell[0]);
}

Cell* builtin_caar(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) { return err; }
    return car__(car__(a->cell[0]));
}

Cell* builtin_cadr(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) { return err; }
    return car__(cdr__(a->cell[0]));
}

Cell* builtin_cdar(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) { return err; }
    return cdr__(car__(a->cell[0]));
}

Cell* builtin_cddr(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) { return err; }
    return cdr__(cdr__(a->cell[0]));
}

/* 'list' -> CELL_PAIR - returns a nil-terminated proper list */
Cell* builtin_list(const Lex* e, const Cell* a) {
    (void)e;
    /* start with nil */
    Cell* result = make_cell_nil();

    const int len = a->count;
    /* build backwards so it comes out in the right order */
    for (int i = len - 1; i >= 0; i--) {
        result = make_cell_pair(a->cell[i], result);
        result->len = len - i;
    }
    return result;
}

/* 'length' -> CELL_INTEGER - returns the member count of a proper list */
Cell* builtin_list_length(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_PAIR|CELL_NIL);
    if (err) { return err; }

    const Cell* list = a->cell[0];

    if (list->type == CELL_NIL) {
        return make_cell_integer(0);
    }

    /* If len is not -1, we can trust the cached value. */
    if (list->len != -1) {
        return make_cell_integer(list->len);
    }

    /* If len is -1, this could be an improper list or a proper list
     * built with `cons`. We need to traverse it to find out. */
    int count = 0;
    const Cell* p = list;
    while (p->type == CELL_PAIR) {
        count++;
        p = p->cdr;
    }
    /* The R7RS standard for `length` requires a proper list.
     * If the list doesn't end in `nil`, it's an error. */
    if (p->type != CELL_NIL) {
        return make_cell_error("length: proper list required", TYPE_ERR);
    }

    /* It's a proper list. */
    return make_cell_integer(count);
}

/* 'list-ref' -> ANY - returns the list member at the zero-indexed
 * integer specified in the second arg. First arg is the list to act on*/
Cell* builtin_list_ref(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    if (a->cell[0]->type != CELL_PAIR) {
        return make_cell_error("list-ref: arg 1 must be a list", TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error("list-ref: arg 2 must be an integer", TYPE_ERR);
    }
    int i = (int)a->cell[1]->integer_v;
    const int len = a->cell[0]->len;

    if (i >= len && len != -1) {
        return make_cell_error("list-ref: arg 2 out of range", INDEX_ERR);
    }

    Cell* p = a->cell[0];
    while (i > 0) {
        p = p->cdr;
        i--;
    }
    return p->car;
}

/* 'list-append' -> CELL_PAIR - returns a proper list of all
 * args appended to the result, in order */
Cell* builtin_list_append(const Lex* e, const Cell* a) {
    (void)e;
    /* Base case: (append) -> '() */
    if (a->count == 0) {
        return make_cell_nil();
    }
    /* Base case: (append x) -> x */
    if (a->count == 1) {
        return a->cell[0];
    }

    /* Validate args and calculate total length of copied lists */
    long long total_copied_len = 0;
    for (int i = 0; i < a->count - 1; i++) {
        const Cell* current_list = a->cell[i];
        if (current_list->type == CELL_NIL) {
            continue; /* This is a proper, empty list. */
        }
        /* All but the last argument must be a list */
        if (current_list->type != CELL_PAIR) {
            char buf[128];
            sprintf(buf, "append: arg%d is not a list", i+1);
            return make_cell_error(buf, TYPE_ERR);
        }

        /* Now, walk the list to ensure it's a *proper* list */
        const Cell* p = current_list;
        while (p->type == CELL_PAIR) {
            p = p->cdr;
        }
        if (p->type != CELL_NIL) {
            char buf[128];
            sprintf(buf, "append: arg%d is not a proper list", i+1);
            return make_cell_error(buf, TYPE_ERR);
        }

        /* If we get here, the list is proper. Add its length. */
        total_copied_len += current_list->len;
    }

    /* Determine the final list's properties based on the last argument */
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

    /* Build the new list structure */
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

/* 'reverse' -> CELL_PAIR - returns a proper list with members of
 * arg reversed */
Cell* builtin_list_reverse(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_PAIR|CELL_NIL);
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
        return make_cell_error("reverse: cannot reverse improper list", TYPE_ERR);
    }

    /* Set the length on the final result. */
    if (reversed_list->type != CELL_NIL) {
        reversed_list->len = length;
    }
    return reversed_list;
}

/* 'list-tail' -> CELL_PAIR - returns a proper list of the last nth
 * members of arg */
Cell* builtin_list_tail(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    if (a->cell[0]->type != CELL_PAIR &&
        a->cell[0]->type != CELL_SEXPR &&
        a->cell[0]->type != CELL_NIL) {
        return make_cell_error("list-tail: arg 1 must be a list", TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_INTEGER) {
        return make_cell_error("list-tail: arg 2 must be an integer", TYPE_ERR);
    }

    Cell* lst = a->cell[0];
    const long k = a->cell[1]->integer_v;

    if (k < 0) {
        return make_cell_error("list-tail: arg 2 must be non-negative", VALUE_ERR);
    }

    Cell* p = lst;
    for (long i = 0; i < k; i++) {
        if (p->type != CELL_PAIR) {
            return make_cell_error("list-tail: arg 2 out of range", INDEX_ERR);
        }
        /* Move to the next element in the list */
        p = p->cdr;
    }
    /* After the loop, p is pointing at the k-th cdr of the original list. */
    return p;
}

/* ----------------------------------------------------------*
 *                 List iteration procedures                 *
 * ----------------------------------------------------------*/


Cell* builtin_filter(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_PROC) {
        return make_cell_error("filter: arg 1 must be a procedure", TYPE_ERR);
    }
    if (a->cell[1]->type != CELL_PAIR && a->cell[1]->len == -1) {
        return make_cell_error("filter: arg 2 must be a proper list", TYPE_ERR);
    }

    const Cell* proc = a->cell[0];
    Cell* result = make_cell_nil();
    const Cell* val = a->cell[1];
    for (int i = 0; i < a->cell[1]->len; i++) {
        Cell* pred_outcome = coz_eval((Lex*)e, make_sexpr_len2(proc, val->car));
        if (pred_outcome->type == CELL_ERROR) {
            return pred_outcome;
        }
        /* Create pointer to val in result list if pred is true */
        if (pred_outcome->boolean_v == 1) {
            result = make_cell_pair(val->car, result);
        }
        val = val->cdr;
    }
    return builtin_list_reverse(e, make_sexpr_len1(result));
}

Cell* builtin_foldl(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 3);
    if (err) return err;
    if (a->cell[0]->type != CELL_PROC) {
        return make_cell_error("foldl: arg 1 must be a procedure", TYPE_ERR);
    }
    int shortest_list_length = INT32_MAX;

    for (int i = 2; i < a->count; i++) {
        char buf[128];
        if (a->cell[i]->type != CELL_PAIR || a->cell[i]->len == -1) {
            snprintf(buf, 128, "foldl: arg %d must be a proper list", i+1);
            return make_cell_error(buf, TYPE_ERR);
        }
        if (a->cell[i]->len < shortest_list_length) {
            shortest_list_length = a->cell[i]->len;
        }
    }

    const int shortest_len = shortest_list_length;
    const int num_lists = a->count - 2;
    Cell* proc = a->cell[0];
    Cell* init = a->cell[1];

    for (int i = 0; i < shortest_len; i++) {
        /* Build a list of the i-th arguments */
        Cell* arg_list = make_cell_nil();
        /* cons the initial/accumulator */
        arg_list = make_cell_pair(init, arg_list);
        /* Grab vals starting from the last list, so that after the
         * 'reversed' list is constructed, order is correct */
        for (int j = num_lists + 1; j >= 2; j--) {
            Cell* current_list = a->cell[j];
            Cell* nth_item = list_get_nth_cell_ptr(current_list, i);
            arg_list = make_cell_pair(nth_item, arg_list);
            /* len is the number of lists plus the accumulator */
            arg_list->len = num_lists + 1;
        }

        Cell* tmp_result;
        /* If the procedure is a builtin - grab a pointer to it and call it directly
         * otherwise - it is a lambda and needs to be evaluated */
        if (proc->is_builtin) {
            Cell* (*func)(const Lex *, const Cell *) = proc->builtin;
            tmp_result = func(e, make_sexpr_from_list(arg_list));
        } else {
            /* Prepend the procedure to create the application form */
            Cell* application_list = make_cell_pair(proc, arg_list);
            application_list->len = arg_list->len + 1;

            /* Convert the Scheme list to an S-expression for eval */
            Cell* application_sexpr = make_sexpr_from_list(application_list);

            /* Evaluate it */
            tmp_result = coz_eval((Lex*)e, application_sexpr);
        }
        if (tmp_result->type == CELL_ERROR) {
            /* Propagate any evaluation errors */
            return tmp_result;
        }
        /* assign the result to the accumulator */
        init = tmp_result;
    }
    /* Return the accumulator after all list args are evaluated */
    return init;
}
