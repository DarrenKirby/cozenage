#ifndef TEST_META_H
#define TEST_META_H

#include "../src/types.h"

/* Declare global environment */
extern Lex* test_env;

/* Test helpers. */
void setup_each_test(void);
void teardown_each_test(void);
void suite_setup_wrapper(void);
void teardown_suite(void);
char* t_eval(const char* input);
char* t_eval_math_lib(const char* input);

#endif // TEST_META_H
