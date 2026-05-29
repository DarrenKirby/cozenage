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

#include <math.h>
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


/* Just takes body statements and stuffs them in a 'begin' expression. */
Cell* sequence_sf_body(const Cell* body)
{
    Cell* seq = make_cell_sexpr();
    cell_add(seq, G_begin_sym);

    /* Iterate over the list of expressions in the body
     * and add each complete expression to our new 'begin' block. */
    for (int i = 0; i < body->count; i++) {
        cell_add(seq, body->cell[i]);
    }
    return seq;
}


/* Helper to extract procedure args from s-expr. */
Cell* get_args_from_sexpr(const Cell* v)
{
    Cell* args = make_cell_sexpr();
    for (int i = 1; i < v->count; i++) {
        cell_add(args, v->cell[i]);
    }
    return args;
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
    Cell* target = a->cell[first];

    /* Disallow rebinding of keywords. */
    if (is_syntactic_keyword(target)) {
        Cell* err = make_cell_error(
            fmt_err("define: syntax keyword '%s' cannot be used as a variable", target->sym),
            VALUE_ERR);
        return return_val(err);
    }

    /* (define <symbol> <expr>) */
    if (target->type == CELL_SYMBOL) {
        Cell* val = coz_eval(e, a->cell[second]);
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
        const Cell* fname = target->cell[first];

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
        Cell* body = a->cell[second];
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
    Cell* qexpr = a->cell[first];

    return return_val(make_list_from_sexpr(qexpr));
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

    Cell* formals = a->cell[first];   /* first arg */
    Cell* body = a->cell[second];     /* remaining args */

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
    if (test != False_Obj) {
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
HandlerResult sf_cond(Lex* e, Cell* a)
{
    if (a->count == 0) {
        Cell* err = make_cell_error(
            "cond: ill-formed cond expression",
            VALUE_ERR);
        return return_val(err);
    }

    for (int i = 0; i < a->count; i++) {
        const Cell* clause = a->cell[i];

        /* Clause must be a list. */
        if (clause->type != CELL_SEXPR || clause->count == 0) {
            Cell* err = make_cell_error(
                "cond: clause must be a non-empty list",
                SYNTAX_ERR);
            return return_val(err);
        }

        /* Check for 'else' clause and if found evaluate any expressions. */
        if (clause->cell[first]->type == CELL_SYMBOL && clause->cell[first] == G_else_sym) {
            /* else clause must be last */
            if (i != last(a)) {
                Cell* err = make_cell_error(
                    "cond: else clause must be last in the cond expression",
                    SYNTAX_ERR);
                return return_val(err);
            }
            /* eval the first n-1 expressions. */
            for (int j = 1; j < last(clause); j++) {
                Cell* exp = coz_eval(e, clause->cell[j]);
                if (exp && exp->type == CELL_ERROR) {
                    return return_val(exp);
                }
            }
            /* return the tail call. */
            return continue_with(clause->cell[last(clause)], e);
        }

        /* Not an else, so evaluate the test. */
        Cell* test = coz_eval(e, clause->cell[first]);

        /* Propagate any errors. */
        if (test->type == CELL_ERROR) return return_val(test);

        /* Move along if current test is #f. */
        if (test == False_Obj) continue;

        /* Test is truthy - first see if there is an expression. */
        if (clause->count == 1) {
            /* No expression, return the test result. */
            return return_val(test);
        }

        /* Check for cond '=>' form. */
        if (clause->cell[1] == G_arrow_sym) {
            if (clause->count <= 2) {
                Cell* err = make_cell_error(
                    "cond: '=>' form must have an expression",
                    SYNTAX_ERR);
                return return_val(err);
            }

            /* '=>' form can only have one expression after the test. */
            if (clause->count > 3) {
                Cell* err = make_cell_error(
                    "cond: '=>' form can only have 1 expression after the test",
                    SYNTAX_ERR);
                return return_val(err);
            }
            const Cell* proc = coz_eval(e, clause->cell[2]);
            /* Expression must evaluate to a procedure. */
            if (proc->type != CELL_PROC) {
                Cell* err = make_cell_error(
                    "cond: expression after '=>' must evaluate to a procedure",
                    SYNTAX_ERR);
                return return_val(err);
            }
            /* A tail call. */
            Cell* tmp = make_sexpr_len2(proc, test);
            return continue_with(tmp, e);
        }
        /* Expressions present. eval all but the last. */
        for (int j = 1; j < last(clause); j++) {
            Cell* exp = coz_eval(e, clause->cell[j]);
            if (exp && exp->type == CELL_ERROR) {
                return return_val(exp);
            }
        }
        /* Return the last expression itself for the tail call. */
        return continue_with(clause->cell[last(clause)], e);
    }
    /* All tests #f with no else is unspecified, so just return unspecified. */
    return return_val(USP_Obj);
}



/* (import ⟨import-set⟩ ...)
 * An import declaration provides a way to import identifiers exported by a library. Each
 * ⟨import set⟩ names a set of bindings from a library and possibly specifies local names for the
 * imported bindings. */
/* TODO: implement 'only', 'except', 'prefix', and 'rename' */
HandlerResult sf_import(Lex* e, Cell* a)
{
    for (int i = 0; i < a->count; i++) {
        Cell* i_set = a->cell[i];

        if (i_set->type != CELL_SEXPR || i_set->count < 2) {
            Cell* err = make_cell_error("import: invalid import set", SYNTAX_ERR);
            return (HandlerResult){ .action = ACTION_RETURN, .value = err };
        }

        ImportSpec spec = {
            .mode         = IMPORT_ALL,
            .filter_names = nullptr,
            .filter_count = 0,
            .renames      = nullptr,
            .rename_count = 0,
            .prefix       = "",
        };

        Cell* libname_cell; /* Will point to the (lib name) s-expression. */

        /* Distinguish simple (lib name) from (modifier (lib name) ...). */
        if (i_set->count == 2
            && i_set->cell[0]->type == CELL_SYMBOL
            && i_set->cell[1]->type == CELL_SYMBOL) {
            /* Simple unmodified import set. */
            libname_cell = i_set;
        } else {
            /* Modified import set: first element is the modifier name,
             * second element must be an inner (lib name) s-expression. */
            if (i_set->cell[1]->type != CELL_SEXPR || i_set->cell[1]->count != 2) {
                Cell* err = make_cell_error(
                    "import: modifier requires a library name (lib name) as second element",
                    SYNTAX_ERR);
                return (HandlerResult){ .action = ACTION_RETURN, .value = err };
            }
            libname_cell = i_set->cell[1];
            const char* mod = i_set->cell[0]->sym;

            if (strcmp(mod, "only") == 0) {
                spec.mode         = IMPORT_ONLY;
                spec.filter_count = i_set->count - 2;
                spec.filter_names = GC_malloc(spec.filter_count * sizeof(char*));
                for (int j = 0; j < spec.filter_count; j++)
                    spec.filter_names[j] = i_set->cell[j + 2]->sym;

            } else if (strcmp(mod, "except") == 0) {
                spec.mode         = IMPORT_EXCEPT;
                spec.filter_count = i_set->count - 2;
                spec.filter_names = GC_malloc(spec.filter_count * sizeof(char*));
                for (int j = 0; j < spec.filter_count; j++)
                    spec.filter_names[j] = i_set->cell[j + 2]->sym;

            } else if (strcmp(mod, "prefix") == 0) {
                if (i_set->count != 3) {
                    Cell* err = make_cell_error(
                        "import: 'prefix' requires exactly one argument", SYNTAX_ERR);
                    return (HandlerResult){ .action = ACTION_RETURN, .value = err };
                }
                spec.prefix = i_set->cell[2]->str;  /* string cell */

            } else if (strcmp(mod, "rename") == 0) {
                spec.rename_count = i_set->count - 2;
                spec.renames      = GC_malloc(spec.rename_count * sizeof(CznRename));
                for (int j = 0; j < spec.rename_count; j++) {
                    Cell* pair = i_set->cell[j + 2];
                    if (pair->type != CELL_SEXPR || pair->count != 2) {
                        Cell* err = make_cell_error(
                            "import: 'rename' expects (old-name new-name) pairs",
                            SYNTAX_ERR);
                        return (HandlerResult){ .action = ACTION_RETURN, .value = err };
                    }
                    spec.renames[j].from = pair->cell[0]->sym;
                    spec.renames[j].to   = pair->cell[1]->sym;
                }
            } else {
                Cell* err = make_cell_error("import: unknown import modifier", SYNTAX_ERR);
                return (HandlerResult){ .action = ACTION_RETURN, .value = err };
            }
        }

        /* Extract library identifier and library name from libname_cell. */
        const char* lib  = libname_cell->cell[0]->sym;
        const char* name = libname_cell->cell[1]->sym;

        if (strcmp(lib, "base") != 0) {
            Cell* err = make_cell_error(
                "import: user-defined libraries not yet supported", GEN_ERR);
            return (HandlerResult){ .action = ACTION_RETURN, .value = err };
        }

        if (!internal_cozenage_load_lib(name, e, &spec)) {
            Cell* err = make_cell_error("import: failed to load library", GEN_ERR);
            return (HandlerResult){ .action = ACTION_RETURN, .value = err };
        }
    }

    if (is_repl) populate_dynamic_completions(e);
    return (HandlerResult){ .action = ACTION_RETURN, .value = True_Obj };
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
            return return_val(err);
        }
        if (b->count != 2) {
            Cell* err = make_cell_error(
                "let: bindings must contain exactly 2 items",
                VALUE_ERR);
            return return_val(err);
        }
        if (b->cell[0]->type != CELL_SYMBOL) {
            Cell* err = make_cell_error(
                "let: first value in binding must be a symbol",
                VALUE_ERR);
            return return_val(err);
        }

        /* FIXME: raise explicit error if symbols/variables are not unique */

        /* Evaluate the value in the parent environment. */
        Cell* val = coz_eval(e, b->cell[1]);
        if (val->type == CELL_ERROR) return return_val(val);

        lex_put_local(local_env, b->cell[0], val);
    }

    /* Handle the body (indices 1 to count-1). */
    const int body_count = a->count - 1;

    if (body_count == 0) return return_val(USP_Obj);

    /* Tail call the body expression. If it contains multiple expressions,
     * the transformer will have sequenced it in a (begin ...) */
    return continue_with(a->cell[last(a)], local_env);
}


/* (let* ⟨bindings⟩ ⟨body⟩) where ⟨Bindings⟩ has the form ((⟨variable1⟩ ⟨init1⟩) ...)
 * where each ⟨init⟩ is an expression, and ⟨body⟩ is a sequence of zero or more definitions followed
 * by a sequence of one or more expressions.
 *
 * The let* binding construct is similar to let, but the bindings are performed sequentially from
 * left to right. Also, the region of a binding indicated by (⟨variable⟩ ⟨init⟩) is that part of the
 * let* expression to the right of the binding. Thus, the second binding is done in an environment
 * in which the first binding is visible, and so on. The ⟨variable⟩s need not be distinct. */
HandlerResult sf_let_star(Lex* e, Cell* a)
{
    const Cell* bindings = a->cell[0];
    if (bindings->type != CELL_SEXPR) {
        Cell* err = make_cell_error(
            "let*: Bindings must be a list",
            VALUE_ERR);
        return return_val(err);
    }

    /* Start with the outer environment. */
    Lex* current_env = e;

    for (int i = 0; i < bindings->count; i++) {
        const Cell* local_b = bindings->cell[i];
        if (local_b->type != CELL_SEXPR) {
            Cell* err = make_cell_error(
                "let*: Bindings must be a list",
                VALUE_ERR);
            return return_val(err);
        }
        if (local_b->count != 2) {
            Cell* err = make_cell_error(
                "let*: bindings must contain exactly 2 items",
                VALUE_ERR);
            return return_val(err);
        }
        if (local_b->cell[0]->type != CELL_SYMBOL) {
            Cell* err = make_cell_error(
                "let*: first value in binding must be a symbol",
                VALUE_ERR);
            return return_val(err);
        }
        const Cell* formal = local_b->cell[0];
        Cell* arg = local_b->cell[1];

        /* Create the new environment for THIS binding.
         * The parent is the *previous* environment in the chain. */
        Lex* new_env = new_child_env(current_env);

        /* Evaluate the argument expression in the *current* environment. */
        Cell* val = coz_eval(current_env, arg);
        if (val->type == CELL_ERROR) return return_val(val);

        /* Put the new binding into the new environment. */
        lex_put_local(new_env, formal, val);

        /* Update current_env to point to the new environment. */
        current_env = new_env;
    }

    /* Tail call the body expression. If it contains multiple expressions,
     * the transformer will have sequenced it in a (begin ...) */
    return continue_with(a->cell[last(a)], current_env);
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

    /* Evaluate all init-expressions first (all vars still USP_Obj). */
    Cell** init_vals = GC_MALLOC(sizeof(Cell*) * bindings->count);
    for (int i = 0; i < bindings->count; i++) {
        Cell* local_bind = bindings->cell[i]->cell[1];
        init_vals[i] = coz_eval(local_env, local_bind);
        if (init_vals[i]->type == CELL_ERROR) {
            return return_val(init_vals[i]);
        }
    }

    /* Now bind them all. */
    for (int i = 0; i < bindings->count; i++) {
        lex_put_local(local_env, bindings->cell[i]->cell[0], init_vals[i]);
    }

    const int body_count = a->count - 1;
    if (body_count <= 0) {
        return return_val(USP_Obj);
    }

    /* Tail call the body expression. If it contains multiple expressions,
     * the transformer will have sequenced it in a (begin ...) */
    return continue_with(a->cell[last(a)], local_env);
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
HandlerResult sf_letrec_star(Lex* e, Cell* a)
{
    if (a->count < 1) return return_val(make_cell_error(
        "letrec*: missing bindings",
        SYNTAX_ERR));

    const Cell* bindings = a->cell[0];
    if (bindings->type != CELL_SEXPR) {
        return return_val(make_cell_error(
            "letrec*: Bindings must be a list",
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

    /* Tail call the body expression. If it contains multiple expressions,
     * the transformer will have sequenced it in a (begin ...) */
    return continue_with(a->cell[last(a)], local_env);
}


/* (set! ⟨variable⟩ ⟨expression⟩)
 * ⟨Expression⟩ is evaluated, and the resulting value is stored in the location to which ⟨variable⟩
 * is bound. It is an error if ⟨variable⟩ is not bound either in some region enclosing the set!
 * expression or else globally. The result of the set! expression is unspecified. */
HandlerResult sf_set_bang(Lex* e, Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 2, "set!");
    if (err) return return_val(err);

    const Cell* variable = a->cell[first];
    if (variable->type != CELL_SYMBOL) {
        err = make_cell_error(
            "set!: arg1 must be a symbol",
            TYPE_ERR);
        return return_val(err);
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
                    fprintf(stdout, "%s\n", cell_to_string(value_to_set, MODE_REPL));
                }
                return return_val(USP_Obj);
            }
        }
    } else {
        /* The variable was not in any local frame. Check global.
         * Use ht_get to see if it *exists* before we set it. */
        if (ht_get(e->global, sym_to_set)) {
            /* It exists globally, so update it in the hash table. */
            ht_set(e->global, sym_to_set, value_to_set);
            if (is_repl) {
                fprintf(stdout, "%s\n", cell_to_string(value_to_set, MODE_REPL));
            }
            return return_val(USP_Obj);
        }
    }

    /* The variable was not found anywhere. This is an error. */
    err = make_cell_error(
        fmt_err("set!: Unbound symbol: '%s'", sym_to_set),
        TYPE_ERR);
    return return_val(err);
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
        return return_val(True_Obj);

    for (int i = 0; i < a->count; i++) {
        /* If it's the last element, tail-call it. */
        if (i == a->count - 1) {
            return continue_with(a->cell[i], e);
        }

        Cell* result = coz_eval(e, a->cell[i]);
        if (result->type == CELL_ERROR)
            return return_val(result);

        /* Short-circuit if False. */
        if (result == False_Obj) {
            return return_val(False_Obj);
        }
    }
    /* This line is technically unreachable due to the tail-call above.*/
    return return_val(True_Obj);
}


/* (or ⟨test1⟩ ... )
 * The ⟨test⟩ expressions are evaluated from left to right, and the value of the first expression
 * that evaluates to a true value is returned. Any remaining expressions are not evaluated. If all
 * expressions evaluate to #f or if there are no expressions, then #f is returned. */
HandlerResult sf_or(Lex* e, Cell* a)
{
    /* (or) -> #f. */
    if (a->count == 0) {
        return return_val(False_Obj);
    }
    /* (or <tail expression>). */
    if (a->count == 1) {
        return continue_with(a->cell[0], e);
    }
    /* (or e1 e2 ...). */
    Cell *test_result = coz_eval(e, a->cell[0]);
    if (test_result->type == CELL_ERROR) return return_val(test_result);

    if (test_result == False_Obj) {
        /* It's #f. Continue the search by tail-calling with the rest of the form. */
        Cell* rest_of_or = make_cell_sexpr();
        cell_add(rest_of_or, G_or_sym);
        for (int i = 1; i < a->count; i++) {
            cell_add(rest_of_or, a->cell[i]);
        }
        return continue_with(rest_of_or, e);
    }
    /* It's a TRUTHY value. We're done. Short-circuit and return this value. */
    return return_val(test_result);
}


/* (when ⟨test⟩ ⟨expression1⟩ ⟨expression2⟩ ... )
 * The test is evaluated, and if it evaluates to a true value, the expressions are evaluated in
 * order. The result of the 'when' expression is unspecified, per R7RS, but Cozenage returns the value
 * of the last expression evaluated, or unspecified if the test evaluates to #f. */
HandlerResult sf_when(Lex* e, Cell* a)
{
    Cell* err = CHECK_ARITY_MIN(a, 2, "when");
    if (err) return return_val(err);

    /* Evaluate the test. */
    Cell* test = coz_eval(e, a->cell[0]);
    if (test->type == CELL_ERROR) return return_val(test);

    /* Check for literal #f */
    if (test == False_Obj) {
        /* Test is false, return unspecified. */
        return return_val(USP_Obj);
    }

    /* Sequence remaining expressions into a 'begin' and tail-call */
    Cell* body_block = sequence_sf_body(get_args_from_sexpr(a));
    return continue_with(body_block, e);
}


/*  (unless ⟨test⟩ ⟨expression1⟩ ⟨expression2⟩ ... )
 *  The test is evaluated, and if it evaluates to #f, the expressions are evaluated in order. The
 *  result of the unless expression is unspecified, per R7RS, but Cozenage returns the value of the
 *  last expression evaluated, or unspecified if the test is truthy. */
HandlerResult sf_unless(Lex* e, Cell* a)
{
    Cell* err = CHECK_ARITY_MIN(a, 2, "unless");
    if (err) return return_val(err);

    /* Evaluate the test. */
    const Cell* test = coz_eval(e, a->cell[0]);

    /* Safety check for NULL from eval, treat it as truthy
     * and check for literal #f. */
    if (test && test->type == CELL_BOOLEAN && test->boolean_v == 0) {
        /* Sequence remaining expressions into a 'begin' and tail-call. */
        Cell* body_block = sequence_sf_body(get_args_from_sexpr(a));
        return continue_with(body_block, e);
    }

    /* Test was true (or null), return unspecified. */
    return return_val(USP_Obj);
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
