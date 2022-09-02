#pragma once
#include <poglib/basic.h>


f32         radians(const f32 deg);


#ifndef IGNORE_MATH_UTIL_IMPLEMENTATION

f32 radians(const f32 deg) 
{
    return deg * PI / 180.0f;
}

#endif
