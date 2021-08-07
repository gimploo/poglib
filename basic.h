#ifndef _BASIC_H_
#define _BASIC_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#define global      static 
#define internal    static 

typedef unsigned char   u8;
typedef char            i8;
typedef u_int16_t       u16;
typedef int16_t         i16;
typedef u_int32_t       u32;
typedef u_int64_t       u64;
typedef int             i32;
typedef long            i64;
typedef float           f32;
typedef double          f64;

#define WORD    512
#define KB      (WORD * 2)
#define MB      (KB * 1024)
#define GB      (MB * 1024)

// Used with struct declaration to avoid padding
#define NOPADDING __attribute__((packed)) 

#define ERROR {\
    fprintf(stderr, "[(%s:%d): %s] %s\n",__FILE__, __LINE__, __func__, strerror(errno)); \
    exit(1);                                    \
}

#define eprint(fmt, ...) {\
    fprintf(stderr, "[(%s:%d): %s] " fmt "\n",__FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    exit(1);                                    \
}


#endif // _BASIC_H_
