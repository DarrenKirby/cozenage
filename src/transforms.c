/*
 * 'src/transforms.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 Darren Kirby <darren@dragonbyte.ca>
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

/* This file defines internal functions which implement the transforms
 * of derived syntax forms to primitive forms. It also implements other
 * 'housekeeping' tasks like transforming inner-defines into lambda bodies,
 * and wraps multiple body expressions into explicit (begin ...) expressions.
 *
 * The code in this file is run directly between the parser and evaluator.
 */

#include "transforms.h"
#include "cell.h"
#include "symbols.h"


static int gen_sym_counter = 0;

static Cell* gen_sym(const char* prefix) {
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


/* (when ⟨test⟩ ⟨expression1⟩ ⟨expression2⟩ ... )
 * The test is evaluated, and if it evaluates to a true value, the expressions are evaluated in
 * order. The result of the 'when' expression is unspecified, per R7RS, but Cozenage returns the value
 * of the last expression evaluated, or null if the test evaluates to #f. */
static Cell* expand_when(const Cell* c) {
    /* (when test body...) */
    if (c->count < 3) {
        return make_cell_error(
            "when: missing test or body",
            SYNTAX_ERR);
    }

    Cell* result = make_cell_sexpr();
    cell_add(result, G_if_sym);
    cell_add(result, expand(c->cell[1]));  /* The test. */

    /* The 'then' branch: wrap body in begin/letrec. */
    cell_add(result, expand_body_expressions(c, 2));

    /* The 'else' branch: return unspecified/void. */
    cell_add(result, USP_Obj);

    return result;
}


/*  (unless ⟨test⟩ ⟨expression1⟩ ⟨expression2⟩ ... )
 *  The test is evaluated, and if it evaluates to #f, the expressions are evaluated in order. The
 *  result of the unless expression is unspecified, per R7RS, but Cozenage returns the value of the
 *  last expression evaluated, or null if the test is truthy. */
static Cell* expand_unless(const Cell* c) {
    /* (unless test body...) */
    if (c->count < 3) {
        return make_cell_error(
            "unless: missing test or body",
            SYNTAX_ERR);
    }

    Cell* result = make_cell_sexpr();
    cell_add(result, G_if_sym);
    cell_add(result, expand(c->cell[1]));  /* The test. */

    /* The 'then' branch: unless is false when the test is true. */
    cell_add(result, USP_Obj);

    /* The 'else' branch: wrap body in begin/letrec. */
    cell_add(result, expand_body_expressions(c, 2));

    return result;
}


/* (or ⟨test1⟩ ... )
 * The ⟨test⟩ expressions are evaluated from left to right, and the value of the first expression
 * that evaluates to a true value is returned. Any remaining expressions are not evaluated. If all
 * expressions evaluate to #f or if there are no expressions, then #f is returned. */
static Cell* expand_or(const Cell* c) {
    /* (or) -> #f. */
    if (c->count == 1) return False_Obj;

    /* (or e1) -> e1. */
    if (c->count == 2) return expand(c->cell[1]);

    /* (or e1 e2 ...). ->
       (let ((tmp e1)) (if tmp tmp (or e2 ...))) */
    Cell* tmp_sym = gen_sym("or");

    /* (or e2 ...). */
    Cell* rest_or = make_cell_sexpr();
    cell_add(rest_or, G_or_sym);
    for (int i = 2; i < c->count; i++) {
        cell_add(rest_or, c->cell[i]);
    }

    /* (if tmp tmp (or ...)). */
    Cell* if_expr = make_cell_sexpr();
    cell_add(if_expr, G_if_sym);
    cell_add(if_expr, tmp_sym);
    cell_add(if_expr, tmp_sym);
    cell_add(if_expr, rest_or);

    /* (let ((tmp e1)) if_expr). */
    Cell* binding_pair = make_cell_sexpr();
    cell_add(binding_pair, tmp_sym);
    cell_add(binding_pair, expand(c->cell[1]));

    Cell* bindings = make_cell_sexpr();
    cell_add(bindings, binding_pair);

    Cell* let_expr = make_cell_sexpr();
    cell_add(let_expr, G_let_sym);
    cell_add(let_expr, bindings);
    cell_add(let_expr, if_expr);

    return expand(let_expr); /* Recurse to handle the rest_or. */
}


/* (letrec* ⟨bindings⟩ ⟨body⟩)
 * ⟨Bindings⟩ has the form (⟨variable1⟩ ⟨init1⟩) ...), and ⟨body⟩ is a sequence of zero or more definitions followed by
 * one or more expressions. It is an error for a ⟨variable⟩ to appear more than once in the list of variables being
 * bound.
 *
 * Semantics: The ⟨variable⟩s are bound to fresh locations, each ⟨variable⟩ is assigned in left-to-right order to the
 * result of evaluating the corresponding ⟨init⟩, the ⟨body⟩ is evaluated in the resulting environment, and the values
 * of the last expression in ⟨body⟩ are returned. Despite the left- to-right evaluation and assignment order, each
 * binding of a ⟨variable⟩ has the entire letrec* expression as its region, making it possible to define mutually
 * recursive procedures.
 *
 * If it is not possible to evaluate each ⟨init⟩ without assigning or referring to the value of the corresponding
 * ⟨variable⟩ or the ⟨variable⟩ of the bindings that follow it in ⟨bindings⟩, it is an error. Another restriction is
 * that it is an error to invoke the continuation of an ⟨init⟩ more than once. */
static Cell* expand_letrec_star(const Cell* c) {
    /* (letrec* ((var init) ...) body...) */
    if (c->count < 3) return make_cell_error(
        "letrec*: malformed expression",
        SYNTAX_ERR);

    const Cell* bindings = c->cell[1];

    /* Base case: (letrec* () body...) -> (letrec () body...) */
    /* Check this FIRST before accessing any cells! */
    if (bindings->count == 0) {
        Cell* res = make_cell_sexpr();
        cell_add(res, G_letrec_sym);
        cell_add(res, make_cell_sexpr());
        for (int i = 2; i < c->count; i++) {
            cell_add(res, expand(c->cell[i]));
        }
        return res;
    }

    /* Peeling: (letrec* ((v1 i1) (v2 i2) ...) body...)
       becomes (letrec ((v1 i1)) (letrec* ((v2 i2) ...) body...)) */

    Cell* first_pair = bindings->cell[0];

    /* Build the list of rest bindings. */
    Cell* rest_bindings = make_cell_sexpr();
    for (int i = 1; i < bindings->count; i++) {
        cell_add(rest_bindings, bindings->cell[i]);
    }

    /* Build the inner letrec* expression. */
    Cell* inner_letrec_star = make_cell_sexpr();
    cell_add(inner_letrec_star, G_letrec_star_sym);
    cell_add(inner_letrec_star, rest_bindings);
    for (int i = 2; i < c->count; i++) {
        cell_add(inner_letrec_star, c->cell[i]);
    }

    /* Build the outer letrec expression:
     * (letrec (first_pair) inner_letrec_star). */
    Cell* outer_letrec = make_cell_sexpr();
    cell_add(outer_letrec, G_letrec_sym);

    Cell* outer_bindings_list = make_cell_sexpr();
    cell_add(outer_bindings_list, first_pair);
    cell_add(outer_letrec, outer_bindings_list);

    cell_add(outer_letrec, inner_letrec_star);

    /* Return to the expander. It will see the 'letrec', process it,
       then see the 'inner_letrec_star' and recurse back here. */
    return expand(outer_letrec);
}


/* (let* ⟨bindings⟩ ⟨body⟩) where ⟨Bindings⟩ has the form ((⟨variable1⟩ ⟨init1⟩) ...)
 * where each ⟨init⟩ is an expression, and ⟨body⟩ is a sequence of zero or more definitions followed
 * by a sequence of one or more expressions.
 *
 * The let* binding construct is similar to let, but the bindings are performed sequentially from
 * left to right. Also, the region of a binding indicated by (⟨variable⟩ ⟨init⟩) is that part of the
 * let* expression to the right of the binding. Thus, the second binding is done in an environment
 * in which the first binding is visible, and so on. The ⟨variable⟩s need not be distinct. */
static Cell* expand_let_star(const Cell* c) {
    /* c is (let* ((var init) ...) body...) */
    if (c->count < 3) return make_cell_error("let*: malformed expression", SYNTAX_ERR);

    const Cell* bindings = c->cell[1];

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


/* (cond ⟨clause1⟩ ⟨clause2⟩ ... )
 * where ⟨clause⟩ is (⟨test⟩ ⟨expression1⟩ ...) OR (⟨test⟩ => ⟨expression⟩)
 * The last ⟨clause⟩ can be an “else clause”. A cond expression is evaluated by evaluating the
 * ⟨test⟩ expressions of successive ⟨clause⟩s in order until one of them evaluates to a true value.
 * When a ⟨test⟩ evaluates to a true value, the remaining ⟨expression⟩s in its ⟨clause⟩ are
 * evaluated in order, and the results of the last ⟨expression⟩ in the ⟨clause⟩ are returned as the
 * results of the entire cond expression.
 *
 * If the selected ⟨clause⟩ contains only the ⟨test⟩ and no ⟨expression⟩s, then the value of the
 * ⟨test⟩ is returned as the result. If the selected ⟨clause⟩ uses the => alternate form, then the
 * ⟨expression⟩ is evaluated. It is an error if its value is not a procedure that accepts one
 * argument. This procedure is then called on the value of the ⟨test⟩ and the values returned by
 * this procedure are returned by the cond expression.
 *
 * If all ⟨test⟩s evaluate to #f, and there is no else clause, then the result of the conditional
 * expression is unspecified; if there is an else clause, then its ⟨expression⟩s are evaluated in
 * order, and the values of the last one are returned.
 */
static Cell* expand_cond(const Cell* c) {
    if (c->count < 2) return make_cell_error(
        "cond: malformed",
        SYNTAX_ERR);

    /* Get the first clause: (test ...) */
    const Cell* clause = c->cell[1];
    Cell* test = clause->cell[0];

    /* Helper: build the 'else' branch (the rest of the cond). */
    Cell* rest_cond;
    if (c->count > 2) {
        rest_cond = make_cell_sexpr();
        cell_add(rest_cond, G_cond_sym);
        for (int i = 2; i < c->count; i++) {
            cell_add(rest_cond, c->cell[i]);
        }
    } else {
        rest_cond = USP_Obj;
    }

    /* Handle Else: (else body...). */
    if (is_same_symbol(test, G_else_sym)) {
        return expand_body_expressions(clause, 1);
    }

    /* Handle Arrow: (test => proc) */
    if (clause->count == 3 && is_same_symbol(clause->cell[1], G_arrow_sym)) {
        Cell* tmp = gen_sym("cond");

        /* (let ((tmp test)) (if tmp (proc tmp) rest_cond)). */
        Cell* let_expr = make_cell_sexpr();
        cell_add(let_expr, G_let_sym);

        Cell* bindings = make_cell_sexpr();
        Cell* pair = make_cell_sexpr();
        cell_add(pair, tmp);
        cell_add(pair, expand(test));
        cell_add(bindings, pair);
        cell_add(let_expr, bindings);

        Cell* if_expr = make_cell_sexpr();
        cell_add(if_expr, G_if_sym);
        cell_add(if_expr, tmp);

        Cell* call = make_cell_sexpr();
        cell_add(call, expand(clause->cell[2]));
        cell_add(call, tmp);
        cell_add(if_expr, call);
        cell_add(if_expr, rest_cond);

        cell_add(let_expr, if_expr);
        return expand(let_expr);
    }

    /* Handle Test-Only: (test) */
    if (clause->count == 1) {
        Cell* tmp = gen_sym("cond");

        /* (let ((tmp test)) (if tmp tmp rest_cond)). */
        Cell* let_expr = make_cell_sexpr();
        cell_add(let_expr, G_let_sym);

        Cell* bindings = make_cell_sexpr();
        Cell* pair = make_cell_sexpr();
        cell_add(pair, tmp);
        cell_add(pair, expand(test));
        cell_add(bindings, pair);
        cell_add(let_expr, bindings);

        Cell* if_expr = make_cell_sexpr();
        cell_add(if_expr, G_if_sym);
        cell_add(if_expr, tmp);
        cell_add(if_expr, tmp);
        cell_add(if_expr, rest_cond);

        cell_add(let_expr, if_expr);
        return expand(let_expr);
    }

    /* Standard Clause: (test body...). */
    Cell* if_expr = make_cell_sexpr();
    cell_add(if_expr, G_if_sym);
    cell_add(if_expr, expand(test));
    cell_add(if_expr, expand_body_expressions(clause, 1));
    cell_add(if_expr, rest_cond);

    return expand(if_expr);
}


/* (let ⟨variable⟩ ⟨bindings⟩ ⟨body⟩)
 * Semantics: “Named let” is a variant on the syntax of let which provides a more general looping construct than do and
 * can also be used to express recursion. It has the same syntax and semantics as ordinary let except that ⟨variable⟩ is
 * bound within ⟨body⟩ to a procedure whose formal arguments are the bound variables and whose body is ⟨body⟩. Thus, the
 * execution of ⟨body⟩ can be repeated by invoking the procedure named by ⟨variable⟩. */
static Cell* expand_named_let(const Cell* c) {
    /* c is (let name ((var init) ...) body...). */
    Cell* name = c->cell[1];
    const Cell* bindings = c->cell[2];

    Cell* vars = make_cell_sexpr();
    Cell* inits = make_cell_sexpr();

    for (int i = 0; i < bindings->count; i++) {
        cell_add(vars, bindings->cell[i]->cell[0]);
        cell_add(inits, expand(bindings->cell[i]->cell[1]));
    }

    /* Build the lambda: (lambda (vars...) body...). */
    Cell* lambda = make_cell_sexpr();
    cell_add(lambda, G_lambda_sym);
    cell_add(lambda, vars);
    /* Add all body expressions (starting from index 3 in the original let). */
    for (int i = 3; i < c->count; i++) {
        cell_add(lambda, expand(c->cell[i]));
    }

    /* Build the letrec: (letrec ((name lambda)) (name inits...)). */
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


/* (do ((⟨variable1⟩ ⟨init1⟩ ⟨step1⟩) ...)
 *     (⟨test⟩ ⟨expression⟩ ...) ⟨command⟩ ...)
 * Syntax: All of ⟨init⟩, ⟨step⟩, ⟨test⟩, and ⟨command⟩ are expressions.
 *
 * Semantics: A do expression is an iteration construct. It specifies a set of variables to be bound, how they are to be
 * initialized at the start, and how they are to be updated on each iteration. When a termination condition is met, the
 * loop exits after evaluating the ⟨expression⟩s.
 *
 * A do expression is evaluated as follows: The ⟨init⟩ expressions are evaluated (in some unspecified order), the
 * ⟨variable⟩s are bound to fresh locations, the results of the ⟨init⟩ expressions are stored in the bindings of the
 * ⟨variable⟩s, and then the iteration phase begins.
 *
 * Each iteration begins by evaluating ⟨test⟩; if the result is false (see section 6.3), then the ⟨command⟩ expressions
 * are evaluated in order for effect, the ⟨step⟩ expressions are evaluated in some unspecified order, the ⟨variable⟩s
 * are bound to fresh locations, the results of the ⟨step⟩s are stored in the bindings of the ⟨variable⟩s, and the next
 * iteration begins.
 *
 * If ⟨test⟩ evaluates to a true value, then the ⟨expression⟩s are evaluated from left to right and the values of the
 * last ⟨expression⟩ are returned. If no ⟨expression⟩s are present, then the value of the do expression is unspecified.
 *
 * The region of the binding of a ⟨variable⟩ consists of the entire do expression except for the ⟨init⟩s. It is an
 * error for a ⟨variable⟩ to appear more than once in the list of do variables.
 *
 * A ⟨step⟩ can be omitted, in which case the effect is the same as if (⟨variable⟩ ⟨init⟩ ⟨variable⟩) had been written
 * instead of (⟨variable⟩ ⟨init⟩). */
static Cell* expand_do(const Cell* c) {
    if (c->count < 3) return make_cell_error(
        "Malformed do expression",
        SYNTAX_ERR);

    const Cell* bindings_input = c->cell[1];
    const Cell* test_clause = c->cell[2];

    /* Use ONE symbol for the loop name to ensure pointer equality. */
    Cell* loop_name = make_cell_symbol("loop");

    Cell* let_bindings = make_cell_sexpr();
    Cell* loop_steps = make_cell_sexpr();
    cell_add(loop_steps, loop_name);

    for (int i = 0; i < bindings_input->count; i++) {
        const Cell* b = bindings_input->cell[i];

        Cell* binding_pair = make_cell_sexpr();
        cell_add(binding_pair, b->cell[0]);
        cell_add(binding_pair, expand(b->cell[1]));
        cell_add(let_bindings, binding_pair);

        if (b->count > 2) {
            cell_add(loop_steps, expand(b->cell[2]));
        } else {
            cell_add(loop_steps, b->cell[0]);
        }
    }

    /* Build the IF logic. */
    Cell* if_expr = make_cell_sexpr();
    cell_add(if_expr, G_if_sym);
    cell_add(if_expr, expand(test_clause->cell[0]));

    /* The test clause can have multiple expressions (R7RS) */
    /* (test expr1 expr2 ...) should run all and return the last. */
    if (test_clause->count > 1) {
        Cell* test_result_body = make_cell_sexpr();
        cell_add(test_result_body, G_begin_sym);

        for (int i = 1; i < test_clause->count; i++) {
            cell_add(test_result_body, expand(test_clause->cell[i]));
        }

        cell_add(if_expr, test_result_body);
    } else {
        cell_add(if_expr, USP_Obj);
    }

    Cell* begin_block = make_cell_sexpr();
    cell_add(begin_block, G_begin_sym);

    for (int i = 3; i < c->count; i++) {
        cell_add(begin_block, expand(c->cell[i]));
    }

    cell_add(begin_block, loop_steps);
    cell_add(if_expr, begin_block);

    /* Wrap in Named Let. */
    Cell* named_let = make_cell_sexpr();
    cell_add(named_let, G_let_sym);
    cell_add(named_let, loop_name);
    cell_add(named_let, let_bindings);
    cell_add(named_let, if_expr);

    /* Expand the resulting named let. */
    return expand(named_let);
}


/* (case ⟨key⟩ ⟨clause 1⟩ ⟨clause 2⟩ ... )
 * Syntax: ⟨Key⟩ can be any expression. Each ⟨clause⟩ has the form ((⟨datum1⟩ ...) ⟨expression1⟩ ⟨expression2⟩ ...),
 * where each ⟨datum⟩ is an external representation of some object. It is an error if any of the ⟨datum⟩s are the same
 * anywhere in the expression. Alternatively, a ⟨clause⟩ can be of the form ((⟨datum1⟩ ...) => ⟨expression⟩).
 * The last ⟨clause⟩ can be an “else clause,” which has one of the forms (else ⟨expression1⟩ ⟨expression2⟩ ...) or
 * (else => ⟨expression⟩).
 *
 * Semantics: A case expression is evaluated as follows. ⟨Key⟩ is evaluated and its result is compared against each
 * ⟨datum⟩. If the result of evaluating ⟨key⟩ is the same (in the sense of eqv?; see section 6.1) to a ⟨datum⟩, then the
 * expressions in the corresponding ⟨clause⟩ are evaluated in order and the results of the last expression in the
 * ⟨clause⟩ are returned as the results of the case expression.
 *
 * If the result of evaluating ⟨key⟩ is different from every ⟨datum⟩, then if there is an else clause, its expressions
 * are evaluated and the results of the last are the results of the case expression; otherwise the result of the case
 * expression is unspecified.
 *
 * If the selected ⟨clause⟩ or else clause uses the => alternate form, then the ⟨expression⟩ is evaluated. It is an
 * error if its value is not a procedure accepting one argument. This procedure is then called on the value of the ⟨key⟩
 * and the values returned by this procedure are returned by the case expression. */
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


/* Helper to handle the append logic for transform_qq(). */
Cell* transform_qq_list_logic(const Cell* input, const int depth) {
    Cell* out_expr = make_cell_sexpr();
    cell_add(out_expr, make_cell_symbol("append"));

    for (int i = 0; i < input->count; i++) {
        const Cell* item = input->cell[i];

        if (item->type == CELL_SEXPR &&
            item->count > 0 &&
            item->cell[0] == G_unquote_splicing_sym &&
            depth == 1) {

            cell_add(out_expr, item->cell[1]);
            } else {
                cell_add(out_expr,
                    make_sexpr_len2(
                        make_cell_symbol("list"),
                        transform_qq(item, depth)));
            }
    }
    return out_expr;
}


Cell* transform_qq(const Cell* input, const int depth) {
    if (input->type == CELL_VECTOR) {
        /* Create an S-expression that will call (list->vector <the_expanded_list>). */
        const Cell* expanded_list = transform_qq_list_logic(input, depth);
        return make_sexpr_len2(
            make_cell_symbol("list->vector"),
            expanded_list);
    }

    /* Atoms (non-S-expr) get quoted. */
    if (input->type != CELL_SEXPR) {
        return make_sexpr_len2(G_quote_sym, input);
    }

    const Cell* first = input->cell[0];

    /* UNQUOTE: If depth is 1, return the expression directly to be evaluated. */
    if (first == G_unquote_sym) {
        if (depth == 1) return input->cell[1];

        /* If depth > 1, this is inside a nested quote, so rebuild (list 'unquote ...). */
        return make_sexpr_len3(
            make_cell_symbol("list"),
            make_sexpr_len2(G_quote_sym, G_unquote_sym),
            transform_qq(input->cell[1], depth - 1)
        );
    }

    /* NESTED QUASIQUOTE: Rebuild (list 'quasiquote ...). */
    if (first == G_quasiquote_sym) {
        return make_sexpr_len3(
            make_cell_symbol("list"),
            make_sexpr_len2(G_quote_sym, G_quasiquote_sym),
            transform_qq(input->cell[1], depth + 1)
        );
    }

    /* STANDARD S-EXPR: Build (append (list ...) ...). */
    Cell* out_expr = make_cell_sexpr();
    cell_add(out_expr, make_cell_symbol("append"));

    for (int i = 0; i < input->count; i++) {
        const Cell* item = input->cell[i];

        /* Check if this specific item is an (unquote-splicing ...) S-expr. */
        if (item->type == CELL_SEXPR &&
            item->count > 0 &&
            item->cell[0] == G_unquote_splicing_sym &&
            depth == 1) {

            /* Splicing: Add the inner expression
             * directly to the append S-expr. */
            cell_add(out_expr, item->cell[1]);

            } else {
                /* Normal: transform the item and wrap it in (list ...)
                 * so append treats it as a single element. */
                const Cell* transformed_item = transform_qq(item, depth);
                cell_add(out_expr,
                    make_sexpr_len2(
                        make_cell_symbol("list"),
                        transformed_item));
            }
    }
    return transform_qq_list_logic(input, depth);
}


Cell* expand(Cell* c) {
    /* Base case: only S-expressions can be expanded */
    if (c->type != CELL_SEXPR || c->count == 0) return c;

    const Cell* head = c->cell[0];

    if (head->type == CELL_SYMBOL) {
        /* 'define' - primitive - transform nested 'define's into 'let's. */
        if (is_same_symbol(head, G_define_sym) && c->count > 2 && c->cell[1]->type == CELL_SEXPR) {
            return expand_define(c);
        }

        /* 'lambda' - primitive - sequence body expressions in (begin ... ) expression. */
        if (is_same_symbol(head, G_lambda_sym) && c->count > 2) {
            return expand_lambda(c);
        }

        /* 'cond' - derived - transform into nested 'if's. */
        if (is_same_symbol(head, G_cond_sym)) {
            return expand_cond(c);
        }

        /* 'case' - derived - transform into named let, then
         * transform that 'let' into a 'letrec' or recurse. */
        if (is_same_symbol(head, G_case_sym)) {
            return expand(expand_case(c));
        }

        /* 'do' - derived - transform into named let, then
           recurse so the named let becomes a letrec. */
        if (is_same_symbol(head, G_do_sym)) {
            return expand(expand_do(c));
        }

        /* 'let*' - derived - transform into nested 'let's. */
        if (is_same_symbol(head, G_let_star_sym)) {
            return expand_let_star(c);
        }

        /* letrec* - derived - transform into nested 'letrec's. */
        if (is_same_symbol(head, G_letrec_star_sym)) {
            return expand_letrec_star(c);
        }

        /* 'when' - derived - transform into 'if's. */
        if (is_same_symbol(head, G_when_sym)) {
            return expand_when(c);
        }

        /* 'unless' - derived - transform to 'if's. */
        if (is_same_symbol(head, G_unless_sym)) {
            return expand_unless(c);
        }

        /* 'or' - derived - transform to nested 'let's and 'if's. */
        if (is_same_symbol(head, G_or_sym)) {
            return expand_or(c);
        }

        /* 'quasiquote' - */
        if (is_same_symbol(head, G_quasiquote_sym)) {
            return transform_qq(c->cell[1], 1);
        }

        /* These symbols at top-level are syntax errors... */

        if (is_same_symbol(head, G_unquote_sym)) {
            return make_cell_error(
                "unquote: must be contained within a 'quasiquote' expression",
                SYNTAX_ERR);
        }

        if (is_same_symbol(head, G_unquote_splicing_sym)) {
            return make_cell_error(
                "unquote-splice: must be contained within a 'quasiquote' expression",
                SYNTAX_ERR);
        }

        /* 'let' - primitive - and Named let - derived. */
        if (is_same_symbol(head, G_let_sym)) {
            /* Named let -> letrec. */
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
