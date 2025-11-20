/*
 * 'control_features.c'
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

#include "control_features.h"
#include "types.h"
#include "eval.h"
#include "pairs.h"
#include "strings.h"
#include "runner.h"
#include "lexer.h"
#include "repl.h"
#include "repr.h"

#include <stdlib.h>


extern int is_repl;
extern int g_argc;
extern char **g_argv;

/*-------------------------------------------------------*
 *    Control features and list iteration procedures     *
 * ------------------------------------------------------*/

Cell* builtin_eval(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;
    Cell* args;
    /* Convert list to s-expr if we are handed a quote */
    if (a->cell[0]->type == CELL_PAIR) {
        args = make_sexpr_from_list(a->cell[0]);
        for (int i = 0; i < args->count; i++ ) {
            if (args->cell[i]->type == CELL_PAIR && args->cell[i]->len != -1) {
                Cell* tmp = args->cell[i];
                args->cell[i] = make_sexpr_from_list(tmp);
            }
        }
        /* Otherwise just send straight to eval */
    } else {
        args = a->cell[0];
    }
    return coz_eval((Lex*)e, args);
}

/* (apply proc arg1 ... args)
 * The apply procedure calls proc with the elements of the list (append (list arg1 ... ) args)
 * as the actual arguments. */
Cell* builtin_apply(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_PROC) {
        return make_cell_error(
            "apply: arg 1 must be a procedure",
            ARITY_ERR);
    }
    Cell* final_sexpr = make_cell_sexpr();
    /* Add the proc */
    cell_add(final_sexpr, a->cell[0]);
    /* Collect individual args, if any */
    const int last_arg_index = a->count - 1;
    for (int i = 1; i < last_arg_index; i++) {
        cell_add(final_sexpr, a->cell[i]);
    }
    const Cell* final_list = a->cell[last_arg_index];
    /* Ensure last arg is a list */
    if (final_list->type != CELL_PAIR || final_list->len == -1) {
        return make_cell_error(
            "apply: last arg must be a proper list",
            TYPE_ERR);
    }
    const Cell* current_item = final_list;
    while (current_item->type != CELL_NIL) {
        cell_add(final_sexpr, current_item->car);
        current_item = current_item->cdr;
    }
    /* Give the s-expr a gentle kiss on the forehead,
     * and make it a CELL_TRAMPOLINE... */
    final_sexpr->type = CELL_TRAMPOLINE;
    return final_sexpr;
}


/* (map proc list1 list2 ... )
* It is an error if proc does not accept as many arguments as there are lists and return a single value. The map
* procedure applies proc element-wise to the elements of the lists and returns a list of the results, in order. If more
* than one list is given and not all lists have the same length, map terminates when the shortest list runs out. The
* lists can be circular, but it is an error if all of them are circular. It is an error for proc to mutate any of the
* lists. */
Cell* builtin_map(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_PROC) {
        return make_cell_error(
            "map: arg 1 must be a procedure",
            TYPE_ERR);
    }
    int shortest_list_length = INT32_MAX;
    for (int i = 1; i < a->count; i++) {
        /* If list arg is empty, return empty list */
        if (a->cell[i]->type == CELL_NIL) {
            return make_cell_nil();
        }

        if (a->cell[i]->type != CELL_PAIR || a->cell[i]->len == -1) {
            return make_cell_error(
                fmt_err("map: arg %d must be a proper list", i+1),
                TYPE_ERR);
        }
        if (a->cell[i]->len < shortest_list_length) {
            shortest_list_length = a->cell[i]->len;
        }
    }

    const int shortest_len = shortest_list_length;
    const int num_lists = a->count - 1;
    const Cell* proc = a->cell[0];

    Cell* final_result = make_cell_nil();

    for (int i = 0; i < shortest_len; i++) {
        /* Build a (reversed) list of the i-th arguments */
        Cell* arg_list = make_cell_nil();
        for (int j = 0; j < num_lists; j++) {
            const Cell* current_list = a->cell[j + 1];
            Cell* nth_item = list_get_nth_cell_ptr(current_list, i);
            arg_list = make_cell_pair(nth_item, arg_list);
            arg_list->len = j + 1;
        }

        /* Correct the argument order */
        Cell* reversed_arg_list = builtin_list_reverse(e, make_sexpr_len1(arg_list));

        Cell* tmp_result;
        /* If the procedure is a builtin - grab a pointer to it and call it directly
         * otherwise - it is a lambda and needs to be evaluated and applied to the args. */
        if (proc->is_builtin) {
            Cell* (*func)(const Lex *, const Cell *) = proc->builtin;
            tmp_result = func(e, make_sexpr_from_list(reversed_arg_list));
        } else {
            Cell* arg_sexpr = make_sexpr_from_list(reversed_arg_list);
            tmp_result = coz_apply_and_get_val(proc, arg_sexpr, (Lex*)e);
        }
        /* Deal with legitimate null result */
        if (!tmp_result) continue;
        if (tmp_result->type == CELL_ERROR) {
            /* Propagate any evaluation errors */
            return tmp_result;
        }

        /* Cons the result onto our (reversed) final list */
        final_result = make_cell_pair(tmp_result, final_result);
        final_result->len = i + 1;
    }

    /* Reverse the final list to get the correct order and return */
    return builtin_list_reverse(e, make_sexpr_len1(final_result));
}


/* (vector-map proc vector1 vector2 ... )
* It is an error if proc does not accept as many arguments as there are vectors and return a single value. The
* vector-map procedure applies proc element-wise to the elements of the vectors and returns a vector of the results,
* in order. If more than one vector is given and not all vectors have the same length, vector-map terminates when the
* shortest vector runs out. */
Cell* builtin_vector_map(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_PROC) {
        return make_cell_error(
            "vector-map: arg 1 must be a procedure",
            TYPE_ERR);
    }
    int shortest_vec_length = INT32_MAX;
    for (int i = 1; i < a->count; i++) {
        /* If vector arg is empty, return empty vector. */
        if (a->cell[i]->count == 0) {
            return make_cell_vector();
        }

        if (a->cell[i]->type != CELL_VECTOR) {
            return make_cell_error(
                fmt_err("vector-map: arg %d must be a proper list", i+1),
                TYPE_ERR);
        }
        if (a->cell[i]->len < shortest_vec_length) {
            shortest_vec_length = a->cell[i]->len;
        }
    }

    const int shortest_len = shortest_vec_length;
    const int num_args = a->count - 1;
    const Cell* proc = a->cell[0];

    Cell* final_result = make_cell_vector();

    for (int i = 0; i < shortest_len; i++) {
        /* Build a S-expr of the i-th arguments */
        Cell* arg_list = make_cell_sexpr();
        for (int j = 0; j < num_args; j++) {
            const Cell* current_vec = a->cell[j + 1];
            Cell* nth_item = current_vec->cell[i];
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
        /* Deal with legitimate null value. */
        if (!tmp_result) continue;
        if (tmp_result->type == CELL_ERROR) {
            /* Propagate any evaluation errors */
            return tmp_result;
        }

        /* Add tmp result to the final vector. */
        cell_add(final_result, tmp_result);
    }

    return final_result;
}

/* (string-map proc string1 string2 ... )
 * The string-map procedure applies proc element-wise to the elements of the strings and returns a string of the
 * results, in order. If more than one string is given and not all strings have the same length, string-map terminates
 * when the shortest string runs out. */
Cell* builtin_string_map(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;
    if (a->cell[0]->type != CELL_PROC) {
        return make_cell_error(
            "string-map: arg 1 must be a procedure",
            TYPE_ERR);
    }
    /* Build the S-expr to send to map. */
    Cell* sexp_for_map = make_cell_sexpr();
    /* Add the proc */
    cell_add(sexp_for_map, a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        /* Ensure we have nothing but lists in a[1:]. */
        if (a->cell[i]->type != CELL_STRING) {
            return make_cell_error(
                fmt_err("string-map: arg %d must be a string", i+1),
                TYPE_ERR);
        }
        Cell* str_to_lst = builtin_string_list(e, make_sexpr_len1(a->cell[i]));
        cell_add(sexp_for_map, str_to_lst);
    }

    /* We now have an S-exp which contains the proc, and a list
     * of chars for each string arg. Let's send it to map. */
    const Cell* result = builtin_map(e, sexp_for_map);
    Cell* str_result = builtin_list_string(e, make_sexpr_len1(result));
    return str_result;
}

/* (for-each proc list1 list2 ... )
 * The arguments to for-each are like the arguments to map, but for-each calls proc for its side effects
 * rather than for its values. */
Cell* builtin_foreach(const Lex* e, const Cell* a)
{
    /* Just send the args to map and discard results. */
    (void)builtin_map(e, a);
    return USP_Obj;
}


/* (vector-for-each proc vector1 vector2 ... )
 * The arguments to vector-for-each are like the arguments to vector-map, but vector-for-each calls proc for its side
 * effects rather than for its values. */
Cell* builtin_vector_foreach(const Lex* e, const Cell* a)
{
    /* Just send the args to vector-map and discard results. */
    (void)builtin_vector_map(e, a);
    return USP_Obj;
}


/* (string-for-each proc string1 string2 ... )
 * The arguments to string-for-each are like the arguments to string-map, but string-for-each calls proc for its side
 * effects rather than for its values. */
Cell* builtin_string_foreach(const Lex* e, const Cell* a)
{
    /* Just send the args to string-map and discard results. */
    (void)builtin_string_map(e, a);
    return USP_Obj;
}

Cell* builtin_load(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "load: arg must be a string",
            TYPE_ERR);
    }
    const char* file = a->cell[0]->str;
    const char* input = read_file_to_string(file);
    TokenArray* ta = scan_all_tokens(input);
    const Cell* result = parse_all_expressions((Lex*)e, ta, false);

    if (result && result->type == CELL_ERROR) {
        fprintf(stderr, "%s\n", cell_to_string(result, MODE_REPL));
        return False_Obj;
    }
    return True_Obj;
}

Cell* builtin_exit(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_BOOLEAN, "exit");
    if (err) { return err; }

    /* Save history if we're in REPL mode. */
    if (is_repl) {
        save_history_to_file();
    }

    /* Convert boolean value to exit success or failure. */
    if (a->count == 1) {
        if (a->cell[0]->type == CELL_BOOLEAN) {
            const int es = a->cell[0]->boolean_v;
            if (es) {
                exit(0); /* flip boolean 1 (#t) to exit success (0). */
            }
            exit(1);
        }
        /* If not bool, int. Just return directly. */
        exit((int)a->cell[0]->integer_v);
    }
    exit(0); /* exit success if no arg. */
}

Cell* builtin_command_line(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    /* Return list of just one empty string if using REPL. */
    if (is_repl) {
        return make_cell_pair(make_cell_string(""), make_cell_nil());
    }
    /* Construct the list of args. */
    Cell* args = make_cell_sexpr();
    for (int i = 0; i < g_argc; i++) {
        cell_add(args, make_cell_string(g_argv[i]));
    }
    return make_list_from_sexpr(args);
}
