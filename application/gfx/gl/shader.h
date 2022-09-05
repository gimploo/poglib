#pragma once
#include <poglib/application/gfx/gl/common.h>
#include <poglib/ds/list.h>

/*==============================================================================
                    - OPENGL SHADER HANDLING LIBRARY -
===============================================================================*/

typedef struct glshader_t {

    GLuint      id;
    const char  *vs_file_path;
    const char  *fg_file_path;

} glshader_t;

const char * const DEFAULT_VSHADER = 
    "#version 330 core\n"
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
    "#version 330 core\n"
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


#define         glshader_default_init(...)                                      glshader_from_cstr_init(DEFAULT_VSHADER, DEFAULT_FSHADER)

glshader_t      glshader_from_file_init(const char *file_vs, const char *file_fs);
glshader_t      glshader_from_cstr_init(const char *vs_code, const char *fs_code);

void            glshader_send_uniform_fval(const glshader_t *shader, const char *uniform, float val);
void            glshader_send_uniform_uival(const glshader_t *shader, const char *uniform, unsigned int val);
void            glshader_send_uniform_ival(const glshader_t *shader, const char *uniform, int val);
void            glshader_send_uniform_vec2f(const glshader_t *shader, const char *uniform, vec2f_t val);
void            glshader_send_uniform_vec3f(const glshader_t *shader, const char *uniform, vec3f_t val);
void            glshader_send_uniform_vec4f(const glshader_t *shader, const char *uniform, vec4f_t val);
void            glshader_send_uniform_matrix4f(const glshader_t *shader, const char *uniform, matrix4f_t val);

void            glshader_bind(const glshader_t *shader);

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

static inline void __shader_load_code(glshader_t *shader, const char *vs_code, const char *fs_code)
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
        eprint("Vertex Error:\n\t%s\n", error_log);
    }
    GL_LOG("Vertex Shader successfully compiled");

    GLuint fragmentShader;
    GL_CHECK(fragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
    GL_CHECK(glShaderSource(fragmentShader, 1, &fs_code, NULL));
    GL_CHECK(glCompileShader(fragmentShader));
    GL_CHECK(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status));
    if (!status) {
        GL_CHECK(glGetShaderInfoLog(fragmentShader, KB, NULL, error_log));
        eprint("Fragment Error:\n\t%s\n", error_log);
    }
    GL_LOG("Fragment Shader successfully compiled");

    GLuint shaderProgram; 
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


static inline void __shader_load_from_file(glshader_t *shader, const char *vertex_source_path, const char *fragment_source_path)
{
    if (shader == NULL) eprint("shader argument is null");

    char vs_code[KB] = {0}; 
    char fs_code[KB] = {0}; 

    file_t *vs_file = file_init(vertex_source_path, "r");
        file_readall(vs_file, vs_code, sizeof(vs_code));
    file_destroy(vs_file);

    file_t *fg_file = file_init(fragment_source_path, "r");
        file_readall(fg_file, fs_code, sizeof(fs_code));
    file_destroy(fg_file);

    __shader_load_code(shader, vs_code, fs_code);
}

glshader_t glshader_from_file_init(const char *file_vs, const char *file_fs)
{
    if (file_vs == NULL) eprint("file_vs argument is null");
    if (file_fs == NULL) eprint("file_fs arguemnt is null");

    glshader_t shader;
    shader.fg_file_path = file_fs;
    shader.vs_file_path = file_vs;

    __shader_load_from_file(&shader, file_vs, file_fs);

    return shader;
}

glshader_t  glshader_from_cstr_init(const char *vs_code, const char *fs_code)
{
    if (vs_code == NULL) eprint("vs_code argument is null");
    if (fs_code == NULL) eprint("fs_code arguemnt is null");

    glshader_t shader;
    shader.fg_file_path = NULL;
    shader.vs_file_path = NULL;

    __shader_load_code(&shader, vs_code, fs_code);

    return shader;
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
    GL_CHECK(glUniform3f(location, val[0], val[1], val[2]));
}

void glshader_send_uniform_vec4f(const glshader_t *shader, const char *uniform, vec4f_t val)
{
    GL_SHADER_BIND(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] `%s` uniform doesnt exist", uniform);
    GL_CHECK(glUniform4f(location, val[0], val[1], val[2], val[3]));
}

void glshader_send_uniform_vec2f(const glshader_t *shader, const char *uniform, vec2f_t val)
{
    GL_SHADER_BIND(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] `%s` uniform doesnt exist", uniform);
    GL_CHECK(glUniform2f(location, val[0], val[1]));
}

void glshader_send_uniform_matrix4f(const glshader_t *shader, const char *uniform, matrix4f_t val)
{
    GL_SHADER_BIND(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] `%s` uniform doesnt exist", uniform);

    GL_CHECK(glUniformMatrix4fv(location, 1, GL_TRUE, (f32 *)val));
}



void glshader_destroy(glshader_t *shader)
{
    if (shader == NULL) eprint("shader argument is null");

    GL_CHECK(glDeleteProgram(shader->id));

    GL_LOG("Shader `%i` successfully deleted", shader->id);
}
#endif
