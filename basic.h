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

#include "./color.h"
#include "./dbg/stacktrace.h"

#ifdef DEBUG
    #include "./dbg/dbg.h"
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


//NOTE: Data types enum 

#define DT_type(TYPE) DT_ ## TYPE

typedef enum 
{
    DT_str_t,

    DT_char,
    DT_u8,
    DT_i8,
    DT_u16,
    DT_i16,
    DT_u32,
    DT_i32,
    DT_u64,
    DT_i64,
    DT_f32,
    DT_f64,

    DT_ptr,

    DT_COUNT

} data_type;

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
    fprintf(stderr, "\n[(%s:%d): %s] " fmt "\n",__FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    print_stack_trace();\
    exit(-1);\
\
} while (0)

#define ARRAY_LEN(ARR) sizeof((ARR)) / sizeof(*(ARR))


int randint(int min, int max)
{
    srand(time(NULL));
    return (rand() % ((max-1) - min + 1)) + min;
}

