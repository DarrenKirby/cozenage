/*
 * 'src/buffer.h'
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

#ifndef COZENAGE_BUFFER_H
#define COZENAGE_BUFFER_H

#include <stdio.h>


typedef struct {
    char *buffer;    /* The data buffer. */
    size_t length;   /* How many bytes are currently used. */
    size_t capacity; /* How many bytes are allocated. */
} str_buf_t;


str_buf_t* sb_new(void);
void sb_append_str(str_buf_t *sb, const char *s);
void sb_append_data(str_buf_t *sb, const void *data, size_t len);
void sb_append_char(str_buf_t *sb, char c);
void sb_append_fmt(str_buf_t *sb, const char *fmt, ...);

#endif //COZENAGE_BUFFER_H
