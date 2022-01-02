#pragma once

#include "../../simple/glrenderer2d.h"
#include "../components/sprite.h"
#include "../components/shape.h"
#include "../components/shader.h"
#include "../components/texture.h"

#include "../entitymanager.h"


void s_renderer2d_draw(const entitymanager_t *manager);



#ifndef IGNORE_RENDER2D_SYSTEM_IMPLEMENTATION


void s_renderer2d_draw(const entitymanager_t *manager)
{
    assert(manager);

    glquad_t quads[MAX_QUAD_CAPACITY_PER_DRAW_CALL / 4] = {0};
    i64 quad_count = -1;

    gltri_t tris[MAX_QUAD_CAPACITY_PER_DRAW_CALL / 4] = {0};
    i64 tri_count = -1;


    bool ready_to_draw      = false;
    glshader_t *shader      = NULL;
    gltexture2d_t *texture  = NULL;

    const list_t *entities = &manager->entities;

    for (u64 i = 0; i < entities->len; i++)
    {
        const entity_t *e = *(entity_t **)list_get_element_by_index(entities, i);
        assert(e);

        if (!entity_has_component(e, c_shape2d_t )) continue;

        // VERTICES
        const c_shape2d_t *shape = (c_shape2d_t *)entity_get_component(e, c_shape2d_t );
        assert(shape);

        // SHADER
        c_shader_t *cshader = (c_shader_t *)entity_get_component(e , c_shader_t );
        assert(cshader);
        shader = &cshader->glshader;

        //TODO: setup uv
        quadf_t uv = quadf(0);

        switch(shape->type)
        {
            case CST_SQUARE: {
                glquad_t glquad = glquad_init(
                        *(quadf_t *)shape->__vertices, 
                        (vec3f_t ) { shape->fill.cmp[X], shape->fill.cmp[Y] },
                        uv, 0);
                memcpy(&quads[++quad_count], &glquad, sizeof(glquad_t ));
            } break;
            case CST_TRIANGLE: {
                gltri_t gltri = gltri_init(
                        *(trif_t *)shape->__vertices, 
                        (vec3f_t ) { shape->fill.cmp[X], shape->fill.cmp[Y] },
                        uv, 0);
                memcpy(&tris[++tri_count], &gltri, sizeof(gltri_t ));
            } break;
            default: eprint("shape not accounted for");
        }

        ready_to_draw = true;
    }



    if (!ready_to_draw) return;


    glrenderer2d_t rd = glrenderer2d_init(shader, texture);

        if(quad_count != -1) {
            glbatch_t tmp = glbatch_init(quads, sizeof(quads), glquad_t);
                glrenderer2d_draw_from_batch(&rd, &tmp);
            glbatch_destroy(&tmp);
        } else if (tri_count != -1) {
            glbatch_t tmp = glbatch_init(tris, sizeof(tris), gltri_t);
                glrenderer2d_draw_from_batch(&rd, &tmp);
            glbatch_destroy(&tmp);
        } else {
            printf("quad_count %li\n", quad_count);
            printf("tri_count  %li\n", tri_count);
            eprint("Error, nothings drawn");
        }

    glrenderer2d_destroy(&rd);
}


#endif
