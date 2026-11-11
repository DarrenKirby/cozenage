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
 * It also exports Racket-style positional list accessors, up to 'tenth':
 *
 *  (first '(1 2 3 4 5))
 *  1
 *  (fifth '(1 2 3 4 5))
 *  5
 */

#include "load_library.h"
#include "pairs.h"
#include "types.h"


/* The call signature for all 24 cxr procedures is the same:
 *
 * (c*r pair)
 */

static Cell* cxr_caaar( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caaar");
    if (err) { return err; }

    return car__(car__(car__(a->cell[0])));
}


static Cell* cxr_caaaar( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caaaar");
    if (err) { return err; }

    return car__(car__(car__(car__(a->cell[0]))));
}


static Cell* cxr_caaddr( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caaddr");
    if (err) { return err; }

    return car__(car__(cdr__(cdr__(a->cell[0]))));
}


static Cell* cxr_cadaar( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cadaar");
    if (err) { return err; }

    return car__(cdr__(car__(car__(a->cell[0]))));
}


static Cell* cxr_cadar( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cadar");
    if (err) { return err; }

    return car__(cdr__(car__(a->cell[0])));
}


static Cell* cxr_cadddr( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cadddr");
    if (err) { return err; }

    return car__(cdr__(cdr__(cdr__(a->cell[0]))));
}


static Cell* cxr_cdaaar( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdaaar");
    if (err) { return err; }

    return cdr__(car__(car__(car__(a->cell[0]))));
}


static Cell* cxr_cdaar( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdaar");
    if (err) { return err; }

    return cdr__(car__(car__(a->cell[0])));
}


static Cell* cxr_cdaddr( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdaddr");
    if (err) { return err; }

    return cdr__(car__(cdr__(cdr__(a->cell[0]))));
}


static Cell* cxr_cddaar( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cddaar");
    if (err) { return err; }

    return cdr__(cdr__(car__(car__(a->cell[0]))));
}


static Cell* cxr_cddar( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cddar");
    if (err) { return err; }

    return cdr__(cdr__(car__(a->cell[0])));
}


static Cell* cxr_cddddr( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdddddr");
    if (err) { return err; }

    return cdr__(cdr__(cdr__(cdr__(a->cell[0]))));
}


static Cell* cxr_caaadr( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caaaddr");
    if (err) { return err; }

    return car__(car__(car__(cdr__(a->cell[0]))));
}


static Cell* cxr_caadar( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caadar");
    if (err) { return err; }

    return car__(car__(cdr__(car__(a->cell[0]))));
}


static Cell* cxr_caadr( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caadr");
    if (err) { return err; }

    return car__(car__(cdr__(a->cell[0])));
}


static Cell* cxr_cadadr( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cadadr");
    if (err) { return err; }

    return car__(cdr__(car__(cdr__(a->cell[0]))));
}


static Cell* cxr_caddar( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caddar");
    if (err) { return err; }

    return car__(cdr__(cdr__(car__(a->cell[0]))));
}


static Cell* cxr_caddr( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "caddr");
    if (err) { return err; }

    return car__(cdr__(cdr__(a->cell[0])));
}


static Cell* cxr_cdaadr( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdaadr");
    if (err) { return err; }

    return cdr__(car__(car__(cdr__(a->cell[0]))));
}


static Cell* cxr_cdadar( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdadar");
    if (err) { return err; }

    return cdr__(car__(cdr__(car__(a->cell[0]))));
}


static Cell* cxr_cdadr( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdadr");
    if (err) { return err; }

    return cdr__(car__(cdr__(a->cell[0])));
}


static Cell* cxr_cddadr( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cddadr");
    if (err) { return err; }

    return cdr__(cdr__(car__(cdr__(a->cell[0]))));
}


static Cell* cxr_cdddar( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdddar");
    if (err) { return err; }

    return cdr__(cdr__(cdr__(car__(a->cell[0]))));
}


static Cell* cxr_cdddr( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "cdddr");
    if (err) { return err; }

    return cdr__(cdr__(cdr__(a->cell[0])));
}


/* Racket-style list accessors. */

static Cell* cxr_first( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "first");
    if (err) { return err; }

    return car__(a->cell[0]);
}


static Cell* cxr_second( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "second");
    if (err) { return err; }

    return car__(cdr__(a->cell[0]));
}


static Cell* cxr_third( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "third");
    if (err) { return err; }

    return car__(cdr__(cdr__(a->cell[0])));
}


static Cell* cxr_fourth( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "fourth");
    if (err) { return err; }

    return car__(cdr__(cdr__(cdr__(a->cell[0]))));
}


static Cell* cxr_fifth( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "fifth");
    if (err) { return err; }

    return car__(cdr__(cdr__(cdr__(cdr__(
        a->cell[0])))));
}


static Cell* cxr_sixth( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "sixth");
    if (err) { return err; }

    return car__(cdr__(cdr__(cdr__(cdr__(
        cdr__(a->cell[0]))))));
}


static Cell* cxr_seventh( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "seventh");
    if (err) { return err; }

    return car__(cdr__(cdr__(cdr__(cdr__(
        cdr__(cdr__(a->cell[0])))))));
}


static Cell* cxr_eighth( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "eighth");
    if (err) { return err; }

    return car__(cdr__(cdr__(cdr__(cdr__(
        cdr__(cdr__(cdr__(a->cell[0]))))))));
}


static Cell* cxr_ninth( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "ninth");
    if (err) { return err; }

    return car__(cdr__(cdr__(cdr__(cdr__(
        cdr__(cdr__(cdr__(cdr__(a->cell[0])))))))));
}


static Cell* cxr_tenth( [[ maybe_unused ]] const Lex* e, const Cell* a)
{
    Cell* err = CHECK_ARITY_EXACT(a, 1, "tenth");
    if (err) { return err; }

    return car__(cdr__(cdr__(cdr__(cdr__(
        cdr__(cdr__(cdr__(cdr__(cdr__(a->cell[0]))))))))));
}


static const CznExport cxr_exports[] = {
    { .scheme_name = "caaaar",  .func = cxr_caaaar },
    { .scheme_name = "caaar",   .func = cxr_caaar },
    { .scheme_name = "caaddr",  .func = cxr_caaddr },
    { .scheme_name = "cadaar",  .func = cxr_cadaar },
    { .scheme_name = "cadar",   .func = cxr_cadar },
    { .scheme_name = "cadddr",  .func = cxr_cadddr },
    { .scheme_name = "cdaaar",  .func = cxr_cdaaar },
    { .scheme_name = "cdaar",   .func = cxr_cdaar },
    { .scheme_name = "cdaddr",  .func = cxr_cdaddr },
    { .scheme_name = "cddaar",  .func = cxr_cddaar },
    { .scheme_name = "cddar",   .func = cxr_cddar },
    { .scheme_name = "cddddr",  .func = cxr_cddddr },
    { .scheme_name = "caaadr",  .func = cxr_caaadr },
    { .scheme_name = "caadar",  .func = cxr_caadar },
    { .scheme_name = "caadr",   .func = cxr_caadr },
    { .scheme_name = "cadadr",  .func = cxr_cadadr },
    { .scheme_name = "caddar",  .func = cxr_caddar },
    { .scheme_name = "caddr",   .func = cxr_caddr },
    { .scheme_name = "cdaadr",  .func = cxr_cdaadr },
    { .scheme_name = "cdadar",  .func = cxr_cdadar },
    { .scheme_name = "cdadr",   .func = cxr_cdadr },
    { .scheme_name = "cddadr",  .func = cxr_cddadr },
    { .scheme_name = "cdddar",  .func = cxr_cdddar },
    { .scheme_name = "cdddr",   .func = cxr_cdddr },
    { .scheme_name = "first",   .func = cxr_first },
    { .scheme_name = "second",  .func = cxr_second },
    { .scheme_name = "third",   .func = cxr_third },
    { .scheme_name = "fourth",  .func = cxr_fourth},
    { .scheme_name = "fifth",   .func = cxr_fifth},
    { .scheme_name = "sixth",   .func = cxr_sixth},
    { .scheme_name = "seventh", .func = cxr_seventh},
    { .scheme_name = "eighth",  .func = cxr_eighth},
    { .scheme_name = "ninth",   .func = cxr_ninth},
    { .scheme_name = "tenth",   .func = cxr_tenth},
};


static const CznExportTable cxr_table = {
    .exports = cxr_exports,
    .count   = sizeof(cxr_exports) / sizeof(cxr_exports[0]),
};


extern const CznExportTable* cozenage_library_init()
{
    return &cxr_table;
}
