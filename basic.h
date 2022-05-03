#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#include <poglib/basic/color.h>
#include "./dbg.h"

// GET ONLY FILENAME AND NOT THE ENTIRE PATH
#if defined(_WIN64)
    #define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#elif defined(__linux__)
    #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define global      static 
#define local       static 

#define INTERNAL    static inline

typedef unsigned char   u8;
typedef char            i8;
typedef uint16_t        u16;
typedef int16_t         i16;
typedef uint32_t        u32;
typedef int             i32;
typedef uint64_t        u64;
typedef long            i64;
typedef float           f32;
typedef double          f64;

#define cstr(VALUE) #VALUE

#define WORD    512
#define KB      (WORD * 2)
#define MB      (KB * 1024)
#define GB      (MB * 1024)

#define PI      3.141592653589793238463

// Used with struct declaration to avoid padding
#define NOPADDING __attribute__((packed)) 

// Used with function declaration to force inlining
#ifndef _WIN64
    #define FORCEINLINE __attribute__((always_inline))
#endif

// Eprint for for both linux and windows
#define eprint(fmt, ...) do {\
\
    fprintf(stderr, "[❌] [(%s:%d): %s] " fmt "\n",__FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    stacktrace_print();\
    exit(-1);\
\
} while (0)


#define ARRAY_LEN(ARR) sizeof((ARR)) / sizeof(*(ARR))

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int randint(int min, int max)
{
    srand(time(NULL));
    return (rand() % ((max-1) - min + 1)) + min;
}
