/*
 * 'src/repr.c'
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

#include "repr.h"
#include "parser.h"
#include "buffer.h"
#include "main.h"
#include "types.h"
#include <stdio.h>
#include <string.h>


/* Forward declaration for helpers */
static void cell_to_string_worker(const Cell* v, string_builder_t *sb, print_mode_t mode);

/* Formats reals to have a trailing '.0' for visual feedback to distinguish from an int */
static void repr_long_double(const long double x, string_builder_t *sb) {
    char buf[128];
    snprintf(buf, sizeof buf, "%.15Lg", x);

    /* If there's no '.' or exponent marker, force a ".0" */
    if (!strchr(buf, '.') && !strchr(buf, 'e') && !strchr(buf, 'E')) {
        strcat(buf, ".0");
    }
    sb_append_str(sb, buf);
}

/* Generate external representation of proper lists and dotted pairs. */
static void repr_pair(const Cell* v, string_builder_t *sb, print_mode_t mode) {
    sb_append_char(sb, '(');
    const Cell* cur = v;

    for (;;) {
        /* Always print the car of the current pair. */
        cell_to_string_worker(cur->car, sb, mode);

        /* The list continues (cdr is another pair). */
        if (cur->cdr->type == CELL_PAIR) {
            sb_append_char(sb, ' ');
            cur = cur->cdr;
        }
        /* This is the end of a proper list. */
        else if (cur->cdr->type == CELL_NIL) {
            break;
        }
        /* This is an improper list. */
        else {
            sb_append_str(sb, " . ");
            cell_to_string_worker(cur->cdr, sb, mode);
            break;
        }
    }
    sb_append_char(sb, ')');
}

/* Generate external representation of sequence types:
 * vector, bytevector, and s-expr. */
static void repr_sequence(const Cell* v,
                          const char* prefix,
                          // ReSharper disable once CppDFAConstantParameter
                          const char open,
                          // ReSharper disable once CppDFAConstantParameter
                          const char close,
                          string_builder_t *sb,
                          const print_mode_t mode) {
    if (!v) return;

    if (prefix) sb_append_fmt(sb, "%s", prefix);
    sb_append_fmt(sb, "%c", open);

    for (int i = 0; i < v->count; i++) {
        cell_to_string_worker(v->cell[i], sb, mode);
        if (i != v->count - 1) sb_append_char(sb, ' ');
    }
    sb_append_fmt(sb, "%c", close);
}

/* Generate external representations of all Cozenage/Scheme types */
static void cell_to_string_worker(const Cell* v, string_builder_t *sb, print_mode_t mode) {
    switch (v->type) {
    case CELL_REAL:
        repr_long_double(v->real_v, sb);
        break;

    case CELL_INTEGER:
        sb_append_fmt(sb, "%lld", v->integer_v);
        break;

    case CELL_RATIONAL:
        sb_append_fmt(sb, "%ld/%ld", v->num, v->den);
        break;

    case CELL_COMPLEX: {
        cell_to_string_worker(v->real, sb, mode);

        const long double im = cell_to_long_double(v->imag);
        if (im < 0) {
            /* already negative */
            cell_to_string_worker(v->imag, sb, mode);
        } else {
            sb_append_char(sb, '+');
            cell_to_string_worker(v->imag, sb, mode);
        }
        sb_append_char(sb, 'i');
        break;
    }

    case CELL_BOOLEAN: {
        char* val = v->boolean_v ? "#true" : "#false";
        if (mode == MODE_REPL) {
            sb_append_fmt(sb, "%s%s%s", ANSI_MAGENTA, val, ANSI_RESET);
        } else {
            sb_append_str(sb, val);
        }
        break;
    }

    case CELL_ERROR: {
        char *err_str;
        switch (v->err_t) {
            case FILE_ERR: { err_str = "File error:"; break; }
            case READ_ERR: { err_str = "Read error:"; break; }
            case SYNTAX_ERR: { err_str = "Syntax error:"; break; }
            case ARITY_ERR: { err_str = "Arity error:"; break; }
            case TYPE_ERR: { err_str = "Type error:"; break; }
            case INDEX_ERR: { err_str = "Index error:"; break; }
            case VALUE_ERR: { err_str = "Value error:"; break; }
            default: { err_str = "Error: "; break; }
        }

        if (mode == MODE_REPL) {
            sb_append_fmt(sb, " %s%s %s %s", ANSI_RED_B, err_str, ANSI_RESET, v->error_v);
        } else {
            sb_append_fmt(sb, " %s %s", err_str, v->error_v);
        }
        break;
    }
    case CELL_CHAR:
        if (mode == MODE_DISPLAY) {
            sb_append_fmt(sb, "%C", v->char_v);
        } else {
            switch (v->char_v) {
                case '\n': sb_append_str(sb, "#\\newline");   break;
                case ' ':  sb_append_str(sb, "#\\space");     break;
                case '\t': sb_append_str(sb, "#\\tab");       break;
                case 0x7:  sb_append_str(sb, "#\\alarm");     break;
                case 0x8:  sb_append_str(sb, "#\\backspace"); break;
                case 0x1b: sb_append_str(sb, "#\\escape");    break;
                case 0xd:  sb_append_str(sb, "#\\return");    break;
                case 0x7f: sb_append_str(sb, "#\\delete");    break;
                case '\0': sb_append_str(sb, "#\\null");      break;

                default:   sb_append_fmt(sb, "#\\%C", v->char_v); break;
            }
        }
        break;

    case CELL_STRING:
        if (mode == MODE_DISPLAY) {
            /* `display` prints the raw string */
            sb_append_str(sb, v->str);
        } else {
            /* `write` and `REPL` print the quoted/escaped string */
            sb_append_char(sb, '"');
            /* TODO: escape quotes/backslashes in v->str */
            sb_append_str(sb, v->str);
            sb_append_char(sb, '"');
        }
        break;

    case CELL_PROC:
        if (v->is_builtin) {
            if (mode == MODE_REPL) {
                sb_append_fmt(sb, "<builtin procedure '%s%s%s'>", ANSI_GREEN_B, v->f_name, ANSI_RESET);
            } else {
                sb_append_fmt(sb, "<builtin procedure '%s'>", v->f_name);
            }
        } else {
            if (mode == MODE_REPL) {
                sb_append_fmt(sb, "<lambda '%s%s%s'>", ANSI_GREEN_B, v->l_name ? v->l_name : "anonymous", ANSI_RESET);
            } else {
                sb_append_fmt(sb, "<lambda '%s'>", v->l_name ? v->l_name : "anonymous");
            }
        }
        break;

    case CELL_PORT:
        if (mode == MODE_REPL) {
            printf("<%s%s %s-port '%s%s%s'>", v->is_open ? "open:" : "closed:",
                v->stream_t == TEXT_PORT ? "text" : "binary",
                v->port_t == INPUT_PORT ? "input" : "output",
                ANSI_BLUE_B, v->path, ANSI_RESET);
        } else {
            printf("<%s%s %s-port '%s'>", v->is_open ? "open:" : "closed:",
                v->stream_t == TEXT_PORT ? "text" : "binary",
                v->port_t == INPUT_PORT ? "input" : "output",
                v->path);
        }
        break;

    case CELL_SYMBOL:
        sb_append_fmt(sb, "%s", v->sym);
        break;

    case CELL_PAIR:
        repr_pair(v, sb, mode);
            break;

    case CELL_NIL:
        sb_append_str(sb, "()");
        break;

    case CELL_EOF:
        sb_append_str(sb,"!EOF");
        break;

    case CELL_SEXPR:
    case CELL_TRAMPOLINE:
        repr_sequence(v, nullptr, '(', ')', sb, mode);
        break;
    case CELL_VECTOR:
        repr_sequence(v, "#", '(', ')', sb, mode);
        break;
    case CELL_BYTEVECTOR:
        repr_sequence(v, "#u8", '(', ')', sb, mode);
        break;

    default:
        /* This code should never run, but it's here if a cell type gets
         * corrupted internally somehow */
        printf("%sError:%s cell_to_string_worker: unknown type: '%s%d%s'", ANSI_RED_B,
            ANSI_RESET, ANSI_RED_B, v->type, ANSI_RESET);
    }
}

/* Generates the external representation of a Cell as a string. */
char* cell_to_string(const Cell* cell, print_mode_t mode) {
    string_builder_t *sb = sb_new();
    cell_to_string_worker(cell, sb, mode);
    return sb->buffer;
}

/* Helper for debugging */
void debug_print_cell(const Cell* v) {
    char* s = cell_to_string(v, MODE_REPL);
    printf("%s\n", s);
}
