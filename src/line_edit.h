/*
 * 'src/line_edit.h'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2026 Darren Kirby <darren@dragonbyte.ca>
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


#ifndef COZENAGE_LINE_EDIT_H
#define COZENAGE_LINE_EDIT_H


/* Main readline replacement */
char* readline(const char* prompt);

/* History management */
int read_history(const char* filename);
int write_history(const char* filename);
void add_history_entry(const char* line);

/* Path expansion */
char* tilde_expand(const char* path);

/* Completion interface */
//typedef char** (*rl_completion_func_t)(const char* text, int start, int end);
typedef char **rl_completion_func_t(const char *, int, int);
extern rl_completion_func_t *rl_attempted_completion_function;
extern int rl_attempted_completion_over;
extern char* rl_line_buffer;

/* Completion matching */
char** rl_completion_matches(const char* text, char* (*generator)(const char*, int));
char* rl_filename_completion_function(const char* text, int state);

#endif //COZENAGE_LINE_EDIT_H