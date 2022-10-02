#pragma once
#include <poglib/basic.h>
#define STB_IMAGE_IMPLEMENTATION
#include <poglib/external/stb_image.h>

typedef struct image_t {

    char filepath[32];
    void *data;             // Image raw data
    i32  width;             // Image base width
    i32  height;            // Image base height
    i32  format;            // Data format (PixelFormat type)
                               
} image_t ;

image_t image_init(const char *filepath)
{
    assert(filepath);
    image_t o = {0};
    cstr_copy(o.filepath, filepath);
    stbi_set_flip_vertically_on_load(1);                                
    o.data = stbi_load(filepath, &o.width, &o.height, &o.format, 0); //RGB
    if (o.data == NULL) 
        eprint("Failed to load image");
    return o;
}

void image_destroy(image_t *self)
{
    assert(self);
    stbi_image_free(self->data);
    self->data = NULL;
}

