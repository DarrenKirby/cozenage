/*
 * 'src/control_features.c'
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
#include "runner.h"
#include "lexer.h"
#include "polymorph.h"
#include "repl.h"
#include "repr.h"

#include <stdlib.h>
#include <gc/gc.h>
#include <unicode/utf8.h>


extern int is_repl;
extern int g_argc;
extern char **g_argv;


/*-------------------------------------------------------*
 *    Control features and list iteration procedures     *
 * ------------------------------------------------------*/


Cell* builtin_eval(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 1, "eval");
    if (err) return err;
    Cell* args;
    /* Convert list to s-expr if we are handed a quote. */
    if (a->cell[0]->type == CELL_PAIR) {
        args = make_sexpr_from_list(a->cell[0]);
        for (int i = 0; i < args->count; i++ ) {
            if (args->cell[i]->type == CELL_PAIR && args->cell[i]->len != -1) {
                Cell* tmp = args->cell[i];
                args->cell[i] = make_sexpr_from_list(tmp);
            }
        }
        /* Otherwise just send straight to eval. */
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
    Cell* err = CHECK_ARITY_MIN(a, 2, "apply");
    if (err) return err;
    if (a->cell[0]->type != CELL_PROC) {
        return make_cell_error(
            "apply: arg 1 must be a procedure",
            ARITY_ERR);
    }
    Cell* final_sexpr = make_cell_sexpr();
    /* Add the proc. */
    cell_add(final_sexpr, a->cell[0]);
    /* Collect individual args, if any. */
    const int last_arg_index = a->count - 1;
    for (int i = 1; i < last_arg_index; i++) {
        cell_add(final_sexpr, a->cell[i]);
    }
    const Cell* final_list = a->cell[last_arg_index];
    /* Ensure last arg is a list. */
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
    Cell* err = CHECK_ARITY_MIN(a, 2, "map");
    if (err) return err;

    const Cell* proc = a->cell[0];
    if (proc->type != CELL_PROC)
        return make_cell_error(
            "map: arg 1 must be a procedure",
            TYPE_ERR);

    const int num_lists = a->count - 1;
    int shortest_len = INT32_MAX;

    /* Validation and Length Hint check. */
    /* Create an array of pointers to track our position in each list. */
    const Cell** cursors = GC_MALLOC(sizeof(Cell*) * num_lists);

    for (int i = 0; i < num_lists; i++) {
        Cell* lst = a->cell[i + 1];
        if (lst->type == CELL_NIL) return make_cell_nil();
        if (lst->type != CELL_PAIR) {
            return make_cell_error(
                fmt_err("map: arg %d must be a proper list", i+2),
                TYPE_ERR);
        }

        /* If len is -1, we can't trust the cache; calculate it once now. */
        if (lst->len <= 0) {
            const Cell* len_obj = builtin_list_length(e, make_sexpr_len1(lst));

            /* If this call fails, it's because lst is an improper list,
             * so we'll return an appropriate error message, rather than
             * the error from list_length(); */
            if (len_obj->type == CELL_ERROR) {
                return make_cell_error(
                fmt_err("map: arg %d must be a proper list", i+2),
                TYPE_ERR);
            }
            lst->len = (int)len_obj->integer_v;
        }

        if (lst->len < shortest_len) shortest_len = lst->len;
        cursors[i] = lst;
    }

    /* Main Loop: $O(N)$ execution. */
    Cell* head = make_cell_nil();
    Cell* tail = nullptr;

    for (int i = 0; i < shortest_len; i++) {
        /* Prepare the argument list for this iteration. */
        Cell* args_sexpr = make_cell_sexpr();

        for (int j = 0; j < num_lists; j++) {
            cell_add(args_sexpr, cursors[j]->car);

            /* Advance the cursor for the next iteration. */
            cursors[j] = cursors[j]->cdr;
        }

        /* Apply procedure. */
        Cell* val;

        if (proc->is_builtin) {
            val = proc->builtin(e, args_sexpr);
        } else {
            val = coz_apply_and_get_val(proc, args_sexpr, (Lex*)e);
        }

        if (val && val->type == CELL_ERROR) return val;
        /* Ignore unspecified results. */
        if (val == USP_Obj) continue;

        /* Append to final result list. */
        Cell* result_pair = make_cell_pair(val, make_cell_nil());
        result_pair->len = shortest_len - i;

        if (!tail) {
            head = result_pair;
            tail = result_pair;
        } else {
            tail->cdr = result_pair;
            tail = result_pair;
        }
    }
    return head;
}


/* (vector-map proc vector1 vector2 ... )
* It is an error if proc does not accept as many arguments as there are vectors and return a single value. The
* vector-map procedure applies proc element-wise to the elements of the vectors and returns a vector of the results,
* in order. If more than one vector is given and not all vectors have the same length, vector-map terminates when the
* shortest vector runs out. */
Cell* builtin_vector_map(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 2, "vector-map");
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
        /* Build a S-expr of the i-th arguments. */
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
    Cell* err = CHECK_ARITY_MIN(a, 2, "string-map");
    if (err) return err;

    const Cell* proc = a->cell[0];
    if (proc->type != CELL_PROC)
        return make_cell_error(
            "string-map: arg 1 must be a procedure",
            TYPE_ERR);

    const int num_strings = a->count - 1;
    int shortest_len = INT32_MAX;

    /* Setup Cursors and Find Shortest Char Length. */
    const Cell** s_cells = GC_MALLOC(sizeof(Cell*) * num_strings);
    int32_t* byte_offsets = GC_MALLOC(sizeof(int32_t) * num_strings);

    for (int i = 0; i < num_strings; i++) {
        const Cell* s = a->cell[i + 1];
        if (s->type != CELL_STRING)
            return make_cell_error(
                fmt_err("string-map: arg %d must be a string", i+2),
                TYPE_ERR);

        if (s->char_count < shortest_len) shortest_len = s->char_count;
        s_cells[i] = s;
        byte_offsets[i] = 0;
    }

    if (shortest_len == 0) return make_cell_string("");

    /* Collect Resulting Characters in a temporary array. */
    /* Use an array of UChar32 to avoid repeated UTF-8 encoding/shifting inside the loop. */
    UChar32* res_chars = GC_MALLOC_ATOMIC(sizeof(UChar32) * shortest_len);
    int32_t total_bytes = 0;
    int is_ascii = 1;

    for (int i = 0; i < shortest_len; i++) {
        Cell* args_sexpr = make_cell_sexpr();

        for (int j = 0; j < num_strings; j++) {
            UChar32 c;
            U8_NEXT(s_cells[j]->str, byte_offsets[j], s_cells[j]->count, c);
            cell_add(args_sexpr, make_cell_char(c));
        }

        Cell* val;
        if (proc->is_builtin) {
            val = proc->builtin(e, args_sexpr);
        } else {
            val = coz_apply_and_get_val(proc, args_sexpr, (Lex*)e);
        }

        if (!val) return nullptr;
        if (val->type == CELL_ERROR) return val;
        if (val->type != CELL_CHAR)
            return make_cell_error(
                "string-map: procedure must return a char",
                TYPE_ERR);

        const UChar32 res_c = val->char_v;
        res_chars[i] = res_c;

        const int b_len = U8_LENGTH(res_c);
        total_bytes += b_len;
        if (res_c > 0x7F) {
            is_ascii = 0;
        }
    }

    /* Encode the result array into the final UTF-8 string. */
    char* buffer = GC_MALLOC_ATOMIC(total_bytes + 1);
    int32_t write_idx = 0;
    for (int i = 0; i < shortest_len; i++) {
        UBool error = 0;
        U8_APPEND(buffer, write_idx, total_bytes, res_chars[i], error);

        if (error) {
            /* If the codepoint is invalid, fall back to the Unicode
               Replacement Character: U+FFFD.
               In UTF-8, this is 3 bytes: 0xEF, 0xBF, 0xBD */
            buffer[0] = (char)0xEF;
            buffer[1] = (char)0xBF;
            buffer[2] = (char)0xBD;
            total_bytes = 3;
        }
    }
    buffer[total_bytes] = '\0';

    /* Manual Metadata Construction. */
    Cell* v = GC_MALLOC_ATOMIC(sizeof(Cell));
    v->type = CELL_STRING;
    v->str = buffer;
    v->count = total_bytes;
    v->char_count = shortest_len;
    v->ascii = is_ascii;

    return v;
}


/* (for-each proc list1 list2 ... )
 * The arguments to for-each are like the arguments to map, but for-each calls proc for its side effects
 * rather than for its values. */
Cell* builtin_foreach(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 2, "for-each");
    if (err) return err;

    const Cell* proc = a->cell[0];
    if (proc->type != CELL_PROC)
        return make_cell_error(
            "for-each: arg 1 must be a procedure",
            TYPE_ERR);

    const int num_lists = a->count - 1;
    int shortest_len = INT32_MAX;

    /* Setup cursors. */
    const Cell** cursors = GC_MALLOC(sizeof(Cell*) * num_lists);
    for (int i = 0; i < num_lists; i++) {
        Cell* lst = a->cell[i + 1];
        if (lst->type == CELL_NIL) return USP_Obj;

        /* Ensure a valid length for the loop. */
        if (lst->len <= 0) {
            Cell* len_obj = builtin_len(e, make_sexpr_len1(lst));
            if (len_obj->type == CELL_ERROR) return len_obj;
            lst->len = (int)len_obj->integer_v;
        }
        if (lst->len < shortest_len) shortest_len = lst->len;
        cursors[i] = lst;
    }

    /* Execute without allocating a result list. */
    for (int i = 0; i < shortest_len; i++) {
        Cell* args_sexpr = make_cell_sexpr();
        for (int j = 0; j < num_lists; j++) {
            cell_add(args_sexpr, cursors[j]->car);
            cursors[j] = cursors[j]->cdr; /* Advance. */
        }

        Cell* val;
        if (proc->is_builtin) {
            val = proc->builtin(e, args_sexpr);
        } else {
            val = coz_apply_and_get_val(proc, args_sexpr, (Lex*)e);
        }

        /* If the procedure returns an error, stop and propagate it. */
        if (val && val->type == CELL_ERROR) return val;
    }
    return USP_Obj;
}


/* (vector-for-each proc vector1 vector2 ... )
 * The arguments to vector-for-each are like the arguments to vector-map, but vector-for-each calls proc for its side
 * effects rather than for its values. */
Cell* builtin_vector_foreach(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 2, "vector-for-each");
    if (err) return err;

    const Cell* proc = a->cell[0];
    if (proc->type != CELL_PROC)
        return make_cell_error(
            "vector-for-each: arg 1 must be a procedure",
            TYPE_ERR);

    const int num_vectors = a->count - 1;
    int shortest_len = INT32_MAX;

    /* Calculate the shortest length and validate types. */
    for (int i = 0; i < num_vectors; i++) {
        const Cell* v = a->cell[i + 1];
        if (v->type != CELL_VECTOR)
            return make_cell_error(
                fmt_err("vector-for-each: arg %d must be a vector", i+2),
                TYPE_ERR);

        if (v->len < shortest_len) shortest_len = v->len;
    }

    if (shortest_len == 0) return USP_Obj;

    /* Side effect loop: No allocations for result collection! */
    for (int i = 0; i < shortest_len; i++) {
        Cell* arg_list = make_cell_sexpr();
        for (int j = 0; j < num_vectors; j++) {
            cell_add(arg_list, a->cell[j + 1]->cell[i]);
        }

        Cell* tmp_result;
        if (proc->is_builtin) {
            tmp_result = proc->builtin(e, arg_list);
        } else {
            tmp_result = coz_apply_and_get_val(proc, arg_list, (Lex*)e);
        }

        /* Stop execution and return if the procedure returns an error. */
        if (tmp_result && tmp_result->type == CELL_ERROR) return tmp_result;
    }
    return USP_Obj;
}


/* (string-for-each proc string1 string2 ... )
 * The arguments to string-for-each are like the arguments to string-map, but string-for-each calls proc for its side
 * effects rather than for its values. */
Cell* builtin_string_foreach(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_MIN(a, 2, "string-for-each");
    if (err) return err;

    const Cell* proc = a->cell[0];
    if (proc->type != CELL_PROC)
        return make_cell_error(
            "string-for-each: arg 1 must be a procedure",
            TYPE_ERR);

    const int num_strings = a->count - 1;
    int shortest_len = INT32_MAX;

    /* Setup cursors. */
    const Cell** s_cells = GC_MALLOC(sizeof(Cell*) * num_strings);
    int32_t* byte_offsets = GC_MALLOC(sizeof(int32_t) * num_strings);

    for (int i = 0; i < num_strings; i++) {
        const Cell* s = a->cell[i + 1];
        if (s->type != CELL_STRING)
            return make_cell_error(
                fmt_err("string-for-each: arg %d must be a string", i+2),
                TYPE_ERR);

        if (s->char_count < shortest_len) shortest_len = s->char_count;
        s_cells[i] = s;
        byte_offsets[i] = 0;
    }

    /* Execute Side Effects. */
    for (int i = 0; i < shortest_len; i++) {
        Cell* args_sexpr = make_cell_sexpr();

        for (int j = 0; j < num_strings; j++) {
            UChar32 c;
            U8_NEXT(s_cells[j]->str, byte_offsets[j], s_cells[j]->count, c);
            cell_add(args_sexpr, make_cell_char(c));
        }

        Cell* val;
        if (proc->is_builtin) {
            val = proc->builtin(e, args_sexpr);
        } else {
            val = coz_apply_and_get_val(proc, args_sexpr, (Lex*)e);
        }

        if (val && val->type == CELL_ERROR) return val;
        /* Return value is ignored in for-each. */
    }
    return USP_Obj;
}


Cell* builtin_load(const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "load");
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


/* (exit)
 * (exit bool)
 * (exit int)
 * Immediately terminates the running program. An optional boolean or integer value may be passed to denote the exit
 * status. #true = exit(0), and #false = exit(1). An integer argument will be directly passed as the exit code to the
 * system. */
Cell* builtin_exit(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_INTEGER|CELL_BOOLEAN, "exit");
    if (err) { return err; }

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


/* (command-line)
 * Returns a list of arguments passed to the script. When called from the REPL, it will just return the empty list. Note
 * that these are not the arguments passed to cozenage, but rather, a method for passing arguments to be used within a
 * script or program interpreted by cozenage. Therefore, two dashes '--' must be used to separate cozenage args from
 * script arguments. The zeroeth value of this list is always the script name. */
Cell* builtin_command_line(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    /* Return empty list if using REPL. */
    if (is_repl) {
        return Nil_Obj;
    }
    /* Construct the list of args. */
    Cell* args = make_cell_sexpr();
    for (int i = 0; i < g_argc; i++) {
        cell_add(args, make_cell_string(g_argv[i]));
    }
    return make_list_from_sexpr(args);
}
