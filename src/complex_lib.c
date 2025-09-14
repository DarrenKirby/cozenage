#include "complex_lib.h"
#include "ops.h"
#include <math.h>


Cell* builtin_real_part(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    Cell* sub = a->cell[0];
    if (sub->type == VAL_INT ||
        sub->type == VAL_REAL ||
        sub->type == VAL_RAT) {
        return sub;
    }
    if (sub->type == VAL_COMPLEX) {
        return sub->real;
    }
    return check_arg_types(make_sexpr_len1(sub), VAL_COMPLEX|VAL_REAL|VAL_RAT|VAL_INT);
}

Cell* builtin_imag_part(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    const Cell* sub = a->cell[0];
    if (sub->type == VAL_INT ||
        sub->type == VAL_REAL ||
        sub->type == VAL_RAT) {
        return make_val_int(0);
        }
    if (sub->type == VAL_COMPLEX) {
        return sub->imag;
    }
    return check_arg_types(make_sexpr_len1(sub), VAL_COMPLEX|VAL_REAL|VAL_RAT|VAL_INT);
}

Cell* builtin_make_rectangular(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    err = check_arg_types(a, VAL_REAL|VAL_RAT|VAL_INT);
    if (err) return err;

    return make_val_complex(a->cell[0], a->cell[1]);
}

Cell* builtin_angle(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, VAL_REAL|VAL_RAT|VAL_INT|VAL_COMPLEX);
    if (err) return err;

    const Cell* arg = a->cell[0];
    switch (arg->type) {
        case VAL_COMPLEX: {
            const long double r = cell_to_long_double(arg->real);
            const long double i = cell_to_long_double(arg->imag);
            return make_cell_from_double(atan2l(i, r));
        }
        default:
            return make_cell_from_double(atan2l(0, cell_to_long_double(arg)));
    }
}

Cell* builtin_make_polar(Lex* e, Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    err = check_arg_types(a, VAL_REAL|VAL_RAT|VAL_INT);
    if (err) return err;

    const long double M = cell_to_long_double(a->cell[0]);
    const long double A = cell_to_long_double(a->cell[1]);

    const long double real_part = M * cosl(A);
    const long double imag_part = M * sinl(A);

    return make_val_complex(
        make_cell_from_double(real_part),
        make_cell_from_double(imag_part)
        );
}

void lex_add_complex_lib(Lex* e) {
    lex_add_builtin(e, "real-part", builtin_real_part);
    lex_add_builtin(e, "imag-part", builtin_imag_part);
    lex_add_builtin(e,"make-rectangular", builtin_make_rectangular);
    lex_add_builtin(e,"magnitude", builtin_abs);
    lex_add_builtin(e,"angle", builtin_angle);
    lex_add_builtin(e,"make-polar", builtin_make_polar);
}
