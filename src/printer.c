/* printer.c - print l_vals */

#include "printer.h"
#include "parser.h"
#include "types.h"
#include <stdio.h>

#include "main.h"


void print_lval_seq(const Cell* v, const char* prefix, const char open, const char close) {
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
    while (cur->type == VAL_PAIR) {
        print_cell(cur->car);
        if (cur->cdr->type == VAL_NIL) {
            break;  // proper end of list
        }
        if (cur->cdr->type == VAL_PAIR) {
            printf(" ");
            cur = cur->cdr;
            continue;
        }
        printf(" . ");
        print_cell(cur->cdr);
        break;
    }
    printf(")");
}

void print_cell(const Cell* v) {
    switch (v->type) {
    case VAL_REAL:
        printf("%Lg", v->real_v);
        break;

    case VAL_INT:
        printf("%lld", v->int_v);
        break;

    case VAL_RAT:
        printf("%ld/%ld", v->num, v->den);
        break;

    case VAL_COMPLEX:
        print_cell(v->real);
        printf("+");
        print_cell(v->imag);
        printf("i");
        break;

    case VAL_BOOL:
        printf("%s%s%s", ANSI_MAGENTA,
               v->boolean ? "#true" : "#false",
               ANSI_RESET);
        break;

    case VAL_ERR:
        printf(" %sError:%s %s", ANSI_RED_B, ANSI_RESET, v->str);
        break;

    case VAL_CHAR:
        switch (v->char_val) {
        case '\n': printf("#\\newline");   break;
        case ' ':  printf("#\\space");     break;
        case '\t': printf("#\\tab");       break;
        case 0x7:  printf("#\\alarm");     break;
        case 0x8:  printf("#\\backspace"); break;
        case 0x1b: printf("#\\escape");    break;
        case 0xd:  printf("#\\return");    break;
        case 0x7f: printf("#\\delete");    break;
        case '\0': printf("#\\null");      break;

        default:   printf("#\\%c", v->char_val); break;
        }
        break;

    case VAL_STR:
        printf("\"%s\"", v->str);
        break;

    case VAL_PROC:
        if (v->name) {
            printf("<builtin procedure '%s%s%s'>", ANSI_GREEN_B, v->name, ANSI_RESET);
        } else {
            printf("<lambda>");
        }
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

    case VAL_SEXPR:
        print_lval_seq(v, NULL, '(', ')');
        break;
    case VAL_VEC:
        print_lval_seq(v, "#", '(', ')');
        break;
    case VAL_BYTEVEC:
        print_lval_seq(v, "#u8", '(', ')');
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
