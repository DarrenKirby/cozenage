/*
 * 'config.c'
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

#include "config.h"
#include "load_library.h"


void load_initial_libraries(const Lex* e, const lib_load_config load_libs) {
    if (load_libs.file) {
        (void)load_library("file", e);
    }
    if (load_libs.math) {
        (void)load_library("math", e);
    }
    if (load_libs.proc_env) {
        (void)load_library("proc_env", e);
    }
    if (load_libs.cxr) {
        (void)load_library("cxr", e);
    }
    if (load_libs.time) {
        (void)load_library("time", e);
    }
    if (load_libs.bits) {
        (void)load_library("bits", e);
    }
}
