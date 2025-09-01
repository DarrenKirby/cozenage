
#ifndef COZENAGE_COMPAT_READLINE_H
#define COZENAGE_COMPAT_READLINE_H


#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* Fake readline function */
char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

#else

#include <stdio.h>

#if __has_include(<readline/readline.h>)
#  include <readline/readline.h>
#  include <readline/history.h>
#elif __has_include(<editline/readline.h>)
#  include <editline/readline.h>
#else
#  error "No readline-compatible headers found"
#endif

#endif

#endif //COZENAGE_COMPAT_READLINE_H
