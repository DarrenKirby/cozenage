/*
 * 'special_forms.h'
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

#ifndef COZENAGE_SPECIAL_FORMS_H
#define COZENAGE_SPECIAL_FORMS_H

#include "cell.h"


typedef enum {
    ACTION_RETURN,   /* The handler produced a final value. Exit the eval loop. */
    ACTION_CONTINUE  /* The handler produced a new expression. Continue the loop. */
} HandlerAction;

typedef struct {
    HandlerAction action;
    Cell* value;     /* The final value OR the next expression */
} HandlerResult;

int is_syntactic_keyword(const char* s);
Cell* sexpr_to_list(Cell* c);
Lex* build_lambda_env(const Lex* env, const Cell* formals, const Cell* args);
Cell* sequence_sf_body(const Cell* body);
/* Special forms */
HandlerResult sf_define(Lex* e, Cell* a);
HandlerResult sf_quote(Lex* e, Cell* a);
HandlerResult sf_lambda(Lex* e, Cell* a);;
HandlerResult sf_if(Lex* e, Cell* a);
HandlerResult sf_when(Lex* e, Cell* a);
HandlerResult sf_unless(Lex* e, Cell* a);
HandlerResult sf_cond(Lex* e, Cell* a);
HandlerResult sf_else(Lex* e, Cell* a);
HandlerResult sf_import(Lex* e, Cell* a);
HandlerResult sf_let(Lex* e, Cell* a);
HandlerResult sf_let_star(Lex* e, Cell* a);
HandlerResult sf_letrec(Lex* e, Cell* a);
HandlerResult sf_set_bang(Lex* e, Cell* a);
HandlerResult sf_begin(Lex* e, Cell* a);
HandlerResult sf_or(Lex* e, Cell* a);
HandlerResult sf_and(Lex* e, Cell* a);

#endif //COZENAGE_SPECIAL_FORMS_H
