/*
 * 'src/special_forms.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 - 2026 Darren Kirby <darren@dragonbyte.ca>
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
#include "repr.h"
#include "load_library.h"
#include "line_edit.h"

#include <string.h>
#include <gc/gc.h>


/* Helpers for iteration clarity. */
#define first  0
#define second 1
#define third  2
#define last(ptr) ((ptr)->count - 1)

/* import needs to know if we're in the REPL. */
extern int is_repl;

/* Disable 'foo may be made const' linter warnings. */
/* ReSharper disable twice CppParameterMayBeConstPtrOrRef */


/* A helper to check if a symbol name is a reserved syntactic keyword. */
int is_syntactic_keyword(const Cell* s)
{
    /* Primitive and derived syntax keyword symbols have SF_ID > 0. */
    if (s->type == CELL_SYMBOL && s->sf_id > 0) {
        return 1;
    }
    return 0;
}


/* This function binds formals to argument values in a local
 * environment. It is called when needed from coz_apply(). */
Lex* build_lambda_env(const Lex* env, Cell* formals, Cell* args)
{
    /* Create a new child environment. */
    Lex* local_env = new_child_env(env);

    /* Bind formals to arguments. */
    /* Fully variadic (lambda args ...) */
    if (formals->type == CELL_SYMBOL) {
        const Cell* arg_list = make_list_from_sexpr(args);
        lex_put_local(local_env, formals, arg_list);
        return local_env;
    }

    /* Standard or dotted-tail (lambda (a b) ...) or (lambda (a . r) ...). */
    const Cell* lf = make_list_from_sexpr(formals);
    int arg_idx = 0;

    /* Iterate through positional arguments. */
    while (lf->type == CELL_PAIR) {
        if (arg_idx >= args->count) {
            fprintf(stderr, "lambda: Arity error: wrong number of args for lambda call\n");
            return nullptr;
        }
        const Cell* sym = lf->car;
        const Cell* val = args->cell[arg_idx];
        lex_put_local(local_env, sym, val);

        lf = lf->cdr;
        arg_idx++;
    }

    /* Check what's at the end of the formal list. */
    if (lf->type == CELL_NIL) {
        if (arg_idx != args->count) {
            /* Too many arguments supplied for a non-variadic lambda. */
            fprintf(stderr, "lambda: Arity error: too many args for lambda call\n");
            return nullptr;
        }
    } else if (lf->type == CELL_SYMBOL) {
        /* Dotted-tail: this is the 'rest' parameter. */
        Cell* rest = make_cell_sexpr();
        for (int i = arg_idx; i < args->count; i++) {
            cell_add(rest, args->cell[i]);
        }
        const Cell* rest_list = make_list_from_sexpr(rest);
        lex_put_local(local_env, lf, rest_list);
    } else {
        fprintf(stderr, "lambda: malformed lambda call: bad args\n");
        return nullptr;
    }
    return local_env;
}


/* Searches the local environment chain and returns the specific frame
 * where 'sym' is bound. Returns NULL if not found in any local frame. */
static Ch_Env* lex_find_local_frame(const Lex* e, const char* sym)
{
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

/* These two functions are helpers that build the appropriate
 * return values (final result, or tail call) for code clarity
 * in the sf_* functions below. */

/* Return a tail-call object with an expression and an environment. */
static HandlerResult continue_with(Cell* expr, Lex* env) {
    return (HandlerResult){ .action = ACTION_CONTINUE, .value = expr, .env = env };
}

/* Return an evaluated final value. */
static HandlerResult return_val(Cell* val) {
    return (HandlerResult){ .action = ACTION_RETURN, .value = val, .env = nullptr };
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
 */
HandlerResult sf_define(Lex* e, Cell* a)
{
    if (a->count < 2) {
        Cell* err = make_cell_error(
            "define: define requires at least 2 arguments",
            ARITY_ERR);
        return return_val(err);
    }
    Cell* target = a->cell[0];

    /* Disallow rebinding of keywords. */
    if (is_syntactic_keyword(target)) {
        Cell* err = make_cell_error(
            fmt_err("define: syntax keyword '%s' cannot be used as a variable", target->sym),
            VALUE_ERR);
        return return_val(err);
    }

    /* (define <symbol> <expr>) */
    if (target->type == CELL_SYMBOL) {
        Cell* val = coz_eval(e, a->cell[1]);
        /* Bail out if error encountered during evaluation. */
        if (val->type == CELL_ERROR) {
            return return_val(val);
        }
        /* Grab the name for the un-sugared define lambda. */
        if (val->type == CELL_PROC) {
            val->lambda->l_name = target->sym;
        }
        lex_put_global(e, target, val);

        /* For lambdas, return the lambda for REPL pretty-print.
         * For variable bindings, return the bound symbol. */
        if (val->type == CELL_PROC) {
            return return_val(val);
        }
        return return_val(target);
    }

    /* (define (<f-name> <args>) <body>) */
    if (target->type == CELL_SEXPR && target->count > 0 &&
        target->cell[0]->type == CELL_SYMBOL) {

        /* First element is function name. */
        const Cell* fname = target->cell[0];

        /* Rest are formal args. */
        Cell* formals = make_cell_sexpr();
        for (int i = 1; i < target->count; i++) {
            if (target->cell[i]->type != CELL_SYMBOL) {
                Cell* err = make_cell_error(
                    "lambda: formals must be symbols",
                    TYPE_ERR);
                return return_val(err);
            }
            cell_add(formals, target->cell[i]);
        }

        /* Build lambda with args + body. */
        Cell* body = a->cell[1];
        Cell* lam = lex_make_named_lambda(fname->sym, formals, body, e);

        lex_put_global(e, fname, lam);
        return return_val(lam);
    }

    Cell* err = make_cell_error(
        "define: invalid define syntax",
        SYNTAX_ERR);
    return return_val(err);
}


/* (quote ⟨datum⟩) OR '⟨datum⟩
 * (quote ⟨datum⟩) evaluates to ⟨datum⟩. ⟨Datum⟩ can be any external representation of a Scheme
 * object. This notation is used to include literal constants in Scheme code. */
HandlerResult sf_quote(Lex* e, Cell* a)
{
    (void)e;
    if (a->count != 1) {
        Cell* err = make_cell_error(
            "quote: takes exactly one argument",
            ARITY_ERR);
        return return_val(err);
    }

    /* Extract the expression that was quoted. */
    Cell* qexpr = a->cell[0];

    Cell* result = make_list_from_sexpr(qexpr);
    return return_val(result);
}


/* (lambda ⟨formals⟩ ⟨body⟩)
 * A lambda expression evaluates to a procedure. The environment in effect when the
 * lambda expression is evaluated is remembered as part of the procedure. It is called the closing
 * environment. When the procedure is later called with some arguments, the closing environment is
 * extended by binding the variables in the formal parameter list to fresh locations. Then the
 * locations are filled with the arguments according to rules about to be given. The new environment
 * created by this process is referred to as the invocation environment. */
HandlerResult sf_lambda(Lex* e, Cell* a)
{
    if (a->count < 2) {
        Cell* err = make_cell_error(
            "lambda: requires formals and a body",
            SYNTAX_ERR);
        return return_val(err);
    }

    Cell* formals = a->cell[0];   /* first arg */
    Cell* body = a->cell[1];      /* remaining args */

    /* Formals should be a symbol or list of symbols. */
    if (formals->type != CELL_SYMBOL) {
        for (int i = 0; i < formals->count; i++) {
            if (formals->cell[i]->type != CELL_SYMBOL) {
                Cell* err = make_cell_error(
                    "lambda: formals must be symbols",
                    TYPE_ERR);
                return return_val(err);
            }
        }
    }

    /* Build the lambda cell. */
    Cell* lambda = lex_make_lambda(formals, body, e);
    return return_val(lambda);
}


/* (if ⟨test⟩ ⟨consequent⟩ ⟨alternate⟩)
 * An if expression is evaluated as follows: first, ⟨test⟩ is evaluated. If it yields a true value,
 * then ⟨consequent⟩ is evaluated and its values are returned. Otherwise, ⟨alternate⟩ is evaluated
 * and its values are returned. If no <alternate> is provided to evaluate, it returns null */
HandlerResult sf_if(Lex* e, Cell* a)
{
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3, "if");
    if (err) {
        return return_val(err);
    }

    Cell* test = coz_eval(e, a->cell[first]);
    if (test->type == CELL_ERROR) {
        return return_val(test);
    }

    /* Check if the result is TRUTHY */
    if (!(test->type == CELL_BOOLEAN && test->boolean_v == 0)) {
        /* Test was true, so evaluate the consequent as a tail call. */
        return continue_with(a->cell[second], e);
    }

    /* Test was false.
     * Check if an alternative exists before accessing it. */
    if (a->count == 3) {
        /* It exists, so evaluate it as a tail call. */
        return continue_with(a->cell[third], e);
    }

    /* No alternative was provided. Return an unspecified value. */
    return return_val(USP_Obj);
}


/* (import ⟨import-set⟩ ...)
 * An import declaration provides a way to import identifiers exported by a library. Each
 * ⟨import set⟩ names a set of bindings from a library and possibly specifies local names for the
 * imported bindings. */
/* TODO: implement 'only', 'except', 'prefix', and 'rename' */
HandlerResult sf_import(Lex* e, Cell* a)
{
    Cell* import_set = make_cell_sexpr();
    import_set->cell = GC_MALLOC(sizeof(Cell*) * a->count);
    /* Make a new sexpr which contains pairs of (library . name), */
    int i;
    for (i = 0; i < a->count; i++) {
        if (a->cell[i]->count < 2) {
            Cell* err = make_cell_error(
                "import: import-set must include 'library' and 'name' identifiers. (Did you forget 'base'?)",
                GEN_ERR);
            return (HandlerResult){ .action = ACTION_RETURN, .value = err };
        }
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

        if (strcmp(library_type, "base") == 0) {
            /* Load the Library */
            result = load_library(library_name, e);
        } else {
            /* TODO: Handle User Libraries Here
             * For example, (import (my-libs utils)). */
            Cell* err = make_cell_error(
                "import: user-defined libraries not yet supported",
                GEN_ERR);
            return (HandlerResult) { .action = ACTION_RETURN, .value = err };
        }
    }
    /* Re-generate the completion array if we're in the REPL. */
    if (is_repl) {
        populate_dynamic_completions(e);
    }
    return (HandlerResult) { .action = ACTION_RETURN, .value = result };
}


/* (let ⟨bindings⟩ ⟨body⟩) where ⟨Bindings⟩ has the form ((⟨variable1⟩ ⟨init1⟩) ...)
 * where each ⟨init⟩ is an expression, and ⟨body⟩ is a sequence of zero or more definitions followed
 * by a sequence of one or more expressions. It is an error for a ⟨variable⟩ to appear more than
 * once in the list of variables being bound.
 *
 * The ⟨init⟩s are evaluated in the current environment (in some unspecified order). The ⟨variable⟩s
 * are bound to fresh locations holding the results. The ⟨body⟩ is evaluated in the extended
 * environment, and the values of the last expression of ⟨body⟩ are returned. Each binding of a
 * ⟨variable⟩ has ⟨body⟩ as its region. */
HandlerResult sf_let(Lex* e, Cell* a) {
    if (a->count < 2) {
        return return_val(make_cell_error(
            "let: missing bindings or body",
            SYNTAX_ERR));
    }

    const Cell* bindings = a->cell[0];
    if (bindings->type != CELL_SEXPR) {
        return return_val(make_cell_error(
            "let: Bindings must be a list",
            VALUE_ERR));
    }

    /* Set up the child environment. */
    Lex* local_env = new_child_env(e);
    for (int i = 0; i < bindings->count; i++) {
        const Cell* b = bindings->cell[i];
        if (b->type != CELL_SEXPR) {
            Cell* err = make_cell_error(
                "let: Bindings must be a list",
                VALUE_ERR);
            return (HandlerResult) { .action = ACTION_RETURN, .value = err };
        }
        if (b->count != 2) {
            Cell* err = make_cell_error(
                "let: bindings must contain exactly 2 items",
                VALUE_ERR);
            return (HandlerResult) { .action = ACTION_RETURN, .value = err };
        }
        if (b->cell[0]->type != CELL_SYMBOL) {
            Cell* err = make_cell_error(
                "let: first value in binding must be a symbol",
                VALUE_ERR);
            return (HandlerResult) { .action = ACTION_RETURN, .value = err };
        }

        /* Evaluate the value in the parent environment. */
        Cell* val = coz_eval(e, b->cell[1]);
        if (val->type == CELL_ERROR) return return_val(val);

        lex_put_local(local_env, b->cell[0], val);
    }

    /* Handle the body (indices 1 to count-1). */
    const int body_count = a->count - 1;

    if (body_count == 0) return return_val(USP_Obj);

    /* Eval all but last body expression in local_env. */
    for (int i = 1; i < a->count - 1; i++) {
        Cell* res = coz_eval(local_env, a->cell[i]);
        if (res->type == CELL_ERROR) return return_val(res);
    }

    /* Tail call the last expression. */
    return continue_with(a->cell[a->count - 1], local_env);
}


/* (letrec ⟨bindings⟩ ⟨body⟩)
* ⟨Bindings⟩ has the form ((⟨variable1⟩ ⟨init1⟩) ...), and ⟨body⟩ is a sequence of zero or more definitions followed by
* one or more expressions. It is an error for a ⟨variable⟩ to appear more than once in the list of variables being
* bound.
*
* Semantics: The ⟨variable⟩s are bound to fresh locations holding unspecified values, the ⟨init⟩s are evaluated in the
* resulting environment (in some unspecified order), each ⟨variable⟩ is assigned to the result of the corresponding
* ⟨init⟩, the ⟨body⟩ is evaluated in the resulting environment, and the values of the last expression in ⟨body⟩ are
* returned. Each binding of a ⟨variable⟩ has the entire letrec expression as its region, making it possible to define
* mutually recursive procedures.
*
* One restriction on letrec is very important: if it is not possible to evaluate each ⟨init⟩ without assigning or
* referring to the value of any ⟨variable⟩, it is an error. The restriction is necessary because letrec is defined in
* terms of a procedure call where a lambda expression binds the ⟨variable⟩s to the values of the ⟨init⟩s. In the most
* common uses of letrec, all the ⟨init⟩s are lambda expressions and the restriction is satisfied automatically. */
HandlerResult sf_letrec(Lex* e, Cell* a)
{
    if (a->count < 1) return return_val(make_cell_error(
        "letrec: missing bindings",
        SYNTAX_ERR));

    const Cell* bindings = a->cell[0];
    if (bindings->type != CELL_SEXPR) {
        return return_val(make_cell_error(
            "letrec: Bindings must be a list",
            VALUE_ERR));
    }

    /* Create a new child environment. */
    Lex* local_env = new_child_env(e);

    /* Iterate and bind 'unspecified' placeholders. */
    for (int i = 0; i < bindings->count; i++) {
        const Cell* variable = bindings->cell[i]->cell[0];
        lex_put_local(local_env, variable, USP_Obj);
    }
    /* Iterate and bind init-expressions (lambdas) to variables (lambda names). */
    for (int i = 0; i < bindings->count; i++) {
        const Cell* variable = bindings->cell[i]->cell[0];
        Cell* local_bind = bindings->cell[i]->cell[1];
        Cell* init_exp = coz_eval(local_env, local_bind);

        if (init_exp->type == CELL_ERROR) return return_val(init_exp);
        lex_put_local(local_env, variable, init_exp);
    }

    const int body_count = a->count - 1;
    if (body_count <= 0) {
        return return_val(USP_Obj);
    }

    /* * Iterate over all but last body expression. */
    for (int i = 1; i < a->count - 1; i++) {
        Cell* result = coz_eval(local_env, a->cell[i]);
        if (result->type == CELL_ERROR) return return_val(result);
    }

    /* Tail call the last expression. */
    return continue_with(a->cell[a->count - 1], local_env);
}


/* (set! ⟨variable⟩ ⟨expression⟩)
 * ⟨Expression⟩ is evaluated, and the resulting value is stored in the location to which ⟨variable⟩
 * is bound. It is an error if ⟨variable⟩ is not bound either in some region enclosing the set!
 * expression or else globally. The result of the set! expression is unspecified. */
HandlerResult sf_set_bang(Lex* e, Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set!");
    if (err) return (HandlerResult){ .action = ACTION_RETURN, .value = err };

    const Cell* variable = a->cell[first];
    if (variable->type != CELL_SYMBOL) {
        err = make_cell_error("set!: arg1 must be a symbol", TYPE_ERR);
        return (HandlerResult) { .action = ACTION_RETURN, .value = err };
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
                 * feedback that the operation was successful (REPL-only). */
                if (is_repl) {
                    fprintf(stderr, "%s\n", cell_to_string(value_to_set, MODE_REPL));
                }
                return (HandlerResult) { .action = ACTION_RETURN, .value = USP_Obj };
            }
        }
    } else {
        /* The variable was not in any local frame. Check global.
         * Use ht_get to see if it *exists* before we set it. */
        if (ht_get(e->global, sym_to_set)) {
            /* It exists globally, so update it in the hash table. */
            ht_set(e->global, sym_to_set, value_to_set);
            if (is_repl) {
                fprintf(stderr, "%s\n", cell_to_string(value_to_set, MODE_REPL));
            }
            return (HandlerResult) { .action = ACTION_RETURN, .value = USP_Obj };
        }
    }

    /* The variable was not found anywhere. This is an error. */
    err = make_cell_error(fmt_err("set!: Unbound symbol: '%s'", sym_to_set), TYPE_ERR);
    return (HandlerResult) { .action = ACTION_RETURN, .value = err };
}


/* (begin ⟨expression1 ⟩ ⟨expression2 ⟩ ... )
 * This form of begin can be used as an ordinary expression. The ⟨expression⟩s are evaluated
 * sequentially from left to right, and the values of the last ⟨expression⟩ are returned. This
 * expression type is used to sequence side effects such as assignments or input and output. */
HandlerResult sf_begin(Lex* e, Cell* a)
{
    /* Evaluate all but last expr. */
    const long long n_expressions = a->count;
    /* If there is just one expression, return it to eval. */
    if (n_expressions == 1) {
        return continue_with(a->cell[0], e);
    }
    /* Otherwise, eval all but the last. */
    for (int i = 0; i < n_expressions-1; i++) {
        Cell *result = coz_eval(e, a->cell[i]);
        /* null return will segfault the error check. */
        if (!result) { continue; }
        if (result->type == CELL_ERROR) {
            return return_val(result);
        }
    }
    /* Send last expr back to eval. */
    return continue_with(a->cell[n_expressions-1], e);

}


/* (and ⟨test1⟩ ... )
 * The ⟨test⟩ expressions are evaluated from left to right, and if any expression evaluates to #f,
 * then #f is returned. Any remaining expressions are not evaluated. If all the expressions evaluate
 * to true values, the values of the last expression are returned. If there are no expressions, then
 * #t is returned. */
HandlerResult sf_and(Lex* e, Cell* a) {
    /* (and) -> #t */
    if (a->count == 0)
        return (HandlerResult){.action = ACTION_RETURN, .value = True_Obj};

    for (int i = 0; i < a->count; i++) {
        /* If it's the last element, tail-call it. */
        if (i == a->count - 1) {
            return continue_with(a->cell[i], e);
        }

        Cell* result = coz_eval(e, a->cell[i]);
        if (result->type == CELL_ERROR)
            return (HandlerResult){.action = ACTION_RETURN, .value = result};

        /* Short-circuit if False. */
        if (result->type == CELL_BOOLEAN && result->boolean_v == 0) {
            return (HandlerResult){.action = ACTION_RETURN, .value = False_Obj};
        }
    }
    /* This line is technically unreachable due to the tail-call above.*/
    return (HandlerResult){.action = ACTION_RETURN, .value = True_Obj};
}


/* (defmacro name ⟨formals⟩ ⟨body⟩
 * Defines ⟨name⟩ as a macro. ⟨Formals⟩ is a symbol or a list of symbols that
 * name the macro’s arguments. ⟨Body⟩ is an expression that is evaluated at
 * macro-expansion time with the macro arguments bound to the unevaluated
 * subforms of the macro call.
 *
 * The value produced by evaluating ⟨body⟩ must be a syntactic form, which is
 * substituted into the program in place of the macro invocation. Quasiquote,
 * unquote, and unquote-splicing are typically used to construct this form,
 * but are not required.
 *
 * Macros defined by defmacro are non-hygienic: identifiers introduced by the
 * macro expansion may capture or be captured by bindings at the macro use site.
 *
 * It is an error if ⟨name⟩ is not a symbol, or if ⟨formals⟩ is neither a symbol
 * nor a list of symbols.
 *
 *   ;; The nested-test macro
 *   (defmacro nested-test (x)
 *     `(list ,x (list ,x)))
 *
 *   ;; The kond macro (a simplified 'cond' implementation)
 *   (defmacro kond (test then else)
 *     `(if ,test
 *          ,then
 *          ,else))
 *
 *   (nested-test 5)
 *   ;; Output: (5 (5))
 *
 *   (kond (= 1 1) 'yes 'no)
 *   ;; Output: yes
 */
HandlerResult sf_defmacro(Lex* e, Cell* a) {
    if (a->count < 3) {
        Cell* err = make_cell_error(
            "defmacro: requires name, formals, and a body",
            SYNTAX_ERR);
        return return_val(err);
    }


    const Cell* name = a->cell[0]; /* name of the macro. */
    Cell* formals = a->cell[1];    /* first arg */
    Cell* body = a->cell[2];       /* remaining args */

    /* Formals should be a symbol or list of symbols. */
    if (formals->type != CELL_SYMBOL) {
        for (int i = 0; i < formals->count; i++) {
            if (formals->cell[i]->type != CELL_SYMBOL) {
                Cell* err = make_cell_error(
                    "defmacro: formals must be symbols",
                    TYPE_ERR);
                return return_val(err);
            }
        }
    }

    /* Build the lambda cell. */
    Cell* lambda = lex_make_defmacro(name->str, formals, body, e);
    lex_put_global(e, make_cell_symbol(name->str), lambda);
    return return_val(lambda);
}


/* (with-gc-stats ⟨expression1⟩)
 * A helper debug procedure that checks GC allocation/deallocation.
 * Implemented as a special form so the arg doesn't get evaluated before
 * the first collect/get_heap_size calls. */
HandlerResult sf_with_gc_stats(Lex* env, Cell* a) {
    if (a->count < 1) return return_val(make_cell_error(
        "Expected an expression to evaluate",
        SYNTAX_ERR));

    GC_gcollect();
    const size_t before = GC_get_heap_size();

    Cell* result = coz_eval(env, a->cell[0]);

    GC_gcollect();
    const size_t after = GC_get_heap_size();

    printf("\n--- GC Monitor ---\n");
    printf("Heap Before: %zu bytes\n", before);
    printf("Heap After:  %zu bytes\n", after);
    printf("Growth:      %zd bytes\n", (ssize_t)after - (ssize_t)before);
    printf("------------------\n");

    return return_val(result);
}
