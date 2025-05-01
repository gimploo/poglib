#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <errno.h> 
#include <time.h>
#include <ctype.h>

#define MAX_TYPE_CHARACTER_LENGTH 32

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

#define WORD    512
#define KB      (WORD * 2)
#define MB      (KB * 1024)
#define GB      (MB * 1024)
#define PI      3.141592653589793238463f

// Used with struct declaration to avoid padding
#define NOPADDING __attribute__((packed)) 

// Used with function declaration to force inlining
#ifndef _WIN64
    #define FORCEINLINE __attribute__((always_inline))
#endif

#if defined(DEBUG) && !defined(DISABLE_LOGGING)
    #define logging(FMT, ...) do {\
        fprintf(stdout, "[LOG : %s] \t"FMT"\n", __func__, ##__VA_ARGS__);\
    } while(0)
#else
    #define logging(FMT, ...)
#endif


#define ARRAY_LEN(ARR)      sizeof((ARR)) / sizeof(*(ARR))
#define MAX(x, y)           (((x) > (y)) ? (x) : (y))
#define MIN(x, y)           (((x) < (y)) ? (x) : (y))

int randint(int min, int max)
{
    srand(time(NULL));
    return (rand() % ((max-1) - min + 1)) + min;
}

f32 randf32(const f32 min, const f32 max)
{
    srand(time(NULL));
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}

void swap(void **x, void **y)
{
    void *t = *x;
    *x = *y;
    *y = t;
}
