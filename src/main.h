/*
 * 'src/main.h'
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

#ifndef COZENAGE_H
#define COZENAGE_H

#define APP_NAME "Cozenage"
#define APP_VERSION "0.9.1"
#define PS1_PROMPT "--> \001\x1b[37;1m\002"
#define PS2_PROMPT "...    \001\x1b[37;1m\002"
#define HIST_FILE "~/.cozenage_history"

/* ANSI colour codes */
/* normal */
#define ANSI_RED       "\x1b[31m"
#define ANSI_GREEN     "\x1b[32m"
#define ANSI_YELLOW    "\x1b[33m"
#define ANSI_BLUE      "\x1b[34m"
#define ANSI_MAGENTA   "\x1b[35m"
#define ANSI_CYAN      "\x1b[36m"
#define ANSI_WHITE     "\x1b[37m"

/* bold */
#define ANSI_RED_B     "\x1b[31;1m"
#define ANSI_GREEN_B   "\x1b[32;1m"
#define ANSI_YELLOW_B  "\x1b[33;1m"
#define ANSI_BLUE_B    "\x1b[34;1m"
#define ANSI_MAGENTA_B "\x1b[35;1m"
#define ANSI_CYAN_B    "\x1b[36;1m"
#define ANSI_WHITE_B   "\x1b[37;1m"

#define ANSI_RESET     "\x1b[0m"

#endif
