/*
 * 'symbols.c'
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

#include "symbols.h"
#include "cell.h"


/* Declare the symbol table */
ht_table* symbol_table = nullptr;

/* Definitions of Global Symbols for Special Forms */
Cell* G_define_sym = nullptr;
Cell* G_quote_sym = nullptr;
Cell* G_lambda_sym = nullptr;
Cell* G_if_sym = nullptr;
Cell* G_when_sym = nullptr;
Cell* G_unless_sym = nullptr;
Cell* G_cond_sym = nullptr;
Cell* G_import_sym = nullptr;
Cell* G_let_sym = nullptr;
Cell* G_let_star_sym = nullptr;
Cell* G_letrec_sym = nullptr;
Cell* G_set_bang_sym = nullptr;
Cell* G_begin_sym = nullptr;
Cell* G_and_sym = nullptr;
Cell* G_or_sym = nullptr;

/* Initialize canonical symbols and configure their special form IDs */
void init_special_forms(void) {
    G_define_sym = make_cell_symbol("define");
    G_define_sym->sf_id = SF_ID_DEFINE;

    G_quote_sym = make_cell_symbol("quote");
    G_quote_sym->sf_id = SF_ID_QUOTE;

    G_lambda_sym = make_cell_symbol("lambda");
    G_lambda_sym->sf_id = SF_ID_LAMBDA;

    G_if_sym = make_cell_symbol("if");
    G_if_sym->sf_id = SF_ID_IF;

    G_when_sym = make_cell_symbol("when");
    G_when_sym->sf_id = SF_ID_WHEN;

    G_unless_sym = make_cell_symbol("unless");
    G_unless_sym->sf_id = SF_ID_UNLESS;

    G_cond_sym = make_cell_symbol("cond");
    G_cond_sym->sf_id = SF_ID_COND;

    G_import_sym = make_cell_symbol("import");
    G_import_sym->sf_id = SF_ID_IMPORT;

    G_let_sym = make_cell_symbol("let");
    G_let_sym->sf_id = SF_ID_LET;

    G_let_star_sym = make_cell_symbol("let*");
    G_let_star_sym->sf_id = SF_ID_LET_STAR;

    G_letrec_sym = make_cell_symbol("letrec");
    G_letrec_sym->sf_id = SF_ID_LETREC;

    G_set_bang_sym = make_cell_symbol("set!");
    G_set_bang_sym->sf_id = SF_ID_SET_BANG;

    G_begin_sym = make_cell_symbol("begin");
    G_begin_sym->sf_id = SF_ID_BEGIN;

    G_and_sym = make_cell_symbol("and");
    G_and_sym->sf_id = SF_ID_AND;

    G_or_sym = make_cell_symbol("or");
    G_or_sym->sf_id = SF_ID_OR;
}
