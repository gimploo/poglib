#pragma once
#include <poglib/basic.h>
#include <poglib/external/cglm/cglm.h>

// Wrapper around cglm, why tf would typedef to any array 💀

typedef vec2 vec2f_t ;
typedef vec3 vec3f_t ;
typedef vec4 vec4f_t ;
typedef mat2 matrix2f_t;
typedef mat3 matrix3f_t;
typedef mat4 matrix4f_t;
typedef ivec2 vec2i_t;
typedef ivec3 vec3i_t;
typedef ivec4 vec4i_t;

#define normalize(x, min, max)   (((x) - (min))/((max) - (min)))
#define denormalize(x, min, max) ((x) * ((max) - (min)) + (min))

//From a random on discord
#define map(value, from_low, from_high, to_low, to_high) ((value - from_low) * (to_high - to_low) / (from_high - from_low) + to_low)

#define X 0
#define Y 1
#define Z 2
#define W 3

