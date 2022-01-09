#pragma once

#include "../../simple/glrenderer2d.h"
#include "../components/sprite.h"
#include "../components/shape.h"
#include "../components/shader.h"
#include "../components/texture.h"

#include "../entitymanager.h"



typedef struct s_renderer2d_t s_renderer2d_t;

s_renderer2d_t      s_renderer2d_init(void);
#define             s_renderer2d_draw(PSRENDERER2D, PENTITYMANGER) (PSRENDERER2D)->render((PSRENDERER2D), (PENTITYMANGER))
void                s_renderer2d_destroy(s_renderer2d_t *renderer);





#ifndef IGNORE_RENDER2D_SYSTEM_IMPLEMENTATION


struct s_renderer2d_t {

    glbatch_t glbatches[GLBT_COUNT];
    void (*render)( s_renderer2d_t *, entitymanager_t *);
};

void s_renderer2d_destroy(s_renderer2d_t *renderer)
{
    renderer->render = NULL;

    for (u32 i = 0; i < GLBT_COUNT; i++)
    {
        glbatch_t batch = renderer->glbatches[i]; 
        glbatch_destroy(&batch);
    }
}



void __impl_s_renderer2d_draw(s_renderer2d_t *sys, entitymanager_t *manager )
{
    assert(manager);
    assert(sys);

    glbatch_t *batches = sys->glbatches; 
    assert(batches);

    list_t *entitymap = &manager->entitymap; assert(entitymap);
    for (u64 i = 0; i < entitymap->len; i++)
    {
        glshader_t *shader      = NULL;
        gltexture2d_t *texture  = NULL;

        const list_t *entities = (list_t *)list_get_element_by_index(entitymap, i);
        assert(entities);

        for (u64 j = 0; j < entities->len; j++)
        {
            const entity_t *e = *(entity_t **)list_get_element_by_index(entities, j);
            assert(e);

            if (!entity_has_component(e, c_shape2d_t )) continue;

            // VERTICES
            c_shape2d_t *shape = (c_shape2d_t *)entity_get_component(e, c_shape2d_t );
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
                            shape->fill,
                            uv, 0);

                    glbatch_put(&batches[CST_SQUARE], glquad);

                } break;

                case CST_CIRCLE: {

                    glcircle_t glcircle = glcircle_init(
                            *(circle_t *)shape->__vertices,
                            shape->fill,
                            uv, 0);
                    glbatch_put(&batches[CST_CIRCLE], glcircle);

                } break;

                case CST_TRIANGLE: {

                    gltri_t gltri = gltri_init(
                            *(trif_t *)shape->__vertices, 
                            shape->fill,
                            uv, 0);
                    glbatch_put(&batches[CST_TRIANGLE], gltri);

                } break;

                default: eprint("shape not accounted for");
            }
        }

        if (shader) {

            glrenderer2d_t renderer = glrenderer2d_init(shader, texture);
            for (u32 k = 0; k < GLBT_COUNT; k++)
            {
                glbatch_t *batch = &batches[k];

                if(glbatch_is_empty(batch)) continue; 

                glrenderer2d_draw_from_batch(&renderer, batch);

                glbatch_clear(batch);
            }
            glrenderer2d_destroy(&renderer);
        }

    }


}


s_renderer2d_t s_renderer2d_init(void)
{
    return(s_renderer2d_t ) {
        .glbatches = {
            [GLBT_gltri_t]      = glbatch_init(MAX_QUAD_CAPACITY_PER_DRAW_CALL * 2, gltri_t ),
            [GLBT_glquad_t]     = glbatch_init(MAX_QUAD_CAPACITY_PER_DRAW_CALL, glquad_t ),
            [GLBT_glcircle_t]   = glbatch_init(MAX_QUAD_CAPACITY_PER_DRAW_CALL * 2 / MAX_TRIANGLES_PER_CIRCLE, glcircle_t )
        },
        .render = __impl_s_renderer2d_draw
    };
}



#endif
