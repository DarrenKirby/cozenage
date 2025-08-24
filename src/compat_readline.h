
#ifndef COZENAGE_COMPAT_READLINE_H
#define COZENAGE_COMPAT_READLINE_H

#include <stdio.h>

#if __has_include(<readline/readline.h>)
#  include <readline/readline.h>
#  include <readline/history.h>
#elif __has_include(<editline/readline.h>)
#  include <editline/readline.h>
#else
#  error "No readline-compatible headers found"
#endif

#endif //COZENAGE_COMPAT_READLINE_H
