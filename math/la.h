#ifndef __MY_LA_H__
#define __MY_LA_H__

#include "../basic.h"
#include <string.h>

#define R 0
#define G 1
#define B 2
#define A 3

#define X 0
#define Y 1
#define Z 2

typedef struct vec2 vec2;
typedef struct vec3 vec3;
typedef struct vec4 vec4;

struct vec2 { 
    u8 cmp[2];
}; 
struct vec3 { 
    u8 cmp[3];
};
struct vec4 { 
    u8 cmp[4];
};


typedef struct vec2f vec2f;
typedef struct vec3f vec3f;
typedef struct vec4f vec4f;

struct vec2f {
    f32 cmp[2];
};
struct vec3f {
    f32 cmp[3];
};
struct vec4f {
    f32 cmp[4];
};





#endif //__MY_LA_H__
