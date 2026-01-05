/*
 * 'src/symbols.c'
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

#include "symbols.h"
#include "cell.h"
#include "types.h"
#include "main.h"

/* For _POSIX_VERSION */
#include <unistd.h>
/* For uname */
#include <string.h>
#include <errno.h>
#include <sys/utsname.h>


/* Declare the symbol table. */
ht_table* symbol_table = nullptr;

/* Definitions of Global Symbols for Special Forms. */
Cell* G_define_sym = nullptr;
Cell* G_quote_sym = nullptr;
Cell* G_lambda_sym = nullptr;
Cell* G_if_sym = nullptr;
Cell* G_when_sym = nullptr;
Cell* G_unless_sym = nullptr;
Cell* G_cond_sym = nullptr;
Cell* G_case_sym = nullptr;
Cell* G_import_sym = nullptr;
Cell* G_let_sym = nullptr;
Cell* G_let_star_sym = nullptr;
Cell* G_letrec_star_sym = nullptr;
Cell* G_letrec_sym = nullptr;
Cell* G_set_bang_sym = nullptr;
Cell* G_begin_sym = nullptr;
Cell* G_and_sym = nullptr;
Cell* G_or_sym = nullptr;
Cell* G_do_sym = nullptr;
Cell* G_arrow_sym = nullptr;
Cell* G_else_sym = nullptr;
Cell* G_defmacro_sym = nullptr;
Cell* G_debug_sym = nullptr;
Cell* G_quasiquote_sym = nullptr;
Cell* G_unquote_sym = nullptr;
Cell* G_unquote_splicing_sym = nullptr;


/* Initialize canonical symbols and configure their special form IDs. */
void init_special_forms(void) {
    /* Primitive syntax. */
    G_define_sym = make_cell_symbol("define");
    G_define_sym->sf_id = SF_ID_DEFINE;

    /* Primitive syntax. */
    G_quote_sym = make_cell_symbol("quote");
    G_quote_sym->sf_id = SF_ID_QUOTE;

    /* Primitive syntax. */
    G_lambda_sym = make_cell_symbol("lambda");
    G_lambda_sym->sf_id = SF_ID_LAMBDA;

    /* Primitive syntax. */
    G_if_sym = make_cell_symbol("if");
    G_if_sym->sf_id = SF_ID_IF;

    /* Transformed syntax. */
    G_when_sym = make_cell_symbol("when");
    G_when_sym->sf_id = SF_ID_WHEN;

    /* Transformed syntax. */
    G_unless_sym = make_cell_symbol("unless");
    G_unless_sym->sf_id = SF_ID_UNLESS;

    /* Transformed syntax. */
    G_cond_sym = make_cell_symbol("cond");
    G_cond_sym->sf_id = SF_ID_COND;

    /* Transformed syntax. */
    G_case_sym = make_cell_symbol("case");
    G_case_sym->sf_id = SF_ID_CASE;

    /* Primitive syntax. */
    G_import_sym = make_cell_symbol("import");
    G_import_sym->sf_id = SF_ID_IMPORT;

    /* Primitive syntax. */
    G_let_sym = make_cell_symbol("let");
    G_let_sym->sf_id = SF_ID_LET;

    /* Transformed syntax. */
    G_let_star_sym = make_cell_symbol("let*");
    G_let_star_sym->sf_id = SF_ID_LET_STAR;

    /* Transformed syntax. */
    G_letrec_star_sym = make_cell_symbol("letrec*");
    G_letrec_star_sym->sf_id = SF_ID_LETREC_STAR;

    /* Primitive syntax. */
    G_letrec_sym = make_cell_symbol("letrec");
    G_letrec_sym->sf_id = SF_ID_LETREC;

    /* Primitive syntax. */
    G_set_bang_sym = make_cell_symbol("set!");
    G_set_bang_sym->sf_id = SF_ID_SET_BANG;

    /* Primitive syntax. */
    G_begin_sym = make_cell_symbol("begin");
    G_begin_sym->sf_id = SF_ID_BEGIN;

    /* Primitive syntax. */
    G_and_sym = make_cell_symbol("and");
    G_and_sym->sf_id = SF_ID_AND;

    /* Primitive syntax. */
    G_defmacro_sym = make_cell_symbol("defmacro");
    G_defmacro_sym->sf_id = SF_ID_DEFMACRO;

    /* Transformed syntax. */
    G_or_sym = make_cell_symbol("or");
    G_or_sym->sf_id = SF_ID_OR;

    /* Transformed syntax */
    G_do_sym = make_cell_symbol("do");
    G_do_sym->sf_id = SF_ID_DO;

    /* This is basically just a sentinel object. */
    G_else_sym = make_cell_symbol("else");
    G_else_sym->sf_id = SF_ID_ELSE;

    G_debug_sym = make_cell_symbol("with-gc-stats");
    G_debug_sym->sf_id = SF_ID_DEBUG;

    /* Not actually special forms - but symbols that should
     * be interned on startup - no SF_IDs */
    G_arrow_sym = make_cell_symbol("=>");
    G_quasiquote_sym = make_cell_symbol("quasiquote");
    G_unquote_sym = make_cell_symbol("unquote");
    G_unquote_splicing_sym = make_cell_symbol("unquote-splicing");
}


/*-------------------------------------------------------*
 *                  Symbol procedures                    *
 * ------------------------------------------------------*/


Cell* builtin_symbol_equal_pred(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = check_arg_types(a, CELL_SYMBOL, "symbol=?");
    if (err) return err;
    err = CHECK_ARITY_MIN(a, 1, "symbol=?");
    if (err) return err;

    for (int i = 0; i < a->count - 1; i++) {
        const char* lhs = a->cell[i]->sym;
        const char* rhs = a->cell[i+1]->sym;

        if (lhs != rhs)  {
            return False_Obj;
        }
    }
    /* If we get here, we're equal. */
    return True_Obj;
}


/* (string->symbol string )
 * Returns the symbol whose name is string. This procedure can create symbols with names containing special characters
 * that would require escaping when written, but does not interpret escapes in its input. */
Cell* builtin_string_to_symbol(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "string->symbol");
    if (err) return err;
    if (a->cell[0]->type != CELL_STRING) {
        return make_cell_error(
            "string->symbol: arg 1 must be a string",
            TYPE_ERR);
    }
    return make_cell_symbol(a->cell[0]->str);
}


/* (symbol->string symbol)
 * Returns the name of symbol as a string, but without adding escapes. It is an error to apply mutation procedures like
 * string-set! to strings returned by this procedure. */
Cell* builtin_symbol_to_string(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 1, "symbol->string");
    if (err) return err;
    if (a->cell[0]->type != CELL_SYMBOL) {
        return make_cell_error(
            "symbol->string: arg 1 must be a symbol",
            TYPE_ERR);
    }
    return make_cell_string(a->cell[0]->sym);
}


/* (features)
 * Returns a list of the feature identifiers which cond-expand treats as true. It is an error to modify this list. */
Cell* builtin_features(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_EXACT(a, 0, "features");
    if (err) return err;

    Cell* result_l = make_cell_sexpr();
    /* Start by adding all the builtin features always present. */
    char* feature_array[] =  {
        "exact-closed", "exact-complex", "ieee-float", "full-unicode", "ratios"
    };
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const int feature_count = sizeof(feature_array) / sizeof(feature_array[0]);

    for (int i = 0; i < feature_count; i++) {
        cell_add(result_l, make_cell_symbol(feature_array[i]));
    }

#ifdef _POSIX_VERSION
    cell_add(result_l, make_cell_symbol("posix"));
#endif

    struct utsname uts;
    if (uname(&uts) == -1) {
        fprintf(stderr, "uname failed: %s\n", strerror(errno));
    }
    /* Add OS and arch. */
    cell_add(result_l, make_cell_symbol(uts.sysname));
    cell_add(result_l, make_cell_symbol(uts.machine));

    /* Check byte order. */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    const char* endianess = "little-endian";
#else
    const char* endianess = "big-endian";
#endif
    cell_add(result_l, make_cell_symbol(endianess));

    /* Add Cozenage name and version. */
    cell_add(result_l, make_cell_symbol(APP_NAME));

    char buf[64];
    snprintf(buf, sizeof(buf), "%s-%s", APP_NAME, APP_VERSION);
    cell_add(result_l, make_cell_symbol(buf));

    return make_list_from_sexpr(result_l);
}
