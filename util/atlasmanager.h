#pragma once
#include <poglib/gfx/glrenderer3d.h>

typedef struct {
    gltexture2d_t texture;
    slot_t sprites;
} atlasmanager_t;

atlasmanager_t  atlasmanager_init(
                    const char *filepath, 
                    const u32 tile_count_width, 
                    const u32 tile_count_height);
#define         atlasmanager_get_sprite(PATLAS, INDEX)    ((rect_t *)slot_get_buffer(&(PATLAS)->sprites))[INDEX]
void            atlasmanager_destroy(atlasmanager_t *self);

atlasmanager_t atlasmanager_init(const char *filepath, const u32 tile_count_width, const u32 tile_count_height)
{
    if (filepath == NULL) eprint("filepath argument is null");

    gltexture2d_t texture = gltexture2d_init(filepath);

    const f32 norm_font_width    = normalize(texture.width / (f32)tile_count_width, 0.f, texture.width);
    const f32 norm_font_height   = normalize(texture.height / (f32)tile_count_height, 0.f, texture.height);

    slot_t atlas = slot_init(tile_count_width * tile_count_height, rect_t);

    for (u32 v = 0, tile_index = 0; v < tile_count_height; v++)
    {
        for (u32 u = 0; u < tile_count_width; u++)
        {
            f32 left_U      = u * norm_font_width;
            f32 right_U     = left_U + norm_font_width; 
            f32 top_V       = -(v * norm_font_height);
            f32 bottom_V    = top_V - norm_font_height ;

            const rect_t rect = (rect_t ){
                left_U,     top_V,
                right_U,    top_V,
                right_U,    bottom_V,
                left_U,     bottom_V,
            };
            slot_insert(&atlas, tile_index, &rect, sizeof(rect));

            ++tile_index;
        }
    }
    return (atlasmanager_t) {
        .sprites = atlas,
        .texture = texture
    };
}

void atlasmanager_destroy(atlasmanager_t *self)
{
    gltexture2d_destroy(&self->texture);
    slot_destroy(&self->sprites);
}
