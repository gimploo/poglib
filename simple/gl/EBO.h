#ifndef __EBO__H__
#define __EBO__H__

#include "_common.h"
#include "VBO.h"

typedef struct ebo_t ebo_t;

struct ebo_t {

    GLuint id;
    u32 count;

};

/*
 *
 * FUNCTIONS:
 * =========
 *      1. ebo_init()
 *      2. ebo_destroy()
 *
 *
 */


static inline ebo_t ebo_init(vbo_t * const vbo, const unsigned int *indices, const u32 count)
{
    if (vbo == NULL) eprint("ebo_init: vbo argument is null");
    if (indices == NULL) eprint("ebo_init: indices argumen is null");
    if (count <= 0) eprint("ebo_init: isize is not greater than 0");

    ebo_t ebo;
    GL_CHECK(glGenBuffers(1, &ebo.id));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.id));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW));

    vbo->indices_count = count;
    ebo.count = count;

    GL_LOG("EBO `%i` created", ebo.id);
    return ebo;
}

static inline void ebo_destroy(const ebo_t *ebo)
{
    if (ebo == NULL) eprint("ebo_bind: ebo is null argument");

    GL_LOG("EBO `%i` deleted", ebo->id);
    GL_CHECK(glDeleteBuffers(1, &ebo->id));
}




#endif 
