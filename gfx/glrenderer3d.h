#pragma once
#include "gl/types.h"
#include "poglib/basic/ds/list.h"
#include "poglib/gfx/gl/common.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/gfx/gl/texture2d.h"
#include "poglib/gfx/gl/renderconfig.h"
#include "poglib/gfx/gl/vbo_stream_types.h"

//NOTE: Attributes are only handelled for GL_FLOAT (default) and GL_INT


/*=============================================================================
                        - OPENGL 3D RENDERER -
=============================================================================*/

#define MAX_DRAW_CALLS_PER_FRAME_COUNT 100

typedef struct glrenderer3d_t {

    const glshader_t    *shader;
    struct {
        gltexture2d_t   *data;
        int             top;
    } textures;

} glrenderer3d_t ;

typedef struct {

    // Draw type
    enum {
        LINE = GL_LINE,
        TRIANGLES = GL_TRIANGLES, //default
    } draw_mode;

    bool is_wireframe; //default false
    bool allow_empty_vtx_buffer; //default to false
    bool disable_depth_buffer;
    struct {
        bool enable;
        u32 count;
    } instancing;

    // Vertex data
    buffer_t vtx[VBO_STREAM_TYPE_COUNT];

    // Index data
    struct {
        u8 *data;
        u32 nmemb;
    } idx;

    // Attributes
    glvtx_attributelist_t attrs;

    // Textures
    gltexturelist_t textures;

    // Shader Config { uniform and shader }
    glshaderconfig_t shader_config;

} glrendercall_t;

typedef struct {

    //NOTE: we are in the assumption that only float values are allowed via attributes
    //other types are not supported 

    // Draw Call
    struct {
        u8 count;
        glrendercall_t call[MAX_DRAW_CALLS_PER_FRAME_COUNT];
    } calls;

} glrendererconfig_t;


void                glrenderer3d_draw_cube(const glrenderer3d_t *renderer);
void                glrenderer3d_drawcall(const glrendercall_t call);
void                glrenderer3d_draw(const glrendererconfig_t config);


/*-----------------------------------------------------------------------------
                            IMPLEMENTATION
-----------------------------------------------------------------------------*/

#ifndef IGNORE_GLRENDERER2D_IMPLEMENTATION

void glrenderer3d_drawcall(const glrendercall_t call)
{
    glrenderer3d_draw((glrendererconfig_t){
       .calls = {
            .count = 1,
            .call = {
                [0] = call
            }
       }
    });
}

void glrenderer3d_draw(const glrendererconfig_t config)
{
    ASSERT(config.calls.count > 0);
    for (u8 call_idx = 0; call_idx < config.calls.count; call_idx++)
    {
        bool is_idx_null = config.calls.call[call_idx].idx.data ? false : true;
        const bool enable_instancing = config.calls.call[call_idx].instancing.enable;

        //Defaults to GL_TRIANGES if not set else to whatever modes that is available
        u8 draw_mode = GL_TRIANGLES;
        if (config.calls.call[call_idx].draw_mode) {
            draw_mode = config.calls.call[call_idx].draw_mode;
        }

        vao_t vao = vao_init();
        vao_bind(&vao);

        vbo_t vbo = {0};
        ebo_t ebo = {0};

        // Vertex and Index buffer init
        if (config.calls.call[call_idx].allow_empty_vtx_buffer && !config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_GEOMETRY].size) 
            return;
        else ASSERT(config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_GEOMETRY].size > 0);

        ASSERT(config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_GEOMETRY].raw_data);
        if (!is_idx_null) {

            ASSERT(config.calls.call[call_idx].idx.nmemb > 0); 

            vbo = vbo_init((vbo_config_t) {
                .usage = GL_STATIC_DRAW,
                .chunks = {
                    [VBO_STREAM_TYPE_GEOMETRY] = {
                        .index_count = config.calls.call[call_idx].idx.nmemb, 
                        .buffer = config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_GEOMETRY]
                    },
                    [VBO_STREAM_TYPE_INSTANCE] = enable_instancing ? (vbo_stream_t){ 
                        .instance_count = config.calls.call[call_idx].instancing.count, 
                        .buffer = config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_INSTANCE]
                    } : (vbo_stream_t){0},
                }
            });
            vbo_bind(&vbo);
            ebo = ebo_init(
                    &vbo, 
                    (u8 *)config.calls.call[call_idx].idx.data, 
                    config.calls.call[call_idx].idx.nmemb, sizeof(u32));
            ebo_bind(&ebo);

        } else {
            u32 total_ncmp = 0;
            ASSERT(config.calls.call[call_idx].attrs.count > 0);
            for(u8 attr_idx = 0; 
                    attr_idx < config.calls.call[call_idx].attrs.count; ++attr_idx)
                total_ncmp += config.calls.call[call_idx].attrs.attr[attr_idx].ncmp;

            const u32 total_geometry_count = config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_GEOMETRY].size / (total_ncmp * sizeof(f32));

            vbo = vbo_init((vbo_config_t) {
                .usage = GL_STATIC_DRAW,
                .chunks = {
                    [VBO_STREAM_TYPE_GEOMETRY] = {
                        .index_count = total_geometry_count, 
                        .buffer = config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_GEOMETRY]
                    },
                    [VBO_STREAM_TYPE_INSTANCE] = enable_instancing ? (vbo_stream_t){ 
                        .instance_count = config.calls.call[call_idx].instancing.count, 
                        .buffer = config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_INSTANCE]
                    } : (vbo_stream_t){0},
                }
            });

            vbo_bind(&vbo);
        }

        //Attributes
        for(u32 attr_idx = 0; attr_idx < config.calls.call[call_idx].attrs.count; ++attr_idx)
        {
            const u32 data_type = config.calls.call[call_idx].attrs.attr[attr_idx].type;

            vao_set_attributes(
                &vao,
                &vbo, 
                config.calls.call[call_idx].attrs.attr[attr_idx].ncmp, 
                data_type != GL_FLOAT ? data_type : GL_FLOAT,
                false, 
                config.calls.call[call_idx].attrs.attr[attr_idx].interleaved.stride, 
                config.calls.call[call_idx].attrs.attr[attr_idx].interleaved.offset,
                config.calls.call[call_idx].attrs.attr[attr_idx].vbo_chunk_index == VBO_STREAM_TYPE_INSTANCE,
                config.calls.call[call_idx].attrs.attr[attr_idx].vbo_chunk_index); 
        }

        // Shader
        ASSERT(config.calls.call[call_idx].shader_config.shader);
        glshader_bind(config.calls.call[call_idx].shader_config.shader); 

        //uniforms
        ASSERT(config.calls.call[call_idx].shader_config.uniforms.count >= 0);
        glshader_upload_uniforms(
            config.calls.call[call_idx].shader_config.shader, 
            config.calls.call[call_idx].shader_config.uniforms);

        //Textures
        for (u8 txt_idx = 0; txt_idx < config.calls.call[call_idx].textures.count; ++txt_idx)
        {
            switch(config.calls.call[call_idx].textures.items[txt_idx].type)
            {
                case GL_TEXTURE_TYPE_DEFAULT:
                    gltexture2d_bind(
                        config.calls.call[call_idx].textures.items[txt_idx].tex_id, 
                        txt_idx, 
                        config.calls.call[call_idx].textures.items[txt_idx].is_srgb
                    );
                break;
                case GL_TEXTURE_TYPE_CUBEMAP:
                    GL_CHECK(glDepthMask(false));
                    GL_CHECK(glDepthFunc(GL_LEQUAL));
                    GL_CHECK(glBindTexture(
                        GL_TEXTURE_CUBE_MAP, 
                        config.calls.call[call_idx].textures.items[txt_idx].tex_id
                    ));
                break;
                default: eprint("Not implemented");
            }
        }

        if (config.calls.call[call_idx].is_wireframe) {
            GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        } else {
            GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        if (enable_instancing) {
            if (!is_idx_null)   vao_draw_with_ebo_in_mode_instanced(&vao, &ebo, draw_mode);
            else                vao_draw_with_vbo_in_mode_instanced(&vao, &vbo, draw_mode);
        } else {
            if (!is_idx_null)   vao_draw_with_ebo(&vao, &ebo);
            else                vao_draw_with_vbo_in_mode(&vao, &vbo, draw_mode);
        }

        GL_CHECK(glDepthMask(true));
        GL_CHECK(glDepthFunc(GL_LESS));
        gltexture2d_unbind();

        if (!is_idx_null) ebo_destroy(&ebo);
        vao_destroy(&vao);
        vbo_destroy(&vbo);
    }

}
#endif


//TODO: disable_depth_buffer introducing this broke the rendering - removed it for now.
