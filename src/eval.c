/* eval.c - recursive evaluator */

#include "eval.h"
#include "ops.h"
#include "printer.h"
#include "main.h"
#include <gc.h>
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
        const Cell* sym = lambda->formals->cell[i];
        const Cell* val = args->cell[i];
        lex_put(local_env, sym, val);  /* sym should be VAL_SYM, val evaluated */
    }

    /* Evaluate body expressions in this environment */
    Cell* result = NULL;
    for (int i = 0; i < lambda->body->count; i++) {
        result = coz_eval(local_env, cell_copy(lambda->body->cell[i]));
    }
    //lex_delete(local_env);
    return result;
}

Cell* sexpr_to_list(const Cell* c) {
    /* If the item is not an S-expression, it's an atom. Return a copy. */
    if (c->type != VAL_SEXPR) {
        return cell_copy(c);
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

    /* NOTE: These special forms need to be dispatched out of
     * eval_sexpr() early, so that their arguments are not evaluated */

    /* special form: define */
    if (first->type == VAL_SYM && strcmp(first->sym, "define") == 0) {
        return builtin_define(e, v);
    }

    /* Special form: quote */
    if (first->type == VAL_SYM && strcmp(first->sym, "quote") == 0) {
        if (v->count != 1) {
            return make_val_err("quote takes exactly one argument");
        }
        /* Extract the S-expression that was quoted. */
        const Cell* quoted_sexpr = cell_take(v, 0);

        /* Convert the VAL_SEXPR into a proper VAL_PAIR list. */
        Cell* result = sexpr_to_list(quoted_sexpr);
        return result;
    }

    /* Special form: lambda */
    if (first->type == VAL_SYM && strcmp(first->sym, "lambda") == 0) {

        if (v->count < 2) {
            return make_val_err("lambda requires formals and a body");
        }

        const Cell* formals = cell_pop(v, 0);   /* first arg */
        const Cell* body    = cell_copy(v);       /* remaining args */

        /* formals should be a list of symbols */
        for (int i = 0; i < formals->count; i++) {
            if (formals->cell[i]->type != VAL_SYM) {
                return make_val_err("lambda formals must be symbols");
            }
        }

        /* Build the lambda cell */
        Cell* lambda = lex_make_lambda(formals, body, e);
        return lambda;
    }

    /* special form - if */
    if (first->type == VAL_SYM && strcmp(first->sym, "if") == 0) {
        return builtin_if(e, v);
    }

    /* special form - when */
    if (first->type == VAL_SYM && strcmp(first->sym, "when") == 0) {
        return builtin_when(e, v);
    }

    /* special form - unless */
    if (first->type == VAL_SYM && strcmp(first->sym, "unless") == 0) {
        return builtin_unless(e, v);
    }

    /* special form - cond */
    if (first->type == VAL_SYM && strcmp(first->sym, "cond") == 0) {
        return builtin_cond(e, v);
    }

    /* special form - import */
    if (first->type == VAL_SYM && strcmp(first->sym, "import") == 0) {

        Cell* import_set = make_val_sexpr();
        import_set->cell = GC_MALLOC(sizeof(Cell*) * v->count);
        /* 'v' is a sexpr of sexpr's. Make a new sexpr which contains
         * pairs of (library . name), and pass to builtin_import */
        int i;
        for (i = 0; i < v->count; i++) {
            const char* lib = GC_strdup(v->cell[i]->cell[0]->sym);
            const char* name = GC_strdup(v->cell[i]->cell[1]->sym);
            import_set->cell[i] = make_val_pair(make_val_str(lib),
                         make_val_str(name));
        }
        import_set->count = i;

        return builtin_import(e, import_set);
    }

    /* Otherwise, evaluate first element normally (should become a function) */
    Cell* f = coz_eval(e, first);
    if (f->type != VAL_PROC) {
        printf("Bad token: ");
        printf(ANSI_RED_B);
        print_cell(f);
        printf("%s: ", ANSI_RESET);
        return make_val_err("S-expression does not start with a procedure");
    }

    /* Now evaluate arguments (since it's not a special form) */
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = coz_eval(e, v->cell[i]);
        if (v->cell[i]->type == VAL_ERR) {
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
    return result;
}
