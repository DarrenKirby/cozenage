/*
 * 'src/base-lib/datetime_lib.c'
 * This file is part of Cozenage - https://github.com/DarrenKirby/cozenage
 * Copyright © 2025 -Darren Kirby <darren@dragonbyte.ca>
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

#include "types.h"
#include "cell.h"
#include "load_library.h"

#include <time.h>

/* Cozenage uses 1 billion, ie: nanoseconds for j/s. */
#define JIFFIES_PER_SECOND 1000000000
#define JIFFIES_PER_SECOND_DBL 1000000000.0
/* Size of strftime() buffer. */
#define STRF_BUF_SIZE 128
/* R7RS's "suitable constant" (TAI-UTC offset)
 * As of January 2026, this is 37.0 seconds. */
#define TAI_UTC_OFFSET 37.0;


/* (current-second)
 * Returns an inexact number representing the current time on the International Atomic Time (TAI)
 * scale. The value 0.0 represents midnight on January 1, 1970, TAI (equivalent to ten seconds before
 * midnight Universal Time) and the value 1.0 represents one TAI second later. Neither high accuracy
 * nor high precision are required; in particular, returning Coordinated Universal Time plus a
 * suitable constant might be the best an implementation can do. */
static Cell* datetime_current_second(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    struct timespec ts;

    /* CLOCK_REALTIME gives POSIX time (seconds since UTC epoch). */
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        /* Handle error. */
        return make_cell_real(0.0);
    }

    /* Convert timespec (sec, nano sec) to a double */
    const double posix_time = (double)ts.tv_sec + (double)ts.tv_nsec / JIFFIES_PER_SECOND_DBL;
    const long double tai_time = posix_time + TAI_UTC_OFFSET;

    return make_cell_real(tai_time);
}


/* (current-jiffy)
 * Returns the number of jiffies as an exact integer that have elapsed since an arbitrary,
 * implementation-defined epoch. A jiffy is an implementation-defined fraction of a second which is
 * defined by the return value of the jiffies-per-second procedure. The starting epoch is guaranteed
 * to be constant during a run of the program, but may vary between runs. */
static Cell* datetime_current_jiffy(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    struct timespec ts;

    /* Use CLOCK_MONOTONIC, not CLOCK_REALTIME. */
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        /* Handle error. */
        return make_cell_integer(0);
    }

    /* Convert (seconds + nanoseconds) into total nanoseconds. */
    const int64_t jiffies = (int64_t)ts.tv_sec * JIFFIES_PER_SECOND + (int64_t)ts.tv_nsec;

    return make_cell_integer(jiffies);
}


/* (jiffies-per-second)
 * Returns an exact integer representing the number of jiffies per SI second. This value is an
 * implementation-specified constant. */
static Cell* datetime_jiffies_per_second(const Lex* e, const Cell* a)
{
    (void)e; (void)a;
    return make_cell_integer(JIFFIES_PER_SECOND);
}


/* (current-dt-utc [fmt string])
 * Can be called with zero or one argument. If an argument is provided, it must be a string which is
 * a format specification as per the C library function strftime(3). With no argument, the format
 * specifier is "%Y-%m-%d %H:%M:%S", which prints the date/time as: "2025-10-23 17:00:17" in UTC. */
static Cell* datetime_current_datetime_utc(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1, "current-datetime-utc");
    if (err) return err;

    char* fmt_str;
    if (a->count > 0 && a->cell[0]->type == CELL_STRING) {
        fmt_str = a->cell[0]->str;
    } else {
        fmt_str = "%Y-%m-%d %H:%M:%S";
    }
    const time_t t = time(nullptr);
    struct tm ts;
    gmtime_r(&t, &ts);
    char buf[128];
    const size_t result = strftime(buf, sizeof(buf), fmt_str, &ts);
    /* strftime returns zero if the buffer is too small, and buf will be garbage.
     * There are also legitimate 0-length conversions, so we just return an empty
     * string in either of these cases. */
    if (result < 1) {
        return make_cell_string("");
    }
    return make_cell_string(buf);
}


/* (current-dt-local [fmt string])
 * Can be called with zero or one argument. If an argument is provided, it must be a string which is
 * a format specification as per the C library function strftime(3). With no argument, the format
 * specifier is "%Y-%m-%d %H:%M:%S", which prints the date/time as: "2025-10-23 17:00:17" in the
 * local time. */
static Cell* datetime_current_datetime_local(const Lex* e, const Cell* a)
{
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1, "current-datetime-local");
    if (err) return err;

    char* fmt_str;
    if (a->count > 0 && a->cell[0]->type == CELL_STRING) {
        fmt_str = a->cell[0]->str;
    } else {
        fmt_str = "%Y-%m-%d %H:%M:%S";
    }
    const time_t t = time(nullptr);
    struct tm ts;
    localtime_r(&t, &ts);
    char buf[STRF_BUF_SIZE];
    const size_t result = strftime(buf, sizeof(buf), fmt_str, &ts);
    /* strftime returns zero if the buffer is too small, and buf will be garbage.
     * There are also legitimate 0-length conversions, so we just return an empty
     * string in either of these cases. */
    if (result < 1) {
        return make_cell_string("");
    }
    return make_cell_string(buf);
}


static const CznExport date_exports[] = {
    { .scheme_name = "current-second",    .func = datetime_current_second},
    { .scheme_name = "current-jiffy",     .func = datetime_current_jiffy},
    { .scheme_name = "jiffies-per-second",.func = datetime_jiffies_per_second},
    { .scheme_name = "current-dt-utc",    .func = datetime_current_datetime_utc},
    { .scheme_name = "current-dt-local",  .func = datetime_current_datetime_local},
};


static const CznExportTable date_table = {
    .exports = date_exports,
    .count   = sizeof(date_exports) / sizeof(date_exports[0]),
};


extern const CznExportTable* cozenage_library_init()
{
    return &date_table;
}
