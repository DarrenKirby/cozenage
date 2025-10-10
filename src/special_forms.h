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


int is_syntactic_keyword(const char* s);
Cell* sexpr_to_list(Cell* c);
Cell* apply_lambda(Cell* lambda, const Cell* args);
/* Special forms */
Cell* sf_define(Lex* e, const Cell* a);
Cell* sf_quote(const Lex* e, Cell* a);
Cell* sf_lambda(Lex* e, Cell* a);;
Cell* sf_if(Lex* e, const Cell* a);
Cell* sf_when(Lex* e, const Cell* a);
Cell* sf_unless(Lex* e, const Cell* a);
Cell* sf_cond(Lex* e, const Cell* a);
Cell* sf_else(const Lex* e, const Cell* a);
Cell* sf_import(const Lex* e, const Cell* a);
Cell* sf_let(Lex* e, Cell* a);
Cell* sf_let_star(Lex* e, Cell* a);
Cell* sf_set_bang(Lex* e, const Cell* a);
Cell* sf_begin(Lex* e, const Cell* a);
Cell* sf_or(Lex* e, const Cell* a);
Cell* sf_and(Lex* e, const Cell* a);

#endif //COZENAGE_SPECIAL_FORMS_H
