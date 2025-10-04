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
#include "load_library.h"
#include <string.h>
#include <gc/gc.h>

#include "printer.h"


/* A helper to check if a symbol name is a reserved syntactic keyword */
int is_syntactic_keyword(const char* s) {
    const char* keywords[] = {
        "define", "quote", "lambda", "if", "when", "unless",
        "cond", "import", "set!", "let", "let*" ,"letrec",
        "begin", "do", "case", nullptr
    };

    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(s, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Convert a VAL_SEXPR to a proper VAL_PAIR linked-list */
Cell* sexpr_to_list(Cell* c) {
    /* If the item is not an S-expression, it's an atom. Return it */
    if (c->type != VAL_SEXPR) {
        return c;
    }

    /* It is an S-expression. Check for improper list syntax. */
    int dot_pos = -1;
    if (c->count > 1) {
        const Cell* dot_candidate = c->cell[c->count - 2];
        if (dot_candidate->type == VAL_SYM && strcmp(dot_candidate->sym, ".") == 0) {
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
            list_head = make_val_pair(element, list_head);
        }
        return list_head;
    }
    /* Handle Proper List */
    Cell* list_head = make_val_nil();
    const int len = c->count;

    for (int i = len - 1; i >= 0; i--) {
        /* Recursively call this function on each element to ensure
         * any nested S-expressions are also converted. */
        Cell* element = sexpr_to_list(c->cell[i]);

        /* Prepend the new element to the head of our list. */
        list_head = make_val_pair(element, list_head);
        list_head->len = len - i;
    }
    return list_head;
}

/* Evaluate a lambda call, and return the value */
Cell* apply_lambda(Cell* lambda, const Cell* args) {
    if (!lambda || lambda->type != VAL_PROC || lambda->is_builtin) {
        return make_val_err("Not a lambda", TYPE_ERR);
    }

    /* Create a new child environment */
    Lex* local_env = lex_new_child(lambda->env);

    /* Bind formals to arguments */
    if (lambda->formals->count != args->count) {
        return make_val_err("Lambda: wrong number of arguments", ARITY_ERR);
    }

    for (int i = 0; i < args->count; i++) {
        const Cell* sym = lambda->formals->cell[i];
        const Cell* val = args->cell[i];
        lex_put(local_env, sym, val);  /* sym should be VAL_SYM, val evaluated */
    }

    /* Evaluate body expressions in this environment */
    Cell* result = nullptr;
    for (int i = 0; i < lambda->body->count; i++) {
        result = coz_eval(local_env, cell_copy(lambda->body->cell[i]));
    }
    return result;
}

/* -----------------------------*
 *         Special forms        *
 * -----------------------------*/

/* */
Cell* sf_define(Lex* e, const Cell* a) {
    if (a->count < 2) {
        return make_val_err("define requires at least 2 arguments", ARITY_ERR);
    }
    const Cell* target = a->cell[0];

    /* Disallow rebinding of keywords */
    if (is_syntactic_keyword(target->sym)) {
        char err_buf[128];
        snprintf(err_buf, sizeof(err_buf),
                 "Syntax keyword '%s' cannot be used as a variable", target->sym);
        return make_val_err(err_buf, VALUE_ERR);
    }

    /* (define <symbol> <expr>) */
    if (target->type == VAL_SYM) {
        Cell* val = coz_eval(e, a->cell[1]);
        /* Bail out if error encountered during evaluation */
        if (val->type == VAL_ERR) {
            return val;
        }
        /* Grab the name for the un-sugared define lambda */
        if (val->type == VAL_PROC) {
            val->l_name = GC_strdup(target->sym);
        }
        lex_put(e, target, val);
        return val;
    }

    /* (define (<f-name> <args>) <body>) */
    if (target->type == VAL_SEXPR && target->count > 0 &&
        target->cell[0]->type == VAL_SYM) {

        /* first element is function name */
        const Cell* fname = target->cell[0];

        /* rest are formal args */
        Cell* formals = make_val_sexpr();
        for (int i = 1; i < target->count; i++) {
            if (target->cell[i]->type != VAL_SYM) {
                return make_val_err("lambda formals must be symbols", TYPE_ERR);
            }
            cell_add(formals, cell_copy(target->cell[i]));
        }

        /* build lambda with args + body */
        Cell* body = make_val_sexpr();
        for (int i = 1; i < a->count; i++) {
            cell_add(body, cell_copy(a->cell[i]));
        }

        Cell* lam = lex_make_named_lambda(fname->sym, formals, body, e);
        lex_put(e, fname, lam);
        return lam;
        }

    return make_val_err("invalid define syntax", SYNTAX_ERR);
}

/* (quote ⟨datum⟩) OR '⟨datum⟩
 * (quote ⟨datum⟩) evaluates to ⟨datum⟩. ⟨Datum⟩ can be any external representation of a Scheme
 * object. This notation is used to include literal constants in Scheme code. */
Cell* sf_quote(const Lex* e, Cell* a) {
    (void)e;
    if (a->count != 1) {
        return make_val_err("quote takes exactly one argument", ARITY_ERR);
    }
    /* Extract the S-expression that was quoted. */
    Cell* quoted_sexpr = cell_take(a, 0);

    /* Flag whether to do env lookup */
    for (int i = 0; i < quoted_sexpr->count; i++) {
        if (quoted_sexpr->cell[i]->type == VAL_SYM) {
            quoted_sexpr->cell[i]->quoted = true;
        }
    }
    /* Convert the VAL_SEXPR into a proper VAL_PAIR list. */
    Cell* result = sexpr_to_list(quoted_sexpr);
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
        return make_val_err("lambda requires formals and a body", SYNTAX_ERR);
    }

    const Cell* formals = cell_pop(a, 0);   /* first arg */
    const Cell* body    = cell_copy(a);       /* remaining args */

    /* formals should be a list of symbols */
    for (int i = 0; i < formals->count; i++) {
        if (formals->cell[i]->type != VAL_SYM) {
            return make_val_err("lambda formals must be symbols", TYPE_ERR);
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
    if (test->type == VAL_BOOL && test->b_val == 0) {
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
    if (test->type == VAL_BOOL && test->b_val == 0) {
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

    /* Check for literal#f */
    if (test->type == VAL_BOOL && test->b_val == 0) {
        Cell* result = nullptr;
        for (int i = 1; i < a->count; i++) {
            result = coz_eval(e, a->cell[i]);
        }
        return result;
    }
    return nullptr;
}

Cell* sf_cond(Lex* e, const Cell* a) {
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;

    Cell* result = nullptr;
    for (int i = 0; i < a->count; i++) {
        const Cell* clause = a->cell[i];
        const Cell* test = coz_eval(e, clause->cell[0]);
        if (test->type == VAL_PROC && strcmp(test->f_name, "else") == 0) {
            result = coz_eval(e, clause->cell[1]);
            break;
        }
        if (test->type != VAL_BOOL) {
            result = make_val_err("'cond' test must be a predicate", VALUE_ERR);
        }
        if (test->b_val == 1) {
            for (int j = 1; j < clause->count; j++) {
                result = coz_eval(e, clause->cell[j]);
            }
            break;
        }
    }
    return result;
}

/* dummy function */
Cell* sf_else(const Lex* e, const Cell* a) {
    (void)e;
    (void)a;
    return make_val_bool(1);
}

Cell* sf_import(Lex* e, const Cell* a) {
    Cell* import_set = make_val_sexpr();
    import_set->cell = GC_MALLOC(sizeof(Cell*) * a->count);
    /* Make a new sexpr which contains pairs of (library . name), */
    int i;
    for (i = 0; i < a->count; i++) {
        const char* lib = GC_strdup(a->cell[i]->cell[0]->sym);
        const char* name = GC_strdup(a->cell[i]->cell[1]->sym);
        import_set->cell[i] = make_val_pair(make_val_str(lib),
                     make_val_str(name));
    }
    import_set->count = i;

    Cell* result = nullptr;
    for (int j = 0; j < import_set->count; j++) {
        const char* library_type = import_set->cell[j]->car->str;
        const char* library_name = import_set->cell[j]->cdr->str;

        if (strcmp(library_type, "scheme") == 0) {
            /* Load the Library */
            result = load_scheme_library(library_name, e);
        // } else if (strcmp(library_type, "cozenage") == 0){
        //     result = load_scheme_library(library_name, e);
        } else {
            /* TODO: Handle User Libraries Here
             * For example, (import (my-libs utils)). */
            return make_val_err("import: user-defined libraries not yet supported", GEN_ERR);
        }
    }
    return result;
}

Cell* sf_let(Lex* e, Cell* a) {
    /* TODO: implement named let */
    const Cell* bindings = cell_pop(a, 0);
    if (bindings->type != VAL_SEXPR) {
        return make_val_err("Bindings must be a list", VALUE_ERR);
    }
    const Cell* body = a;

    /* separate formals and args from bindings */
    Cell* formals = make_val_sexpr();
    Cell* args = make_val_sexpr();
    for (int i = 0; i < bindings->count; i++) {
        const Cell* local_b = bindings->cell[i];
        if (local_b->type != VAL_SEXPR) {
            return make_val_err("Bindings must be a list", VALUE_ERR);
        }
        if (local_b->count != 2) {
            return make_val_err("bindings must contain exactly 2 items", VALUE_ERR);
        }
        if (local_b->cell[0]->type != VAL_SYM) {
            return make_val_err("first value in binding must be a symbol", VALUE_ERR);
        }
        cell_add(formals, local_b->cell[0]);
        cell_add(args, local_b->cell[1]);
    }

    /* Create a new child environment */
    Lex* local_env = lex_new_child(e);

    for (int i = 0; i < args->count; i++) {
        const Cell* sym = formals->cell[i];
        const Cell* val = coz_eval(e, args->cell[i]);
        lex_put(local_env, sym, val);
    }

    /* Evaluate body expressions in this environment */
    Cell* result = nullptr;
    for (int i = 0; i < body->count; i++) {
        result = coz_eval(local_env, body->cell[i]);
    }
    return result;
}

Cell* sf_let_star(Lex* e, Cell* a) {
    const Cell* bindings = cell_pop(a, 0);
    if (bindings->type != VAL_SEXPR) {
        return make_val_err("Bindings must be a list", VALUE_ERR);
    }
    const Cell* body = a;

    /* Start with the outer environment. */
    Lex* current_env = e;

    for (int i = 0; i < bindings->count; i++) {
        const Cell* local_b = bindings->cell[i];
        if (local_b->type != VAL_SEXPR) {
            return make_val_err("Bindings must be a list", VALUE_ERR);
        }
        if (local_b->count != 2) {
            return make_val_err("bindings must contain exactly 2 items", VALUE_ERR);
        }
        if (local_b->cell[0]->type != VAL_SYM) {
            return make_val_err("first value in binding must be a symbol", VALUE_ERR);
        }
        const Cell* formal = local_b->cell[0];
        Cell* arg = local_b->cell[1];

        /* Create the new environment for THIS binding.
         * The parent is the *previous* environment in the chain. */
        Lex* new_env = lex_new_child(current_env);

        /* Evaluate the argument expression in the *current* environment. */
        const Cell* val = coz_eval(current_env, arg);

        /* Put the new binding into the new environment. */
        lex_put(new_env, formal, val);

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
    if (variable->type != VAL_SYM) {
        return make_val_err("arg1 must be a symbol", TYPE_ERR);
    }
    /* Ensure the variable is already bound in the environment */
    Cell* result = lex_get(e, variable);
    if (result->type == VAL_ERR) {
        return result;
    }
    /* Now evaluate new expression */
    Cell* expr = a->cell[1];
    const Cell* val = coz_eval(e, expr);
    /* Rebind the variable with the new value */
    lex_put(e, variable, val);
    /* No meaningful return value */
    return nullptr;
}

Cell* sf_begin(Lex* e, Cell* a) {
    Cell* result = nullptr;
    for (int i = 0; i< a->count; i++) {
        result = coz_eval(e, a->cell[i]);
    }
    return result;
}
