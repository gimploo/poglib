#ifndef __GL__ENTITY__H__
#define __GL__ENTITY__H__

#include "internal/VAO.h"
#include "internal/VBO.h"
#include "internal/EBO.h"

#include "shader.h"
#include "texture.h"


typedef struct gl_object_t gl_object_t;

struct gl_object_t {

    VertexArray     vao;
    VertexBuffer    vbo;
    ElementBuffer   ebo;

    Shader          *shader;
    Texture2D       *texture2d;
};


static inline gl_object_t gl_object_init(void * vertices, size_t vsize, unsigned int *indices, size_t icount, Shader *shader, Texture2D *texture)
{
    gl_object_t obj;

    obj.vao = vao_init();
    obj.vbo = vbo_init(vertices, vsize);
    obj.ebo = ebo_init(indices, icount);

    if (shader != NULL) obj.shader = shader;
    else obj.shader = NULL;

    if (texture != NULL) obj.texture2d = texture;
    else obj.texture2d = NULL;

    return obj;
}

static inline void gl_object_draw(gl_object_t *entity, GLenum mode)
{
    if (entity == NULL) eprint("entity_draw: entity argument is null");

    if (entity->shader)     shader_bind(entity->shader);
    if (entity->texture2d)  texture_bind(entity->texture2d, 0);

    glDrawElements(mode, entity->ebo.count, GL_UNSIGNED_INT, 0);
}

static inline void gl_object_destroy(gl_object_t *entity)
{
    if (entity == NULL) eprint("entity_draw: entity argument is null");

    ebo_destroy(&entity->ebo);
    vbo_destroy(&entity->vbo);
    vao_destroy(&entity->vao);

    shader_destroy(entity->shader);
    texture_destroy(entity->texture2d);
}



#endif //__GL__ENTITY__H__
