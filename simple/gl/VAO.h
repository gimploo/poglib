#ifndef __VAO__H__
#define __VAO__H__

#include "_common.h"

#include "VBO.h"


typedef struct vao_t vao_t;

struct vao_t {

    GLuint  id;
    stack_t vbos;
};

#define vao_bind(pvao)  GL_CHECK(glBindVertexArray((pvao)->id));
#define vao_unbind()    GL_CHECK(glBindVertexArray(0))

static inline vao_t vao_init(u32 vbo_count)
{
    vao_t vao;

    GL_CHECK(glGenVertexArrays(1, &vao.id)); 
    GL_CHECK(glBindVertexArray(vao.id)); 
    
    vbo_t **vbos_array = (vbo_t **)malloc(sizeof(vbo_t*) * vbo_count);

    vao.vbos = stack_init((void **)vbos_array, vbo_count);

    GL_LOG("VAO `%i` created", vao.id);

    return vao;
}

static inline void vao_push(vao_t *vao, vbo_t *vbo)
{
    if (vao == NULL) eprint("vao_push: vao argument is null");
    if (vbo == NULL) eprint("vao_push: vbo argument is null");

    vao_bind(vao);
        stack_push(&vao->vbos, vbo);
    vao_unbind();
}

static inline vbo_t * vao_pop(vao_t *vao)
{
    if (vao == NULL) eprint("vao_push: vao argument is null");

    vao_bind(vao);
        vbo_t *ret = (vbo_t *)stack_pop(&vao->vbos);
    vao_unbind();

    return ret;
}


static inline void vao_set_attributes(
            vao_t *vao,
            GLuint stack_vbo_index,
            u8 component_count, 
            GLenum type,
            bool normalized,
            size_t stride,
            size_t offset
)
{
    if (vao == NULL) eprint("vao_set_attribute: vao argument is null");

    if (stack_vbo_index > vao->vbos.top) eprint("vao_set_attribute: index out of bound");
    
    vao_bind(vao);

    vbo_t *vbo = (vbo_t *)vao->vbos.array[stack_vbo_index]; 
    vbo_bind(vbo); {

        GL_CHECK(glEnableVertexAttribArray(++vbo->attribute_index));

        GL_CHECK(glVertexAttribPointer(
                    vbo->attribute_index,
                    component_count, 
                    type, 
                    normalized == false ? GL_FALSE : GL_TRUE, 
                    stride, 
                    (void *)offset));

        ++vbo->attribute_count;

    } vbo_unbind();

    vao_unbind();
    
}


static inline bool vao_draw(vao_t *vao)
{
    if (vao == NULL) eprint("vao_draw: vao argument is null");
    if(stack_is_empty(&vao->vbos)) eprint("vao_draw: vao stack is empty");

    stack_t *stack = &vao->vbos;
    vbo_t *vbo = NULL;

    vao_bind(vao);
    {
        for (i64 i = stack->top; i >= 0; i--) 
        {
            vbo = (vbo_t *)stack->array[i];
            vbo_bind(vbo); {

                if (vbo->indices_count == 0) eprint("vao_draw: vbo[%li] indices_count is %i", i, vbo->indices_count);

                glDrawElements(GL_TRIANGLES, vbo->indices_count, GL_UNSIGNED_INT, 0);

            } vbo_unbind();

        } 
    }
    vao_unbind();
    
    return true;
}

static inline void vao_destroy(vao_t *vao)
{
    if (vao == NULL) eprint("vao_bind: vao argument is null");


    stack_t *stack = &vao->vbos;
    vbo_t *vbo = (vbo_t *)stack_pop(stack);
    while (vbo != NULL) {
        vbo_destroy(vbo);
        vbo = (vbo_t *)stack_pop(stack);
    }

    free(vao->vbos.array);

    GL_LOG("VAO `%i` deleted", vao->id);
    GL_CHECK(glDeleteBuffers(1, &vao->id));
}

#endif //__VAO__H__
