/*
 * 'cxr_lib.h'
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

#ifndef COZENAGE_CXR_LIB_H
#define COZENAGE_CXR_LIB_H

#include "types.h"


Cell* builtin_caaar(const Lex* e, const Cell* a);
Cell* builtin_caaaar(const Lex* e, const Cell* a);
Cell* builtin_caaddr(const Lex* e, const Cell* a);
Cell* builtin_cadaar(const Lex* e, const Cell* a);
Cell* builtin_cadar(const Lex* e, const Cell* a);
Cell* builtin_cadddr(const Lex* e, const Cell* a);
Cell* builtin_cdaaar(const Lex* e, const Cell* a);
Cell* builtin_cdaar(const Lex* e, const Cell* a);
Cell* builtin_cdaddr(const Lex* e, const Cell* a);
Cell* builtin_cddaar(const Lex* e, const Cell* a);
Cell* builtin_cddar(const Lex* e, const Cell* a);
Cell* builtin_cddddr(const Lex* e, const Cell* a);
Cell* builtin_caaadr(const Lex* e, const Cell* a);
Cell* builtin_caadar(const Lex* e, const Cell* a);
Cell* builtin_caadr(const Lex* e, const Cell* a);
Cell* builtin_cadadr(const Lex* e, const Cell* a);
Cell* builtin_caddar(const Lex* e, const Cell* a);
Cell* builtin_caddr(const Lex* e, const Cell* a);
Cell* builtin_cdaadr(const Lex* e, const Cell* a);
Cell* builtin_cdadar(const Lex* e, const Cell* a);
Cell* builtin_cdadr(const Lex* e, const Cell* a);
Cell* builtin_cddadr(const Lex* e, const Cell* a);
Cell* builtin_cdddar(const Lex* e, const Cell* a);
Cell* builtin_cdddr(const Lex* e, const Cell* a);
void lex_add_cxr_lib(Lex* e);

#endif //COZENAGE_CXR_LIB_H
