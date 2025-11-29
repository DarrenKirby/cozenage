/*
 * 'src/base-lib/cxr_lib.c'
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


#include "types.h"
#include "pairs.h"


static Cell* builtin_caaar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caaar");
    if (err) { return err; }

    return car__(car__(car__(a->cell[0])));
}

static Cell* builtin_caaaar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caaaar");
    if (err) { return err; }

    return car__(car__(car__(car__(a->cell[0]))));
}

static Cell* builtin_caaddr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caaddr");
    if (err) { return err; }

    return car__(car__(cdr__(cdr__(a->cell[0]))));
}

static Cell* builtin_cadaar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cadaar");
    if (err) { return err; }

    return car__(cdr__(car__(car__(a->cell[0]))));
}

static Cell* builtin_cadar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cadar");
    if (err) { return err; }

    return car__(cdr__(car__(a->cell[0])));
}

static Cell* builtin_cadddr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cadddr");
    if (err) { return err; }

    return car__(cdr__(cdr__(cdr__(a->cell[0]))));
}

static Cell* builtin_cdaaar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdaaar");
    if (err) { return err; }

    return cdr__(car__(car__(car__(a->cell[0]))));
}

static Cell* builtin_cdaar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdaar");
    if (err) { return err; }

    return cdr__(car__(car__(a->cell[0])));
}

static Cell* builtin_cdaddr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdaddr");
    if (err) { return err; }

    return cdr__(car__(cdr__(cdr__(a->cell[0]))));
}

static Cell* builtin_cddaar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cddaar");
    if (err) { return err; }

    return cdr__(cdr__(car__(car__(a->cell[0]))));
}

static Cell* builtin_cddar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cddar");
    if (err) { return err; }

    return cdr__(cdr__(car__(a->cell[0])));
}

static Cell* builtin_cddddr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdddddr");
    if (err) { return err; }

    return cdr__(cdr__(cdr__(cdr__(a->cell[0]))));
}

static Cell* builtin_caaadr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caaaddr");
    if (err) { return err; }

    return car__(car__(car__(cdr__(a->cell[0]))));
}

static Cell* builtin_caadar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caadar");
    if (err) { return err; }

    return car__(car__(cdr__(car__(a->cell[0]))));
}

static Cell* builtin_caadr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caadr");
    if (err) { return err; }

    return car__(car__(cdr__(a->cell[0])));
}

static Cell* builtin_cadadr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cadadr");
    if (err) { return err; }

    return car__(cdr__(car__(cdr__(a->cell[0]))));
}

static Cell* builtin_caddar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caddar");
    if (err) { return err; }

    return car__(cdr__(cdr__(car__(a->cell[0]))));
}

static Cell* builtin_caddr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caddr");
    if (err) { return err; }

    return car__(cdr__(cdr__(a->cell[0])));
}

static Cell* builtin_cdaadr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdaadr");
    if (err) { return err; }

    return cdr__(car__(car__(cdr__(a->cell[0]))));
}

static Cell* builtin_cdadar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdadar");
    if (err) { return err; }

    return cdr__(car__(cdr__(car__(a->cell[0]))));
}

static Cell* builtin_cdadr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdadr");
    if (err) { return err; }

    return cdr__(car__(cdr__(a->cell[0])));
}

static Cell* builtin_cddadr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cddadr");
    if (err) { return err; }

    return cdr__(cdr__(car__(cdr__(a->cell[0]))));
}

static Cell* builtin_cdddar(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdddar");
    if (err) { return err; }

    return cdr__(cdr__(cdr__(car__(a->cell[0]))));
}

static Cell* builtin_cdddr(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdddr");
    if (err) { return err; }

    return cdr__(cdr__(cdr__(a->cell[0])));
}

void cozenage_library_init(const Lex* e) {
    lex_add_builtin(e, "caaaar", builtin_caaaar);
    lex_add_builtin(e, "caaar", builtin_caaar);
    lex_add_builtin(e, "caaddr", builtin_caaddr);
    lex_add_builtin(e, "cadaar", builtin_cadaar);
    lex_add_builtin(e, "cadar", builtin_cadar);
    lex_add_builtin(e, "cadddr", builtin_cadddr);
    lex_add_builtin(e, "cdaaar", builtin_cdaaar);
    lex_add_builtin(e, "cdaar", builtin_cdaar);
    lex_add_builtin(e, "cdaddr", builtin_cdaddr);
    lex_add_builtin(e, "cddaar", builtin_cddaar);
    lex_add_builtin(e, "cddar", builtin_cddar);
    lex_add_builtin(e, "cddddr", builtin_cddddr);
    lex_add_builtin(e, "caaadr", builtin_caaadr);
    lex_add_builtin(e, "caadar", builtin_caadar);
    lex_add_builtin(e, "caadr", builtin_caadr);
    lex_add_builtin(e, "cadadr", builtin_cadadr);
    lex_add_builtin(e, "caddar", builtin_caddar);
    lex_add_builtin(e, "caddr", builtin_caddr);
    lex_add_builtin(e, "cdaadr", builtin_cdaadr);
    lex_add_builtin(e, "cdadar", builtin_cdadar);
    lex_add_builtin(e, "cdadr", builtin_cdadr);
    lex_add_builtin(e, "cddadr", builtin_cddadr);
    lex_add_builtin(e, "cdddar", builtin_cdddar);
    lex_add_builtin(e, "cdddr", builtin_cdddr);
}
