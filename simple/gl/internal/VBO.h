#ifndef __VBO__H__
#define __VBO__H__

#include "gl_common.h"


typedef struct VertexBuffer VertexBuffer;
struct VertexBuffer {

    GLuint id; 

};

/*
 *
 *  FUNCTION:
 *  ========
 *      1. vbo_init()
 *      2. vbo_bind()
 *      3. vbo_unbind()
 *      4. vbo_destroy()
 *
 */


static inline VertexBuffer vbo_init(const void *vertices, const size_t vsize)
{
    VertexBuffer VBO;

    GL_CHECK(glGenBuffers(1, &VBO.id)); 
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO.id));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vsize, vertices, GL_STATIC_DRAW));

    return VBO;
}

static inline void vbo_bind(const VertexBuffer *obj)
{
    if (obj == NULL) eprint("vbo_bind: obj argument is null\n");

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, obj->id));
}
static inline void vbo_unbind(void)
{
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

static inline void vbo_destroy(const VertexBuffer *obj)
{
    if (obj == NULL) eprint("vbo_destroy: obj argument is null\n");

    GL_CHECK(glDeleteBuffers(1, &obj->id));
}

#endif //__VBO__H__
