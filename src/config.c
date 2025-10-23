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

    if (load_libs.coz_ext) {
        (void)load_scheme_library("coz-ext", e);
    }
    if (load_libs.file) {
        (void)load_scheme_library("file", e);
    }
    if (load_libs.process_context) {
        (void)load_scheme_library("process_context", e);
    }
    if (load_libs.inexact) {
        (void)load_scheme_library("inexact", e);
    }
    if (load_libs.complex) {
        (void)load_scheme_library("complex", e);
    }
    if (load_libs.char_lib) {
        (void)load_scheme_library("char", e);
    }
    if (load_libs.read) {
        (void)load_scheme_library("read", e);
    }
    if (load_libs.write) {
        (void)load_scheme_library("write", e);
    }
    if (load_libs.eval) {
        (void)load_scheme_library("eval", e);
    }
    if (load_libs.cxr) {
        (void)load_scheme_library("cxr", e);
    }
    /* Cozenage libs */
    if (load_libs.coz_bits) {
        (void)load_scheme_library("bits", e);
    }
}
