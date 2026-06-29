#pragma once
#include <poglib/gfx/glrenderer3d.h>

typedef struct {
    gltexture2d_t   texture;
    slot_t          sprites;
    vec2i_t         tile_count;
} spriteatlas_t;

spriteatlas_t          spriteatlas_init(const str_t filepath, const u32 tile_count_width, const u32 tile_count_height, arena_t *const arena);
#define                spriteatlas_get_sprite_uv(PATLAS, INDEX)\
                       ((rect_t *)slot_get_buffer(&(PATLAS)->sprites))[INDEX]
box_t                  spriteatlas_get_sprite(const spriteatlas_t * const self, const u32 slot_index);
void                   spriteatlas_destroy(spriteatlas_t *self);

#ifndef IGNORE_SPRITEATLAS_IMPLEMENTATION

spriteatlas_t spriteatlas_init(const str_t filepath, const u32 tile_count_width, const u32 tile_count_height, arena_t * const arena)
{
    ASSERT(filepath.len);
    const gltexture2d_t texture     = gltexture2d_init(filepath.data);
    const f32 norm_tile_width       = 1.0f / (f32)tile_count_width;
    const f32 norm_tile_height      = 1.0f / (f32)tile_count_height;

    slot_t atlas = slot_init(tile_count_width * tile_count_height, sizeof(rect_t), arena);
    for (u32 v = 0, tile_index = 0; v < tile_count_height; v++)
    {
        for (u32 u = 0; u < tile_count_width; u++)
        {
            const f32 left_U   = u * norm_tile_width;
            const f32 right_U  = left_U + norm_tile_width;
            const f32 top_V    = 1.0f - (v * norm_tile_height);
            const f32 bottom_V = 1.0f - ((v + 1) * norm_tile_height);

            const rect_t rect = (rect_t){
                left_U,  top_V,
                right_U, top_V,
                right_U, bottom_V,
                left_U,  bottom_V,
            };

            slot_insert(&atlas, tile_index, &rect, sizeof(rect));
            tile_index++;
        }
    }
    return (spriteatlas_t) {
        .sprites = atlas,
        .texture = texture,
        .tile_count = { tile_count_width, tile_count_height }
    };
}

box_t spriteatlas_get_sprite(const spriteatlas_t *const self, const u32 slot_index)
{
    ASSERT(self);
    ASSERT(slot_index < self->sprites.len);

    const rect_t *uvs = (rect_t *)slot_get_value(&self->sprites, slot_index);
    return (box_t) {
        .x      = uvs->vertex[BOTTOM_LEFT].x,
        .y      = uvs->vertex[BOTTOM_LEFT].y,
        .width  = uvs->vertex[BOTTOM_RIGHT].x - uvs->vertex[BOTTOM_LEFT].x,
        .height = uvs->vertex[TOP_LEFT].y - uvs->vertex[BOTTOM_LEFT].y,
    };
}

void spriteatlas_destroy(spriteatlas_t *const self)
{
    gltexture2d_destroy(&self->texture);
    slot_destroy(&self->sprites);
}
#endif
