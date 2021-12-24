#ifndef __VBO__H__
#define __VBO__H__

#include "_common.h"

/*========================================================================
 // GL Vertex Buffer Object Abstraction (internal)
========================================================================*/
typedef enum vbo_type {

    VBO_TYPE_STATIC = 0,
    VBO_TYPE_DYNAMIC,
    VBO_TYPE_COUNT

} vbo_type;

typedef struct vbo_t {

    GLuint      id; 
    vbo_type    type;
    u32         indices_count;
    u32         attribute_count;
    i32         attribute_index; // this is basically (count-1), i have this to avoid possible headaches in the future

} vbo_t ;


/*------------------------------------------------------------------------
 // Declarations
------------------------------------------------------------------------*/

//NOTE:(macro)          vbo_init(vertices[], vsize) -> vbo_t
static inline vbo_t     vbo_static_init(const void *vertices, const size_t vsize);
static inline vbo_t     vbo_dynamic_init(const size_t vsize);

static inline void      vbo_bind(const vbo_t *obj);
static inline void      vbo_unbind(void);

static inline void      vbo_destroy(const vbo_t *obj);

/*------------------------------------------------------------------------
 // Implementations
------------------------------------------------------------------------*/

#define vbo_init(v, s)  vbo_static_init(v, s)

static inline vbo_t vbo_static_init(const void *vertices, const size_t vsize)
{
    vbo_t VBO;

    GL_CHECK(glGenBuffers(1, &VBO.id)); 
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO.id));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vsize, vertices, GL_STATIC_DRAW));

    VBO.type = VBO_TYPE_STATIC;

    VBO.indices_count = 0;

    VBO.attribute_index = -1;
    VBO.attribute_count = 0;

    GL_LOG("VBO (STATIC)\t `%i` created", VBO.id);
    vbo_unbind();
    return VBO;
}

static inline vbo_t vbo_dynamic_init(const size_t vsize)
{
    vbo_t VBO;

    GL_CHECK(glGenBuffers(1, &VBO.id)); 
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO.id));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vsize, NULL, GL_DYNAMIC_DRAW));

    VBO.type = VBO_TYPE_DYNAMIC;

    VBO.indices_count = 0;

    VBO.attribute_index = -1;
    VBO.attribute_count = 0;

    GL_LOG("VBO (DYNAMIC)\t `%i` created", VBO.id);

    vbo_unbind();
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
