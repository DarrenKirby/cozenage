/* ops.c - definitions of built-in functions */

#include "ops.h"
#include "types.h"
#include "environment.h"
#include "eval.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "printer.h"


/* Convert any Cell number to long double for calculation */
long double VAL_to_ld(Cell* v) {
    return (v->type == VAL_INT) ? (long double)v->i_val : v->r_val;
}

/* Helper which determines if there is a meaningful fractional portion
 * in the result, and returns VAL_INT or VAL_FLOAT accordingly */
Cell* make_VAL_from_double(const long double x) {
    /* epsilon: what counts as “effectively an integer” */
    const long double EPS = 1e-12L;

    const long double rounded = roundl(x);
    if (fabsl(x - rounded) < EPS) {
        /* treat as integer */
        return make_val_int((long long)rounded);
    }
    /* treat as float */
    return make_val_real(x);
}

/*------------------------------------*
 *     Basic arithmetic operators     *
 * -----------------------------------*/

/* '+' -> VAL_INT|VAL_REAL|VAL_RAT|VAL_COMP - returns the sum of its arguments */
Cell* builtin_add(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    /* identity law logic */
    if (a->count == 0) return make_val_int(0);
    if (a->count == 1) return cell_copy(a->cell[0]);

    Cell* result = cell_copy(a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        Cell* rhs = cell_copy(a->cell[i]);
        numeric_promote(&result, &rhs);

        switch (result->type) {
            case VAL_INT:
                result->i_val += rhs->i_val;
                break;
            case VAL_RAT:
                /* (a/b) + (c/d) = (ad + bc)/bd */
                result->num = result->num * rhs->den + rhs->num * result->den;
                result->den = result->den * rhs->den;
                result = simplify_rational(result);
                break;
            case VAL_REAL:
                result->r_val += rhs->r_val;
                break;
            case VAL_COMPLEX:
                complex_apply(builtin_add, e, result, rhs);
                break;
            default:
                return make_val_err("<builtin '+'> Oops, this shouldn't have happened.");
        }
        result->exact = result->exact && rhs->exact;
        cell_delete(rhs); /* free temp */
    }
    return result;
}

/* '-' -> VAL_INT|VAL_REAL|VAL_RAT|VAL_COMP - returns the difference of its arguments */
Cell* builtin_sub(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }

    /* Handle unary minus */
    if (a->count == 1) {
        return negate_numeric(a->cell[0]);
    }
    /* multiple args */
    Cell* result = cell_copy(a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        Cell* rhs = cell_copy(a->cell[i]);
        numeric_promote(&result, &rhs);

        switch (result->type) {
            case VAL_INT:
                result->i_val -= rhs->i_val;
                break;
            case VAL_RAT:
                /* (a/b) - (c/d) = (ad - bc)/bd */
                /* 1/1 - 1/2 = ((1)(2) - (1)(1))/(1)(2) = (2 - 1)/2 = 1/2 */
                result->num = result->num * rhs->den - rhs->num * result->den;
                result->den = result->den * rhs->den;
                result = simplify_rational(result);
                break;
            case VAL_REAL:
                result->r_val -= rhs->r_val;
                break;
            case VAL_COMPLEX:
                complex_apply(builtin_sub, e, result, rhs);
                break;
            default:
                return make_val_err("<builtin '-'> Oops, this shouldn't have happened.");
        }
        result->exact = result->exact && rhs->exact;
        cell_delete(rhs); /* free temp */
    }
    return result;
}

/* '*' -> VAL_INT|VAL_FLOAT|VAL_RAT|VAL_COMP - returns the product of its arguments */
Cell* builtin_mul(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    /* identity law logic */
    if (a->count == 0) return make_val_int(1);
    if (a->count == 1) return cell_copy(a->cell[0]);

    Cell* result = cell_copy(a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        Cell* rhs = cell_copy(a->cell[i]);
        numeric_promote(&result, &rhs);

        switch (result->type) {
            case VAL_INT:
                result->i_val *= rhs->i_val;
                break;
            case VAL_RAT:
                /* (a/b) * (c/d) = (a * c)/(b * d) */
                result->num = result->num * rhs->num;
                result->den = result->den * rhs->den;
                result = simplify_rational(result);
                break;
            case VAL_REAL:
                result->r_val *= rhs->r_val;
                break;
            case VAL_COMPLEX:
                complex_apply(builtin_mul, e, result, rhs);
                break;
            default:
                return make_val_err("<builtin '*'> Oops, this shouldn't have happened.");
        }
        result->exact = result->exact && rhs->exact;
        cell_delete(rhs); /* free temp */
    }
    return result;
}

/* '+' -> VAL_INT|VAL_REAL|VAL_RAT|VAL_COMP - returns the quotient of its arguments */
Cell* builtin_div(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }

    /* unary division reciprocal */
    if (a->count == 1) {
        if (a->cell[0]->type == VAL_INT) {
            return make_val_rat(1, a->cell[0]->i_val, 1);
        }
        if (a->cell[0]->type == VAL_RAT) {
            const long int n = a->cell[0]->num;
            const long int d = a->cell[0]->den;
            return make_val_rat(d, n, 1);
        }
        if (a->cell[0]->type == VAL_REAL) {
            return make_val_real(1.0L / a->cell[0]->r_val);
        }
        if (a->cell[0]->type == VAL_COMPLEX) {
            Cell* z = a->cell[0];
            Cell* a_part = z->real;
            Cell* b_part = z->imag;

            /* Calculate the denominator: a^2 + b^2 */
            Cell* a_sq_args = make_sexpr_len2(a_part, a_part);
            Cell* b_sq_args = make_sexpr_len2(b_part, b_part);
            Cell* a_sq = builtin_mul(e, a_sq_args);
            Cell* b_sq = builtin_mul(e, b_sq_args);
            cell_delete(a_sq_args);
            cell_delete(b_sq_args);

            Cell* denom_args = make_sexpr_len2(a_sq, b_sq);
            Cell* denom = builtin_add(e, denom_args);
            cell_delete(denom_args);

            /* Calculate the new real part: a / (a^2 + b^2) */
            Cell* real_args = make_sexpr_len2(a_part, denom);
            Cell* new_real = builtin_div(e, real_args);
            cell_delete(real_args);

            /* Calculate the new imaginary part: -b / (a^2 + b^2) */
            Cell* zero = make_val_int(0);
            Cell* neg_b_args = make_sexpr_len2(zero, b_part);
            Cell* neg_b = builtin_sub(e, neg_b_args);
            cell_delete(neg_b_args);

            Cell* imag_args = make_sexpr_len2(neg_b, denom);
            Cell* new_imag = builtin_div(e, imag_args);
            cell_delete(imag_args);

            /* Create the final result */
            Cell* result = make_val_complex(new_real, new_imag);

            /* Clean up all intermediate cells */
            cell_delete(a_sq);
            cell_delete(b_sq);
            cell_delete(denom);
            cell_delete(zero);
            cell_delete(neg_b);

            return result;
        }
    }
    /* multiple args */
    Cell* result = cell_copy(a->cell[0]);

    for (int i = 1; i < a->count; i++) {
        Cell* rhs = cell_copy(a->cell[i]);
        numeric_promote(&result, &rhs);

        switch (result->type) {
        case VAL_INT:
            if (rhs->i_val == 0) {
                cell_delete(result);
                cell_delete(rhs);
                return make_val_err("Division by zero.");
            }
            /* pretty hacky way to get (/ 9 3) -> 3 but (/ 10 3) -> 10/3 */
            const double r = remainder((double)result->i_val, (double)rhs->i_val);
            if (r == 0 || r == 0.0) {
                result->i_val /= rhs->i_val;
            } else {
                Cell* new_result = make_val_rat(result->i_val, rhs->i_val, 1);
                cell_delete(result);
                result = new_result;
            }
            break;
        case VAL_RAT:
            if (rhs->num == 0) {
                cell_delete(result);
                cell_delete(rhs);
                return make_val_err("Division by zero.");
            }
            /* (a/b) / (c/d) = (a * d)/(b * c)   */
            result->num = result->num * rhs->den;
            result->den = result->den * rhs->num;
            Cell* simplified = simplify_rational(result);
            if (simplified != result) {
                /* simplify_rational returned a new cell, free the old one */
                cell_delete(result);
                result = simplified;
            }
            break;
        case VAL_REAL:
            if (rhs->r_val == 0) {
                cell_delete(result);
                cell_delete(rhs);
                return make_val_err("Division by zero.");
            }
            result->r_val /= rhs->r_val;
            break;
        case VAL_COMPLEX:
            complex_apply(builtin_div, e, result, rhs);
            break;
        default:
            cell_delete(result);
            cell_delete(rhs);
            return make_val_err("<builtin '/'> Oops, this shouldn't have happened.");
        }
        result->exact = result->exact && rhs->exact;
        cell_delete(rhs); /* free temp */
    }
    return result;
}
// Cell* builtin_div(Lex* e, Cell* a) {
//     Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
//     if (err) { return err; }
//     if ((err = CHECK_ARITY_MIN(a, 1))) { return err; }
//
//     /* unary division reciprocal */
//     if (a->count == 1) {
//         if (a->cell[0]->type == VAL_INT) {
//             return make_val_rat(1, a->cell[0]->i_val, 1);
//         }
//         if (a->cell[0]->type == VAL_RAT) {
//             const long int n = a->cell[0]->num;
//             const long int d = a->cell[0]->den;
//             return make_val_rat(d, n, 1);
//         }
//         if (a->cell[0]->type == VAL_REAL) {
//             return make_val_real(1.0L / a->cell[0]->r_val);
//         }
//         if (a->cell[0]->type == VAL_COMPLEX) {
//             Cell* z = a->cell[0];
//             Cell* a_part = z->real;
//             Cell* b_part = z->imag;
//
//             /* Calculate the denominator: a^2 + b^2 */
//             Cell* a_sq = builtin_mul(e, make_sexpr_len2(a_part, a_part));
//             Cell* b_sq = builtin_mul(e, make_sexpr_len2(b_part, b_part));
//             Cell* denom = builtin_add(e, make_sexpr_len2(a_sq, b_sq));
//
//             /* Calculate the new real part: a / (a^2 + b^2) */
//             Cell* new_real = builtin_div(e, make_sexpr_len2(a_part, denom));
//
//             /* Calculate the new imaginary part: -b / (a^2 + b^2) */
//             Cell* zero = make_val_int(0);
//             Cell* neg_b = builtin_sub(e, make_sexpr_len2(zero, b_part));
//             Cell* new_imag = builtin_div(e, make_sexpr_len2(neg_b, denom));
//
//             /* Create the final result */
//             Cell* result = make_val_complex(new_real, new_imag);
//
//             /* Clean up all intermediate cells */
//             cell_delete(a_sq);
//             cell_delete(b_sq);
//             cell_delete(denom);
//             cell_delete(zero);
//             cell_delete(neg_b);
//
//             return result;
//         }
//     }
//     /* multiple args */
//     Cell* result = cell_copy(a->cell[0]);
//
//     for (int i = 1; i < a->count; i++) {
//         Cell* rhs = cell_copy(a->cell[i]);
//         numeric_promote(&result, &rhs);
//
//         switch (result->type) {
//         case VAL_INT:
//             if (rhs->i_val == 0) {
//                 cell_delete(rhs);
//                 return make_val_err("Division by zero.");
//             }
//             /* pretty hacky way to get (/ 9 3) -> 3 but (/ 10 3) -> 10/3 */
//             const double r = remainder((double)result->i_val, (double)rhs->i_val);
//             if (r == 0 || r == 0.0) {
//                 result->i_val /= rhs->i_val;
//             } else {
//                 result = make_val_rat(result->i_val, rhs->i_val, 1);
//             }
//             break;
//         case VAL_RAT:
//             /* (a/b) / (c/d) = (a * d)/(b * c)   */
//             result->num = result->num * rhs->den;
//             result->den = result->den * rhs->num;
//             result = simplify_rational(result);
//             break;
//         case VAL_REAL:
//             if (rhs->r_val == 0) {
//                 cell_delete(rhs);
//                 return make_val_err("Division by zero.");
//             }
//             result->r_val /= rhs->r_val;
//             break;
//         case VAL_COMPLEX:
//             complex_apply(builtin_div, e, result, rhs);
//             break;
//         default:
//             return make_val_err("<builtin '/'> Oops, this shouldn't have happened.");
//         }
//         result->exact = result->exact && rhs->exact;
//         cell_delete(rhs); /* free temp */
//     }
//     return result;
// }

/* -----------------------------*
 *     Comparison operators     *
 * -----------------------------*/

/* Helper for '=' which recursively compares complex numbers */
static int complex_eq_op(Lex* e, Cell* lhs, Cell* rhs) {
    Cell* args_real = make_sexpr_len2(lhs->real, rhs->real);
    Cell* args_imag = make_sexpr_len2(lhs->imag, rhs->imag);

    Cell* real_result = builtin_eq_op(e, args_real);
    Cell* imag_result = builtin_eq_op(e, args_imag);

    const int eq = (real_result->b_val && imag_result->b_val);

    cell_delete(real_result);
    cell_delete(imag_result);
    cell_delete(args_real);
    cell_delete(args_imag);

    return eq;
}

/* '=' -> VAL_BOOL - returns true if all arguments are equal. */
Cell* builtin_eq_op(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int the_same = 0;
        Cell* lhs = cell_copy(a->cell[i]);
        Cell* rhs = cell_copy(a->cell[i+1]);
        numeric_promote(&lhs, &rhs);

        switch (lhs->type) {
            case VAL_INT:
                if (lhs->i_val == rhs->i_val) { the_same = 1; }
                break;
            case VAL_RAT:
                if ((lhs->den == rhs->den) && (lhs->num == rhs->num)) { the_same = 1; }
                break;
            case VAL_REAL:
                if (lhs->r_val == rhs->r_val) { the_same = 1; }
                break;
            case VAL_COMPLEX:
                if (complex_eq_op(e, lhs, rhs)) { the_same = 1; }
                break;
            default: ; /* this will never run as the types are pre-checked, but without the linter complains */
        }
        cell_delete(lhs);
        cell_delete(rhs);
        if (!the_same) {
            return make_val_bool(0);
        }
    }
    return make_val_bool(1);
}

/* '>' -> VAL_BOOL - returns true if each argument is greater than the one that follows. */
Cell* builtin_gt_op(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int ok = 0;
        Cell* lhs = cell_copy(a->cell[i]);
        Cell* rhs = cell_copy(a->cell[i+1]);
        numeric_promote(&lhs, &rhs);
        switch (lhs->type) {
            case VAL_INT: {
                if (lhs->i_val > rhs->i_val) { ok = 1; }
                break;
            }
            case VAL_REAL: {
                if (lhs->r_val > rhs->r_val) { ok = 1; }
                break;
            }
            case VAL_RAT: {
                if ((lhs->num * rhs->den) > (lhs->den * rhs->num)) { ok = 1; }
                break;
            }
            default: ;
        }
        cell_delete(lhs);
        cell_delete(rhs);
        if (!ok) {
            return make_val_bool(0);
        }
    }
    return make_val_bool(1);
}

/* '<' -> VAL_BOOL - returns true if each argument is less than the one that follows. */
Cell* builtin_lt_op(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int ok = 0;
        Cell* lhs = cell_copy(a->cell[i]);
        Cell* rhs = cell_copy(a->cell[i+1]);
        numeric_promote(&lhs, &rhs);
        switch (lhs->type) {
            case VAL_INT: {
                if (lhs->i_val < rhs->i_val) { ok = 1; }
                break;
            }
            case VAL_REAL: {
                if (lhs->r_val < rhs->r_val) { ok = 1; }
                break;
            }
            case VAL_RAT: {
                if ((lhs->num * rhs->den) < (lhs->den * rhs->num)) { ok = 1; }
                break;
            }
            default: ;
        }
        cell_delete(lhs);
        cell_delete(rhs);
        if (!ok) {
            return make_val_bool(0);
        }
    }
    return make_val_bool(1);
}

/* '>=' -> VAL_BOOL - */
Cell* builtin_gte_op(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int ok = 0;
        Cell* lhs = cell_copy(a->cell[i]);
        Cell* rhs = cell_copy(a->cell[i+1]);
        numeric_promote(&lhs, &rhs);
        switch (lhs->type) {
            case VAL_INT: {
                if (lhs->i_val >= rhs->i_val) { ok = 1; }
                break;
            }
            case VAL_REAL: {
                if (lhs->r_val >= rhs->r_val) { ok = 1; }
                break;
            }
            case VAL_RAT: {
                if ((lhs->num * rhs->den) >= (lhs->den * rhs->num)) { ok = 1; }
                break;
            }
            default: ;
        }
        cell_delete(lhs);
        cell_delete(rhs);
        if (!ok) {
            return make_val_bool(0);
        }
    }
    return make_val_bool(1);
}

/* '<=' -> VAL_BOOL - */
Cell* builtin_lte_op(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT);
    if (err) { return err; }
    for (int i = 0; i < a->count - 1; i++) {
        int ok = 0;
        Cell* lhs = cell_copy(a->cell[i]);
        Cell* rhs = cell_copy(a->cell[i+1]);
        numeric_promote(&lhs, &rhs);
        switch (lhs->type) {
            case VAL_INT: {
                if (lhs->i_val <= rhs->i_val) { ok = 1; }
                break;
            }
            case VAL_REAL: {
                if (lhs->r_val <= rhs->r_val) { ok = 1; }
                break;
            }
            case VAL_RAT: {
                if ((lhs->num * rhs->den) <= (lhs->den * rhs->num)) { ok = 1; }
                break;
            }
            default: ;
        }
        cell_delete(lhs);
        cell_delete(rhs);
        if (!ok) {
            return make_val_bool(0);
        }
    }
    return make_val_bool(1);
}

/* ---------------------------------------*
 *    Generic unary numeric procedures    *
 * ---------------------------------------*/

/* 'zero?' -> VAL - returns #t if arg is == 0 else #f */
Cell* builtin_zero(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long double result = VAL_to_ld(a->cell[0]);
    if (result == 0 || result == 0.0) { return make_val_bool(1); }
    return make_val_bool(0);
}

/* 'positive?' -> VAL_BOOL - returns #t if arg is >= 0 else #f */
Cell* builtin_positive(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long double result = VAL_to_ld(a->cell[0]);
    if (result >= 0) { return make_val_bool(1); }
    return make_val_bool(0);
}

/* 'negative?' -> VAL_BOOL - returns #t if arg is < 0 else #f */
Cell* builtin_negative(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT | VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long double result = VAL_to_ld(a->cell[0]);
    if (result < 0) { return make_val_bool(1); }
    return make_val_bool(0);
}

/* 'odd?' -> VAL_BOOL - returns #t if arg is odd else #f */
Cell* builtin_odd(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long long n = a->cell[0]->i_val;
    if (n % 2 == 0) { return make_val_bool(0); }
    return make_val_bool(1);
}

/* 'even?' -> VAL_BOOL - returns #t if arg is even else #f */
Cell* builtin_even(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const long long n = a->cell[0]->i_val;
    if (n % 2 == 0) { return make_val_bool(1); }
    return make_val_bool(0);
}

/* -----------------------------*
 *         Special forms        *
 * -----------------------------*/

/* 'quote' -> VAL_SEXPR -  returns the sole argument unevaluated */
Cell* builtin_quote(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    /* Take the first argument and do NOT evaluate it */
    return cell_take(a, 0);
}

/* 'define' -> binds a value (or proc) to a symbol, and places it
 * into the environment */
Cell* builtin_define(Lex* e, Cell* a) {
    if (a->count < 2) {
        return make_val_err("define requires at least 2 arguments");
    }
    Cell* target = a->cell[0];

    /* (define <symbol> <expr>) */
    if (target->type == VAL_SYM) {
        Cell* val = coz_eval(e, a->cell[1]);
        /* Grab the name for the un-sugared define lambda */
        if (val->type == VAL_PROC) {
            if (!val->name) {
                val->name = strdup(target->name);
            }
        }
        lex_put(e, target, val);
        return val;
    }

    /* (define (<f-name> <args>) <body>) */
    if (target->type == VAL_SEXPR && target->count > 0 &&
        target->cell[0]->type == VAL_SYM) {

        /* first element is function name */
        Cell* fname = target->cell[0];

        /* rest are formal args */
        Cell* formals = make_val_sexpr();
        for (int i = 1; i < target->count; i++) {
            if (target->cell[i]->type != VAL_SYM) {
                return make_val_err("lambda formals must be symbols");
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

        cell_delete(formals);
        cell_delete(body);
        cell_delete(fname);

        return lam;
        }

    return make_val_err("invalid define syntax");
}

Cell* builtin_if(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_RANGE(a, 2, 3);
    if (err) return err;

    Cell* test = coz_eval(e, a->cell[0]);
    if (test->type != VAL_BOOL) {
        cell_delete(test);
        return make_val_err("'if' test must be a predicate");
    }
    Cell* result;
    if (test->b_val == 1) {
        result = coz_eval(e, a->cell[1]);
    } else {
        if (a->count == 2) {
            result = NULL;
        } else {
            result = coz_eval(e, a->cell[2]);
        }
    }
    cell_delete(test);
    return result;
}

Cell* builtin_when(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;

    Cell* test = coz_eval(e, a->cell[0]);
    if (test->type != VAL_BOOL) {
        cell_delete(test);
        return make_val_err("'when' test must be a predicate");
    }
    Cell* result = NULL;
    if (test->b_val == 1) {
        for (int i = 1; i < a->count; i++) {
            result = coz_eval(e, a->cell[i]);
        }
    }
    cell_delete(test);
    return result;
}

Cell* builtin_unless(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;

    Cell* test = coz_eval(e, a->cell[0]);
    if (test->type != VAL_BOOL) {
        cell_delete(test);
        return make_val_err("'unless' test must be a predicate");
    }
    Cell* result = NULL;
    if (test->b_val == 0) {
        for (int i = 1; i < a->count; i++) {
            result = coz_eval(e, a->cell[i]);
        }
    }
    cell_delete(test);
    return result;
}

Cell* builtin_cond(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_MIN(a, 2);
    if (err) return err;

    Cell* result = NULL;
    Cell* clause = NULL;
    Cell* test = NULL;
    for (int i = 0; i < a->count; i++) {
        clause = a->cell[i];
        test = coz_eval(e, clause->cell[0]);
        if (test->type == VAL_PROC && strcmp(test->name, "else") == 0) {
            result = coz_eval(e, clause->cell[1]);
            break;
        }
        if (test->type != VAL_BOOL) {
            result = make_val_err("'cond' test must be a predicate");
        }
        if (test->b_val == 1) {
            for (int j = 1; j < clause->count; j++) {
                result = coz_eval(e, clause->cell[j]);
            }
            break;
        }

    }
    cell_delete(test);
    return result;
}

/* dummy function */
Cell* builtin_else(Lex* e, Cell* a) {
    return make_val_bool(1);
}

/* ------------------------------------------*
*    Equality and equivalence comparators    *
* -------------------------------------------*/

/* 'eq?' -> VAL_BOOL - Tests whether its two arguments are the exact same object
 * (pointer equality). Typically used for symbols and other non-numeric atoms.
 * May not give meaningful results for numbers or characters, since distinct but
 * equal values aren’t guaranteed to be the same object. */
Cell* builtin_eq(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    Cell* x = a->cell[0];
    Cell* y = a->cell[1];

    /* Strict pointer equality */
    return make_val_bool(x == y);
}

/* 'eqv?' -> VAL_BOOL - Like 'eq?', but also considers numbers and characters
 * with the same value as equivalent. (eqv? 2 2) is true, even if those 2s are
 * not the same object. Use when: you want a general-purpose equality predicate
 * that works for numbers, characters, and symbols, but you don’t need deep
 * structural comparison. */
Cell* builtin_eqv(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    Cell* x = a->cell[0];
    Cell* y = a->cell[1];

    if (x->type != y->type) return make_val_bool(0);

    switch (x->type) {
        case VAL_BOOL: return make_val_bool(x->b_val == y->b_val);
        case VAL_INT:  return make_val_bool(x->i_val == y->i_val);
        case VAL_REAL: return make_val_bool(x->r_val == y->r_val);
        case VAL_RAT:  return make_val_bool((x->den == y->den) && (x->num == y->num));
        case VAL_COMPLEX: return make_val_bool(complex_eq_op(e, x, y));
        case VAL_CHAR: return make_val_bool(x->c_val == y->c_val);
        default:       return make_val_bool(x == y); /* fall back to identity */
    }
}

/* Helper for equal? */
Cell* val_equal(Lex* e, Cell* x, Cell* y) {
    if (x->type != y->type) return make_val_bool(0);

    switch (x->type) {
        case VAL_BOOL: return make_val_bool(x->b_val == y->b_val);
        case VAL_CHAR: return make_val_bool(x->c_val == y->c_val);
        case VAL_INT:  return make_val_bool(x->i_val == y->i_val);
        case VAL_REAL: return make_val_bool(x->r_val == y->r_val);
        case VAL_RAT:  return make_val_bool((x->den == y->den) && (x->num == y->num));
        case VAL_SYM:  return make_val_bool(strcmp(x->sym, y->sym) == 0);
        case VAL_STR:  return make_val_bool(strcmp(x->str, y->str) == 0);
        case VAL_COMPLEX: return make_val_bool(complex_eq_op(e, x, y));

        case VAL_SEXPR:
        case VAL_VEC:
            if (x->count != y->count) return make_val_bool(0);
            for (int i = 0; i < x->count; i++) {
                Cell* eq = val_equal(e, x->cell[i], y->cell[i]);
                if (!eq->b_val) { cell_delete(eq); return make_val_bool(0); }
                cell_delete(eq);
            }
            return make_val_bool(1);

        default:
            return make_val_bool(0);
    }
}

/* 'equal?' -> VAL_BOOL - Tests structural (deep) equality, comparing contents
 * recursively in lists, vectors, and strings. (equal? '(1 2 3) '(1 2 3)) → true,
 * even though the two lists are distinct objects.
 * Use when: you want to compare data structures by content, not identity.*/
Cell* builtin_equal(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    return val_equal(e, a->cell[0], a->cell[1]);
}

/* -----------------------------------*
 *     Generic numeric operations     *
 * -----------------------------------*/

/* 'abs' -> VAL_INT|VAL_REAL|VAL_RAT - returns the absolute value of its argument */
Cell* builtin_abs(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    switch (a->cell[0]->type) {
        case VAL_INT:
            return (a->cell[0]->i_val < 0) ? negate_numeric(a->cell[0]) : cell_copy(a->cell[0]);
        case VAL_REAL:
            return (a->cell[0]->r_val < 0) ? negate_numeric(a->cell[0]) : cell_copy(a->cell[0]);
        case VAL_RAT:
            return (a->cell[0]->num < 0) ? negate_numeric(a->cell[0]) : cell_copy(a->cell[0]);
        default:
            return make_val_err("abs: Oops, this isn't right");
    }
}

/* 'expt' -> VAL_INT|VAL_REAL - returns its first arg calculated
 * to the power of its second arg */
Cell* builtin_expt(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT | VAL_REAL);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }

    long double base = VAL_AS_NUM(a->cell[0]);
    Cell* exp_val = a->cell[1];
    long double result;

    if (exp_val->type == VAL_INT) {
        /* integer exponent: use fast exponentiation */
        const long long n = exp_val->i_val;
        result = 1.0;
        long long abs_n = n > 0 ? n : -n;

        while (abs_n > 0) {
            if (abs_n & 1) result *= base;
            base *= base;
            abs_n >>= 1;
        }
        if (n < 0) result = 1.0 / result;
    } else {
        /* float exponent: delegate to powl */
        result = powl(base, VAL_AS_NUM(exp_val));
    }
    return make_VAL_from_double(result);
}

/* 'modulo' -> VAL_INT - returns the remainder of dividing the first argument
 * by the second, with the result having the same sign as the divisor.*/
Cell* builtin_modulo(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    long long r = a->cell[0]->i_val % a->cell[1]->i_val;
    if ((r != 0) && ((a->cell[1]->i_val > 0 && r < 0) || (a->cell[1]->i_val < 0 && r > 0))) {
        r += a->cell[1]->i_val;
    }
    return make_val_int(r);
}

/* 'quotient' -> VAL_INT - returns the integer result of dividing
 * the first argument by the second, discarding any remainder.*/
Cell* builtin_quotient(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    return make_val_int(a->cell[0]->i_val / a->cell[1]->i_val);
}

/* 'remainder' -> VAL_INT - returns the remainder of dividing the first argument
 * by the second, with the result having the same sign as the dividend.*/
Cell* builtin_remainder(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }
    return make_val_int(a->cell[0]->i_val % a->cell[1]->i_val);
}

Cell* builtin_lcm(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 2))) { return err; }

    const long long int x = a->cell[0]->i_val;
    const long long int y = a->cell[1]->i_val;

    Cell* gcd = builtin_gcd(e, make_sexpr_len2(make_val_int(x), make_val_int(y)));
    long long int tmp = x * y / gcd->i_val;
    /* return only positive value */
    if (tmp < 0) {
        tmp = -tmp;
    }
    cell_delete(gcd);
    return make_val_int(tmp);
}

Cell* builtin_gcd(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT);
    if (err) return err;
    if ((err = CHECK_ARITY_EXACT(a, 2))) return err;

    long long x = a->cell[0]->i_val;
    long long y = a->cell[1]->i_val;

    while (x != 0) {
        const long long tmp = x;
        x = y % x;
        y = tmp;
    }

    /* return only positive value */
    if (y < 0) {
        y = -y;
    }
    return make_val_int(y);
}

/* 'max' -> */
Cell* builtin_max(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    Cell* largest_so_far = cell_copy(a->cell[0]);
    for (int i = 0; i < a->count - 1; i++) {
        Cell* rhs = cell_copy(a->cell[i+1]);
        Cell* result = builtin_lt_op(e, make_sexpr_len2(largest_so_far, rhs));
        if (result->b_val == 1) {
            largest_so_far = cell_copy(rhs);
        }
        cell_delete(rhs);
        cell_delete(result);
    }
    return largest_so_far;
}

/* 'min' -> */
Cell* builtin_min(Lex* e, Cell* a) {
    (void)e;
    Cell* err = check_arg_types(a, VAL_INT|VAL_RAT|VAL_REAL);
    if (err) { return err; }
    Cell* smallest_so_far = cell_copy(a->cell[0]);
    for (int i = 0; i < a->count - 1; i++) {
        Cell* rhs = cell_copy(a->cell[i+1]);
        Cell* result = builtin_gt_op(e, make_sexpr_len2(smallest_so_far, rhs));
        if (result->b_val == 1) {
            smallest_so_far = cell_copy(rhs);
        }
        cell_delete(rhs);
        cell_delete(result);
    }
    return smallest_so_far;
}

/* -------------------------------------------------*
 *       Type identity predicate procedures         *
 * -------------------------------------------------*/

/* 'number?' -> VAL_BOOL - returns #t if obj is numeric, else #f  */
Cell* builtin_number_pred(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX;
    if (a->cell[0]->type & mask) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

/* 'boolean?' -> VAL_BOOL  - returns #t if obj is either #t or #f
    and returns #f otherwise. */
Cell* builtin_boolean_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_BOOL);
}

/* 'null?' -> VAL_BOOL - return #t if obj is null, else #f */
Cell* builtin_null_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_NIL);
}

/* 'pair?' -> VAL_BOOL - return #t if obj is a pair, else #f */
Cell* builtin_pair_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_PAIR);
}

/* 'procedure?' -> VAL_BOOL - return #t if obj is a procedure, else #f */
Cell* builtin_proc_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_PROC);
}

/* 'symbol?' -> VAL_BOOL - return #t if obj is a symbol, else #f */
Cell* builtin_sym_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_SYM);
}

/* 'string?' -> VAL_BOOL - return #t if obj is a string, else #f */
Cell* builtin_string_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_STR);
}

/* 'char?' -> VAL_BOOL - return #t if obj is a char, else #f */
Cell* builtin_char_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_CHAR);
}

/* 'vector?' -> VAL_BOOL - return #t if obj is a vector, else #f */
Cell* builtin_vector_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_VEC);
}

/* 'bytevector?' -> VAL_BOOL - return #t if obj is a byte vector, else #f */
Cell* builtin_byte_vector_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_BYTEVEC);
}

/* 'port?' -> VAL_BOOL - return #t if obj is a port, else #f */
Cell* builtin_port_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_bool(a->cell[0]->type == VAL_PORT);
}

/* 'eof-object?' -> VAL_BOOL - return #t if obj is an eof, else #f */
Cell* builtin_eof_pred(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    return make_val_err("Not implemented yet");
}

/* ---------------------------------------*
 *      Numeric identity procedures       *
 * ---------------------------------------*/

/* 'exact?' -> VAL_BOOL -  */
Cell* builtin_exact(Lex *e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == VAL_COMPLEX) {
        if (a->cell[0]->exact && a->cell[1]->exact) {
            return make_val_bool(1);
        }
        return make_val_bool(0);
    }
    return make_val_bool(a->cell[0]->exact);
}

/* 'inexact?' -> VAL_BOOL -  */
Cell* builtin_inexact(Lex *e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_INT|VAL_REAL|VAL_RAT|VAL_COMPLEX);
    if (err) { return err; }
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }

    if (a->cell[0]->type == VAL_COMPLEX) {
        if (a->cell[0]->exact && a->cell[1]->exact) {
            return make_val_bool(0);
        }
        return make_val_bool(1);
    }
     if (a->cell[0]->exact) {
         return make_val_bool(0);
     }
    return make_val_bool(1);
}

/* NOTE: these do not yet match the Scheme standard,
 * in terms of the 'tower of numeric types'. For now,
 * they simply return true if the underlying Cell type
 * matches */

/* 'complex?' -> VAL_BOOL -  */
Cell* builtin_complex(Lex *e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = VAL_COMPLEX;
    if (a->cell[0]->type & mask) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

/* 'real?' -> VAL_BOOL -  */
Cell* builtin_real(Lex *e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = VAL_REAL;
    if (a->cell[0]->type & mask) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

/* 'rational?' -> VAL_BOOL -  */
Cell* builtin_rational(Lex *e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = VAL_RAT;
    if (a->cell[0]->type & mask) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

/* 'integer?' -> VAL_BOOL -  */
Cell* builtin_integer(Lex *e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = VAL_INT;
    if (a->cell[0]->type & mask) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}

/* 'exact-integer?' -> VAL_BOOL -  */
Cell* builtin_exact_integer(Lex *e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;

    const int mask = VAL_INT;
    if ((a->cell[0]->type & mask) && (a->cell[0]->exact)) {
        return make_val_bool(1);
    }
    return make_val_bool(0);
}
/* 'finite?' -> VAL_BOOL -  */

/* 'infinite?' -> VAL_BOOL -  */


/* ---------------------------------------*
 *     Boolean and logical procedures     *
 * ---------------------------------------*/

/* 'not' -> VAL_BOOL - returns #t if obj is false, and returns #f otherwise */
Cell* builtin_not(Lex* e, Cell* a) {
    Cell* err;
    if ((err = CHECK_ARITY_EXACT(a, 1))) { return err; }
    const int is_false = (a->cell[0]->type == VAL_BOOL && a->cell[0]->b_val == 0);
    return make_val_bool(is_false);
}

/* 'boolean' -> VAL_BOOL - converts any value to a strict boolean */
Cell* builtin_boolean(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    int result = (a->cell[0]->type == VAL_BOOL)
                 ? a->cell[0]->b_val
                 : 1; /* everything except #f is true */
    return make_val_bool(result);
}

/* 'and' -> VAL_BOOL|ANY - if any expression evaluates to #f, then #f is
 * returned. Any remaining expressions are not evaluated. If all the expressions
 * evaluate to true values, the values of the last expression are returned.
 * If there are no expressions, then #t is returned.*/
Cell* builtin_and(Lex* e, Cell* a) {
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type == VAL_BOOL && a->cell[i]->b_val == 0) {
            /* first #f encountered → return a copy of it */
            return cell_copy(a->cell[i]);
        }
    }
    /* all truthy → return copy of last element */
    return cell_copy(a->cell[a->count - 1]);
}

/* 'or' -> VAL_BOOL|ANY - the value of the first expression that evaluates
 * to true is returned. Any remaining expressions are not evaluated. If all
 * expressions evaluate to #f or if there are no expressions, #f is returned */
Cell* builtin_or(Lex* e, Cell* a) {
    for (int i = 0; i < a->count; i++) {
        if (!(a->cell[i]->type == VAL_BOOL && a->cell[i]->b_val == 0)) {
            /* first truthy value → return a copy */
            return cell_copy(a->cell[i]);
        }
    }
    /* all false → return copy of last element (#f) */
    return cell_copy(a->cell[a->count - 1]);
}

/* ----------------------------------------------------------*
 *     pair/list constructors, selectors, and procedures     *
 * ----------------------------------------------------------*/

/* 'cons' -> VAL_PAIR - returns a pair made from two arguments */
Cell* builtin_cons(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    return make_val_pair(cell_copy(a->cell[0]), cell_copy(a->cell[1]));
}

/* 'car' -> ANY - returns the first member of a pair */
Cell* builtin_car(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_PAIR|VAL_SEXPR);
    if (err) { return err; }

    if (a->cell[0]->type == VAL_PAIR) {
        return cell_copy(a->cell[0]->car);
    }
    /* This is for the case where the argument list was quoted.
     * It needs to be transformed into a list before taking the car */
    Cell* list = builtin_list(e, a->cell[0]);
    Cell* result = cell_copy(list->car);
    cell_delete(list);
    return result;
}

/* 'cdr' -> ANY - returns the second member of a pair */
Cell* builtin_cdr(Lex* e, Cell* a) {
    Cell* err = check_arg_types(a, VAL_PAIR|VAL_SEXPR);
    if (err) { return err; }

    if (a->cell[0]->type == VAL_PAIR) {
        return cell_copy(a->cell[0]->cdr);
    }
    /* This is for the case where the argument list was quoted.
     * It needs to be transformed into a list before taking the cdr */
    Cell* list = builtin_list(e, a->cell[0]);
    Cell* result = cell_copy(list->cdr);
    cell_delete(list);
    return result;
}

/* 'list' -> VAL_PAIR - returns a nil-terminated proper list */
Cell* builtin_list(Lex* e, Cell* a) {
    /* start with nil */
    Cell* result = make_val_nil();

    /* build backwards so it comes out in the right order */
    for (int i = a->count - 1; i >= 0; i--) {
        result = make_val_pair(cell_copy(a->cell[i]), result);
    }
    return result;
}

/* 'length' -> VAL_INT - returns the member count of a proper list */
Cell* builtin_list_length(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, VAL_PAIR|VAL_SEXPR|VAL_NIL);
    if (err) { return err; }

    if (a->cell[0]->type == VAL_NIL) {
        return make_val_int(0);
    }
    if (a->cell[0]->type == VAL_PAIR) {
        int len = 0;
        Cell* p = a->cell[0];
        while (p->type == VAL_PAIR) {
            len++;
            p = p->cdr;
        }
        if (p->type != VAL_NIL) {
            return make_val_err("Improper list");
        }
        return make_val_int(len);
    }
    if (a->cell[0]->type == VAL_SEXPR) {
        return make_val_int(a->cell[0]->count);
    }
    return make_val_err("Oops, this shouldn't be\n");
}

/* 'list-ref' -> ANY - returns the list member at the zero-indexed
 * integer specified in the second arg. First arg is the list to act on*/
Cell* builtin_list_ref(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;

    if (a->cell[1]->type != VAL_INT) {
        return make_val_err("list-ref: arg 2 must be an integer");
    }
    int i = (int)a->cell[1]->i_val;

    if (a->cell[0]->type == VAL_PAIR) {
        Cell* p = a->cell[0];
        if (p->type != VAL_PAIR) {
            return make_val_err("list-ref: index out of bounds");
        }
        while (i > 0) {
            p = p->cdr;
            i--;
        }
        return cell_copy(p->car);
    }
    if (a->cell[0]->type == VAL_SEXPR) {
        /* else the list is buried in an VAL_SEXPR */
        return cell_copy(a->cell[0]->cell[i]);
    }
    return make_val_err("list-ref: arg 1 must be list or pair.");
}

/*-------------------------------------------------------*
 *     Vector constructors, selectors, and procedures    *
 * ------------------------------------------------------*/

Cell* builtin_vector(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_MIN(a, 1);
    if (err) return err;

    Cell *vec = make_val_vect();
    for (int i = 0; i < a->count; i++) {
        cell_add(vec, cell_copy(a->cell[i]));
    }
    return vec;
}

Cell* builtin_vector_length(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, VAL_VEC);
    if (err) return err;

    return make_val_int(a->cell[0]->count);
}

Cell* builtin_vector_ref(Lex* e, Cell* a) {
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    if (a->cell[0]->type != VAL_VEC) {
        return make_val_err("list-ref: arg 1 must be a vector");
    }
    if (a->cell[1]->type != VAL_INT) {
        return make_val_err("list-ref: arg 2 must be an integer");
    }
    int i = (int)a->cell[1]->i_val;

    if (i >= a->cell[0]->count) {
        return make_val_err("vector-ref: index out of bounds");
    }
    return cell_copy(a->cell[0]->cell[i]);
}

/*------------------------------------------------------------*
 *     Byte vector constructors, selectors, and procedures    *
 * -----------------------------------------------------------*/


/*-------------------------------------------------------*
 *     String constructors, selectors, and procedures    *
 * ------------------------------------------------------*/
