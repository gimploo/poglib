#pragma once
#include <poglib/basic.h>

typedef struct {
    struct {
        u32 width;
        u32 height;
    } resolution;
    struct {
        f32 x;
        f32 y;
    } centerpos;
} viewport_t;


viewport_t viewport(const f32 desired_aspect_ratio, const u32 newWidth, const u32 newHeight)
{
    u32 viewportWidth = newWidth;
    u32 viewportHeight = (u32)((f32)newWidth / desired_aspect_ratio);

    if (viewportHeight > newHeight) {
        viewportHeight = newHeight;
        viewportWidth = (u32)((f32)newHeight * desired_aspect_ratio);
    }

    return (viewport_t) {
        .centerpos = {
            .x = (newWidth - viewportWidth) / 2.f,
            .y = (newHeight - viewportHeight) / 2.f,
        },
        .resolution = {
            .width = newWidth,
            .height = newHeight
        }
    };
}

