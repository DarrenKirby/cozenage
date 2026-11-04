/*
 * 'src/stack_guard.h'
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

#ifndef COZENAGE_STACK_GUARD_H
#define COZENAGE_STACK_GUARD_H

#include <stdint.h>


extern uintptr_t stack_floor;      /* lowest stack address we permit */
void stack_guard_init();

inline static int stack_exhausted()
{
    char probe;
    return (uintptr_t)&probe < stack_floor;
}

#endif //COZENAGE_STACK_GUARD_H
