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

#include "types.h"


/* Special forms */
Cell* builtin_define(Lex* e, Cell* a);
Cell* builtin_if(Lex* e, Cell* a);
Cell* builtin_when(Lex* e, Cell* a);
Cell* builtin_unless(Lex* e, Cell* a);
Cell* builtin_cond(Lex* e, Cell* a);
Cell* builtin_else(Lex* e, Cell* a);
Cell* builtin_import(Lex* e, Cell* a);

#endif //COZENAGE_SPECIAL_FORMS_H
