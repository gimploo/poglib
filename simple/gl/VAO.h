#ifndef __VAO__H__
#define __VAO__H__

#include "_common.h"

#include "VBO.h"


typedef struct vao_t vao_t;

struct vao_t {

    GLuint  id;
};

#define vao_bind(pvao)  GL_CHECK(glBindVertexArray((pvao)->id));
#define vao_unbind()    GL_CHECK(glBindVertexArray(0))

static inline vao_t vao_init(void)
{
    vao_t vao;

    GL_CHECK(glGenVertexArrays(1, &vao.id)); 
    
    GL_LOG("VAO `%i` created", vao.id);

    return vao;
}


static inline void vao_set_attributes(
            vao_t *vao,
            u8 component_count, 
            GLenum type,
            bool normalized,
            size_t stride,
            size_t offset,
            vbo_t *vbo
)
{
    if (vao == NULL) eprint("vao_set_attribute: vao argument is null");
    if (vbo == NULL) eprint("vbo argument is null");

    vao_bind(vao);
    {
        vbo_bind(vbo); 
        {
            GL_CHECK(glEnableVertexAttribArray(++vbo->attribute_index));

            GL_CHECK(glVertexAttribPointer(
                        vbo->attribute_index,
                        component_count, 
                        type, 
                        normalized == false ? GL_FALSE : GL_TRUE, 
                        stride, 
                        (void *)offset));

            ++vbo->attribute_count;
        } 
        vbo_unbind();
    }
    vao_unbind();
    
}


static inline void vao_draw(const vao_t *vao, vbo_t *vbo)
{
    if (vao == NULL) eprint("vao_draw: vao argument is null");

    vao_bind(vao);
    {
        vbo_bind(vbo); 
        {
            if (vbo->indices_count == 0) eprint("vao_draw: vbo`s indices_count is %i", vbo->indices_count);

            glDrawElements(GL_TRIANGLES, vbo->indices_count, GL_UNSIGNED_INT, 0);

        } 
        vbo_unbind();

    }
    vao_unbind();
    
}

static inline void vao_destroy(vao_t *vao)
{
    if (vao == NULL) eprint("vao_bind: vao argument is null");

    GL_LOG("VAO `%i` deleted", vao->id);
    GL_CHECK(glDeleteBuffers(1, &vao->id));
}

#endif //__VAO__H__
