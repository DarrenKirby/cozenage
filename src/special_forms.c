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
#include "load_library.h"
#include <string.h>
#include <gc/gc.h>


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

/* Evaluate a lambda call, and return the value */
Cell* apply_lambda(Cell* lambda, const Cell* args) {
    if (!lambda || lambda->type != CELL_PROC || lambda->is_builtin) {
        return make_cell_error("Not a lambda", TYPE_ERR);
    }

    /* Create a new child environment */
    Lex* local_env = new_child_env(lambda->env);

    /* Bind formals to arguments */
    if (lambda->formals->count != args->count) {
        return make_cell_error("Lambda: wrong number of arguments", ARITY_ERR);
    }

    for (int i = 0; i < args->count; i++) {
        const Cell* sym = lambda->formals->cell[i];
        const Cell* val = args->cell[i];
        lex_put_local(local_env, sym, val);  /* sym should be CELL_SYMBOL, val evaluated */
    }

    /* Evaluate body expressions in this environment */
    Cell* result = nullptr;
    for (int i = 0; i < lambda->body->count; i++) {
        result = coz_eval(local_env, lambda->body->cell[i]);
    }
    return result;
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
Cell* sf_define(Lex* e, const Cell* a) {
    if (a->count < 2) {
        return make_cell_error("define requires at least 2 arguments", ARITY_ERR);
    }
    const Cell* target = a->cell[0];

    /* Disallow rebinding of keywords */
    if (is_syntactic_keyword(target->sym)) {
        char err_buf[128];
        snprintf(err_buf, sizeof(err_buf),
                 "Syntax keyword '%s' cannot be used as a variable", target->sym);
        return make_cell_error(err_buf, VALUE_ERR);
    }

    /* (define <symbol> <expr>) */
    if (target->type == CELL_SYMBOL) {
        Cell* val = coz_eval(e, a->cell[1]);
        /* Bail out if error encountered during evaluation */
        if (val->type == CELL_ERROR) {
            return val;
        }
        /* Grab the name for the un-sugared define lambda */
        if (val->type == CELL_PROC) {
            val->l_name = GC_strdup(target->sym);
        }
        lex_put_global(e, target, val);
        return val;
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
                return make_cell_error("lambda formals must be symbols", TYPE_ERR);
            }
            cell_add(formals, cell_copy(target->cell[i]));
        }

        /* build lambda with args + body */
        Cell* body = make_cell_sexpr();
        for (int i = 1; i < a->count; i++) {
            cell_add(body, cell_copy(a->cell[i]));
        }

        Cell* lam = lex_make_named_lambda(fname->sym, formals, body, e);
        lex_put_global(e, fname, lam);
        return lam;
        }

    return make_cell_error("invalid define syntax", SYNTAX_ERR);
}

/* (quote ⟨datum⟩) OR '⟨datum⟩
 * (quote ⟨datum⟩) evaluates to ⟨datum⟩. ⟨Datum⟩ can be any external representation of a Scheme
 * object. This notation is used to include literal constants in Scheme code. */
Cell* sf_quote(const Lex* e, Cell* a) {
    (void)e;
    if (a->count != 1) {
        return make_cell_error("quote takes exactly one argument", ARITY_ERR);
    }
    /* Extract the S-expression that was quoted. */
    Cell* qexpr = cell_take(a, 0);

    /* Flag whether to do env lookup */
    if (qexpr->count == 1 && qexpr->type == CELL_SYMBOL) {
        qexpr->quoted = true;
    }
    if (qexpr->count > 1) {
        for (int i = 0; i < qexpr->count; i++) {
            if (qexpr->cell[i]->type == CELL_SYMBOL) {
                qexpr->cell[i]->quoted = true;
            }
        }
    }
    /* Convert the CELL_SEXPR into a proper CELL_PAIR list. */
    Cell* result = sexpr_to_list(qexpr);
    return result;
}

/* (lambda ⟨formals⟩ ⟨body⟩)
 * A lambda expression evaluates to a procedure. The environment in effect when the
 * lambda expression is evaluated is remembered as part of the procedure; it is called the closing
 * environment. When the procedure is later called with some arguments, the closing environment is
 * extended by binding the variables in the formal parameter list to fresh locations, and the
 * locations are filled with the arguments according to rules about to be given. The new environment
 * created by this process is referred to as the invocation environment. */
Cell* sf_lambda(Lex* e, Cell* a) {
    if (a->count < 2) {
        return make_cell_error("lambda requires formals and a body", SYNTAX_ERR);
    }

    const Cell* formals = cell_pop(a, 0);   /* first arg */
    const Cell* body    = cell_copy(a);       /* remaining args */

    /* formals should be a list of symbols */
    for (int i = 0; i < formals->count; i++) {
        if (formals->cell[i]->type != CELL_SYMBOL) {
            return make_cell_error("lambda formals must be symbols", TYPE_ERR);
        }
    }

    /* Build the lambda cell */
    Cell* lambda = lex_make_lambda(formals, body, e);
    return lambda;
}

/* (if ⟨test⟩ ⟨consequent⟩ ⟨alternate⟩)
 * An if expression is evaluated as follows: first, ⟨test⟩ is evaluated. If it yields a true value,
 * then ⟨consequent⟩ is evaluated and its values are returned. Otherwise, ⟨alternate⟩ is evaluated
 * and its values are returned. If no <alternate> is provided to evaluate, it returns null */
Cell* sf_if(Lex* e, const Cell* a) {
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3);
    if (err) return err;

    const Cell* test = coz_eval(e, a->cell[0]);

    /* Note: this 'just works' with no <alternative>, as coz_eval() returns null with no args */
    if (test->type == CELL_BOOLEAN && test->boolean_v == 0) {
        return coz_eval(e, a->cell[2]);
    }
    return coz_eval(e, a->cell[1]);
}

/* (when ⟨test⟩ ⟨expression1⟩ ⟨expression2⟩ ... )
 * The test is evaluated, and if it evaluates to a true value, the expressions are evaluated in
 * order. The result of the when expression is unspecified, per R7RS, but Cozenage returns the value
 * of the last expression evaluated, or null if the test evaluates to #f. */
Cell* sf_when(Lex* e, const Cell* a) {
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;

    const Cell* test = coz_eval(e, a->cell[0]);

    /* Check for literal #f */
    if (test->type == CELL_BOOLEAN && test->boolean_v == 0) {
        return nullptr;
    }
    Cell* result = nullptr;
    for (int i = 1; i < a->count; i++) {
        result = coz_eval(e, a->cell[i]);
    }
    return result;
}

/*  (unless ⟨test⟩ ⟨expression1⟩ ⟨expression2⟩ ... )
 *  The test is evaluated, and if it evaluates to #f, the expressions are evaluated in order. The
 *  result of the unless expression is unspecified, per R7RS, but Cozenage returns the value of the
 *  last expression evaluated, or null if the test is truthy. */
Cell* sf_unless(Lex* e, const Cell* a) {
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;

    const Cell* test = coz_eval(e, a->cell[0]);

    /* Check for literal #f */
    if (test->type == CELL_BOOLEAN && test->boolean_v == 0) {
        Cell* result = nullptr;
        for (int i = 1; i < a->count; i++) {
            result = coz_eval(e, a->cell[i]);
        }
        return result;
    }
    return nullptr;
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
Cell* sf_cond(Lex* e, const Cell* a) {
    if (a->count == 0) {
        return make_cell_error("ill-formed cond expression", VALUE_ERR);
    }

    Cell* result = nullptr;
    for (int i = 0; i < a->count; i++) {
        const Cell* clause = a->cell[i];
        /* Check for 'else' clause and if found evaluate any expressions*/
        if (clause->cell[0]->type == CELL_SYMBOL && strcmp(clause->cell[0]->sym, "else") == 0) {
            /* else clause must be last */
            if (i != a->count-1) {
                return make_cell_error("'else' clause must be last in the cond expression",
                                     SYNTAX_ERR);
            }
            for (int j = 1; j < clause->count; j++) {
                result = coz_eval(e, clause->cell[j]);
            }
            break;
        }
        /* Not an else, so evaluate the test */
        Cell* test = coz_eval(e, clause->cell[0]);
        /* Move along if current test is #f */
        if (test->type == CELL_BOOLEAN && test->boolean_v == 0) {
            continue;
        }
        /* Test is truthy - first see if there is an expression */
        if (clause->count == 1) {
            /* No expression, return the test result */
            return test;
        }
        /* Check for cond '=>' form */
        if (clause->cell[1]->type == CELL_SYMBOL && strcmp(clause->cell[1]->sym, "=>") == 0) {
            if (clause->count <= 2) {
                return make_cell_error("cond '=>' form must have an expression", SYNTAX_ERR);
            }
            /* '=>' form can only have one expression after the test */
            if (clause->count > 3) {
                return make_cell_error("cond '=>' form can only have 1 expression after the test",
                                     SYNTAX_ERR);
            }
            const Cell* proc = coz_eval(e, clause->cell[2]);
            /* Expression must evaluate to a procedure */
            if (proc->type != CELL_PROC) {
                return make_cell_error("expression after '=>' must evaluate to a procedure",
                                     SYNTAX_ERR);
            }
            return coz_eval(e, make_sexpr_len2(proc, test));
        }
        /* Expressions present. Evaluate them, and break to return the last */
        for (int j = 1; j < clause->count; j++) {
            result = coz_eval(e, clause->cell[j]);
        }
        break;
    }
    return result;
}

/* (import ⟨import-set⟩ ...)
 * An import declaration provides a way to import identifiers exported by a library. Each
 * ⟨import set⟩ names a set of bindings from a library and possibly specifies local names for the
 * imported bindings. */
/* TODO: implement 'only', 'except', 'prefix', and 'rename' */
Cell* sf_import(const Lex* e, const Cell* a) {
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
            return make_cell_error("import: user-defined libraries not yet supported", GEN_ERR);
        }
    }
    return result;
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
Cell* sf_let(Lex* e, Cell* a) {
    /* TODO: implement named let */
    const Cell* bindings = cell_pop(a, 0);
    if (bindings->type != CELL_SEXPR) {
        return make_cell_error("Bindings must be a list", VALUE_ERR);
    }
    const Cell* body = a;

    /* separate variables and values from bindings */
    /* TODO: raise error if not all variables are unique*/
    Cell* vars = make_cell_sexpr();
    Cell* vals = make_cell_sexpr();
    for (int i = 0; i < bindings->count; i++) {
        const Cell* local_b = bindings->cell[i];
        if (local_b->type != CELL_SEXPR) {
            return make_cell_error("Bindings must be a list", VALUE_ERR);
        }
        if (local_b->count != 2) {
            return make_cell_error("bindings must contain exactly 2 items", VALUE_ERR);
        }
        if (local_b->cell[0]->type != CELL_SYMBOL) {
            return make_cell_error("first value in binding must be a symbol", VALUE_ERR);
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
    return result;
}

/* (let* ⟨bindings⟩ ⟨body⟩) where ⟨Bindings⟩ has the form ((⟨variable1⟩ ⟨init1⟩) ...)
 * where each ⟨init⟩ is an expression, and ⟨body⟩ is a sequence of zero or more definitions followed
 * by a sequence of one or more expressions.
 *
 * The let* binding construct is similar to let, but the bindings are performed sequentially from
 * left to right, and the region of a binding indicated by (⟨variable⟩ ⟨init⟩) is that part of the
 * let* expression to the right of the binding. Thus, the second binding is done in an environment
 * in which the first binding is visible, and so on. The ⟨variable⟩s need not be distinct. */
Cell* sf_let_star(Lex* e, Cell* a) {
    const Cell* bindings = cell_pop(a, 0);
    if (bindings->type != CELL_SEXPR) {
        return make_cell_error("Bindings must be a list", VALUE_ERR);
    }
    const Cell* body = a;

    /* Start with the outer environment. */
    Lex* current_env = e;

    for (int i = 0; i < bindings->count; i++) {
        const Cell* local_b = bindings->cell[i];
        if (local_b->type != CELL_SEXPR) {
            return make_cell_error("Bindings must be a list", VALUE_ERR);
        }
        if (local_b->count != 2) {
            return make_cell_error("bindings must contain exactly 2 items", VALUE_ERR);
        }
        if (local_b->cell[0]->type != CELL_SYMBOL) {
            return make_cell_error("first value in binding must be a symbol", VALUE_ERR);
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
    return result;
}

/* (set! ⟨variable⟩ ⟨expression⟩)
 * ⟨Expression⟩ is evaluated, and the resulting value is stored in the location to which ⟨variable⟩
 * is bound. It is an error if ⟨variable⟩ is not bound either in some region enclosing the set!
 * expression or else globally. The result of the set! expression is unspecified. */
Cell* sf_set_bang(Lex* e, const Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    const Cell* variable = a->cell[0];
    if (variable->type != CELL_SYMBOL) {
        return make_cell_error("arg1 must be a symbol", TYPE_ERR);
    }
    /* Ensure the variable is already bound in the environment */
    Cell* result = lex_get(e, variable);
    if (result->type == CELL_ERROR) {
        return result;
    }
    /* Now evaluate new expression */
    Cell* expr = a->cell[1];
    const Cell* val = coz_eval(e, expr);
    /* Re-bind the variable with the new value */
    lex_put_local(e, variable, val);
    /* No meaningful return value */
    return nullptr;
}

/* (begin ⟨expression1 ⟩ ⟨expression2 ⟩ ... )
 * This form of begin can be used as an ordinary expression. The ⟨expression⟩s are evaluated
 * sequentially from left to right, and the values of the last ⟨expression⟩ are returned. This
 * expression type is used to sequence side effects such as assignments or input and output. */
Cell* sf_begin(Lex* e, const Cell* a) {
    Cell* result = nullptr;
    for (int i = 0; i< a->count; i++) {
        result = coz_eval(e, a->cell[i]);
    }
    return result;
}

/* (and ⟨test1⟩ ... )
 * The ⟨test⟩ expressions are evaluated from left to right, and if any expression evaluates to #f,
 * then #f is returned. Any remaining expressions are not evaluated. If all the expressions evaluate
 * to true values, the values of the last expression are returned. If there are no expressions, then
 * #t is returned. */
Cell* sf_and(Lex* e, const Cell* a) {
    (void)e;
    if (a->count == 0) {
        return make_cell_boolean(1);
    }
    Cell* test_result = nullptr;
    for (int i = 0; i < a->count; i++) {
        test_result = coz_eval(e, a->cell[i]);
        if (test_result->type == CELL_BOOLEAN && test_result->boolean_v == 0) {
            /* first #f encountered → return #f */
            return make_cell_boolean(0);
        }
    }
    /* all truthy → return copy of last element */
    return test_result;
}

/* (or ⟨test1⟩ ... )
 * The ⟨test⟩ expressions are evaluated from left to right, and the value of the first expression
 * that evaluates to a true value is returned. Any remaining expressions are not evaluated. If all
 * expressions evaluate to #f or if there are no expressions, then #f is returned. */
Cell* sf_or(Lex* e, const Cell* a) {
    (void)e;
    if (a->count == 0) {
        return make_cell_boolean(0);
    }
    for (int i = 0; i < a->count; i++) {
        Cell *test_result = coz_eval(e, a->cell[i]);
        if (!(test_result->type == CELL_BOOLEAN && test_result->boolean_v == 0)) {
            /* first truthy value → return it */
            return test_result;
        }
    }
    /* all false → return #f */
    return make_cell_boolean(0);
}
