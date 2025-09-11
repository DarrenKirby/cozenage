
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

/* editline does not provide tilde.h, so given certain system's
 * propensity to provide fake readline stub files, this should
 * be a safe way to determine if we are actually using GNU readline */
#if __has_include(<readline/tilde.h.h>)
#  define HAS_GNU_READLINE 1
#  include <readline/readline.h>
#  include <readline/history.h>
#  include <readline/tilde.h>
#elif __has_include(<editline/readline.h>)
#  define HAS_GNU_READLINE 0
#  include <editline/readline.h>
#else
#  error "No readline-compatible headers found"
#endif

#endif

#endif //COZENAGE_COMPAT_READLINE_H
