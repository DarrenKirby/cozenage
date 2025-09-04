#ifndef TEST_META_H
#define TEST_META_H

#include "../src/types.h"

/* Declare global environment */
extern Lex* test_env;

/* Test helpers. */
void suite_setup_wrapper(void);
void teardown_suite(void);
void eval_and_check(const char* input, const char* expected_output);

#endif // TEST_META_H
