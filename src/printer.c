/* printer.c - print l_vals */

#include "printer.h"
#include "parser.h"
#include "types.h"
#include <stdio.h>

#include "main.h"


void print_lval_list(const l_val* v, const char* prefix, const char open, const char close) {
    if (!v) return;

    if (prefix) printf("%s", prefix);
    printf("%c", open);

    for (int i = 0; i < v->count; i++) {
        print_lval(v->cell[i]);
        if (i != v->count - 1) printf(" ");
    }

    printf("%c", close);
}

void print_lval(const l_val* v) {
    switch (v->type) {
        case LVAL_FLOAT:
            printf("%Lf", v->float_n);
            break;

        case LVAL_INT:
            printf("%lld", v->int_n);
            break;

        case LVAL_BOOL:
            printf("%s%s%s", ANSI_MAGENTA,
                   v->boolean ? "#t" : "#f",
                   ANSI_RESET);
            break;

        case LVAL_ERR:
            printf("%sError:%s %s", ANSI_RED_B, ANSI_RESET, v->str);
            break;

        case LVAL_CHAR:
            switch (v->char_val) {
            case '\n': printf("#\\newline"); break;
            case ' ':  printf("#\\space");   break;
            case '\t': printf("#\\tab");     break;
            default:   printf("#\\%c", v->char_val); break;
            }
            break;

        case LVAL_STR:
            printf("\"%s\"", v->str);
            break;

        case LVAL_FUN:
            if (v->name) {
                printf("<builtin procedure '%s%s%s'>", ANSI_GREEN_B, v->name, ANSI_RESET);
            } else {
                printf("<lambda>");
            }
            break;

        case LVAL_SYM:
            printf("%s", v->sym);
            break;

        case LVAL_NIL:
            printf("()");
            break;
        case LVAL_SEXPR:
            print_lval_list(v, NULL, '(', ')');
            break;
        case LVAL_VECT:
            print_lval_list(v, "#", '(', ')');
            break;
        case LVAL_BYTEVEC:
            print_lval_list(v, "#u8", '(', ')');
            break;

        default:
            printf("%sError:%s unknown type: '%s%d%s'", ANSI_RED_B,
                ANSI_RESET, ANSI_RED_B, v->type, ANSI_RESET);
    }
}

void println_lval(const l_val* v) {
    print_lval(v);
    putchar('\n');
}
