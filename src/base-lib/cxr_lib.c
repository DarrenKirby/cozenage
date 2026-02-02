/*
 * 'src/base-lib/cxr_lib.c'
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

/*
 * The (base cxr) library exports twenty-four procedures which are the compositions
 * of from three to four car and cdr operations. For example caddar could be defined by:
 *
 *  (define caddar
 *     (lambda (x) (car (cdr (cdr (car x))))))
 *
 * The procedures car and cdr themselves and the four two-level compositions are included
 * in the core interpreter.
 * 
 * TODO: define the first, second, third... helpers in this file?
 */

#include "types.h"
#include "pairs.h"


/* The call signature for all 24 cxr procedures is the same:
 *
 * (c*r pair)
 */

static Cell* cxr_caaar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caaar");
    if (err) { return err; }

    return car__(car__(car__(a->cell[0])));
}


static Cell* cxr_caaaar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caaaar");
    if (err) { return err; }

    return car__(car__(car__(car__(a->cell[0]))));
}


static Cell* cxr_caaddr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caaddr");
    if (err) { return err; }

    return car__(car__(cdr__(cdr__(a->cell[0]))));
}


static Cell* cxr_cadaar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cadaar");
    if (err) { return err; }

    return car__(cdr__(car__(car__(a->cell[0]))));
}


static Cell* cxr_cadar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cadar");
    if (err) { return err; }

    return car__(cdr__(car__(a->cell[0])));
}


static Cell* cxr_cadddr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cadddr");
    if (err) { return err; }

    return car__(cdr__(cdr__(cdr__(a->cell[0]))));
}


static Cell* cxr_cdaaar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdaaar");
    if (err) { return err; }

    return cdr__(car__(car__(car__(a->cell[0]))));
}


static Cell* cxr_cdaar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdaar");
    if (err) { return err; }

    return cdr__(car__(car__(a->cell[0])));
}


static Cell* cxr_cdaddr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdaddr");
    if (err) { return err; }

    return cdr__(car__(cdr__(cdr__(a->cell[0]))));
}


static Cell* cxr_cddaar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cddaar");
    if (err) { return err; }

    return cdr__(cdr__(car__(car__(a->cell[0]))));
}


static Cell* cxr_cddar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cddar");
    if (err) { return err; }

    return cdr__(cdr__(car__(a->cell[0])));
}


static Cell* cxr_cddddr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdddddr");
    if (err) { return err; }

    return cdr__(cdr__(cdr__(cdr__(a->cell[0]))));
}


static Cell* cxr_caaadr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caaaddr");
    if (err) { return err; }

    return car__(car__(car__(cdr__(a->cell[0]))));
}


static Cell* cxr_caadar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caadar");
    if (err) { return err; }

    return car__(car__(cdr__(car__(a->cell[0]))));
}


static Cell* cxr_caadr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caadr");
    if (err) { return err; }

    return car__(car__(cdr__(a->cell[0])));
}


static Cell* cxr_cadadr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cadadr");
    if (err) { return err; }

    return car__(cdr__(car__(cdr__(a->cell[0]))));
}


static Cell* cxr_caddar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caddar");
    if (err) { return err; }

    return car__(cdr__(cdr__(car__(a->cell[0]))));
}


static Cell* cxr_caddr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caddr");
    if (err) { return err; }

    return car__(cdr__(cdr__(a->cell[0])));
}


static Cell* cxr_cdaadr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdaadr");
    if (err) { return err; }

    return cdr__(car__(car__(cdr__(a->cell[0]))));
}


static Cell* cxr_cdadar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdadar");
    if (err) { return err; }

    return cdr__(car__(cdr__(car__(a->cell[0]))));
}


static Cell* cxr_cdadr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdadr");
    if (err) { return err; }

    return cdr__(car__(cdr__(a->cell[0])));
}


static Cell* cxr_cddadr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cddadr");
    if (err) { return err; }

    return cdr__(cdr__(car__(cdr__(a->cell[0]))));
}


static Cell* cxr_cdddar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdddar");
    if (err) { return err; }

    return cdr__(cdr__(cdr__(car__(a->cell[0]))));
}


static Cell* cxr_cdddr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdddr");
    if (err) { return err; }

    return cdr__(cdr__(cdr__(a->cell[0])));
}


void cozenage_library_init(const Lex* e) {
    lex_add_builtin(e, "caaaar", cxr_caaaar);
    lex_add_builtin(e, "caaar", cxr_caaar);
    lex_add_builtin(e, "caaddr", cxr_caaddr);
    lex_add_builtin(e, "cadaar", cxr_cadaar);
    lex_add_builtin(e, "cadar", cxr_cadar);
    lex_add_builtin(e, "cadddr", cxr_cadddr);
    lex_add_builtin(e, "cdaaar", cxr_cdaaar);
    lex_add_builtin(e, "cdaar", cxr_cdaar);
    lex_add_builtin(e, "cdaddr", cxr_cdaddr);
    lex_add_builtin(e, "cddaar", cxr_cddaar);
    lex_add_builtin(e, "cddar", cxr_cddar);
    lex_add_builtin(e, "cddddr", cxr_cddddr);
    lex_add_builtin(e, "caaadr", cxr_caaadr);
    lex_add_builtin(e, "caadar", cxr_caadar);
    lex_add_builtin(e, "caadr", cxr_caadr);
    lex_add_builtin(e, "cadadr", cxr_cadadr);
    lex_add_builtin(e, "caddar", cxr_caddar);
    lex_add_builtin(e, "caddr", cxr_caddr);
    lex_add_builtin(e, "cdaadr", cxr_cdaadr);
    lex_add_builtin(e, "cdadar", cxr_cdadar);
    lex_add_builtin(e, "cdadr", cxr_cdadr);
    lex_add_builtin(e, "cddadr", cxr_cddadr);
    lex_add_builtin(e, "cdddar", cxr_cdddar);
    lex_add_builtin(e, "cdddr", cxr_cdddr);
}
