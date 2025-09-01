//
// Created by darren on 8/25/25.
//
#include <stdio.h>
#include <readline/readline.h>

int main(void) {
#ifdef RL_LIBRARY_VERSION
    printf("GNU\n");
#else
    /* libedit masquerades as readline, but doesn’t define RL_LIBRARY_VERSION */
    printf("libedit\n");
#endif
    return 0;
}
