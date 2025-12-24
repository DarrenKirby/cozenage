/*
 * 'src/transforms.c'
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

#include "transforms.h"
#include "cell.h"
#include "symbols.h"


static int gen_sym_counter = 0;

Cell* gen_sym(const char* prefix) {
    char name[64];
    /* Use the prefix "_" to further distinguish from user symbols. */
    snprintf(name, sizeof(name), "_%s%d", prefix, gen_sym_counter++);
    return make_cell_symbol(name);
}


/* Compare symbol identities. */
static bool is_same_symbol(const Cell* a, const Cell* b)
{
    return a == b ? true : false;
}


static Cell* transform_defines_to_bindings(const Cell* inner_defines) {
    Cell* bindings_list = make_cell_sexpr();

    for (int i = 0; i < inner_defines->count; i++) {
        const Cell* def = inner_defines->cell[i];
        Cell* binding_pair = make_cell_sexpr();

        /* def->cell[0] is 'define'
           def->cell[1] is either the name OR (name args...). */
        Cell* target = def->cell[1];

        if (target->type == CELL_SYMBOL) {
            /* Case: (define name value) */
            cell_add(binding_pair, target);
            /* The value is the 3rd element: def->cell[2]
               We expand it in case it's a lambda or has its own inner defines. */
            cell_add(binding_pair, expand(def->cell[2]));
        }
        else if (target->type == CELL_SEXPR) {
            /* Case: (define (name args...) body...)
               The name is the first element of the signature S-expr. */
            cell_add(binding_pair, target->cell[0]);

            /* Wrap the rest in a lambda. */
            Cell* lambda_expr = make_cell_sexpr();
            cell_add(lambda_expr, G_lambda_sym);

            /* Construct the args list from the rest of the signature. */
            Cell* args_list = make_cell_sexpr();
            for (int j = 1; j < target->count; j++) {
                cell_add(args_list, target->cell[j]);
            }
            cell_add(lambda_expr, args_list);

            /* Use our existing body-fixer to handle internal defines and implicit begins. */
            cell_add(lambda_expr, expand_body_expressions(def, 2));
            cell_add(binding_pair, lambda_expr);
        }
        cell_add(bindings_list, binding_pair);
    }
    return bindings_list;
}


Cell* expand_body_expressions(const Cell* body_elements, const int start_index) {
    Cell* inner_defines = make_cell_sexpr();
    int i = start_index;

    /* Collect all leading defines. */
    while (i < body_elements->count) {
        Cell* current = body_elements->cell[i];
        if (current->type == CELL_SEXPR && current->count > 0 &&
            is_same_symbol(current->cell[0], G_define_sym)) {
            cell_add(inner_defines, current);
            i++;
            } else {
                break;
            }
    }

    Cell* final_body_expr;

    /* Build the "executable" part of the body
       If there's more than one expression left, wrap in 'begin'. */
    const int remaining_count = body_elements->count - i;
    if (remaining_count > 1) {
        final_body_expr = make_cell_sexpr();
        cell_add(final_body_expr, G_begin_sym);
        for (int j = i; j < body_elements->count; j++) {
            cell_add(final_body_expr, expand(body_elements->cell[j]));
        }
    } else if (remaining_count == 1) {
        final_body_expr = expand(body_elements->cell[i]);
    } else {
        /* Handle empty body error or return unspecified. */
        return make_cell_error("Procedure body is empty", SYNTAX_ERR);
    }

    /* If there WERE defines, wrap everything in letrec. */
    if (inner_defines->count > 0) {
        Cell* letrec_expr = make_cell_sexpr();
        cell_add(letrec_expr, G_letrec_sym);
        cell_add(letrec_expr, transform_defines_to_bindings(inner_defines));
        cell_add(letrec_expr, final_body_expr);
        return letrec_expr;
    }

    Cell* begin_block = make_cell_sexpr();
    cell_add(begin_block, G_begin_sym);
    for (int j = i; j < body_elements->count; j++) {
        cell_add(begin_block, expand(body_elements->cell[j]));
    }
    return begin_block;
}


static Cell* expand_let_star(const Cell* c) {
    /* c is (let* ((var init) ...) body...) */
    if (c->count < 3) return make_cell_error("let*: malformed expression", SYNTAX_ERR);

    Cell* bindings = c->cell[1];

    /* Base case: (let* () body...) -> (let () body...) */
    if (bindings->count == 0) {
        Cell* res = make_cell_sexpr();
        cell_add(res, G_let_sym);
        cell_add(res, make_cell_sexpr()); // Empty bindings
        for (int i = 2; i < c->count; i++) {
            cell_add(res, expand(c->cell[i]));
        }
        return res;
    }

    /* Recursive case: (let* ((v1 i1) (v2 i2) ...) body...)
       -> (let ((v1 i1)) (let* ((v2 i2) ...) body...)) */

    /* Take the first binding. */
    Cell* first_binding = make_cell_sexpr();
    cell_add(first_binding, bindings->cell[0]);

    /* Collect the rest of the bindings. */
    Cell* rest_bindings = make_cell_sexpr();
    for (int i = 1; i < bindings->count; i++) {
        cell_add(rest_bindings, bindings->cell[i]);
    }

    /* Build the inner let* */
    Cell* inner_let_star = make_cell_sexpr();
    cell_add(inner_let_star, G_let_star_sym);
    cell_add(inner_let_star, rest_bindings);
    for (int i = 2; i < c->count; i++) {
        cell_add(inner_let_star, c->cell[i]);
    }

    /* Wrap in an outer let. */
    Cell* outer_let = make_cell_sexpr();
    cell_add(outer_let, G_let_sym);
    Cell* outer_bindings = make_cell_sexpr();
    cell_add(outer_bindings, bindings->cell[0]);
    cell_add(outer_let, outer_bindings);
    cell_add(outer_let, inner_let_star);

    /* Return and expand! The recursion in expand() will handle the inner let* */
    return expand(outer_let);
}


static Cell* expand_named_let(const Cell* c) {
    /* c is (let name ((var init) ...) body...) */
    Cell* name = c->cell[1];
    Cell* bindings = c->cell[2];

    Cell* vars = make_cell_sexpr();
    Cell* inits = make_cell_sexpr();

    for (int i = 0; i < bindings->count; i++) {
        cell_add(vars, bindings->cell[i]->cell[0]);
        cell_add(inits, expand(bindings->cell[i]->cell[1]));
    }

    /* Build the lambda: (lambda (vars...) body...) */
    Cell* lambda = make_cell_sexpr();
    cell_add(lambda, G_lambda_sym);
    cell_add(lambda, vars);
    /* Add all body expressions (starting from index 3 in the original let) */
    for (int i = 3; i < c->count; i++) {
        cell_add(lambda, expand(c->cell[i]));
    }

    /* Build the letrec: (letrec ((name lambda)) (name inits...)) */
    Cell* lr_binding_pair = make_cell_sexpr();
    cell_add(lr_binding_pair, name);
    cell_add(lr_binding_pair, lambda);

    Cell* lr_bindings_list = make_cell_sexpr();
    cell_add(lr_bindings_list, lr_binding_pair);

    Cell* lr_body_call = make_cell_sexpr();
    cell_add(lr_body_call, name);
    for (int i = 0; i < inits->count; i++) {
        cell_add(lr_body_call, inits->cell[i]);
    }

    Cell* letrec_expr = make_cell_sexpr();
    cell_add(letrec_expr, G_letrec_sym);
    cell_add(letrec_expr, lr_bindings_list);
    cell_add(letrec_expr, lr_body_call);

    return letrec_expr;
}


static Cell* expand_do(const Cell* c) {
    /* c is (do ((var init step) ...) (test expr) body...). */
    if (c->count < 3) return make_cell_error("Malformed do expression", SYNTAX_ERR);

    const Cell* bindings_input = c->cell[1];
    const Cell* test_clause = c->cell[2];

    /* Build the (var init) bindings for the Named Let. */
    Cell* let_bindings = make_cell_sexpr();
    /* Build the (step1 step2 ...) list for the recursive call. */
    Cell* loop_steps = make_cell_sexpr();
    cell_add(loop_steps, make_cell_symbol("loop")); /* The name of our named let. */

    for (int i = 0; i < bindings_input->count; i++) {
        const Cell* b = bindings_input->cell[i];
        /* b is (var init step). */

        /* Add (var init) to let_bindings. */
        Cell* binding_pair = make_cell_sexpr();
        cell_add(binding_pair, b->cell[0]); /* var */
        cell_add(binding_pair, expand(b->cell[1])); /* init */
        cell_add(let_bindings, binding_pair);

        /* If there's a step, use it; otherwise, the var stays the same. */
        if (b->count > 2) {
            cell_add(loop_steps, expand(b->cell[2])); /* step */
        } else {
            cell_add(loop_steps, b->cell[0]); /* var (stays same) */
        }
    }

    /* Build the body of the loop. */
    Cell* if_expr = make_cell_sexpr();
    cell_add(if_expr, G_if_sym);
    cell_add(if_expr, expand(test_clause->cell[0])); /* The test */

    /* If test is true, return the result-expr (or unspecified if empty). */
    if (test_clause->count > 1) {
        cell_add(if_expr, expand(test_clause->cell[1]));
    } else {
        cell_add(if_expr, USP_Obj);
    }

    /* If test is false, run body AND then loop. */
    Cell* begin_block = make_cell_sexpr();
    cell_add(begin_block, G_begin_sym);
    /* Add original body expressions. */
    for (int i = 3; i < c->count; i++) {
        cell_add(begin_block, expand(c->cell[i]));
    }
    /* Add the recursive call: (loop step1 step2 ...). */
    cell_add(begin_block, loop_steps);
    cell_add(if_expr, begin_block);

    /* Wrap everything in the Named Let: (let loop (bindings) if_expr). */
    Cell* named_let = make_cell_sexpr();
    cell_add(named_let, G_let_sym);
    cell_add(named_let, make_cell_symbol("loop"));
    cell_add(named_let, let_bindings);
    cell_add(named_let, if_expr);

    return named_let;
}


static Cell* expand_case(const Cell* c) {
    /* c is (case key clauses...). */
    if (c->count < 3) return make_cell_error(
        "Malformed case expression",
        SYNTAX_ERR);

    Cell* key_expr = expand(c->cell[1]);

    /* Generate a unique symbol for this specific case expansion. */
    Cell* tmp_sym = gen_sym("case");

    /* Create the cond block. */
    Cell* cond_block = make_cell_sexpr();
    cell_add(cond_block, G_cond_sym);

    /* Iterate through clauses starting at index 2. */
    for (int i = 2; i < c->count; i++) {
        const Cell* clause = c->cell[i];
        if (clause->type != CELL_SEXPR || clause->count < 2) continue;

        Cell* cond_clause = make_cell_sexpr();
        Cell* datalist = clause->cell[0];

        if (is_same_symbol(datalist, G_else_sym)) {
            cell_add(cond_clause, G_else_sym);
        } else {
            /* Transform clause to (memv tmp '(datalist)). */
            Cell* memv_call = make_cell_sexpr();
            cell_add(memv_call, make_cell_symbol("memv"));
            cell_add(memv_call, tmp_sym); /* The temp var name. */

            /* The datalist needs to be quoted so it's treated as data. */
            Cell* quoted_data = make_cell_sexpr();
            cell_add(quoted_data, G_quote_sym);
            cell_add(quoted_data, datalist);

            cell_add(memv_call, quoted_data);
            cell_add(cond_clause, memv_call);
        }

        /* Add the result expressions of the clause to the cond clause. */
        for (int j = 1; j < clause->count; j++) {
            cell_add(cond_clause, expand(clause->cell[j]));
        }
        cell_add(cond_block, cond_clause);
    }

    /* Wrap in a let to evaluate key_expr only once:
       (let ((tmp key_expr)) cond_block). */
    Cell* let_expr = make_cell_sexpr();
    cell_add(let_expr, G_let_sym);

    Cell* bindings = make_cell_sexpr();
    Cell* binding_pair = make_cell_sexpr();
    cell_add(binding_pair, tmp_sym);
    cell_add(binding_pair, key_expr);
    cell_add(bindings, binding_pair);

    cell_add(let_expr, bindings);
    cell_add(let_expr, cond_block);

    return let_expr;
}


static Cell* expand_define(const Cell* c)
{
    Cell* first = c->cell[0];
    Cell* result = make_cell_sexpr();
    cell_add(result, first);        /* 'define' */
    cell_add(result, c->cell[1]);   /* '(name args)' */
    cell_add(result, expand_body_expressions(c, 2));
    return result;
}


static Cell* expand_lambda(const Cell* c)
{
    Cell* first = c->cell[0];
    Cell* result = make_cell_sexpr();
    cell_add(result, first);        /* 'lambda' */
    cell_add(result, c->cell[1]);   /* '(args)' */
    cell_add(result, expand_body_expressions(c, 2));
    return result;
}


static Cell* expand_recursive(const Cell* c)
{
    Cell* result = make_cell_sexpr();
    for (int i = 0; i < c->count; i++) {
        cell_add(result, expand(c->cell[i]));
    }
    return result;
}

Cell* expand(Cell* c) {
    /* Base case: only S-expressions can be expanded */
    if (c->type != CELL_SEXPR || c->count == 0) return c;

    const Cell* head = c->cell[0];

    if (head->type == CELL_SYMBOL) {
        /* DEFINE (Primitive-ish) */
        if (is_same_symbol(head, G_define_sym) && c->count > 2 && c->cell[1]->type == CELL_SEXPR) {
            return expand_define(c);
        }

        /* LAMBDA (Primitive-ish) */
        if (is_same_symbol(head, G_lambda_sym) && c->count > 2) {
            return expand_lambda(c);
        }

        /* CASE (Transform -> Let)
           We call expand() on the result to turn that 'let' into a 'letrec' or recurse. */
        if (is_same_symbol(head, G_case_sym)) {
            return expand(expand_case(c));
        }

        /* DO (Transform -> Named Let)
           Recursing here ensures the Named Let becomes a letrec. */
        if (is_same_symbol(head, G_do_sym)) {
            return expand(expand_do(c));
        }

        /* LET* (Transform -> nested Let(s) */
        if (is_same_symbol(head, G_let_star_sym)) {
            return expand_let_star(c);
        }

        /* 5. LET */
        if (is_same_symbol(head, G_let_sym)) {
            /* Named Let -> Letrec */
            if (c->count > 1 && c->cell[1]->type == CELL_SYMBOL) {
                return expand(expand_named_let(c));
            }
            /* Standard let: Primitive. Just expand children. */
            return expand_recursive(c);
        }
    }

    /* Fallback: expand elements of the list. */
    return expand_recursive(c);
}
