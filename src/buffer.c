/*
 * 'src/buffer.c'
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

#include "buffer.h"

#include <string.h>
#include <stdarg.h>
#include <gc/gc.h>
#include <errno.h>
#include <assert.h>


/* A reasonable initial size */
#define INITIAL_BUFFER_CAPACITY 256


string_builder_t* sb_new(void)
{
    /* Allocate the struct itself with the GC. */
    string_builder_t *sb = GC_MALLOC(sizeof(string_builder_t));

    /* Allocate the initial buffer with the GC. */
    sb->buffer = GC_MALLOC(INITIAL_BUFFER_CAPACITY);
    sb->capacity = INITIAL_BUFFER_CAPACITY;
    sb->length = 0;
    sb->buffer[0] = '\0'; /* Always keep it null-terminated */
    return sb;
}


/* Ensure the buffer can hold *at least* `additional_needed` more bytes. */
static void sb_ensure_capacity(string_builder_t *sb, const size_t additional_needed)
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


/* Append a single string. */
void sb_append_str(string_builder_t *sb, const char *s)
{
    size_t len = strlen(s);
    sb_ensure_capacity(sb, len);

    /* Copy the data */
    memcpy(sb->buffer + sb->length, s, len);
    sb->length += len;
    sb->buffer[sb->length] = '\0'; /* Re-terminate. */
}


/* Append a single char. */
void sb_append_char(string_builder_t *sb, const char c)
{
    sb_ensure_capacity(sb, 1);
    sb->buffer[sb->length] = c;
    sb->length++;
    sb->buffer[sb->length] = '\0';
}


/* Append formatted data, like printf. */
void sb_append_fmt(string_builder_t *sb, const char *fmt, ...)
{
    va_list args;

    /* Try to print into the *existing* space. */
    va_start(args, fmt);
    /* vsnprintf returns the number of chars that *would have been* written. */
    int n = vsnprintf(sb->buffer + sb->length,
                      sb->capacity - sb->length,
                      fmt,
                      args);
    va_end(args);

    /* Check for error, so we can cast 'n' to size_t. */
    if (n < 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        return;
    }

    /* Check if it fit. */
    if ((size_t)n < sb->capacity - sb->length) {
        /* It fit. Just update the length. */
        sb->length += n;
    } else {
        /* It did not fit. 'n' tells us exactly how much space we need. */
        sb_ensure_capacity(sb, n);

        /* Try again, now with a guaranteed-large-enough buffer. */
        va_start(args, fmt);
        int n2 = vsnprintf(sb->buffer + sb->length,
                  sb->capacity - sb->length,
                  fmt,
                  args);
        va_end(args);

        if (n2 < 0) {
            /* This is a critical, unexpected failure! */
            fprintf(stderr, "CRITICAL: vsnprintf failed on second pass: %s\n",
                    strerror(errno));
            /* Just return - not sure if it's necessary to abort. */
            return;
        }

        /* Sanity check (optional, but great for debugging)
         * Should remove this when REPL printing, display, and write are
         * thoroughly tested and 'proven' correct. */
        assert((size_t)n2 == (size_t)n && "vsnprintf length mismatch");

        sb->length += n;
    }
}
