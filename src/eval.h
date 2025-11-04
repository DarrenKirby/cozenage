/*
 * 'src/eval.h'
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

#ifndef COZENAGE_EVAL_H
#define COZENAGE_EVAL_H

#include "environment.h"
#include "special_forms.h"


typedef HandlerResult (*special_form_handler_t)(Lex*, Cell*);
Cell* coz_eval(Lex* env, Cell* expr);
Cell* coz_apply_and_get_val(const Cell* proc, const Cell* args, const Lex* env);

#endif //COZENAGE_EVAL_H
