#ifndef __VBO__H__
#define __VBO__H__

#include "_common.h"


typedef struct vbo_t vbo_t;
struct vbo_t {

    GLuint  id; 
    i32     attribute_index;
    u32     indices_count;

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

static inline vbo_t vbo_init(const void *vertices, const size_t vsize)
{
    vbo_t VBO;

    GL_CHECK(glGenBuffers(1, &VBO.id)); 
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO.id));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vsize, vertices, GL_STATIC_DRAW));

    VBO.attribute_index = -1;
    VBO.indices_count = 0;

    GL_LOG("VBO `%i` created", VBO.id);
    return VBO;
}

static inline void vbo_bind(const vbo_t *obj)
{
    if (obj == NULL) eprint("vbo_bind: obj argument is null\n");

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, obj->id));
}
static inline void vbo_unbind(void)
{
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

static inline void vbo_destroy(const vbo_t *obj)
{
    if (obj == NULL) eprint("vbo_destroy: obj argument is null\n");

    GL_LOG("VBO `%i` deleted", obj->id);
    GL_CHECK(glDeleteBuffers(1, &obj->id));
}

#endif //__VBO__H__
