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
 *
 * After running through the transformer, we make some guarantees:
 *
 * 1. Macro calls are transformed to their recursively expanded results.
 * 2. The derived forms are all transformed to equivalent primitive forms.
 * 3. Quasiquotes are transformed away.
 * 4. No unconverted quote data.
 * 5. All bodies have a single expression (sequenced in begin).
 * 6. Inner defines transformed to letrec* expressions.
 *
 * coz_eval(), the next step in the pipeline, depends on these guarantees.
 */

#include "cell.h"
#include "eval.h"
#include "special_forms.h"
#include "symbols.h"
#include "transforms.h"

#include <assert.h>

#define GEN_SYM_BUF_SIZE 64
#define MAX_EXPAND_DEPTH 64


static int gen_sym_counter = 0;
static int expand_depth = 0;


static Cell* gen_sym(const char* hint) {
    char name[GEN_SYM_BUF_SIZE];
    /* Use the separator "_" to further distinguish from user symbols. */
    snprintf(name, sizeof(name), "%s_%d", hint, gen_sym_counter++);
    return make_cell_symbol(name);
}


/* Compare symbol identities. */
static bool is_same_symbol(const Cell* a, const Cell* b)
{
    return a == b;
}


static Cell* make_quote_form(Cell* datum) {
    Cell* q = make_cell_sexpr();
    cell_add(q, G_quote_sym);
    cell_add(q, make_list_from_sexpr(datum));
    return q;
}


static Cell* builtin(const Lex* env, const char* name) {
    Cell* p = ht_get(env->bootstrap, name);
    assert(p && "core builtin missing from bootstrap");
    return p;
}


static Cell* transform_defines_to_bindings(const Cell* inner_defines, Lex* env) {
    Cell* bindings_list = make_cell_sexpr();

    for (int i = 0; i < inner_defines->count; i++) {
        const Cell* def = inner_defines->cell[i];
        if (def->count < 3) {
            return make_cell_error(
                "define: not enough args",
                ARITY_ERR);
        }
        Cell* binding_pair = make_cell_sexpr();

        /* def->cell[0] is 'define'
           def->cell[1] is either the name OR (name args...). */
        Cell* target = def->cell[1];

        if (target->type == CELL_SYMBOL) {
            /* Case: (define name value) */
            cell_add(binding_pair, target);
            /* The value is the 3rd element: def->cell[2]
               We expand it in case it's a lambda or has its own inner defines. */
            Cell* exp = expand(def->cell[2], env);
            if (exp->type == CELL_ERROR) { return exp; }
            cell_add(binding_pair, exp);
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
            Cell* be = expand_body_expressions(def, 2, env);
            if (be->type == CELL_ERROR) { return be; }
            cell_add(lambda_expr, be);
            cell_add(binding_pair, lambda_expr);
        } else {
            return make_cell_error(
                "define: invalid form",
                SYNTAX_ERR);
        }
        cell_add(bindings_list, binding_pair);
    }
    return bindings_list;
}


Cell* expand_body_expressions(const Cell* body_elements, const int start_index, Lex* env) {
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

    /* Error on defines appearing after non-define expressions. */
    for (int j = i; j < body_elements->count; j++) {
        const Cell* f = body_elements->cell[j];
        if (f->type == CELL_SEXPR && f->count > 0 &&
            is_same_symbol(f->cell[0], G_define_sym)) {
            return make_cell_error(
                "define: definitions must come before expressions in a body",
                SYNTAX_ERR);
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
            Cell* exp = expand(body_elements->cell[j], env);
            if (exp->type == CELL_ERROR) { return exp; }
            cell_add(final_body_expr, exp);
        }
    } else if (remaining_count == 1) {
        Cell* exp = expand(body_elements->cell[i], env);
        if (exp->type == CELL_ERROR) { return exp; }
        final_body_expr = exp;
    } else {
        /* Handle empty body error or return unspecified. */
        return make_cell_error(
            "Procedure body is empty",
            SYNTAX_ERR);
    }

    /* If there WERE defines, wrap everything in letrec*. */
    if (inner_defines->count > 0) {
        Cell* letrec_expr = make_cell_sexpr();
        cell_add(letrec_expr, G_letrec_star_sym);
        Cell* t = transform_defines_to_bindings(inner_defines, env);
        if (t->type == CELL_ERROR) { return t; }
        cell_add(letrec_expr, t);
        cell_add(letrec_expr, final_body_expr);
        return letrec_expr;
    }

    return final_body_expr;
}


/* (let ⟨variable⟩ ⟨bindings⟩ ⟨body⟩)
 * Semantics: “Named let” is a variant on the syntax of let which provides a more general looping construct than do and
 * can also be used to express recursion. It has the same syntax and semantics as ordinary let except that ⟨variable⟩ is
 * bound within ⟨body⟩ to a procedure whose formal arguments are the bound variables and whose body is ⟨body⟩. Thus, the
 * execution of ⟨body⟩ can be repeated by invoking the procedure named by ⟨variable⟩. */
static Cell* expand_named_let(const Cell* c, [[maybe_unused]] Lex* env) {
    if (c->count < 4) {
        return make_cell_error(
            "let: Not enough args",
            ARITY_ERR);
    }

    /* c is (let name ((var init) ...) body...). */
    Cell* name = c->cell[1];
    const Cell* bindings = c->cell[2];

    if (bindings->type !=  CELL_SEXPR) {
        return make_cell_error(
            "let: bindings must be enclosed in S-expr",
            SYNTAX_ERR);
    }

    Cell* vars = make_cell_sexpr();
    Cell* inits = make_cell_sexpr();

    for (int i = 0; i < bindings->count; i++) {
        const Cell* this_binding = bindings->cell[i];
        if (this_binding->type != CELL_SEXPR || this_binding->count != 2) {
            return make_cell_error(
                "let: bindings must be '(symbol <expr>)'",
                SYNTAX_ERR);
        }

        cell_add(vars, this_binding->cell[0]);
        cell_add(inits, this_binding->cell[1]);
    }

    /* Build the lambda: (lambda (vars...) body...). */
    Cell* lambda = make_cell_sexpr();
    cell_add(lambda, G_lambda_sym);
    cell_add(lambda, vars);
    /* Add all body expressions (starting from index 3 in the original let). */
    for (int i = 3; i < c->count; i++) {
        cell_add(lambda, c->cell[i]);
    }

    /* Build the letrec:  ((letrec ((name (lambda (vars...) body...))) name) inits...). */

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

    /* (letrec ((name lambda)) name): evaluates to the loop procedure. */
    Cell* letrec_expr = make_cell_sexpr();
    cell_add(letrec_expr, G_letrec_sym);
    cell_add(letrec_expr, lr_bindings_list);
    cell_add(letrec_expr, name);

    /* ((letrec ...) inits...): inits are evaluated in the OUTER scope. */
    Cell* call = make_cell_sexpr();
    cell_add(call, letrec_expr);
    for (int i = 0; i < inits->count; i++) {
        cell_add(call, inits->cell[i]);
    }
    return call;
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
static Cell* expand_do(const Cell* c, [[maybe_unused]] Lex* env)
{
    if (c->count < 3) {
        return make_cell_error(
        "Malformed do expression",
        SYNTAX_ERR);
    }

    const Cell* bindings_input = c->cell[1];
    if (bindings_input->type != CELL_SEXPR) {
        return make_cell_error(
            "do: bindings must be enclosed in an S-expr",
            SYNTAX_ERR);
    }
    const Cell* test_clause = c->cell[2];
    if (test_clause->type != CELL_SEXPR || test_clause->count == 0) {
        return make_cell_error(
            "do: cannot have empty test expression",
            SYNTAX_ERR);
    }

    /* Use ONE symbol for the loop name to ensure pointer equality. */
    Cell* loop_name = gen_sym("do");

    Cell* let_bindings = make_cell_sexpr();
    Cell* loop_steps = make_cell_sexpr();
    cell_add(loop_steps, loop_name);

    for (int i = 0; i < bindings_input->count; i++) {
        const Cell* b = bindings_input->cell[i];

        if (b->type != CELL_SEXPR || b->count < 2 || b->count > 3) {
            return make_cell_error(
                "do: malformed bindings",
                SYNTAX_ERR);
        }

        if (b->cell[0]->type != CELL_SYMBOL) {
            return make_cell_error(
                "do: first value in binding must be a symbol",
                SYNTAX_ERR);
        }

        Cell* binding_pair = make_cell_sexpr();
        cell_add(binding_pair, b->cell[0]);
        cell_add(binding_pair, b->cell[1]);
        cell_add(let_bindings, binding_pair);

        if (b->count > 2) {
            cell_add(loop_steps, b->cell[2]);
        } else {
            cell_add(loop_steps, b->cell[0]);
        }
    }

    /* Build the IF logic. */
    Cell* if_expr = make_cell_sexpr();
    cell_add(if_expr, G_if_sym);
    cell_add(if_expr, test_clause->cell[0]);

    /* The test clause can have multiple expressions (R7RS) */
    /* (test expr1 expr2 ...) should run all and return the last. */
    if (test_clause->count > 1) {
        Cell* test_result_body = make_cell_sexpr();
        cell_add(test_result_body, G_begin_sym);

        for (int i = 1; i < test_clause->count; i++) {
            cell_add(test_result_body, test_clause->cell[i]);
        }

        cell_add(if_expr, test_result_body);
    } else {
        cell_add(if_expr, USP_Obj);
    }

    Cell* begin_block = make_cell_sexpr();
    cell_add(begin_block, G_begin_sym);

    for (int i = 3; i < c->count; i++) {
        cell_add(begin_block, c->cell[i]);
    }

    cell_add(begin_block, loop_steps);
    cell_add(if_expr, begin_block);

    /* Wrap in Named Let. */
    Cell* named_let = make_cell_sexpr();
    cell_add(named_let, G_let_sym);
    cell_add(named_let, loop_name);
    cell_add(named_let, let_bindings);
    cell_add(named_let, if_expr);

    return named_let;
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
static Cell* expand_case(const Cell* c, const Lex* env)
{
    /* c is (case key clauses...). */
    if (c->count < 3) {
        return make_cell_error(
        "Malformed case expression",
        SYNTAX_ERR);
    }

    Cell* key_expr = c->cell[1];

    /* Generate a unique symbol for this specific case expansion. */
    Cell* tmp_sym = gen_sym("case");

    /* Create the cond block. */
    Cell* cond_block = make_cell_sexpr();
    cell_add(cond_block, G_cond_sym);

    /* Iterate through clauses starting at index 2. */
    for (int i = 2; i < c->count; i++) {
        const Cell* clause = c->cell[i];
        if (clause->type != CELL_SEXPR || clause->count < 2) {
            return make_cell_error(
                "case: malformed clause",
                SYNTAX_ERR);
        }
        Cell* head = clause->cell[0];
        const bool is_else   = is_same_symbol(head, G_else_sym);
        const bool has_arrow = is_same_symbol(clause->cell[1], G_arrow_sym);
        if (has_arrow && clause->count != 3) {
            return make_cell_error(
                "case: => needs exactly one expression",
                SYNTAX_ERR);
        }

        Cell* cond_clause = make_cell_sexpr();

        /* Test part. */
        if (is_else) {
            if (i != c->count - 1) {
                /* error: else must be last */
                return make_cell_error(
                    "case: 'else' clause must be last",
                    SYNTAX_ERR);
            }
            cell_add(cond_clause, G_else_sym);
        } else {
            if (head->type != CELL_SEXPR) {
                /* error: datums must be in a list */
                return make_cell_error(
                    "case: datums must be enclosed in S-expr",
                    SYNTAX_ERR);
            }
            /* Transform clause to (memv tmp '(head)). */
            Cell* memv_call = make_cell_sexpr();
            cell_add(memv_call, builtin(env, "memv"));
            cell_add(memv_call, tmp_sym); /* The temp var name. */

            /* The head needs to be quoted so it's treated as data. */
            Cell* quoted_data = make_cell_sexpr();
            cell_add(quoted_data, G_quote_sym);
            cell_add(quoted_data, head);

            cell_add(memv_call, quoted_data);
            cell_add(cond_clause, memv_call);
        }

        /* Result part. */
        if (has_arrow) {
            Cell* call = make_cell_sexpr();
            cell_add(call, clause->cell[2]);   /* the procedure expression */
            cell_add(call, tmp_sym);           /* applied to the key */
            cell_add(cond_clause, call);
        } else {
            for (int j = 1; j < clause->count; j++) {
                cell_add(cond_clause, clause->cell[j]);
            }
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


static Cell* expand_define(const Cell* c, Lex* env)
{
    Cell* first = c->cell[0];
    Cell* result = make_cell_sexpr();
    cell_add(result, first);        /* 'define' */
    cell_add(result, c->cell[1]);   /* '(name args)' */
    Cell* be = expand_body_expressions(c, 2, env);
    if (be->type == CELL_ERROR) { return be; }
    cell_add(result, be);
    return result;
}


static Cell* expand_lambda(const Cell* c, Lex* env)
{
    Cell* first = c->cell[0];
    Cell* result = make_cell_sexpr();
    cell_add(result, first);        /* 'lambda' */
    cell_add(result, c->cell[1]);   /* '(args)' */
    Cell* be = expand_body_expressions(c, 2, env);
    if (be->type == CELL_ERROR) { return be; }
    cell_add(result, be);
    return result;
}


static Cell* expand_recursive(const Cell* c, Lex* env)
{
    Cell* result = make_cell_sexpr();
    for (int i = 0; i < c->count; i++) {
        Cell* exp = expand(c->cell[i], env);
        if (exp->type == CELL_ERROR) { return exp; }
        cell_add(result, exp);
    }
    return result;
}


static Cell* expand_let_family(const Cell* c, Lex* env)
{
    /* Malformed? Pass through; let the primitive report the error. */
    if (c->count < 3 || c->cell[1]->type != CELL_SEXPR) {
        return expand_recursive(c, env);
    }

    Cell* result = make_cell_sexpr();
    cell_add(result, c->cell[0]); /* let / let* / letrec / letrec* */

    /* Rebuild bindings: keep var names untouched, expand init expressions. */
    const Cell* old_bindings = c->cell[1];
    Cell* new_bindings = make_cell_sexpr();
    for (int i = 0; i < old_bindings->count; i++) {
        const Cell* pair = old_bindings->cell[i];
        if (pair->type != CELL_SEXPR || pair->count != 2) {
            return make_cell_error(
                fmt_err("%s: Invalid form", c->cell[0]->sym),
                SYNTAX_ERR);
        }
        Cell* new_pair = make_cell_sexpr();
        cell_add(new_pair, pair->cell[0]);         /* var — do NOT expand */
        Cell* exp = expand(pair->cell[1], env);
        if (exp->type == CELL_ERROR) { return exp; }
        cell_add(new_pair, exp);                   /* init — DO expand */
        cell_add(new_bindings, new_pair);
    }
    cell_add(result, new_bindings);

    /* Body: inner-define collection + begin wrap. */
    Cell* be = expand_body_expressions(c, 2, env);
    if (be->type == CELL_ERROR) { return be; }
    cell_add(result, be);
    return result;
}


/* Helper to handle the append logic for transform_qq(). */
static Cell* transform_qq_list_logic(const Cell* input, const int depth, Lex* env)
{
    Cell* out_expr = make_cell_sexpr();
    cell_add(out_expr, builtin(env, "append"));

    for (int i = 0; i < input->count; i++) {
        Cell* item = input->cell[i];

        if (item->type == CELL_SEXPR &&
            item->count > 0 &&
            item->cell[0] == G_unquote_splicing_sym &&
            depth == 1) {

            if (item->count != 2) {
                return make_cell_error(
                    "unquote-splicing: expects exactly one expression",
                    SYNTAX_ERR);
            }
            Cell* exp = expand(item->cell[1], env);
            if (exp->type == CELL_ERROR) { return exp; }
            cell_add(out_expr, exp);
        } else {
            Cell* tqq = transform_qq(item, depth, env);
            if (tqq->type == CELL_ERROR) { return tqq; }
            Cell* l = make_cell_sexpr();
            cell_add(l, builtin(env, "list"));
            cell_add(l, tqq);
            cell_add(out_expr, l);
        }
    }
    return out_expr;
}


Cell* transform_qq(Cell* input, const int depth, Lex* env)
{
    if (input->type == CELL_VECTOR) {
        Cell* expanded_list = transform_qq_list_logic(input, depth, env);
        if (expanded_list->type == CELL_ERROR) { return expanded_list; }
        Cell* r = make_cell_sexpr();
        cell_add(r, builtin(env, "list->vector"));
        cell_add(r, expanded_list);
        return r;
    }

    /* Atoms (non-S-expr) get quoted. */
    if (input->type != CELL_SEXPR) {
        return make_quote_form(input);
    }

    /* Quote empty S-expr: yields (quote ()) with a real nil. */
    if (input->count == 0) {
        return make_quote_form(input);
    }

    const Cell* first = input->cell[0];

    /* UNQUOTE */
    if (first == G_unquote_sym) {
        if (input->count != 2) {
            return make_cell_error(
                "unquote: expects exactly one expression",
                SYNTAX_ERR);
        }

        if (depth == 1) {
            return expand(input->cell[1], env);   /* real unquote: code */
        }

        /* Nested: still data. Rebuild (list 'unquote <inner at depth-1>).
         * Do NOT expand it. */
        Cell* inner = transform_qq(input->cell[1], depth - 1, env);
        if (inner->type == CELL_ERROR) { return inner; }

        Cell* r = make_cell_sexpr();
        cell_add(r, builtin(env, "list"));
        Cell* qs = make_cell_sexpr();
        cell_add(qs, G_quote_sym);
        cell_add(qs, G_unquote_sym);
        cell_add(r, qs);
        cell_add(r, inner);
        return r;
    }

    /* UNQUOTE SPLICE */
    if (first == G_unquote_splicing_sym) {
        if (input->count != 2) {
            return make_cell_error(
                "unquote-splicing: expects exactly one expression",
                SYNTAX_ERR);
        }

        if (depth == 1) {
            /* Real splices inside lists are handled in transform_qq_list_logic,
             * so reaching here means it isn't inside a list. */
            return make_cell_error(
                "unquote-splicing: only valid inside a list",
                SYNTAX_ERR);
        }

        /* Nested: still data. Rebuild (list 'unquote-splicing <inner at depth-1>). */
        Cell* inner = transform_qq(input->cell[1], depth - 1, env);
        if (inner->type == CELL_ERROR) { return inner; }
        Cell* r = make_cell_sexpr();
        cell_add(r, builtin(env, "list"));
        Cell* qs = make_cell_sexpr();
        cell_add(qs, G_quote_sym);
        cell_add(qs, G_unquote_splicing_sym);
        cell_add(r, qs);
        cell_add(r, inner);
        return r;
    }

    /* NESTED QUASIQUOTE: Rebuild (list 'quasiquote ...). */
    if (first == G_quasiquote_sym) {
        if (input->count != 2) {
            return make_cell_error(
                "quasiquote: expects exactly one expression",
                SYNTAX_ERR);
        }

        Cell* inner = transform_qq(input->cell[1], depth + 1, env);
        if (inner->type == CELL_ERROR) { return inner; }

        Cell* r = make_cell_sexpr();
        cell_add(r, builtin(env, "list"));
        Cell* qs = make_cell_sexpr();
        cell_add(qs, G_quote_sym);
        cell_add(qs, G_quasiquote_sym);
        cell_add(r, qs);
        cell_add(r, inner);
        return r;
    }

    /* STANDARD S-EXPR: Build (append (list ...) ...). */
    return transform_qq_list_logic(input, depth, env);
}


/* Apply macro m to the call form c, then expand the result. */
Cell* expand_macro_call(const Cell* m, const Cell* c, Lex* env)
{
    if (++expand_depth > MAX_EXPAND_DEPTH) {
        expand_depth--;
        return make_cell_error("macro nested too deeply", VALUE_ERR);
    }
    Cell* out = coz_apply_and_get_val(m, get_args_from_sexpr(c), env);
    if (out->type == CELL_ERROR) {
        expand_depth--;
        return out;
    }
    Cell* r = expand(out->type == CELL_PAIR ? make_sexpr_from_list(out, true) : out, env);
    expand_depth--;
    return r;
}


static Cell* expand_defmacro_body(const Cell* c, Lex* env)
{
    Cell* body = expand_body_expressions(c, 3, env);
    if (body->type == CELL_ERROR) { return body; }
    Cell* r = make_cell_sexpr();
    cell_add(r, c->cell[0]);
    cell_add(r, c->cell[1]);
    cell_add(r, c->cell[2]);
    cell_add(r, body);
    return r;
}


static Cell* lookup_macro(const Lex* env, const Cell* head)
{
    Cell* r = ht_get(env->working, head->sym);
    return (r && r->type == CELL_MACRO) ? r : nullptr;
}


Cell* expand(Cell* c, Lex* env)
{
    /* Base case: only S-expressions can be expanded */
    if (c->type != CELL_SEXPR || c->count == 0) { return c; }

    const Cell* head = c->cell[0];

    if (head->type == CELL_SYMBOL) {

        if (is_same_symbol(head, G_quote_sym)) {
            if (c->count != 2) { return c; }   /* sf_quote reports the arity error */
            Cell* q = make_cell_sexpr();
            cell_add(q, G_quote_sym);
            cell_add(q, make_list_from_sexpr(c->cell[1]));
            return q;
        }

        /* Pass imports through unchanged. */
        if (is_same_symbol(head, G_import_sym)) {
            return c;
        }

        /* defmacro - sequence the body expressions. */
        if (is_same_symbol(head, G_defmacro_sym)) {
            return expand_defmacro_body(c, env);
        }

        const Cell* m = lookup_macro(env, head);   /* NULL unless bound to a CELL_MACRO */
        if (m) { return expand_macro_call(m, c, env); }

        /* 'define' - primitive - transform nested defines into 'let's. */
        if (is_same_symbol(head, G_define_sym) && c->count > 2 && c->cell[1]->type == CELL_SEXPR) {
            return expand_define(c, env);
        }

        /* 'lambda' - primitive - sequence body expressions in (begin ... ) expression. */
        if (is_same_symbol(head, G_lambda_sym) && c->count > 2) {
            return expand_lambda(c, env);
        }

        /* 'case' - derived - transform into named let, then
         * transform that 'let' into a 'letrec' or recurse. */
        if (is_same_symbol(head, G_case_sym)) {
            return expand(expand_case(c, env), env);
        }

        /* 'do' - derived - transform into named let, then
           recurse so the named let becomes a letrec. */
        if (is_same_symbol(head, G_do_sym)) {
            return expand(expand_do(c, env), env);
        }

        /* 'quasiquote' -  */
        if (is_same_symbol(head, G_quasiquote_sym)) {
            if (c->count != 2) {
                return make_cell_error(
                    "quasiquote: not enough args",
                    ARITY_ERR);
            }
            return transform_qq(c->cell[1], 1, env);
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
            if (c->count > 1 && c->cell[1]->type == CELL_SYMBOL) {
                return expand(expand_named_let(c, env), env);
            }
            return expand_let_family(c, env);
        }

        /* let*, letrec, and letrec* primitives handled the same. */
        if (is_same_symbol(head, G_let_star_sym))    { return expand_let_family(c, env); }
        if (is_same_symbol(head, G_letrec_sym))      { return expand_let_family(c, env); }
        if (is_same_symbol(head, G_letrec_star_sym)) { return expand_let_family(c, env); }
    }

    /* Fallback: expand elements of the list. */
    return expand_recursive(c, env);
}
