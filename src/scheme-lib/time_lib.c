/*
 * 'src/scheme-lib/time_lib.c'
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

#include "time_lib.h"
#include "types.h"
#include <time.h>


/* (current-second)
 * Returns an inexact number representing the current time on the International Atomic Time (TAI)
 * scale. The value 0.0 represents midnight on January 1, 1970, TAI (equivalent to ten seconds before
 * midnight Universal Time) and the value 1.0 represents one TAI second later. Neither high accuracy
 * nor high precision are required; in particular, returning Coordinated Universal Time plus a
 * suitable constant might be the best an implementation can do. */
Cell* builtin_current_second(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    struct timespec ts;

    /* CLOCK_REALTIME gives POSIX time (seconds since UTC epoch) */
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        /* Handle error... */
        return make_cell_real(0.0);
    }

    /* Convert timespec (sec, nano sec) to a double */
    const double posix_time = (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
    const long double tai_time = posix_time + TAI_UTC_OFFSET;

    return make_cell_real(tai_time);
}

/* (current-jiffy)
 * Returns the number of jiffies as an exact integer that have elapsed since an arbitrary,
 * implementation-defined epoch. A jiffy is an implementation-defined fraction of a second which is
 * defined by the return value of the jiffies-per-second procedure. The starting epoch is guaranteed
 * to be constant during a run of the program, but may vary between runs. */
Cell* builtin_current_jiffy(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    struct timespec ts;

    /* Use CLOCK_MONOTONIC, not CLOCK_REALTIME */
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        /* Handle error */
        return make_cell_integer(0);
    }

    /* Convert (seconds + nanoseconds) into total nanoseconds */
    const int64_t jiffies = (int64_t)ts.tv_sec * 1000000000 + (int64_t)ts.tv_nsec;

    return make_cell_integer(jiffies);
}

/* (jiffies-per-second)
 * Returns an exact integer representing the number of jiffies per SI second. This value is an
 * implementation-specified constant. Cozenage uses 1 billion, ie: nanoseconds. */
Cell* builtin_jiffies_per_second(const Lex* e, const Cell* a) {
    (void)e; (void)a;
    return make_cell_integer(1000000000);
}

/* (current-dt-utc [fmt string])
 * Can be called with zero or one argument. If an argument is provided, it must be a string which is
 * a format specification as per the C library function strftime(3). With no argument, the format
 * specifier is "%Y-%m-%d %H:%M:%S", which prints the date/time as: "2025-10-23 17:00:17" in UTC. */
Cell* builtin_current_datetime_utc(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1);
    if (err) return err;

    char* fmt_str;
    if (a->count > 0 && a->cell[0]->type == CELL_STRING) {
        fmt_str = a->cell[0]->str;
    } else {
        fmt_str = "%Y-%m-%d %H:%M:%S";
    }
    const time_t t = time(NULL);
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
Cell* builtin_current_datetime_local(const Lex* e, const Cell* a) {
    (void)e;
    Cell* err = CHECK_ARITY_RANGE(a, 0, 1);
    if (err) return err;

    char* fmt_str;
    if (a->count > 0 && a->cell[0]->type == CELL_STRING) {
        fmt_str = a->cell[0]->str;
    } else {
        fmt_str = "%Y-%m-%d %H:%M:%S";
    }
    const time_t t = time(NULL);
    struct tm ts;
    localtime_r(&t, &ts);
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

void lex_add_time_lib(const Lex* e) {
    lex_add_builtin(e, "current-second", builtin_current_second);
    lex_add_builtin(e, "current-jiffy", builtin_current_jiffy);
    lex_add_builtin(e, "jiffies-per-second", builtin_jiffies_per_second);
    lex_add_builtin(e, "current-dt-utc", builtin_current_datetime_utc);
    lex_add_builtin(e, "current-dt-local", builtin_current_datetime_local);
}
