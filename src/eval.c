/* eval.c - recursive evaluator */

#include "eval.h"
#include "ops.h"
#include "printer.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

Cell* apply_lambda(Cell* lambda, Cell* args) {
    if (!lambda || lambda->type != VAL_PROC || lambda->builtin) {
        return make_val_err("Not a lambda");
    }

    /* Create a new child environment */
    Lex* local_env = lex_new_child(lambda->env);

    /* Bind formals to arguments */
    if (lambda->formals->count != args->count) {
        return make_val_err("Lambda: wrong number of arguments");
    }

    for (int i = 0; i < args->count; i++) {
        Cell* sym = lambda->formals->cell[i];
        Cell* val = args->cell[i];
        lex_put(local_env, sym, val);  /* sym should be VAL_SYM, val evaluated */
    }

    /* Evaluate body expressions in this environment */
    Cell* result = NULL;
    for (int i = 0; i < lambda->body->count; i++) {
        if (result) cell_delete(result);
        result = coz_eval(local_env, cell_copy(lambda->body->cell[i]));
    }

    lex_delete(local_env);
    return result;
}

/* coz_eval()
 * Evaluate a Cell in the given environment.
 * Literals evaluate to themselves; symbols are looked up.
 * S-expressions are recursively evaluated.
 */
Cell* coz_eval(Lex* e, Cell* v) {
    if (!v) return NULL;

    switch (v->type) {
        /* Symbols: look them up in the environment */
        case VAL_SYM: {
            Cell* x = lex_get(e, v);
            cell_delete(v);
            return x;
        }

        /* S-expressions: recursively evaluate */
        case VAL_SEXPR:
            return eval_sexpr(e, v);

        /* All literals evaluate to themselves */
        case VAL_INT:
        case VAL_REAL:
        case VAL_RAT:
        case VAL_COMPLEX:
        case VAL_BOOL:
        case VAL_CHAR:
        case VAL_STR:
        /* case VAL_PAIR: is not necessary - no concept of 'pair literal' */
        case VAL_VEC:
        case VAL_BYTEVEC:
        case VAL_NIL:
        /* Functions, ports, continuations, and errors are returned as-is */
        case VAL_PROC:
        case VAL_PORT:
        case VAL_CONT:
        case VAL_ERR:
            return v;

        default:
            return make_val_err("Unknown val type in eval()");
    }
}

/* eval_sexpr()
 * Evaluate an S-expression.
 * 1) Evaluate each child.
 * 2) Handle empty or single-element S-expressions.
 * 3) Treat first element as function (symbol or builtin).
 */
Cell* eval_sexpr(Lex* e, Cell* v) {
    if (v->count == 0) return v;

    /* Grab first element without evaluating yet */
    Cell* first = cell_pop(v, 0);

    if (first->type == VAL_SYM && strcmp(first->sym, "define") == 0) {
        cell_delete(first);
        return builtin_define(e, v);
    }

    /* Special form: quote */
    if (first->type == VAL_SYM && strcmp(first->sym, "quote") == 0) {
        cell_delete(first);
        if (v->count != 1) {
            cell_delete(v);
            return make_val_err("quote takes exactly one argument");
        }
        return cell_take(v, 0);  /* return argument unevaluated */
    }

    /* Special form: lambda */
    if (first->type == VAL_SYM && strcmp(first->sym, "lambda") == 0) {
        cell_delete(first);

        if (v->count < 2) {
            cell_delete(v);
            return make_val_err("lambda requires formals and a body");
        }

        Cell* formals = cell_pop(v, 0);   /* first arg */
        Cell* body    = cell_copy(v);       /* remaining args */

        /* formals should be a list of symbols */
        for (int i = 0; i < formals->count; i++) {
            if (formals->cell[i]->type != VAL_SYM) {
                cell_delete(formals);
                cell_delete(body);
                return make_val_err("lambda formals must be symbols");
            }
        }

        /* Build the lambda cell */
        Cell* lambda = lex_make_lambda(formals, body, e);

        cell_delete(formals);
        cell_delete(body);  /* make_lambda deep-copies body and formals */
        cell_delete(v);
        return lambda;
    }

    /* Otherwise, evaluate first element normally (should become a function) */
    Cell* f = coz_eval(e, first);
    if (f->type != VAL_PROC) {
        printf("Bad token: ");
        printf(ANSI_RED_B);
        print_cell(f);
        printf("%s: ", ANSI_RESET);
        cell_delete(f);
        cell_delete(v);
        return make_val_err("S-expression does not start with a procedure");
    }

    /* Now evaluate arguments (since it's not a special form) */
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = coz_eval(e, v->cell[i]);
        if (v->cell[i]->type == VAL_ERR) {
            cell_delete(f);
            return cell_take(v, i);
        }
    }

    /* Apply function */
    Cell* result;
    if (f->builtin) {
        result = f->builtin(e, v);
    } else {
        result = apply_lambda(f, v);
    }
    cell_delete(v);
    cell_delete(f);
    return result;
}
