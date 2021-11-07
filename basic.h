#ifndef _BASIC_H_
#define _BASIC_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#define global      static 
#define internal    static 

#define INTERNAL    static inline

typedef unsigned char   u8;
typedef char            i8;
typedef uint16_t        u16;
typedef int16_t         i16;
typedef uint32_t        u32;
typedef uint64_t        u64;
typedef int             i32;
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


} data_type;

#define WORD    512
#define KB      (WORD * 2)
#define MB      (KB * 1024)
#define GB      (MB * 1024)

// Used with struct declaration to avoid padding
#define NOPADDING __attribute__((packed)) 

// Used with function declaration to force inlining
#define FORCEINLINE __attribute__((always_inline))

#define ERROR {\
    fprintf(stderr, "[(%s:%d): %s] %s\n",__FILE__, __LINE__, __func__, strerror(errno)); \
    exit(0);                                    \
}

#define eprint(fmt, ...) {\
    fprintf(stderr, "[(%s:%d): %s] " fmt "\n",__FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    exit(0);                                    \
}

#define ARRAY_LEN(arr) sizeof((arr)) / sizeof(*(arr))

//  Buffer type (hopefully this will help in avoiding buffer overflows)
//
typedef struct {

    void *array;
    u64 capacity;

} buffer_t;

#define buffer_init(arr) (buffer_t ){.array = arr, .capacity = ARRAY_LEN(arr)}


int randint(int min, int max)
{
    srand(time(NULL));
    return (rand() % (max - min + 1)) + min;
}

#endif // _BASIC_H_
