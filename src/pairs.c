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


/* ----------------------------------------------------------*
 *     pair/list constructors, selectors, and procedures     *
 * ----------------------------------------------------------*/

/* 'cons' -> VAL_PAIR - returns a pair made from two arguments */
Cell* builtin_cons(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    return make_val_pair(cell_copy(a->cell[0]), cell_copy(a->cell[1]));
}

/* 'car' -> ANY - returns the first member of a pair */
Cell* builtin_car(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_PAIR|VAL_SEXPR);
    if (err) { return err; }

    if (a->cell[0]->type == VAL_PAIR) {
        return cell_copy(a->cell[0]->car);
    }
    /* This is for the case where the argument list was quoted.
     * It needs to be transformed into a list before taking the car */
    Cell* list = builtin_list(e, a->cell[0]);
    Cell* result = cell_copy(list->car);
    return result;
}

/* 'cdr' -> ANY - returns the second member of a pair */
Cell* builtin_cdr(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_PAIR|VAL_SEXPR);
    if (err) { return err; }

    if (a->cell[0]->type == VAL_PAIR) {
        return cell_copy(a->cell[0]->cdr);
    }
    /* This is for the case where the argument list was quoted.
     * It needs to be transformed into a list before taking the cdr */
    const Cell* list = builtin_list(e, a->cell[0]);
    Cell* result = cell_copy(list->cdr);
    return result;
}

/* 'list' -> VAL_PAIR - returns a nil-terminated proper list */
Cell* builtin_list(Lex* e, Cell* a) {
    (void)e;
    /* start with nil */
    Cell* result = make_val_nil();

    const int len = a->count;
    /* build backwards so it comes out in the right order */
    for (int i = len - 1; i >= 0; i--) {
        result = make_val_pair(cell_copy(a->cell[i]), result);
        result->len = len - i;
    }
    return result;
}

/* 'length' -> VAL_INT - returns the member count of a proper list */
Cell* builtin_list_length(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, VAL_PAIR|VAL_NIL);
    if (err) { return err; }

    const Cell* list = a->cell[0];

    if (list->type == VAL_NIL) {
        return make_val_int(0);
    }

    /* If len is not -1, we can trust the cached value. */
    if (list->len != -1) {
        return make_val_int(list->len);
    }

    /* If len is -1, this could be an improper list or a proper list
     * built with `cons`. We need to traverse it to find out. */
    int count = 0;
    const Cell* p = list;
    while (p->type == VAL_PAIR) {
        count++;
        p = p->cdr;
    }
    /* The R7RS standard for `length` requires a proper list.
     * If the list doesn't end in `nil`, it's an error. */
    if (p->type != VAL_NIL) {
        return make_val_err("length: proper list required", GEN_ERR);
    }

    /* It's a proper list. */
    return make_val_int(count);
}

/* 'list-ref' -> ANY - returns the list member at the zero-indexed
 * integer specified in the second arg. First arg is the list to act on*/
Cell* builtin_list_ref(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    if (a->cell[0]->type != VAL_PAIR) {
        return make_val_err("list-ref: arg 1 must be a list", GEN_ERR);
    }
    if (a->cell[1]->type != VAL_INT) {
        return make_val_err("list-ref: arg 2 must be an integer", GEN_ERR);
    }
    int i = (int)a->cell[1]->i_val;
    const int len = a->cell[0]->len;

    if (i >= len && len != -1) {
        return make_val_err("list-ref: arg 2 out of range", GEN_ERR);
    }

    Cell* p = a->cell[0];
    while (i > 0) {
        p = p->cdr;
        i--;
    }
    return cell_copy(p->car);
}

/* 'list-append' -> VAL_PAIR - returns a proper list of all
 * args appended to the result, in order */
Cell* builtin_list_append(Lex* e, Cell* a) {
    (void)e;
    /* Base case: (append) -> '() */
    if (a->count == 0) {
        return make_val_nil();
    }
    /* Base case: (append x) -> x */
    if (a->count == 1) {
        return cell_copy(a->cell[0]);
    }

    /* Validate args and calculate total length of copied lists */
    long long total_copied_len = 0;
    for (int i = 0; i < a->count - 1; i++) {
        const Cell* current_list = a->cell[i];
        if (current_list->type == VAL_NIL) {
            continue; /* This is a proper, empty list. */
        }
        /* All but the last argument must be a list */
        if (current_list->type != VAL_PAIR) {
            return make_val_err("append: argument is not a list", GEN_ERR);
        }

        /* Now, walk the list to ensure it's a *proper* list */
        const Cell* p = current_list;
        while (p->type == VAL_PAIR) {
            p = p->cdr;
        }
        if (p->type != VAL_NIL) {
            return make_val_err("append: middle argument is not a proper list", GEN_ERR);
        }

        /* If we get here, the list is proper. Add its length. */
        total_copied_len += current_list->len;
    }

    /* Determine the final list's properties based on the last argument */
    const Cell* last_arg = a->cell[a->count - 1];
    long long final_total_len = -1; /* Use -1 to signify an improper list. */

    if (last_arg->type == VAL_NIL) {
        final_total_len = total_copied_len;
    } else if (last_arg->type == VAL_PAIR) {
        /* If the last arg has a valid length, the result will be a proper list. */
        if (last_arg->len != -1) {
             final_total_len = total_copied_len + last_arg->len;
        }
    }

    /* Build the new list structure */
    Cell* result_head = make_val_nil();
    Cell* result_tail = NULL;
    long long len_countdown = final_total_len;

    for (int i = 0; i < a->count - 1; i++) {
        const Cell* p = a->cell[i];
        while (p->type == VAL_PAIR) {
            /* Create a new pair with a copy of the element. */
            Cell* new_pair = make_val_pair(cell_copy(p->car), make_val_nil());

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
        return cell_copy(last_arg);
    }
    /* Splice the last argument onto the end of our newly created list. */
    result_tail->cdr = (Cell*)last_arg;
    return result_head;
}

/* 'reverse' -> VAL_PAIR - returns a proper list with members of
 * arg reversed */
Cell* builtin_list_reverse(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, VAL_PAIR|VAL_NIL);
    if (err) { return err; }

    const Cell* original_list = a->cell[0];
    if (original_list->type == VAL_NIL) {
        return make_val_nil();
    }

    Cell* reversed_list = make_val_nil();
    const Cell* current = original_list;
    int length = 0;

    while (current->type == VAL_PAIR) {
        reversed_list = make_val_pair(cell_copy(current->car), reversed_list);
        length++;
        current = current->cdr;
    }

    if (current->type != VAL_NIL) {
        return make_val_err("reverse: cannot reverse improper list", GEN_ERR);
    }

    /* Set the length on the final result. */
    if (reversed_list->type != VAL_NIL) {
        reversed_list->len = length;
    }
    return reversed_list;
}


/* 'list-tail' -> VAL_PAIR - returns a proper list of the last nth
 * members of arg */
Cell* builtin_list_tail(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    if (a->cell[0]->type != VAL_PAIR &&
        a->cell[0]->type != VAL_SEXPR &&
        a->cell[0]->type != VAL_NIL) {
        return make_val_err("list-tail: arg 1 must be a list", GEN_ERR);
    }
    if (a->cell[1]->type != VAL_INT) {
        return make_val_err("list-tail: arg 2 must be an integer", GEN_ERR);
    }

    Cell* lst = a->cell[0];
    const long k = a->cell[1]->i_val;

    if (k < 0) {
        return make_val_err("list-tail: arg 2 must be non-negative", GEN_ERR);
    }

    Cell* p = lst;
    for (long i = 0; i < k; i++) {
        if (p->type != VAL_PAIR) {
            return make_val_err("list-tail: arg 2 out of range", GEN_ERR);
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


Cell* builtin_filter(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    if (a->cell[0]->type != VAL_PROC) {
        return make_val_err("filter: arg 1 must be a procedure", GEN_ERR);
    }
    if (a->cell[1]->type != VAL_PAIR && a->cell[1]->len == -1) {
        return make_val_err("filter: arg 2 must be a proper list", GEN_ERR);
    }

    const Cell* proc = a->cell[0];
    Cell* result = make_val_nil();
    const Cell* val = a->cell[1];
    for (int i = 0; i < a->cell[1]->len; i++) {
        Cell* pred_outcome = coz_eval(e, make_sexpr_len2(proc, val->car));
        if (pred_outcome->type == VAL_ERR) {
            return pred_outcome;
        }
        /* Copy val to result list if pred is true */
        if (pred_outcome->b_val == 1) {
            result = make_val_pair(cell_copy(val->car), result);
        }
        val = val->cdr;
    }
    return builtin_list_reverse(e, make_sexpr_len1(result));
}

Cell* builtin_foldl(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 3);
    if (err) return err;
    if (a->cell[0]->type != VAL_PROC) {
        return make_val_err("foldl: arg 1 must be a procedure", GEN_ERR);
    }
    int shortest_list_length = INT32_MAX;

    for (int i = 2; i < a->count; i++) {
        char buf[128];
        if (a->cell[i]->type != VAL_PAIR || a->cell[i]->len == -1) {
            snprintf(buf, 128, "foldl: arg %d must be a proper list", i+1);
            return make_val_err(buf, GEN_ERR);
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
        Cell* arg_list = make_val_nil();
        /* cons the initial/accumulator */
        arg_list = make_val_pair(init, arg_list);
        /* Grab vals starting from the last list, so that after the
         * 'reversed' list is constructed, order is correct */
        for (int j = num_lists + 1; j >= 2; j--) {
            Cell* current_list = a->cell[j];
            Cell* nth_item = list_get_nth_cell_ptr(current_list, i);
            arg_list = make_val_pair(nth_item, arg_list);
            /* len is the number of lists plus the accumulator */
            arg_list->len = num_lists + 1;
        }

        Cell* tmp_result = NULL;
        /* If the procedure is a builtin - grab a pointer to it and call it directly
         * otherwise - it is a lambda and needs to be evaluated */
        if (proc->builtin) {
            Cell* (*func)(Lex *, Cell *) = proc->builtin;
            tmp_result = func(e, make_sexpr_from_list(arg_list));
        } else {
            /* Prepend the procedure to create the application form */
            Cell* application_list = make_val_pair(proc, arg_list);
            application_list->len = arg_list->len + 1;

            /* Convert the Scheme list to an S-expression for eval */
            Cell* application_sexpr = make_sexpr_from_list(application_list);

            /* Evaluate it */
            tmp_result = coz_eval(e, application_sexpr);
        }
        if (tmp_result->type == VAL_ERR) {
            /* Propagate any evaluation errors */
            return tmp_result;
        }
        /* assign the result to the accumulator */
        init = tmp_result;
    }
    /* Return the accumulator after all list args are evaluated */
    return init;
}
