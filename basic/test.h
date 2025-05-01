#pragma once
#include <stdbool.h>
#include "./common.h"

#define TEST_ASSERT(PARAM)  if (!(PARAM)) return false;

typedef bool (*__TEST_FUNC_PTR)(void);

#ifdef DEBUG
static __TEST_FUNC_PTR __TESTS_LIST[10];
static const char *__TESTS_NAMES_LIST[10];
static u32 __TEST_LIST_COUNT = 0;
#endif

#define TEST(TEST_LABEL, FUNCTION_BODY)\
    bool TEST_LABEL(void) FUNCTION_BODY

#define REGISTER_TEST(TEST_LABEL)do {\
    ASSERT(__TEST_LIST_COUNT <= ARRAY_LEN(__TESTS_LIST));\
    __TESTS_NAMES_LIST[__TEST_LIST_COUNT] = #TEST_LABEL;\
    __TESTS_LIST[__TEST_LIST_COUNT] = TEST_LABEL;\
    __TEST_LIST_COUNT += 1;\
} while(0);


#define RUN_TESTS()\
    for (u32 i = 0; i < __TEST_LIST_COUNT; i++) {\
         if (__TESTS_LIST[i]()) {\
            printf("%s test passed.\n", __TESTS_NAMES_LIST[i]);\
        } else {\
            printf("%s test failed.\n", __TESTS_NAMES_LIST[i]);\
        };\
    }\
