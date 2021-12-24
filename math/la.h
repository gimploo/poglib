#pragma once

#include "shapes.h"
#include "vec.h"

void matrix4f_copy(vec2f_t destination[], vec2f_t src[])
{
    memcpy(destination, src, sizeof(vec2f_t) * 4);
}
