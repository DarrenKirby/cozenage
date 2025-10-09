/*
 * 'src/inexact_lib.h'
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

#ifndef COZENAGE_INEXACT_LIB_H
#define COZENAGE_INEXACT_LIB_H

#include "cell.h"


Cell* builtin_cos(const Lex* e, const Cell* a);
Cell* builtin_acos(const Lex* e, const Cell* a);
Cell* builtin_sin(const Lex* e, const Cell* a);
Cell* builtin_asin(const Lex* e, const Cell* a);
Cell* builtin_tan(const Lex* e, const Cell* a);
Cell* builtin_atan(const Lex* e, const Cell* a);
Cell* builtin_exp(const Lex* e, const Cell* a);
Cell* builtin_log(const Lex* e, const Cell* a);
Cell* builtin_log2(const Lex* e, const Cell* a);
Cell* builtin_log10(const Lex* e, const Cell* a);
Cell* builtin_sqrt(const Lex* e, const Cell* a);
Cell* builtin_cbrt(const Lex* e, const Cell* a);
Cell* builtin_infinite(const Lex* e, const Cell* a);
Cell* builtin_finite(const Lex* e, const Cell* a);
Cell* builtin_nan(const Lex* e, const Cell* a);
void lex_add_inexact_lib(const Lex* e);

#endif //COZENAGE_INEXACT_LIB_H
