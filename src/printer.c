/*
 * 'src/printer.c'
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

#include "printer.h"
#include "parser.h"
#include "main.h"
#include "types.h"
#include <stdio.h>
#include <string.h>


void print_long_double(long double x) {
    char buf[128];
    snprintf(buf, sizeof buf, "%.15Lg", x);

    /* If there's no '.' or exponent marker, force a ".0" */
    if (!strchr(buf, '.') && !strchr(buf, 'e') && !strchr(buf, 'E')) {
        strcat(buf, ".0");
    }
    printf("%s", buf);
}

void print_sequence(const Cell* v, const char* prefix, const char open, const char close) {
    if (!v) return;

    if (prefix) printf("%s", prefix);
    printf("%c", open);

    for (int i = 0; i < v->count; i++) {
        print_cell(v->cell[i]);
        if (i != v->count - 1) printf(" ");
    }
    printf("%c", close);
}

void print_pair(const Cell* v) {
    printf("(");
    const Cell* cur = v;

    for (;;) {
        /* Always print the car of the current pair. */
        print_cell(cur->car);

        /* Case 1: The list continues (cdr is another pair). */
        if (cur->cdr->type == CELL_PAIR) {
            printf(" ");
            cur = cur->cdr;
        }
        /* Case 2: This is the end of a proper list. */
        else if (cur->cdr->type == CELL_NIL) {
            break;
        }
        /* Case 3: This is an improper list. */
        else {
            printf(" . ");
            print_cell(cur->cdr);
            break;
        }
    }
    printf(")");
}

void print_cell(const Cell* v) {
    switch (v->type) {
    case CELL_REAL:
        print_long_double(v->real_v);
        break;

    case CELL_INTEGER:
        printf("%lld", v->integer_v);
        break;

    case CELL_RATIONAL:
        printf("%ld/%ld", v->num, v->den);
        break;

    case CELL_COMPLEX: {
        print_cell(v->real);

        const long double im = cell_to_long_double(v->imag);
        if (im < 0) {
            print_cell(v->imag);  /* already negative */
        } else {
            printf("+");
            print_cell(v->imag);
        }
        printf("i");
        break;
    }

    case CELL_BOOLEAN:
#ifndef TESTING__
        printf("%s%s%s", ANSI_MAGENTA,
               v->boolean_v ? "#true" : "#false",
               ANSI_RESET);
        break;
#else
        printf("%s", v->boolean_v ? "#true" : "#false");
        break;
#endif

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
#ifdef TESTING__
        printf(" %s %s", err_str, v->error_v);
        break;
#else
        printf(" %s%s %s %s", ANSI_RED_B, err_str, ANSI_RESET, v->error_v);
        break;
#endif
    }
    case CELL_CHAR:
        switch (v->char_v) {
        case '\n': printf("#\\newline");   break;
        case ' ':  printf("#\\space");     break;
        case '\t': printf("#\\tab");       break;
        case 0x7:  printf("#\\alarm");     break;
        case 0x8:  printf("#\\backspace"); break;
        case 0x1b: printf("#\\escape");    break;
        case 0xd:  printf("#\\return");    break;
        case 0x7f: printf("#\\delete");    break;
        case '\0': printf("#\\null");      break;

        default:   printf("#\\%C", v->char_v); break;
        }
        break;

    case CELL_STRING:
        printf("\"%s\"", v->str);
        break;

    case CELL_PROC:
        if (v->is_builtin) {
            printf("<builtin procedure '%s%s%s'>", ANSI_GREEN_B, v->f_name, ANSI_RESET);
        } else {
            printf("<lambda '%s%s%s'>", ANSI_GREEN_B, v->l_name ? v->l_name : "anonymous", ANSI_RESET);
        }
        break;

    case CELL_PORT:
        printf("<%s%s %s-port '%s%s%s'>", v->is_open ? "open:" : "closed:",
            v->stream_t == TEXT_PORT ? "text" : "binary",
            v->port_t == INPUT_PORT ? "input" : "output",
            ANSI_BLUE_B, v->path, ANSI_RESET);
        break;

    case CELL_SYMBOL:
        printf("%s", v->sym);
        break;

    case CELL_PAIR:
        print_pair(v);
            break;

    case CELL_NIL:
        printf("()");
        break;

    case CELL_EOF:
        printf("!EOF");
        break;

    case CELL_SEXPR:
        print_sequence(v, nullptr, '(', ')');
        break;
    case CELL_VECTOR:
        print_sequence(v, "#", '(', ')');
        break;
    case CELL_BYTEVECTOR:
        print_sequence(v, "#u8", '(', ')');
        break;

    default:
        printf("%sError:%s print_cell: unknown type: '%s%d%s'", ANSI_RED_B,
            ANSI_RESET, ANSI_RED_B, v->type, ANSI_RESET);
    }
}

void println_cell(const Cell* v) {
    print_cell(v);
    putchar('\n');
}
