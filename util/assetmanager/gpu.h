#pragma once
#include <poglib/gfx/gl/objects.h>

typedef struct {
    u32 vao_id;
    u32 index_count;
    u32 index_type;
    u32 attribute_count;
    u16 vtx_stride;
    struct {
        buffer_t cpu_write;
        buffer_t gpu_read;
        u8 *gpu_mem;
    } vbo;
} gpu_mesh_primitive_t;

typedef struct {
    u8 count;
    gpu_mesh_primitive_t **primitives;
} gpu_mesh_t;

typedef struct {
    u32 asset_id;
    struct {
        u32 count;
        gpu_mesh_t **data; 
    } meshes;
    slot_t material_assetid_lookup;
} gpu_asset_t;


gpu_mesh_primitive_t * gpu_mesh_primtive(
    const vao_t vao,
    const vbo_t vbo,
    const ebo_t ebo,
    const u16 vtx_stride,
    arena_t *const arena
) {
    return arena_store(
        arena, 
        &(gpu_mesh_primitive_t) {
            .vao_id = vao.id,
            .index_count = ebo.indices_count,
            .attribute_count = vbo.internals.attribute_count,
            .index_type = ebo.index_type,
            .vtx_stride = vtx_stride,
            .vbo.gpu_mem = vbo.internals.mapped_ptr,
        },
        sizeof(gpu_mesh_primitive_t)
    );
}

gpu_mesh_t * gpu_mesh(gpu_mesh_primitive_t **primitives, const u8 count, arena_t *const arena)
{
    return arena_store(
        arena,
        &(gpu_mesh_t) {
            .count = count,
            .primitives = arena_store(arena, primitives, sizeof(*primitives) * count)
        },
        sizeof(gpu_mesh_t)
    );
}

gpu_asset_t * gpu_asset(const u32 asset_id, gpu_mesh_t **gpu_mesh, const u16 count, arena_t *const arena)
{
    gpu_asset_t * const gpu_asset = arena_store(
        arena, 
        &(gpu_asset_t) {
            .asset_id = asset_id,
            .meshes = {
                .count = count,
                .data = arena_store(arena, gpu_mesh, sizeof(*gpu_mesh) * count)
            }
        },
        sizeof(gpu_asset_t));

    return gpu_asset;
}
