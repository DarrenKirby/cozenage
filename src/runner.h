/*
 * 'src/runner.h'
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

#ifndef COZENAGE_RUNNER_H
#define COZENAGE_RUNNER_H

#include <stdlib.h> // For EXIT_SUCCESS/EXIT_FAILURE
#include "config.h" // For the lib_load_config struct

/**
 * @brief Executes Scheme code from a specified file.
 *
 * This function handles file opening, reading expressions sequentially,
 * and evaluating them without printing the result of each evaluation (non-REPL mode).
 * It includes a check for standard file extensions (.scm, .ss) and issues a
 * non-fatal warning otherwise.
 *
 * @param file_path The path to the Scheme source file.
 * @param config The configuration struct detailing which libraries to load.
 * @return EXIT_SUCCESS (0) on successful execution of the file,
 * or EXIT_FAILURE (1) if the file cannot be opened or a fatal
 * runtime error occurs during evaluation.
 */
int run_file_script(const char *file_path, lib_load_config config);

#endif //COZENAGE_RUNNER_H