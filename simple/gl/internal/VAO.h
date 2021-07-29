#ifndef __VAO__H__
#define __VAO__H__

#include "gl_common.h"

typedef struct VertexArray VertexArray;

struct VertexArray {

    GLuint id;
    size_t attribute_count;

};

static inline VertexArray vao_init(void)
{
    VertexArray vao;
    vao.attribute_count = -1;
    GL_CHECK(glGenVertexArrays(1, &vao.id)); 
    GL_CHECK(glBindVertexArray(vao.id)); 

    return vao;
}

static inline void vao_bind(VertexArray *vao)
{
    if (vao == NULL) eprint("vao_bind: vao argument is null");

    GL_CHECK(glBindVertexArray(vao->id));
}

static inline void vao_set_attributes(
            VertexArray *vao,
            u8 component_count, 
            GLenum type,
            bool normalized,
            size_t stride,
            size_t offset
)
{
    if (vao == NULL) eprint("vao_set_attribute: vao argument is null");

    GL_CHECK(glEnableVertexAttribArray(++vao->attribute_count));
    GL_CHECK(glVertexAttribPointer(
                vao->attribute_count, 
                component_count, 
                type, 
                false ? GL_FLOAT : GL_TRUE, 
                stride, 
                (void *)offset));
}

static inline void vao_unbind(void)
{
    GL_CHECK(glBindVertexArray(0));
}

static inline void vao_destroy(VertexArray *vao)
{
    if (vao == NULL) eprint("vao_bind: vao argument is null");

    GL_CHECK(glDeleteBuffers(1, &vao->id));
}

#endif //__VAO__H__
