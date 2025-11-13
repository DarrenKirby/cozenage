/*
 * 'src/scheme-lib/time_lib.h'
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

#ifndef COZENAGE_TIME_LIB_H
#define COZENAGE_TIME_LIB_H

#include "cell.h"


/* R7RSs "suitable constant" (TAI-UTC offset)
 * As of 2025, this is 37.0 seconds. */
#define TAI_UTC_OFFSET 37.0;

Cell* builtin_current_second(const Lex* e, const Cell* a);
Cell* builtin_current_jiffy(const Lex* e, const Cell* a);
Cell* builtin_jiffies_per_second(const Lex* e, const Cell* a);
Cell* builtin_current_datetime_utc(const Lex* e, const Cell* a);
Cell* builtin_current_datetime_local(const Lex* e, const Cell* a);
void lex_add_time_lib(const Lex* e);

#endif //COZENAGE_TIME_LIB_H
