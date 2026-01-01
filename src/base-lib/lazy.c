/*
 * 'src/base-lib/lazy.c'
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


#include "repr.h"
#include "../cell.h"
#include "../eval.h"
#include "../types.h"
#include "../symbols.h"
#include "../special_forms.h"
#include "../predicates.h"


/* 'delay', 'delay-force', and 'stream' (ie: cons-stream) are implemented as special forms. */

/* (delay ⟨expression⟩)
 * Semantics: The delay construct is used together with the procedure force to implement lazy evaluation or call by
 * need. (delay ⟨expression⟩) returns an object called a promise which at some point in the future can be asked (by the
 * force procedure) to evaluate ⟨expression⟩, and deliver the resulting value. The effect of ⟨expression⟩ returning
 * multiple values is unspecified. */
HandlerResult sf_delay(Lex* e, Cell* a)
{
    if (a->count != 1) {
        Cell* err = make_cell_error(
            "delay: expected exactly one expression",
            VALUE_ERR);
        return (HandlerResult) { .action = ACTION_RETURN, .value = err, .env = nullptr };
    }
    Cell* promise = make_cell_promise(a->cell[0], e);
    return (HandlerResult) { .action = ACTION_RETURN, .value = promise, .env = nullptr };
}


HandlerResult sf_delay_force(Lex* e, Cell* a)
{
    if (a->count != 1) {
        Cell* err = make_cell_error(
            "delay-force: expected exactly one expression",
            VALUE_ERR);
        return (HandlerResult) { .action = ACTION_RETURN, .value = err, .env = nullptr };
    }

    Cell* promise = make_cell_promise(a->cell[0], e);
    promise->promise->status = LAZY;
    return (HandlerResult) { .action = ACTION_RETURN, .value = promise, .env = nullptr };
}


/* (stream head tail)
 * Stream constructor. */
HandlerResult sf_stream(Lex* e, Cell* a)
{
    if (a->count != 2) {
        Cell* err = make_cell_error(
            "stream: expected head and tail",
            SYNTAX_ERR);
        return (HandlerResult) { .action = ACTION_RETURN, .value = err, .env = nullptr };
    }

    Cell* head = coz_eval(e, a->cell[0]); /* Eager head. */
    Cell* tail_promise = make_cell_promise(a->cell[1], e); /* Lazy tail. */

    Cell* s = make_cell_stream(head, tail_promise);
    return (HandlerResult) { .action = ACTION_RETURN, .value = s, .env = nullptr };
}


/* (force promise)
 * The force procedure forces the value of a promise created by delay, delay-force, or make-promise. If no value has
 * been computed for the promise, then a value is computed and returned. The value of the promise must be cached (or
 * “memoized”) so that if it is forced a second time, the previously computed value is returned. Consequently, a delayed
 * expression is evaluated using the parameter values and exception handler of the call to force which first requested
 * its value. If promise is not a promise, it may be returned unchanged. */
Cell* lazy_force(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "force");
    if (err) return err;

    Cell* p = a->cell[0];
    if (p->type != CELL_PROMISE) return p;

    /* We use a loop to "trampoline" through LAZY promises. */
    while (p->promise->status == READY || p->promise->status == LAZY) {
        if (p->promise->status == RUNNING) {
            return make_cell_error("force: re-entrant promise", GEN_ERR);
        }

        const p_status_t mode = p->promise->status;
        p->promise->status = RUNNING;

        /* Evaluate the expression. */
        Cell* result = coz_eval(p->promise->env, p->promise->expr);

        if (mode == LAZY) {
            /* delay-force requires that the result MUST be a promise. */
            if (result->type != CELL_PROMISE) {
                p->promise->status = DONE; /* Reset state before erroring. */
                return make_cell_error(
                    "delay-force: expression did not return a promise",
                    VALUE_ERR);
            }

            /* THE TRAMPOLINE:
             * Iterate on the new values returned by result. */
            p->promise->expr = result->promise->expr;
            p->promise->env  = result->promise->env;
            p->promise->status = result->promise->status;

            /* If the new status is already DONE, the loop terminates.
             * If it's READY or LAZY, we evaluate it in the next iteration. */
        } else {
            /* Standard 'delay' behavior. */
            p->promise->status = DONE;
            p->promise->env = nullptr;
            p->promise->expr = result;
        }
    }
    return p->promise->expr;
}


/* (make-promise obj)
 * The make-promise procedure returns a promise which, when forced, will return obj. It is similar to delay, but does
 * not delay its argument: it is a procedure rather than syntax. If obj is already a promise, it is returned. */
Cell* lazy_make_promise(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "make-promise");
    if (err) return err;

    if (a->cell[0]->type == CELL_PROMISE) {
        return a->cell[0];
    }

    Cell* p = make_cell_promise(a->cell[0], nullptr);
    p->promise->status = DONE;
    return p;
}


/* (promise? obj)
 * The promise? procedure returns #t if its argument is a promise, and #f otherwise. Note that promises are not
 * necessarily disjoint from other Scheme types such as procedures. */
Cell* lazy_promise_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "promise?");
    if (err) return err;

    if (a->cell[0]->type != CELL_PROMISE) {
        return False_Obj;
    }
    return True_Obj;
}


/* (stream? obj)
 * The stream? procedure returns #t if obj represents a stream, and #f otherwise. */
Cell* lazy_stream_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "stream?");
    if (err) return err;

    if (a->cell[0]->type != CELL_STREAM) {
        return False_Obj;
    }
    return True_Obj;
}


/* (head stream) -> stream.car */
Cell* lazy_head(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "head");
    if (err) return err;

    if (a->cell[0]->type != CELL_STREAM) {
        return make_cell_error(
            "head: expected a stream",
            TYPE_ERR);
    }
    return a->cell[0]->head;
}


/* (tail stream) -> stream.cdr */
Cell* lazy_tail(const Lex* e, const Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1, "tail");
    if (err) return err;
    if (a->cell[0]->type == CELL_NIL) {
        return Nil_Obj;
    }
    if (a->cell[0]->type != CELL_STREAM) {
        return make_cell_error(
            "tail: expected a stream",
            TYPE_ERR);
    }

    /* Automatic Force! */
    Cell* next = lazy_force(e, make_sexpr_len1(a->cell[0]->tail));
    return next;
}


/* (at n stream) -> value */
Cell* lazy_at(const Lex* e, const Cell* a) {
    if (a->cell[0]->type != CELL_INTEGER) {
        return make_cell_error(\
            "at: arg1 must be a positive integer",
            TYPE_ERR);
    }
    long n = a->cell[0]->integer_v;

    if (a->cell[1]->type != CELL_STREAM) {
        return make_cell_error(
            "at: arg2 must be a stream",
            TYPE_ERR);
    }
    Cell* s = a->cell[1];

    while (n > 0) {
        if (s->type != CELL_STREAM) {
            return make_cell_error(
                "at: expected a stream",
                TYPE_ERR);
        }

        /* Use the tail (which is a promise) and force it.
         * The result of forcing the tail MUST be another stream. */
        s = lazy_force(e, make_sexpr_len1(s->tail));

        if (s->type == CELL_ERROR) return s;
        n--;
    }

    if (s->type != CELL_STREAM) {
        return make_cell_error(
            "at: reached end of stream before index",
            INDEX_ERR);
    }
    return s->head;
}


Cell* lazy_take(const Lex* e, const Cell* a) {
    long n = a->cell[0]->integer_v;
    Cell* s = a->cell[1];

    Cell* head = Nil_Obj;
    Cell* tail = Nil_Obj;

    while (n > 0 && s->type == CELL_STREAM) {
        /* Build the result list node. */
        Cell* new_node = make_cell_pair(s->head, Nil_Obj);

        if (head == Nil_Obj) head = new_node;
        else tail->tail = new_node;
        tail = new_node;

        n--;
        if (n > 0) {
            /* Step the stream. */
            s = lazy_tail(e, make_sexpr_len1(s));
            if (s->type == CELL_ERROR) return s;
        }
    }
    return head;
}


/* (drop n stream) -> stream */
Cell* lazy_drop(const Lex* e, const Cell* a) {
    long n = a->cell[0]->integer_v;
    Cell* s = a->cell[1];

    while (n > 0 && s->type != CELL_NIL) {
        s = lazy_tail(e, make_sexpr_len1(s));
        n--;
    }
    return s;
}


// /* (select pred stream) -> promise */
// Cell* lazy_select(const Lex* e, const Cell* a) {
//     Cell* pred = a->cell[0];
//     Cell* s = a->cell[1];
//
//     if (s->type == CELL_NIL) return Nil_Obj;
//
//     while (s->type == CELL_STREAM) {
//         /* Test the current head */
//         Cell* res = coz_apply_and_get_val(pred, make_sexpr_len1(s->head), e);
//         if (res->type == CELL_ERROR) return res;
//
//         if (res == True_Obj) {
//             /* We found a match at node 's' */
//
//             Cell* tail_expr = make_cell_sexpr();
//             cell_add(tail_expr, make_cell_symbol("select"));
//             cell_add(tail_expr, pred);
//
//             Cell* get_next_node_expr = make_cell_sexpr();
//             cell_add(get_next_node_expr, make_cell_symbol("tail"));
//             cell_add(get_next_node_expr, s);
//
//             cell_add(tail_expr, get_next_node_expr);
//
//             Cell* tail_promise = make_cell_promise(tail_expr, (Lex*)e);
//             tail_promise->promise->status = LAZY;
//             return make_cell_stream(s->head, tail_promise);
//
//         }
//
//         /* NO MATCH: Step to the next node manually */
//         /* Force the promise in the tail to get the next stream node */
//         s = lazy_force(e, make_sexpr_len1(s->tail));
//
//         if (s->type == CELL_ERROR) return s;
//         if (s->type == CELL_NIL) return Nil_Obj;
//
//         /* If s->type is not CELL_STREAM here, the user gave us a
//            malformed stream (a promise that doesn't evaluate to a stream). */
//         if (s->type != CELL_STREAM) {
//             return make_cell_error(
//                 "select: stream tail must evaluate to a stream",
//                 TYPE_ERR);
//         }
//     }
//     return Nil_Obj;
// }


void cozenage_library_init(const Lex* e)
{
    /* Register builtin procedures in the global environment. */
    lex_add_builtin(e, "force", lazy_force);
    lex_add_builtin(e, "make-promise", lazy_make_promise);
    lex_add_builtin(e, "head", lazy_head);
    lex_add_builtin(e, "tail", lazy_tail);
    lex_add_builtin(e, "stream?", lazy_stream_pred);
    lex_add_builtin(e, "promise?", lazy_promise_pred);
    lex_add_builtin(e, "at", lazy_at);
    lex_add_builtin(e, "take", lazy_take);
    lex_add_builtin(e, "drop", lazy_drop);
    //lex_add_builtin(e, "select", lazy_select);
    lex_add_builtin(e, "stream-null?", builtin_null_pred);

    /* Intern symbols for the three special forms, and set their SF_IDs. */
    Cell* delay = make_cell_symbol("delay");
    delay->sf_id = SF_ID_DELAY;

    Cell* delay_force = make_cell_symbol("delay-force");
    delay_force->sf_id = SF_ID_DELAY_FORCE;

    Cell* stream = make_cell_symbol("stream");
    stream->sf_id = SF_ID_STREAM;

    /* Register the special forms in the SF lookup table. */
    SF_DISPATCH_TABLE[SF_ID_DELAY]       = &sf_delay;
    SF_DISPATCH_TABLE[SF_ID_DELAY_FORCE] = &sf_delay_force;
    SF_DISPATCH_TABLE[SF_ID_STREAM]      = &sf_stream;
}
