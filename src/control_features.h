/*
 * 'control_features.h'
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

#ifndef COZENAGE_CONTROL_FEATURES_H
#define COZENAGE_CONTROL_FEATURES_H

#include "cell.h"


/* Control features  */
Cell* builtin_apply(const Lex* e, const Cell* a);
Cell* builtin_map(const Lex* e, const Cell* a);

#endif //COZENAGE_CONTROL_FEATURES_H
