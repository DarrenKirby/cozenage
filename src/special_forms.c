/*
 * 'special_forms.c'
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

#include "special_forms.h"
#include "eval.h"
#include "types.h"
#include "symbols.h"
#include "load_library.h"
#include <string.h>
#include <gc/gc.h>


/* Helpers for iteration clarity */
#define first  0
#define second 1
#define third  2
#define last(ptr) ((ptr)->count - 1)


/* TODO - implement transformations, and double-check for all tail calls.
 * - Need to create transformations.c and write functions to transform
 *     derived syntax to common forms.
 * - Need to cross-check with R7RS and make sure all tail call positions
 *     are handled correctly and kicked back to the eval loop. */

/* A helper to check if a symbol name is a reserved syntactic keyword */
int is_syntactic_keyword(const char* s) {
    const char* keywords[] = {
        "define", "quote", "lambda", "if", "when", "unless",
        "cond", "import", "set!", "let", "let*" ,"letrec",
        "begin", "do", "case", "and", "or", nullptr
    };

    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(s, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Convert a CELL_SEXPR to a proper CELL_PAIR linked-list */
Cell* sexpr_to_list(Cell* c) {
    /* If the item is not an S-expression, it's an atom. Return it */
    if (c->type != CELL_SEXPR) {
        return c;
    }

    /* It is an S-expression. Check for improper list syntax. */
    int dot_pos = -1;
    if (c->count > 1) {
        const Cell* dot_candidate = c->cell[c->count - 2];
        if (dot_candidate->type == CELL_SYMBOL && strcmp(dot_candidate->sym, ".") == 0) {
            dot_pos = c->count - 2;
        }
    }

    /* Handle Improper List */
    if (dot_pos != -1) {
        /* The final cdr is the very last element in the S-expression. */
        Cell* final_cdr = sexpr_to_list(c->cell[c->count - 1]);

        /* Build the list chain backwards from the element *before* the dot. */
        Cell* list_head = final_cdr;
        for (int i = dot_pos - 1; i >= 0; i--) {
            Cell* element = sexpr_to_list(c->cell[i]);
            list_head = make_cell_pair(element, list_head);
        }
        return list_head;
    }

    /* Handle Proper List */
    Cell* list_head = make_cell_nil();
    const int len = c->count;

    for (int i = len - 1; i >= 0; i--) {
        /* Recursively call this function on each element to ensure
         * any nested S-expressions are also converted. */
        Cell* element = sexpr_to_list(c->cell[i]);

        /* Prepend the new element to the head of our list. */
        list_head = make_cell_pair(element, list_head);
        list_head->len = len - i;
    }
    return list_head;
}

Lex* build_lambda_env(const Lex* env, const Cell* formals, const Cell* args) {
    /* Create a new child environment */
    Lex* local_env = new_child_env(env);
    /* Bind formals to arguments */

    for (int i = 0; i < args->count; i++) {
        const Cell* sym = formals->cell[i];
        const Cell* val = args->cell[i];
        lex_put_local(local_env, sym, val);  /* sym should be CELL_SYMBOL, val evaluated */
    }
    return local_env;
}

/* Just takes body statements and stuffs them in a 'begin' expression */
Cell* sequence_sf_body(const Cell* body) {
    Cell* seq = make_cell_sexpr();
    cell_add(seq, G_begin_sym);

    /* Iterate over the list of expressions in the body
     * and add each complete expression to our new 'begin' block. */
    for (int i = 0; i < body->count; i++) {
        cell_add(seq, body->cell[i]);
    }
    return seq;
}

/* Searches the local environment chain and returns the specific frame
 * where 'sym' is bound. Returns NULL if not found in any local frame. */
static Ch_Env* lex_find_local_frame(const Lex* e, const char* sym) {
    if (!e) return nullptr;

    Ch_Env* current_frame = e->local;
    while (current_frame != nullptr) {
        for (int i = 0; i < current_frame->count; i++) {
            if (strcmp(current_frame->syms[i], sym) == 0) {
                /* Found it! Return a pointer to this frame. */
                return current_frame;
            }
        }
        current_frame = current_frame->parent;
    }

    /* Not found in any local frame. */
    return nullptr;
}

/* -----------------------------*
 *         Special forms        *
 * -----------------------------*/

/* (define ⟨variable⟩ ⟨expression⟩) OR (define (⟨variable⟩ ⟨formals⟩) ⟨body⟩)
 * ⟨Formals⟩ are either a sequence of zero or more variables, or a sequence of one or more variables
 * followed by a space-delimited period and another variable (as in a lambda expression). This form
 * is equivalent to:
 *
 *    (define ⟨variable⟩
 *        (lambda (⟨formals⟩) ⟨body⟩))
 *
 *  TODO: (define (⟨variable⟩ . ⟨formal⟩) ⟨body⟩) form
 */
HandlerResult sf_define(Lex* e, Cell* a) {
    if (a->count < 2) {
        Cell* err = make_cell_error("define requires at least 2 arguments", ARITY_ERR);
        return (HandlerResult){ .action = ACTION_RETURN, .value = err };
    }
    const Cell* target = a->cell[0];

    /* Disallow rebinding of keywords */
    if (is_syntactic_keyword(target->sym)) {
        char err_buf[128];
        snprintf(err_buf, sizeof(err_buf),
                 "Syntax keyword '%s' cannot be used as a variable", target->sym);
        Cell* err = make_cell_error(err_buf, VALUE_ERR);
        return (HandlerResult){ .action = ACTION_RETURN, .value = err };
    }

    /* (define <symbol> <expr>) */
    if (target->type == CELL_SYMBOL) {
        Cell* val = coz_eval(e, a->cell[1]);
        /* Bail out if error encountered during evaluation */
        if (val->type == CELL_ERROR) {
            return (HandlerResult){ .action = ACTION_RETURN, .value = val };
        }
        /* Grab the name for the un-sugared define lambda */
        if (val->type == CELL_PROC) {
            val->lambda->l_name = target->sym;
        }
        lex_put_global(e, target, val);
        return (HandlerResult){ .action = ACTION_RETURN, .value = val };
    }

    /* (define (<f-name> <args>) <body>) */
    if (target->type == CELL_SEXPR && target->count > 0 &&
        target->cell[0]->type == CELL_SYMBOL) {

        /* first element is function name */
        const Cell* fname = target->cell[0];

        /* rest are formal args */
        Cell* formals = make_cell_sexpr();
        for (int i = 1; i < target->count; i++) {
            if (target->cell[i]->type != CELL_SYMBOL) {
                Cell* err = make_cell_error("lambda formals must be symbols", TYPE_ERR);
                return (HandlerResult){ .action = ACTION_RETURN, .value = err };
            }
            cell_add(formals, target->cell[i]);
        }

        /* build lambda with args + body */
        Cell* body = make_cell_sexpr();
        for (int i = 1; i < a->count; i++) {
            cell_add(body, a->cell[i]);
        }

        Cell* lam = lex_make_named_lambda(fname->sym, formals, body, e);
        lex_put_global(e, fname, lam);
        return (HandlerResult){ .action = ACTION_RETURN, .value = lam };
        }

    Cell* err = make_cell_error("invalid define syntax", SYNTAX_ERR);
    return (HandlerResult){ .action = ACTION_RETURN, .value = err };
}

/* (quote ⟨datum⟩) OR '⟨datum⟩
 * (quote ⟨datum⟩) evaluates to ⟨datum⟩. ⟨Datum⟩ can be any external representation of a Scheme
 * object. This notation is used to include literal constants in Scheme code. */
HandlerResult sf_quote(Lex* e, Cell* a) {
    (void)e;
    if (a->count != 1) {
        Cell* err = make_cell_error("quote takes exactly one argument", ARITY_ERR);
        return (HandlerResult){ .action = ACTION_RETURN, .value = err };
    }

    /* Extract the expression that was quoted. */
    Cell* qexpr = cell_take(a, 0);

    Cell* result = sexpr_to_list(qexpr);
    return (HandlerResult){ .action = ACTION_RETURN, .value = result };
}

/* (lambda ⟨formals⟩ ⟨body⟩)
 * A lambda expression evaluates to a procedure. The environment in effect when the
 * lambda expression is evaluated is remembered as part of the procedure; it is called the closing
 * environment. When the procedure is later called with some arguments, the closing environment is
 * extended by binding the variables in the formal parameter list to fresh locations, and the
 * locations are filled with the arguments according to rules about to be given. The new environment
 * created by this process is referred to as the invocation environment. */
HandlerResult sf_lambda(Lex* e, Cell* a) {
    if (a->count < 2) {
        Cell* err = make_cell_error("lambda requires formals and a body", SYNTAX_ERR);
        return (HandlerResult){ .action = ACTION_RETURN, .value = err };
    }

    Cell* formals = cell_pop(a, first);   /* first arg */
    Cell* body = a;                         /* remaining args */

    /* formals should be a list of symbols */
    for (int i = 0; i < formals->count; i++) {
        if (formals->cell[i]->type != CELL_SYMBOL) {
            Cell* err = make_cell_error("lambda formals must be symbols", TYPE_ERR);
            return (HandlerResult){ .action = ACTION_RETURN, .value = err };
        }
    }
    /* Build the lambda cell */
    Cell* lambda = lex_make_lambda(formals, body, e);
    return (HandlerResult){ .action = ACTION_RETURN, .value = lambda };
}

/* (if ⟨test⟩ ⟨consequent⟩ ⟨alternate⟩)
 * An if expression is evaluated as follows: first, ⟨test⟩ is evaluated. If it yields a true value,
 * then ⟨consequent⟩ is evaluated and its values are returned. Otherwise, ⟨alternate⟩ is evaluated
 * and its values are returned. If no <alternate> is provided to evaluate, it returns null */
HandlerResult sf_if(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3);
    if (err) {
        return (HandlerResult){ .action = ACTION_RETURN, .value = err };
    }
    const Cell* test = coz_eval(e, a->cell[first]);

    /* Check if the result is TRUTHY */
    if (test && !(test->type == CELL_BOOLEAN && test->boolean_v == 0)) {
        /* Test was true, so evaluate the consequent as a tail call. */
        return (HandlerResult){ .action = ACTION_CONTINUE, .value = a->cell[second] };
    }

    /* Test was false.
     * Check if an alternative exists before accessing it. */
    if (a->count == 3) {
        /* It exists, so evaluate it as a tail call. */
        return (HandlerResult){ .action = ACTION_CONTINUE, .value = a->cell[third] };
    }

    /* No alternative was provided. Return an unspecified value. */
    return (HandlerResult){ .action = ACTION_RETURN, .value = nullptr };
}

/* (when ⟨test⟩ ⟨expression1⟩ ⟨expression2⟩ ... )
 * The test is evaluated, and if it evaluates to a true value, the expressions are evaluated in
 * order. The result of the when expression is unspecified, per R7RS, but Cozenage returns the value
 * of the last expression evaluated, or null if the test evaluates to #f. */
HandlerResult sf_when(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return (HandlerResult){ .action = ACTION_RETURN, .value = err };

    /* Pop off the test */
    const Cell* test = coz_eval(e, cell_pop(a, first));

    /* Safety check for NULL from eval, treat it as truthy
     * and check for literal #f */
    if (test && test->type == CELL_BOOLEAN && test->boolean_v == 0) {
        /* Test was false, return unspecified. */
        return (HandlerResult){ .action = ACTION_RETURN, .value = nullptr };
    }

    /* Sequence remaining expressions into a 'begin' and tail-call */
    Cell* body_block = sequence_sf_body(a);
    return (HandlerResult){ .action = ACTION_CONTINUE, .value = body_block };
}

/*  (unless ⟨test⟩ ⟨expression1⟩ ⟨expression2⟩ ... )
 *  The test is evaluated, and if it evaluates to #f, the expressions are evaluated in order. The
 *  result of the unless expression is unspecified, per R7RS, but Cozenage returns the value of the
 *  last expression evaluated, or null if the test is truthy. */
HandlerResult sf_unless(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return (HandlerResult){ .action = ACTION_RETURN, .value = err };

    /* Pop off the test */
    const Cell* test = coz_eval(e, cell_pop(a, first));

    /* Safety check for NULL from eval, treat it as truthy
     * and check for literal #f */
    if (test && test->type == CELL_BOOLEAN && test->boolean_v == 0) {
        /* Sequence remaining expressions into a 'begin' and tail-call */
        Cell* body_block = sequence_sf_body(a);
        return (HandlerResult){ .action = ACTION_CONTINUE, .value = body_block };
    }

    /* Test was true (or null), return unspecified. */
    return (HandlerResult){ .action = ACTION_RETURN, .value = nullptr };
}

/* (cond ⟨clause1⟩ ⟨clause2⟩ ... )
 * where ⟨clause⟩ is (⟨test⟩ ⟨expression1⟩ ...) OR (⟨test⟩ => ⟨expression⟩)
 * The last ⟨clause⟩ can be an “else clause”. A cond expression is evaluated by evaluating the
 * ⟨test⟩ expressions of successive ⟨clause⟩s in order until one of them evaluates to a true value.
 * When a ⟨test⟩ evaluates to a true value, the remaining ⟨expression⟩s in its ⟨clause⟩ are
 * evaluated in order, and the results of the last ⟨expression⟩ in the ⟨clause⟩ are returned as the
 * results of the entire cond expression
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
HandlerResult sf_cond(Lex* e, Cell* a) {
    if (a->count == 0) {
        Cell* err = make_cell_error("ill-formed cond expression", VALUE_ERR);
        return (HandlerResult){ .action = ACTION_RETURN, .value = err };
    }

    for (int i = 0; i < a->count; i++) {
        const Cell* clause = a->cell[i];

        /* Clause must be a list */
        if (clause->type != CELL_SEXPR || clause->count == 0) {
            Cell* err = make_cell_error("cond clause must be a non-empty list", SYNTAX_ERR);
            return (HandlerResult){ .action = ACTION_RETURN, .value = err };
        }

        /* Check for 'else' clause and if found evaluate any expressions*/
        if (clause->cell[first]->type == CELL_SYMBOL && clause->cell[first] == G_else_sym) {
            /* else clause must be last */
            if (i != last(a)) {
                Cell* err = make_cell_error("else clause must be last in the cond expression",
                                     SYNTAX_ERR);
                return (HandlerResult){ .action = ACTION_RETURN, .value = err };
            }
            /* eval the first n-1 expressions */
            for (int j = 1; j < last(clause); j++) {
                Cell* exp = coz_eval(e, clause->cell[j]);
                if (exp && exp->type == CELL_ERROR) {
                    return (HandlerResult){ .action = ACTION_RETURN, .value = exp };
                }
            }
            /* return the tail call */
            return (HandlerResult){ .action = ACTION_CONTINUE, .value = clause->cell[last(clause)] };
        }

        /* Not an else, so evaluate the test */
        Cell* test = coz_eval(e, clause->cell[first]);

        /* Move along if current test is #f */
        if (test->type == CELL_BOOLEAN && test->boolean_v == false) {
            continue;
        }

        /* Test is truthy - first see if there is an expression */
        if (clause->count == 1) {
            /* No expression, return the test result */
            return (HandlerResult){ .action = ACTION_RETURN, .value = test };
        }

        /* Check for cond '=>' form */
        if (clause->cell[1]->type == CELL_SYMBOL && strcmp(clause->cell[1]->sym, "=>") == 0) {
            if (clause->count <= 2) {
                Cell* err = make_cell_error("cond '=>' form must have an expression", SYNTAX_ERR);
                return (HandlerResult){ .action = ACTION_RETURN, .value = err };
            }

            /* '=>' form can only have one expression after the test */
            if (clause->count > 3) {
                Cell* err = make_cell_error("cond '=>' form can only have 1 expression after the test",
                                     SYNTAX_ERR);
                return (HandlerResult){ .action = ACTION_RETURN, .value = err };
            }
            const Cell* proc = coz_eval(e, clause->cell[2]);
            /* Expression must evaluate to a procedure */
            if (proc->type != CELL_PROC) {
                Cell* err = make_cell_error("expression after '=>' must evaluate to a procedure",
                                     SYNTAX_ERR);
                return (HandlerResult){ .action = ACTION_RETURN, .value = err };
            }
            /* A tail call */
            Cell* tmp = make_sexpr_len2(proc, test);
            return (HandlerResult){ .action = ACTION_CONTINUE, .value = tmp };
        }
        /* Expressions present. eval all but the last */
        for (int j = 1; j < last(clause); j++) {
            Cell* exp = coz_eval(e, clause->cell[j]);
            if (exp && exp->type == CELL_ERROR) {
                return (HandlerResult){ .action = ACTION_RETURN, .value = exp };
            }
        }
        /* Return the last expression itself for the tail call */
        return (HandlerResult){ .action = ACTION_CONTINUE, .value = clause->cell[last(clause)] };
    }
    /* All tests #f with no else is unspecified, so just return null */
    return (HandlerResult){ .action = ACTION_RETURN, .value = nullptr };
}

/* (import ⟨import-set⟩ ...)
 * An import declaration provides a way to import identifiers exported by a library. Each
 * ⟨import set⟩ names a set of bindings from a library and possibly specifies local names for the
 * imported bindings. */
/* TODO: implement 'only', 'except', 'prefix', and 'rename' */
HandlerResult sf_import(Lex* e, Cell* a) {
    Cell* import_set = make_cell_sexpr();
    import_set->cell = GC_MALLOC(sizeof(Cell*) * a->count);
    /* Make a new sexpr which contains pairs of (library . name), */
    int i;
    for (i = 0; i < a->count; i++) {
        const char* lib = GC_strdup(a->cell[i]->cell[0]->sym);
        const char* name = GC_strdup(a->cell[i]->cell[1]->sym);
        import_set->cell[i] = make_cell_pair(make_cell_string(lib),
                     make_cell_string(name));
    }
    import_set->count = i;

    Cell* result = nullptr;
    for (int j = 0; j < import_set->count; j++) {
        const char* library_type = import_set->cell[j]->car->str;
        const char* library_name = import_set->cell[j]->cdr->str;

        if (strcmp(library_type, "scheme") == 0 ||
            strcmp(library_type, "cozenage") == 0) {
            /* Load the Library */
            result = load_scheme_library(library_name, e);
        } else {
            /* TODO: Handle User Libraries Here
             * For example, (import (my-libs utils)). */
            Cell* err = make_cell_error("import: user-defined libraries not yet supported", GEN_ERR);
            return (HandlerResult){ .action = ACTION_RETURN, .value = err };
        }
    }
    return (HandlerResult){ .action = ACTION_RETURN, .value = result };
}

/* (let ⟨bindings⟩ ⟨body⟩) where ⟨Bindings⟩ has the form ((⟨variable1⟩ ⟨init1⟩) ...)
 * where each ⟨init⟩ is an expression, and ⟨body⟩ is a sequence of zero or more definitions followed
 * by a sequence of one or more expressions. It is an error for a ⟨variable⟩ to appear more than
 * once in the list of variables being bound.
 *
 * The ⟨init⟩s are evaluated in the current environment (in some unspecified order), the ⟨variable⟩s
 * are bound to fresh locations holding the results, the ⟨body⟩ is evaluated in the extended
 * environment, and the values of the last expression of ⟨body⟩ are returned. Each binding of a
 * ⟨variable⟩ has ⟨body⟩ as its region. */
HandlerResult sf_let(Lex* e, Cell* a) {
    /* Standard let */
    if (a->cell[0]->type != CELL_SYMBOL) {
        const Cell* bindings = cell_pop(a, 0);
        if (bindings->type != CELL_SEXPR) {
            Cell* err = make_cell_error("Bindings must be a list", VALUE_ERR);
            return (HandlerResult){ .action = ACTION_RETURN, .value = err };
        }
        const Cell* body = a;

        /* separate variables and values from bindings */
        /* TODO: raise error if not all variables are unique*/
        Cell* vars = make_cell_sexpr();
        Cell* vals = make_cell_sexpr();
        for (int i = 0; i < bindings->count; i++) {
            const Cell* local_b = bindings->cell[i];
            if (local_b->type != CELL_SEXPR) {
                Cell* err = make_cell_error("Bindings must be a list", VALUE_ERR);
                return (HandlerResult){ .action = ACTION_RETURN, .value = err };
            }
            if (local_b->count != 2) {
                Cell* err = make_cell_error("bindings must contain exactly 2 items", VALUE_ERR);
                return (HandlerResult){ .action = ACTION_RETURN, .value = err };
            }
            if (local_b->cell[0]->type != CELL_SYMBOL) {
                Cell* err = make_cell_error("first value in binding must be a symbol", VALUE_ERR);
                return (HandlerResult){ .action = ACTION_RETURN, .value = err };
            }
            cell_add(vars, local_b->cell[0]);
            cell_add(vals, local_b->cell[1]);
        }

        /* Create a new child environment */
        Lex* local_env = new_child_env(e);

        /* Populate it with sym->val pairs */
        for (int i = 0; i < vals->count; i++) {
            const Cell* sym = vars->cell[i];
            const Cell* val = coz_eval(e, vals->cell[i]);
            lex_put_local(local_env, sym, val);
        }

        /* Evaluate the body expressions in this environment */
        Cell* result = nullptr;
        for (int i = 0; i < body->count; i++) {
            result = coz_eval(local_env, body->cell[i]);
        }
        return (HandlerResult){ .action = ACTION_RETURN, .value = result };
    }
    /* Named let - de-sugar into a letrec */
    /* TODO: this is ugly dog shit
     * Really need to handle this transformation better -
     * Dispatch out all the transformations into a helper
     * straight from eval */
    Cell* nl_name = a->cell[0];
    const Cell* nl_vars = a->cell[1];
    Cell* nl_body = a->cell[2];
    /* Build the lambda formals and letrec body */
    Cell* lambda_formals = make_cell_sexpr();
    Cell* lr_body = make_cell_sexpr();
    cell_add(lr_body, nl_name);
    /* Iterate over bindings */
    for (int i = 0; i < nl_vars->count; i++) {
        cell_add(lambda_formals, nl_vars->cell[i]->cell[0]);
        cell_add(lr_body, nl_vars->cell[i]->cell[1]);
    }
    /* Build the lambda transformation */
    Cell* lambda = make_cell_sexpr();
    cell_add(lambda, G_lambda_sym);
    cell_add(lambda, lambda_formals);
    cell_add(lambda, nl_body);
    /* Build the letrec bindings */
    Cell* lr_bindings = make_cell_sexpr();
    cell_add(lr_bindings, nl_name );
    cell_add(lr_bindings, lambda);
    /* Put it all together */
    Cell* letrec_expr = make_cell_sexpr();
    cell_add(letrec_expr, make_sexpr_len1(lr_bindings));
    cell_add(letrec_expr, lr_body);

    return sf_letrec(e, letrec_expr);
}

/* (let* ⟨bindings⟩ ⟨body⟩) where ⟨Bindings⟩ has the form ((⟨variable1⟩ ⟨init1⟩) ...)
 * where each ⟨init⟩ is an expression, and ⟨body⟩ is a sequence of zero or more definitions followed
 * by a sequence of one or more expressions.
 *
 * The let* binding construct is similar to let, but the bindings are performed sequentially from
 * left to right, and the region of a binding indicated by (⟨variable⟩ ⟨init⟩) is that part of the
 * let* expression to the right of the binding. Thus, the second binding is done in an environment
 * in which the first binding is visible, and so on. The ⟨variable⟩s need not be distinct. */
HandlerResult sf_let_star(Lex* e, Cell* a) {
    const Cell* bindings = cell_pop(a, 0);
    if (bindings->type != CELL_SEXPR) {
        Cell* err = make_cell_error("Bindings must be a list", VALUE_ERR);
        return (HandlerResult){ .action = ACTION_RETURN, .value = err };
    }
    const Cell* body = a;

    /* Start with the outer environment. */
    Lex* current_env = e;

    for (int i = 0; i < bindings->count; i++) {
        const Cell* local_b = bindings->cell[i];
        if (local_b->type != CELL_SEXPR) {
            Cell* err = make_cell_error("Bindings must be a list", VALUE_ERR);
            return (HandlerResult){ .action = ACTION_RETURN, .value = err };
        }
        if (local_b->count != 2) {
            Cell* err = make_cell_error("bindings must contain exactly 2 items", VALUE_ERR);
            return (HandlerResult){ .action = ACTION_RETURN, .value = err };
        }
        if (local_b->cell[0]->type != CELL_SYMBOL) {
            Cell* err = make_cell_error("first value in binding must be a symbol", VALUE_ERR);
            return (HandlerResult){ .action = ACTION_RETURN, .value = err };
        }
        const Cell* formal = local_b->cell[0];
        Cell* arg = local_b->cell[1];

        /* Create the new environment for THIS binding.
         * The parent is the *previous* environment in the chain. */
        Lex* new_env = new_child_env(current_env);

        /* Evaluate the argument expression in the *current* environment. */
        const Cell* val = coz_eval(current_env, arg);

        /* Put the new binding into the new environment. */
        lex_put_local(new_env, formal, val);

        /* Update current_env to point to the new environment. */
        current_env = new_env;
    }

    /* Evaluate body expressions in this environment */
    Cell* result = nullptr;
    for (int i = 0; i < body->count; i++) {
        result = coz_eval(current_env, body->cell[i]);
    }
    return (HandlerResult){ .action = ACTION_RETURN, .value = result };
}

HandlerResult sf_letrec(Lex* e, Cell* a) {
    const Cell* bindings = cell_pop(a, 0);
    if (bindings->type != CELL_SEXPR) {
        Cell* err = make_cell_error("Bindings must be a list", VALUE_ERR);
        return (HandlerResult){ .action = ACTION_RETURN, .value = err };
    }

    const Cell* body = a;
    /* Create a new child environment */
    Lex* local_env = new_child_env(e);
    /* Iterate and bind placeholders */
    for (int i = 0; i < bindings->count; i++) {
        const Cell* variable = bindings->cell[i]->cell[0];
        /* Use CELL_EOF for now to designate an 'unspecified' value,
         * rather than create an explicit CELL_UNSPECIFIED type. */
        lex_put_local(local_env, variable, make_cell_eof());
    }
    /* Iterate and bind init-expressions (lambdas) to variables (lambda names) */
    for (int i = 0; i < bindings->count; i++) {
        const Cell* variable = bindings->cell[i]->cell[0];
        Cell* local_bind = bindings->cell[i]->cell[1];
        const Cell* init_exp = coz_eval(local_env, local_bind);
        if (init_exp->type == CELL_ERROR) {
            /* TODO:change this error message to print the problem expression/variable
               after printer.c is generalized to write to any stream rather than just stdout */
            char buf[128];
            snprintf(buf, sizeof(buf), "letrec: variable used before initialization");
            Cell* err = make_cell_error(buf, VALUE_ERR);
            return (HandlerResult){ .action = ACTION_RETURN, .value = err };
        }
        lex_put_local(local_env, variable, init_exp);
    }
    /* Iterate over all body expressions and return the last value evaluated */
    Cell* result = nullptr;
    for (int i = 0; i < body->count; i++) {
        result = coz_eval(local_env, body->cell[i]);
    }
    //return result;
    return (HandlerResult){ .action = ACTION_RETURN, .value = result };
}

/* (set! ⟨variable⟩ ⟨expression⟩)
 * ⟨Expression⟩ is evaluated, and the resulting value is stored in the location to which ⟨variable⟩
 * is bound. It is an error if ⟨variable⟩ is not bound either in some region enclosing the set!
 * expression or else globally. The result of the set! expression is unspecified. */
HandlerResult sf_set_bang(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return (HandlerResult){ .action = ACTION_RETURN, .value = err };
    const Cell* variable = a->cell[first];
    if (variable->type != CELL_SYMBOL) {
        err = make_cell_error("arg1 must be a symbol", TYPE_ERR);
        return (HandlerResult){ .action = ACTION_RETURN, .value = err };
    }

    const char* sym_to_set = a->cell[0]->sym;
    Cell* value_to_set = coz_eval(e, a->cell[1]);

    /* First, try to find the variable in the local environment chain. */
    const Ch_Env* target_frame = lex_find_local_frame(e, sym_to_set);

    if (target_frame != nullptr) {
        /* We found the correct frame, now find the variable again in *this specific frame*
         * and update its value directly. */
        for (int i = 0; i < target_frame->count; i++) {
            if (strcmp(target_frame->syms[i], sym_to_set) == 0) {
                target_frame->vals[i] = value_to_set;
                /* R7RS says the return from set! is unspecified.
                 * Cozenage will return the value set, for visual
                 * feedback that the operation was successful */
                return (HandlerResult){ .action = ACTION_RETURN, .value = value_to_set };
            }
        }
    } else {
        /* The variable was not in any local frame. Check global.
         * Use ht_get to see if it *exists* before we set it. */
        if (ht_get(e->global, sym_to_set)) {
            /* It exists globally, so update it in the hash table. */
            ht_set(e->global, sym_to_set, value_to_set);
            return (HandlerResult){ .action = ACTION_RETURN, .value = value_to_set };
        }
    }

    /* The variable was not found anywhere. This is an error. */
    char buf[128];
    snprintf(buf, sizeof(buf), "set!: Unbound symbol: '%s'", sym_to_set);
    return (HandlerResult){ .action = ACTION_RETURN, .value = make_cell_error(buf, VALUE_ERR) };
}

/* (begin ⟨expression1 ⟩ ⟨expression2 ⟩ ... )
 * This form of begin can be used as an ordinary expression. The ⟨expression⟩s are evaluated
 * sequentially from left to right, and the values of the last ⟨expression⟩ are returned. This
 * expression type is used to sequence side effects such as assignments or input and output. */
HandlerResult sf_begin(Lex* e, Cell* a) {
    /* Evaluate all but last expr*/
    const long long n_expressions = a->count;
    /* If there is just one expression, return it to eval */
    if (n_expressions == 1) {
        return (HandlerResult){ .action = ACTION_CONTINUE, .value = a->cell[0] };
    }
    /* Otherwise, eval all but the last */
    for (int i = 0; i < n_expressions-1; i++) {
        Cell *result = coz_eval(e, a->cell[i]);
        /* null return will segfault the error check */
        if (!result) { continue; }
        if (result->type == CELL_ERROR) {
            return (HandlerResult){ .action = ACTION_RETURN, .value = result };
        }
    }
    /* Send last expr back to eval */
    return (HandlerResult){ .action = ACTION_CONTINUE, .value = a->cell[n_expressions-1] };
}

/* (and ⟨test1⟩ ... )
 * The ⟨test⟩ expressions are evaluated from left to right, and if any expression evaluates to #f,
 * then #f is returned. Any remaining expressions are not evaluated. If all the expressions evaluate
 * to true values, the values of the last expression are returned. If there are no expressions, then
 * #t is returned. */
HandlerResult sf_and(Lex* e, Cell* a) {
    /* (and) -> #t */
    if (a->count == 0) {
        return (HandlerResult){ .action = ACTION_RETURN, .value = make_cell_boolean(1) };
    }
    /* (and <tail expression>) */
    if (a->count == 1) {
        return (HandlerResult){ .action = ACTION_CONTINUE, .value = a->cell[0] };
    }
    /* (and e1 e2 ...) */
    Cell *test_result = coz_eval(e, a->cell[0]);

    if (test_result && test_result->type == CELL_ERROR) {
        return (HandlerResult){ .action = ACTION_RETURN, .value = test_result };
    }
    /* Check for #f */
    if (test_result && test_result->type == CELL_BOOLEAN && test_result->boolean_v == 0) {
        return (HandlerResult){ .action = ACTION_RETURN, .value = make_cell_boolean(0) };
    }
    /* Truthy or NULL → build a new '(and e2 ...)' and tail-call. */
    Cell* rest_of_and = make_cell_sexpr();
    cell_add(rest_of_and, G_and_sym);
    for (int i = 1; i < a->count; i++) {
        cell_add(rest_of_and, a->cell[i]);
    }
    return (HandlerResult){ .action = ACTION_CONTINUE, .value = rest_of_and };
}

/* (or ⟨test1⟩ ... )
 * The ⟨test⟩ expressions are evaluated from left to right, and the value of the first expression
 * that evaluates to a true value is returned. Any remaining expressions are not evaluated. If all
 * expressions evaluate to #f or if there are no expressions, then #f is returned. */
HandlerResult sf_or(Lex* e, Cell* a) {
    /* (or) -> #f */
    if (a->count == 0) {
        return (HandlerResult){ .action = ACTION_RETURN, .value = make_cell_boolean(0) };
    }
    /* (or <tail expression>) */
    if (a->count == 1) {
        return (HandlerResult){ .action = ACTION_CONTINUE, .value = a->cell[0] };
    }
    /* (or e1 e2 ...) */
    Cell *test_result = coz_eval(e, a->cell[0]);
    if (test_result && test_result->type == CELL_BOOLEAN && test_result->boolean_v == 0) {
        /* It's #f. Continue the search by tail-calling with the rest of the form. */
        Cell* rest_of_or = make_cell_sexpr();
        cell_add(rest_of_or, G_or_sym);
        for (int i = 1; i < a->count; i++) {
            cell_add(rest_of_or, a->cell[i]);
        }
        return (HandlerResult){ .action = ACTION_CONTINUE, .value = rest_of_or };
    }
    /* It's a TRUTHY value (or NULL). We're done. Short-circuit and return this value. */
    return (HandlerResult){ .action = ACTION_RETURN, .value = test_result };
}
