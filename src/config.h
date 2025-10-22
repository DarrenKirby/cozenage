/*
 * 'src/config.h'
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

#ifndef COZENAGE_CONFIG_H
#define COZENAGE_CONFIG_H


typedef struct lib_load {
    unsigned int coz_ext:1;
    unsigned int case_lambda:1;
    unsigned int char_lib:1;
    unsigned int complex:1;
    unsigned int cxr:1;
    unsigned int eval:1;
    unsigned int file:1;
    unsigned int inexact:1;
    unsigned int lazy:1;
    unsigned int load:1;
    unsigned int process_context:1;
    unsigned int read:1;
    unsigned int repl:1;
    unsigned int time:1;
    unsigned int write:1;
    unsigned int coz_bits:1;
} lib_load_config;

#endif // COZENAGE_CONFIG_H
