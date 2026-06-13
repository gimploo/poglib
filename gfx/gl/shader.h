#pragma once
#include "common.h"
#include "material.h"
#include "maps.h"
#include "poglib/basic/arena.h"
#include "poglib/basic/ds/hashtable.h"
#include "poglib/basic/str.h"
#include <string.h>

/*==============================================================================
                    - OPENGL SHADER HANDLING LIBRARY -
===============================================================================*/

//TODO: maybe have all the shader uniform locations localized to one place, 
//so not to need to call 'get location' every time we send it.

#define MAX_UNIFORMS_ALLOWED_IN_SHADER 16

typedef enum gluniform_type {
    GL_UNIFORM_TYPE_MATRIX4F,
    GL_UNIFORM_TYPE_VEC4F,
    GL_UNIFORM_TYPE_VEC3F,
    GL_UNIFORM_TYPE_VEC2F,
    GL_UNIFORM_TYPE_F32,
    GL_UNIFORM_TYPE_I32,
    GL_UNIFORM_TYPE_U32,
    GL_UNIFORM_TYPE_BOOL,
    GL_UNIFORM_TYPE_MATRIX4F_ARRAY,
} gluniform_type;

typedef struct {
    str_t name;
    gluniform_type type;
} gluniform_meta_t;

typedef struct {
    str_t name;
    gluniform_type type;
    struct {
        u32 loc_idx;
    } internal;
} gluniform__internal_meta_t;

typedef union {
    matrix4f_t  mat4;
    vec4f_t     vec4;
    vec3f_t     vec3;
    vec2f_t     vec2;
    f32         f32;
    i32         i32;
    u32         u32;
    bool        boolean;
    struct {
        matrix4f_t *data;
        u32 count;
    } mat4s;
} gluniform_value_t;

typedef struct {
    u8 count;
    gluniform_meta_t data[MAX_UNIFORMS_ALLOWED_IN_SHADER];
} gluniform_registry_t;

typedef struct {
    u8 count;
    struct {
        str_t name;
        gluniform_value_t value;
    } data[MAX_UNIFORMS_ALLOWED_IN_SHADER];
} gluniforms_t;

typedef struct {

    u32 id;
    str_t vs;
    str_t fg;

    struct {
        hashtable_t uniformlocs;
    } internal;

} glshader_t;


const char * const DEFAULT_VSHADER = 
    "#version 430 core\n"
    "layout (location = 0) in vec3 v_pos;\n"
    "layout (location = 1) in vec4 v_color;\n"
    "layout (location = 2) in vec2 v_tex_coord;\n"
    "\n"
    "out vec4 color;\n"
    "out vec2 tex_coord;\n"
    "\n"
    "void main()\n"
    "{\n"
        "gl_Position = vec4(v_pos, 1.0f);\n"
        "color = v_color;\n"
        "tex_coord = v_tex_coord;\n"
    "}";

const char * const DEFAULT_FSHADER = 
    "#version 430 core\n"
    "in vec4 color;\n"
    "in vec2 tex_coord;\n"
    "\n"
    "uniform sampler2D u_texture01;\n"
    "\n"
    "out vec4 FragColor;\n"
    "\n"
    "void main()\n"
    "{\n"
        "FragColor = \n"
        "   texture(u_texture01, tex_coord) * color;\n"
        "\n"
    "}";

const char * const DEFAULT_SIMPLE_SHAPES_VSHADER = 
    "#version 430 core\n"
    "layout (location = 0) in vec3 v_pos;\n"
    "layout (location = 1) in vec4 v_uv;\n"
    "layout (location = 2) in vec2 v_normals;\n"
    "\n"
    "out vec4 color;\n"
    "out vec2 tex_coord;\n"
    "\n"
    "void main()\n"
    "{\n"
        "gl_Position = vec4(v_pos, 1.0f);\n"
        "color = v_color;\n"
        "tex_coord = v_tex_coord;\n"
    "}";

#define         glshader_default_init(...)                                      glshader_from_cstr_init(DEFAULT_VSHADER, DEFAULT_FSHADER)

glshader_t              glshader_init(const str_t vtxpath, const str_t fgpath, const gluniform_registry_t uniforms, arena_t * const arena);

glshader_t              glshader_from_file_init(const char *file_vs, const char *file_fs);
glshader_t              glshader_from_cstr_init(const char *vs_code, const char *fs_code);

//========================= Uniforms ======================================

void            glshader_upload_uniforms(const glshader_t * const shader, const gluniforms_t uniforms);

//                          (or)

void            glshader_send_uniform_fval(const glshader_t *shader, const char *uniform, float val);
void            glshader_send_uniform_uival(const glshader_t *shader, const char *uniform, unsigned int val);
void            glshader_send_uniform_ival(const glshader_t *shader, const char *uniform, int val);
void            glshader_send_uniform_vec2f(const glshader_t *shader, const char *uniform, vec2f_t val);
void            glshader_send_uniform_vec3f(const glshader_t *shader, const char *uniform, vec3f_t val);
void            glshader_send_uniform_vec4f(const glshader_t *shader, const char *uniform, vec4f_t val);
void            glshader_send_uniform_matrix4f(const glshader_t *shader, const char *uniform, matrix4f_t val);
void            glshader_send_uniform_matrix4fv(const glshader_t *shader, const char *uniform, const matrix4f_t *val, const u32 matrices_count);

//=========================================================================

void            glshader_bind(const glshader_t *shader);
u32             glshader_get_uniform_count(const glshader_t * const shader);

void            glshader_destroy(glshader_t *shader);

/*-----------------------------------------------------------------------------------
                                -- IMPLEMENTATION --
-----------------------------------------------------------------------------------*/

#ifndef IGNORE_GL_SHADER_IMPLEMENTATION 

#define GL_SHADER_BIND(pshader) GL_CHECK(glUseProgram((pshader)->id));

void glshader_bind(const glshader_t *shader)
{
    if (shader == NULL) eprint("shader argument is null");

    GL_SHADER_BIND(shader);
}

void glshader__internal_compile_shader(glshader_t *shader, const char *vs_code, const char *fs_code)
{
    if (shader == NULL) eprint("shader argument is null");

    int status;
    char error_log[KB] = {0};

    GLuint vertexShader;
    GL_CHECK(vertexShader = glCreateShader(GL_VERTEX_SHADER));
    GL_CHECK(glShaderSource(vertexShader, 1, &vs_code, NULL));
    GL_CHECK(glCompileShader(vertexShader));
    GL_CHECK(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status));
    if (!status) {
        GL_CHECK(glGetShaderInfoLog(vertexShader, KB, NULL, error_log));
        eprint("FILE: %s\n\tVertex Error:\n\t%s\n", shader->vs.data, error_log);
    }
    GL_LOG("Vertex Shader successfully compiled");

    GLuint fragmentShader;
    GL_CHECK(fragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
    GL_CHECK(glShaderSource(fragmentShader, 1, &fs_code, NULL));
    GL_CHECK(glCompileShader(fragmentShader));
    GL_CHECK(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status));
    if (!status) {
        GL_CHECK(glGetShaderInfoLog(fragmentShader, KB, NULL, error_log));
        eprint("FILE: %s\n\tFragment Error:\n\t%s\n", shader->fg.data, error_log);
    }
    GL_LOG("Fragment Shader successfully compiled");

    u32 shaderProgram; 
    GL_CHECK(shaderProgram = glCreateProgram());
    GL_CHECK(glAttachShader(shaderProgram, vertexShader));
    GL_CHECK(glAttachShader(shaderProgram, fragmentShader));
    GL_CHECK(glLinkProgram(shaderProgram));
    GL_CHECK(glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status));
    if(!status) {
        GL_CHECK(glGetProgramInfoLog(shaderProgram, KB, NULL, error_log));
        eprint("Error: %s\n", error_log);
    }

    GL_CHECK(glDeleteShader(vertexShader));
    GL_CHECK(glDeleteShader(fragmentShader));

    shader->id = shaderProgram;

    GL_LOG("Shader `%i` successfully linked", shader->id);
}


static inline void glshader__internal_load_from_file(glshader_t *shader, const char *vertex_source_path, const char *fragment_source_path, arena_t * const arena)
{
    if (shader == NULL) eprint("shader argument is null");

    file_t vs_file = file_init(vertex_source_path, "r");
        char *vs_code = arena 
            ? arena_reserve(arena, vs_file.size + 1) 
            : (char *)calloc(1, vs_file.size + 1);
        file_readall(&vs_file, vs_code, vs_file.size);
    file_destroy(&vs_file);

    file_t fg_file = file_init(fragment_source_path, "r");
        char *fs_code = arena 
            ? arena_reserve(arena, fg_file.size + 1)
            : (char *)calloc(1, fg_file.size + 1);
        file_readall(&fg_file, fs_code, fg_file.size);
    file_destroy(&fg_file);

    glshader__internal_compile_shader(shader, vs_code, fs_code);

    if (!arena) {
        free(vs_code);
        free(fs_code);
    }
}

glshader_t glshader_from_file_init(const char *file_vs, const char *file_fs)
{
    if (file_vs == NULL) eprint("file_vs argument is null");
    if (file_fs == NULL) eprint("file_fs arguemnt is null");

    glshader_t shader = {
        .vs = str__from_cstr(file_vs, strlen(file_vs)),
        .fg = str__from_cstr(file_fs, strlen(file_vs)),
    };
    glshader__internal_load_from_file(&shader, file_vs, file_fs, NULL);

    return shader;
}

glshader_t glshader_from_cstr_init(const char *vs_code, const char *fs_code)
{
    if (vs_code == NULL) eprint("vs_code argument is null");
    if (fs_code == NULL) eprint("fs_code arguemnt is null");

    glshader_t shader = {
        .vs = {0},
        .fg = {0}
    };

    glshader__internal_compile_shader(&shader, vs_code, fs_code);

    return shader;
}


hashtable_t glshader__internal_uniforms_cache_locs(const u32 shader_id, const gluniform_meta_t *uniforms_array, const u16 uniforms_count, arena_t * const arena)
{
    ASSERT(uniforms_array);
    if (!uniforms_count) 
        return (hashtable_t){0};

    hashtable_t result = hashtable_init(
        MAX_UNIFORMS_ALLOWED_IN_SHADER,
        HT_KEY_TYPE_STR,
        (ht_value_type) {
            .size = sizeof(gluniform__internal_meta_t),
            .type = HT_STORAGE_BY_REFERENCE 
        },
        arena
    );

    for (u32 idx = 0; idx < uniforms_count; idx++)
    {
        const char *name = uniforms_array[idx].name.data;
        if (!name) eprint("Uniform is null, re-check uniform registry for the shader");

        i32 location;
        GL_CHECK(location = glGetUniformLocation(shader_id, name));
        if (location == -1) 
            eprint("[ERROR] `%s` uniform doesnt exist", name);

        const gluniform__internal_meta_t * const meta = arena_store(
            arena, 
            &(gluniform__internal_meta_t) {
                .name = uniforms_array[idx].name,
                .type = uniforms_array[idx].type,
                .internal = {
                    .loc_idx = location
                }
            },
            sizeof(gluniform__internal_meta_t)
        );
        hashtable_insert(&result, (hashtable_key_t){ .str = uniforms_array[idx].name }, meta);
    }
    return result;
}

void glshader_upload_uniforms(const glshader_t * const shader, const gluniforms_t uniforms)
{
    if (!uniforms.count)
        return;

    ASSERT(shader->internal.uniformlocs.entries.len == uniforms.count);

    for (u8 idx = 0; idx < uniforms.count; idx++)
    {
        const str_t name = uniforms.data[idx].name;
        const gluniform_value_t * const value = &uniforms.data[idx].value;

        const gluniform__internal_meta_t *meta = (gluniform__internal_meta_t *)hashtable_get_value(&shader->internal.uniformlocs, (hashtable_key_t){ .str = name });
        const u32 loc_idx = meta->internal.loc_idx;

        switch(meta->type)
        {
            case GL_UNIFORM_TYPE_MATRIX4F:          GL_CHECK(glUniformMatrix4fv(loc_idx, 1, GL_FALSE, (f32 *)value->mat4.raw));
            break;
            case GL_UNIFORM_TYPE_VEC4F:              GL_CHECK(glUniform4f(loc_idx, value->vec4.raw[0], value->vec4.raw[1], value->vec4.raw[2], value->vec4.raw[3]));
            break;
            case GL_UNIFORM_TYPE_VEC3F:              GL_CHECK(glUniform3f(loc_idx, value->vec3.raw[0], value->vec3.raw[1], value->vec3.raw[2]));
            break;
            case GL_UNIFORM_TYPE_VEC2F:              GL_CHECK(glUniform2f(loc_idx, value->vec2.raw[0], value->vec2.raw[1]));
            break;
            case GL_UNIFORM_TYPE_F32:               GL_CHECK(glUniform1f(loc_idx, value->f32));
            break;
            case GL_UNIFORM_TYPE_I32:               GL_CHECK(glUniform1i(loc_idx, value->i32));
            break;
            case GL_UNIFORM_TYPE_U32:
            case GL_UNIFORM_TYPE_BOOL:              GL_CHECK(glUniform1ui(loc_idx, value->u32));
            break;
            case GL_UNIFORM_TYPE_MATRIX4F_ARRAY:    GL_CHECK(glUniformMatrix4fv(loc_idx, value->mat4s.count, GL_FALSE, (f32 *)value->mat4s.data));
            break;
                eprint("unknown uniform type");
        }
    }
}


void glshader_send_uniform_ival(const glshader_t *shader, const char *uniform, int val)
{
    GL_SHADER_BIND(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] `%s` uniform doesnt exist", uniform);

    GL_CHECK(glUniform1i(location, val));
}

void glshader_send_uniform_uival(const glshader_t *shader, const char *uniform, unsigned int val)
{
    GL_SHADER_BIND(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] `%s` uniform doesnt exist", uniform);
    GL_CHECK(glUniform1ui(location, val));
}

void glshader_send_uniform_fval(const glshader_t *shader, const char *uniform, float val)
{
    GL_SHADER_BIND(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] `%s` uniform doesnt exist", uniform);
    GL_CHECK(glUniform1f(location, val));
}

void glshader_send_uniform_vec3f(const glshader_t *shader, const char *uniform, vec3f_t val)
{
    GL_SHADER_BIND(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] `%s` uniform doesnt exist", uniform);
    GL_CHECK(glUniform3f(location, val.raw[0], val.raw[1], val.raw[2]));
}

void glshader_send_uniform_vec4f(const glshader_t *shader, const char *uniform, vec4f_t val)
{
    GL_SHADER_BIND(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] `%s` uniform doesnt exist", uniform);
    GL_CHECK(glUniform4f(location, val.raw[0], val.raw[1], val.raw[2], val.raw[3]));
}

void glshader_send_uniform_vec2f(const glshader_t *shader, const char *uniform, vec2f_t val)
{
    GL_SHADER_BIND(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] `%s` uniform doesnt exist", uniform);
    GL_CHECK(glUniform2f(location, val.raw[0], val.raw[1]));
}

void glshader_send_uniform_matrix4f(const glshader_t *shader, const char *uniform, matrix4f_t val)
{
    GL_SHADER_BIND(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] `%s` uniform doesnt exist", uniform);

    GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, &val.raw[0][0]));
}

void glshader_send_uniform_matrix4fv(const glshader_t *shader, const char *uniform, const matrix4f_t *val, const u32 matrices_count)
{
    GL_SHADER_BIND(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] `%s` uniform doesnt exist", uniform);

    GL_CHECK(glUniformMatrix4fv(location, matrices_count, GL_FALSE, (f32 *)val));
}

void glshader_destroy(glshader_t *shader)
{
    if (shader == NULL) eprint("shader argument is null");

    GL_CHECK(glDeleteProgram(shader->id));
    hashtable_destroy(&shader->internal.uniformlocs);

    GL_LOG("Shader `%i` successfully deleted", shader->id);
}

glshader_t glshader_init(
    const str_t vtxpath, 
    const str_t fgpath, 
    const gluniform_registry_t uniforms,
    arena_t *const arena)
{
    ASSERT(arena);

    glshader_t shader = {
        .fg = fgpath,
        .vs = fgpath,
    };

    glshader__internal_load_from_file(
        &shader, 
        vtxpath.data, 
        fgpath.data, 
        arena
    );

    shader.internal.uniformlocs = glshader__internal_uniforms_cache_locs(
        shader.id, 
        uniforms.data,
        uniforms.count,
        arena
    );

    return shader;
}


u32 glshader_get_uniform_count(const glshader_t * const shader)
{
    return shader->internal.uniformlocs.entries.len;
}
#endif
