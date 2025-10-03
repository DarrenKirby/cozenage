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
        if (cur->cdr->type == VAL_PAIR) {
            printf(" ");
            cur = cur->cdr;
        }
        /* Case 2: This is the end of a proper list. */
        else if (cur->cdr->type == VAL_NIL) {
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
    case VAL_REAL:
        print_long_double(v->r_val);
        break;

    case VAL_INT:
        printf("%lld", v->i_val);
        break;

    case VAL_RAT:
        printf("%ld/%ld", v->num, v->den);
        break;

    case VAL_COMPLEX: {
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

    case VAL_BOOL:
#ifndef TESTING__
        printf("%s%s%s", ANSI_MAGENTA,
               v->b_val ? "#true" : "#false",
               ANSI_RESET);
        break;
#else
        printf("%s", v->b_val ? "#true" : "#false");
        break;
#endif

    case VAL_ERR:
#ifdef TESTING__
        printf(" %s: %s", v->err_t == GEN_ERR ? "Error" : "File error", v->err);
        break;
#else
        printf(" %s%s:%s %s", ANSI_RED_B, v->err_t == GEN_ERR ? "Error" : "File error", ANSI_RESET, v->err);
        break;
#endif

    case VAL_CHAR:
        switch (v->c_val) {
        case '\n': printf("#\\newline");   break;
        case ' ':  printf("#\\space");     break;
        case '\t': printf("#\\tab");       break;
        case 0x7:  printf("#\\alarm");     break;
        case 0x8:  printf("#\\backspace"); break;
        case 0x1b: printf("#\\escape");    break;
        case 0xd:  printf("#\\return");    break;
        case 0x7f: printf("#\\delete");    break;
        case '\0': printf("#\\null");      break;

        default:   printf("#\\%C", v->c_val); break;
        }
        break;

    case VAL_STR:
        printf("\"%s\"", v->str);
        break;

    case VAL_PROC:
        if (!v->formals) {
            printf("<builtin procedure '%s%s%s'>", ANSI_GREEN_B, v->name, ANSI_RESET);
        } else {
            printf("<lambda '%s%s%s'>", ANSI_GREEN_B, v->name, ANSI_RESET);
        }
        break;

    case VAL_PORT:
        printf("<%s%s %s-port '%s%s%s'>", v->is_open ? "open:" : "closed:",
            v->stream_t == TEXT_PORT ? "text" : "binary",
            v->port_t == INPUT_PORT ? "input" : "output",
            ANSI_BLUE_B, v->path, ANSI_RESET);
        break;

    case VAL_SYM:
        printf("%s", v->sym);
        break;

    case VAL_PAIR:
        print_pair(v);
            break;

    case VAL_NIL:
        printf("()");
        break;

    case VAL_EOF:
        printf("!EOF");
        break;

    case VAL_SEXPR:
        print_sequence(v, nullptr, '(', ')');
        break;
    case VAL_VEC:
        print_sequence(v, "#", '(', ')');
        break;
    case VAL_BYTEVEC:
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

void print_env(const Lex* e) {
    for (int i = 0; i < e->count; i++) {
        printf("%s -> ", e->syms[i]);
        print_cell(e->vals[i]);
        if (e->vals[i]->type == VAL_PROC) {
            printf(" Name:  %s \n", e->vals[i]->name);
        } else {
            printf("\n");
        }
    }
}
