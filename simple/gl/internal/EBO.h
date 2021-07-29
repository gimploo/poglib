#ifndef __EBO__H__
#define __EBO__H__

#include "gl_common.h"

typedef struct ElementBuffer ElementBuffer;

struct ElementBuffer {
    GLuint id;
    size_t count;
};

/*
 *
 * FUNCTIONS:
 * =========
 *      1. ebo_init()
 *      2. ebo_bind()
 *      3. ebo_unbind()
 *      4. ebo_destroy()
 *
 *
 */


static inline ElementBuffer ebo_init(const unsigned int *indices, const size_t count)
{
    if (indices == NULL) eprint("ebo_init: indices argumen is null");
    if (count <= 0) eprint("ebo_init: isize is not greater than 0");

    ElementBuffer ebo;
    GL_CHECK(glGenBuffers(1, &ebo.id));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.id));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW));

    ebo.count = count;
    return ebo;
}

static inline void ebo_bind(const ElementBuffer *ebo)
{
    if (ebo == NULL) eprint("ebo_bind: ebo is null argument");

    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->id));
}

static inline void ebo_unbind(const ElementBuffer *ebo)
{
    if (ebo == NULL) eprint("ebo_bind: ebo is null argument");

    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

static inline void ebo_destroy(const ElementBuffer *ebo)
{
    if (ebo == NULL) eprint("ebo_bind: ebo is null argument");

    GL_CHECK(glDeleteBuffers(1, &ebo->id));
}




#endif 
