#pragma once
#include "common.h"

typedef struct vao_t { GLuint  id; } vao_t ;

typedef struct vbo_t {

    GLuint  id; 
    u64     vertex_count;
    i64     __attribute_index; 

} vbo_t ;

typedef struct ebo_t {

    vbo_t   *vbo;
    GLuint  id;
    u32     indices_count;

} ebo_t ;

vbo_t           vbo_static_init(const void *vertices, const size_t vsize, const u64 count);

#define         vbo_bind(PVBO)                                  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, (PVBO)->id))
#define         vbo_unbind()                                    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0))
void            vbo_destroy(const vbo_t *);


ebo_t           ebo_init(vbo_t *vbo, const u32 *array, const u32 nmemb);
#define         ebo_bind(PEBO)  GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (PEBO)->id))
void            ebo_destroy(const ebo_t *ebo);


vao_t           vao_init(void);
#define         vao_bind(PVAO)                                   GL_CHECK(glBindVertexArray((PVAO)->id))
void            vao_set_attributes( vao_t *vao, vbo_t *vbo, u8 component_count, GLenum type, bool normalized, size_t stride, size_t offset);
#define         vao_unbind()                                    GL_CHECK(glBindVertexArray(0))
#define         vao_draw_with_vbo(PVAO, PVBO)                   __impl_vao_draw_with_vbo(PVAO, PVBO, GL_TRIANGLES)
#define         vao_draw_with_vbo_in_mode(PVAO, PVBO, MODE)     __impl_vao_draw_with_vbo(PVAO, PVBO, MODE)
#define         vao_draw_with_ebo(PVAO, PEBO)                   __impl_vao_draw_with_ebo(PVAO, PEBO, GL_TRIANGLES)
#define         vao_draw_with_ebo_in_mode(PVAO, PEBO, MODE)     __impl_vao_draw_with_ebo(PVAO, PEBO, MODE)
void            vao_destroy(const vao_t *vao);




#ifndef IGNORE_GL_OBJECTS_IMPLEMENTATION

//vbo

void vbo_destroy(const vbo_t *obj)
{
    if (obj == NULL) eprint("vbo_destroy: obj argument is null\n");

    GL_LOG("VBO `%i` deleted", obj->id);
    GL_CHECK(glDeleteBuffers(1, &obj->id));
}


vbo_t vbo_static_init(
        const void *vertices, 
        const size_t vsize, 
        const u64 vertex_count)
{
    ASSERT(vsize != 8);

    vbo_t VBO = {
        .vertex_count        = vertex_count,
        .__attribute_index   = -1,
    };

    GL_CHECK(glGenBuffers(1, &VBO.id)); 
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO.id));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vsize, vertices, GL_STATIC_DRAW));
    if (vertices != NULL) {
        ASSERT(vertex_count > 0);
        GL_LOG("VBO (STATIC)\t `%i` created", VBO.id);
    } else {
        GL_LOG("EMPTY VBO (STATIC)\t `%i` created", VBO.id);
    }

    return VBO;
}

//ebo

ebo_t ebo_init(vbo_t *vbo, const u32 *indices, const u32 nindices)
{
    if (indices == NULL) eprint("ebo_init: indices argument is null");
    if (nindices <= 0) eprint("ebo_init: isize is not greater than 0");

    ASSERT(vbo);

    ebo_t ebo = {
        .vbo = vbo,
        .indices_count = nindices,
    };

    GL_CHECK(glGenBuffers(1, &ebo.id));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.id));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, nindices * sizeof(u32), indices, GL_STATIC_DRAW));


    GL_LOG("EBO `%i` created", ebo.id);
    return ebo;
}

void ebo_destroy(const ebo_t *ebo)
{
    if (ebo == NULL) eprint("ebo_bind: ebo is null argument");

    GL_LOG("EBO `%i` deleted", ebo->id);
    GL_CHECK(glDeleteBuffers(1, &ebo->id));
}

//vao

vao_t vao_init(void)
{
    vao_t vao;

    GL_CHECK(glGenVertexArrays(1, &vao.id)); 
    
    GL_LOG("VAO `%i` created", vao.id);

    return vao;
}

void vao_set_attributes(
            vao_t *vao,
            vbo_t *vbo,
            u8 component_count, 
            GLenum type,
            bool normalized,
            size_t stride,
            size_t offset
)
{
    if (vao == NULL) eprint("vao_set_attribute: vao argument is null");
    if (vbo == NULL) eprint("vbo argument is null");

    GL_CHECK(glEnableVertexAttribArray(++vbo->__attribute_index));

    switch(type)
    {
        case GL_INT:
            GL_CHECK(glVertexAttribIPointer(
                        vbo->__attribute_index,
                        component_count, 
                        GL_INT,
                        stride,
                        (const void *)offset));
        break;
        case GL_FLOAT:
            GL_CHECK(glVertexAttribPointer(
                        vbo->__attribute_index,
                        component_count, 
                        GL_FLOAT, 
                        normalized == false ? GL_FALSE : GL_TRUE, 
                        stride, 
                        (const void *)offset));
        break;
        default: eprint("Not implemented");
    }
    GL_CHECK(glVertexAttribDivisor(vbo->__attribute_index, 0));
}

void __impl_vao_draw_with_vbo(const vao_t *vao, const vbo_t *vbo, u64 gldraw_mode)
{
    if (vao == NULL) eprint("vao_draw: vao argument is null");

    if (vbo->vertex_count == 0) eprint("vao_draw: vbo`s vertex_count is %lu", vbo->vertex_count);

    GL_CHECK(glDrawArrays(
                gldraw_mode, 
                0, 
                vbo->vertex_count));

    GL_CHECK(glBindVertexArray(0));
}

void __impl_vao_draw_with_ebo(const vao_t *vao, const ebo_t *ebo, const u64 gldraw_mode)
{
    if (vao == NULL) eprint("vao_draw: vao argument is null");

    if (ebo->indices_count == 0) eprint("vao_draw: vbo`s indices_count is %i", ebo->indices_count);

    GL_CHECK(glDrawElements(
                gldraw_mode, 
                ebo->indices_count, 
                GL_UNSIGNED_INT, 
                0));
    GL_CHECK(glBindVertexArray(0));
}


void vao_destroy(const vao_t *vao)
{
    if (vao == NULL) eprint("vao_bind: vao argument is null");

    GL_LOG("VAO `%i` deleted", vao->id);
    GL_CHECK(glDeleteVertexArrays(1, &vao->id));
}

#endif
