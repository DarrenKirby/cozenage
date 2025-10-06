/*
* 'src/complex_lib.c'
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

#include "complex_lib.h"
#include "types.h"
#include "numerics.h"
#include <math.h>


/* 'real-part' -> CELL_REAL|CELL_RATIONAL|CELL_INTEGER - returns the real part of a
 * complex number */
Cell* builtin_real_part(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    Cell* sub = a->cell[0];
    if (sub->type == CELL_INTEGER ||
        sub->type == CELL_REAL ||
        sub->type == CELL_RATIONAL) {
        return sub;
    }
    if (sub->type == CELL_COMPLEX) {
        return sub->real;
    }
    /* If we didn't return early, we have the wrong arg type */
    return check_arg_types(make_sexpr_len1(sub), CELL_COMPLEX|CELL_REAL|CELL_RATIONAL|CELL_INTEGER);
}

/* 'imag-part' -> CELL_REAL|CELL_RATIONAL|CELL_INTEGER - returns the imaginary part of a
 * complex number */
Cell* builtin_imag_part(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    const Cell* sub = a->cell[0];
    if (sub->type == CELL_INTEGER ||
        sub->type == CELL_REAL ||
        sub->type == CELL_RATIONAL) {
        return make_cell_integer(0);
        }
    if (sub->type == CELL_COMPLEX) {
        return sub->imag;
    }
    /* If we didn't return early, we have the wrong arg type */
    return check_arg_types(make_sexpr_len1(sub), CELL_COMPLEX|CELL_REAL|CELL_RATIONAL|CELL_INTEGER);
}

/* 'make-rectangular' -> CELL_COMPLEX- convert a complex number to rectangular form */
Cell* builtin_make_rectangular(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    err = check_arg_types(a, CELL_REAL|CELL_RATIONAL|CELL_INTEGER);
    if (err) return err;

    return make_cell_complex(a->cell[0], a->cell[1]);
}

/* 'angle' -> CELL_REAL- calculate angle 'θ' of complex number */
Cell* builtin_angle(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1);
    if (err) return err;
    err = check_arg_types(a, CELL_REAL|CELL_RATIONAL|CELL_INTEGER|CELL_COMPLEX);
    if (err) return err;

    const Cell* arg = a->cell[0];
    switch (arg->type) {
        case CELL_COMPLEX: {
            const long double r = cell_to_long_double(arg->real);
            const long double i = cell_to_long_double(arg->imag);
            return make_cell_from_double(atan2l(i, r));
        }
        default:
            return make_cell_from_double(atan2l(0, cell_to_long_double(arg)));
    }
}

/* 'make-polar' -> CELL_COMPLEX- convert a complex number to polar form */
Cell* builtin_make_polar(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 2);
    if (err) return err;
    err = check_arg_types(a, CELL_REAL|CELL_RATIONAL|CELL_INTEGER);
    if (err) return err;

    const long double M = cell_to_long_double(a->cell[0]);
    const long double A = cell_to_long_double(a->cell[1]);

    const long double real_part = M * cosl(A);
    const long double imag_part = M * sinl(A);

    return make_cell_complex(
        make_cell_from_double(real_part),
        make_cell_from_double(imag_part)
        );
}

/* Loader for our (scheme complex) library procedures */
void lex_add_complex_lib(Lex* e) {
    lex_add_builtin(e, "real-part", builtin_real_part);
    lex_add_builtin(e, "imag-part", builtin_imag_part);
    lex_add_builtin(e,"make-rectangular", builtin_make_rectangular);
    /* 'magnitude' is identical to 'abs' for real/complex numbers -
     * so we just make an alias */
    lex_add_builtin(e,"magnitude", builtin_abs);
    lex_add_builtin(e,"angle", builtin_angle);
    lex_add_builtin(e,"make-polar", builtin_make_polar);
}
