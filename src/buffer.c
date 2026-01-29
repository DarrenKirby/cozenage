/*
 * 'src/buffer.c'
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


/* This is a somewhat generic self-resizing buffer that can hold chars or
 * uint8_t bytes. It was originally written for the repr, but is also used to
 * initialize and dynamically resize the backing stores for both string
 * and bytevector memory-backed ports.
 *
 * It defines three methods to add data to the buffer. sb_append_char() allows
 * for appending single chars or bytes. sb_append_str() allows for appending
 * an array of chars or bytes. sb_append_fmt() is only useful for text data,
 * it allows for appending formatted data a la sprintf(). */


#include "buffer.h"

#include <string.h>
#include <stdarg.h>
#include <gc/gc.h>
#include <errno.h>
#include <assert.h>


/* A reasonable initial size. */
#define INITIAL_BUFFER_CAPACITY 256

/* The constructor function to initialize a new buffer. */
str_buf_t* sb_new(void)
{
    /* Allocate the struct itself with the GC. */
    str_buf_t *sb = GC_MALLOC(sizeof(str_buf_t));

    /* Allocate the initial buffer with the GC. */
    sb->buffer = GC_MALLOC(INITIAL_BUFFER_CAPACITY);
    sb->capacity = INITIAL_BUFFER_CAPACITY;
    sb->length = 0;
    sb->buffer[0] = '\0';
    return sb;
}


/* The 'private' buffer reallocator. This function is called early in each of the append
 * procedures, and it ensures the buffer can hold *at least* `additional_needed` more bytes. */
static void sb_ensure_capacity(str_buf_t *sb, const size_t additional_needed)
{
    /* +1 for the null terminator. */
    const size_t needed_capacity = sb->length + additional_needed + 1;

    if (needed_capacity > sb->capacity) {
        /* Simple doubling strategy, or grow to *at least* the needed size. */
        size_t new_capacity = sb->capacity * 2;
        if (new_capacity < needed_capacity) {
            new_capacity = needed_capacity;
        }

        sb->buffer = GC_REALLOC(sb->buffer, new_capacity);
        sb->capacity = new_capacity;
    }
}


/* Append a single char. */
void sb_append_char(str_buf_t *sb, const char c)
{
    sb_ensure_capacity(sb, 1);
    sb->buffer[sb->length] = c;
    sb->length++;
    sb->buffer[sb->length] = '\0';
}


/* Append a single string. */
void sb_append_str(str_buf_t *sb, const char *s)
{
    size_t len = strlen(s);
    sb_ensure_capacity(sb, len);

    /* Copy the data. */
    memcpy(sb->buffer + sb->length, s, len);
    sb->length += len;
    sb->buffer[sb->length] = '\0';
}


/* Append binary data (used by generic port I/O. */
void sb_append_data(str_buf_t *sb, const void *data, size_t len)
{
    sb_ensure_capacity(sb, len);
    memcpy(sb->buffer + sb->length, data, len);
    sb->length += len;
    sb->buffer[sb->length] = '\0';
}


/* Append formatted data, like sprintf. */
void sb_append_fmt(str_buf_t *sb, const char *fmt, ...)
{
    va_list args;

    /* Try to print into the existing space. */
    va_start(args, fmt);
    /* vsnprintf returns the number of chars that would have been written. */
    int n = vsnprintf(sb->buffer + sb->length,
                      sb->capacity - sb->length,
                      fmt,
                      args);
    va_end(args);

    if (n < 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        return;
    }

    /* Cast 'n' to size_t, check if it fit. */
    if ((size_t)n < sb->capacity - sb->length) {
        /* It fit. Just update the length. */
        sb->length += n;
    } else {
        /* It did not fit. 'n' is exactly how much space is needed. */
        sb_ensure_capacity(sb, n);

        /* Try again with a guaranteed-large-enough buffer. */
        va_start(args, fmt);
        int n2 = vsnprintf(sb->buffer + sb->length,
                  sb->capacity - sb->length,
                  fmt,
                  args);
        va_end(args);

        if (n2 < 0) {
            /* This is an unexpected failure. */
            fprintf(stderr, "CRITICAL: vsnprintf failed on second pass: %s\n",
                    strerror(errno));
            /* Just return - not sure if it's necessary to abort. */
            return;
        }
        sb->length += n;
    }
}
