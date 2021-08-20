#ifndef __GL_SHADER_H__
#define __GL_SHADER_H__

/*=======================================================
 // OpenGL shader handling library
=======================================================*/

#include <string.h>
#include "_common.h"
#include "../../generics.h"

#define MAX_UNIFORM_ARRAY_CAPACITY 10
#define MAX_UNIFORM_VALUE_SIZE (4 * sizeof(f32))

const char * const default_vshader = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 v_pos;\n"
    "layout (location = 1) in vec3 v_color;\n"
    "layout (location = 2) in vec2 v_tex_coord;\n"
    "\n"
    "out vec3 color;\n"
    "out vec2 tex_coord;\n"
    "\n"
    "void main()\n"
    "{\n"
        "gl_Position = vec4(v_pos, 1.0f);\n"
        "color = v_color;\n"
        "tex_coord = v_tex_coord;\n"
    "}";

const char * const default_fshader = 
    "#version 330 core\n"
    "in vec3 color;\n"
    "in vec2 tex_coord;\n"
    "\n"
    "uniform sampler2D u_texture01;\n"
    "\n"
    "out vec4 FragColor;\n"
    "\n"
    "void main()\n"
    "{\n"
        "FragColor = texture(u_texture01, tex_coord) + vec4(color, 1.0f);\n"
    "}";


typedef struct gl_shader_t {

    GLuint      id;
    const char  *vs_file_path;
    const char  *fg_file_path;
    stack_t     uniforms;

} gl_shader_t;


typedef enum gl_shader_uniform_type {

    UT_UINT = 0,
    UT_INT,
    UT_FLOAT,

    UT_VEC3F,
    UT_VEC4F,

    UT_COUNT

} gl_uniform_type;

typedef struct __uniform_meta_data_t {

    const char                      *variable_name;
    const u64                       data_size;
    const gl_uniform_type           uniform_type;
    u8                              data_buffer[MAX_UNIFORM_VALUE_SIZE];

} __uniform_meta_data_t;


/*-----------------------------------------------------
 // Declarations
-----------------------------------------------------*/

//NOTE:(macro)  gl_shader_default_init(void) --> gl_shader_t

gl_shader_t     gl_shader_from_file_init(const char *file_vs, const char *file_fs);
gl_shader_t     gl_shader_from_cstr_init(const char *vs_code, const char *fs_code);

void            gl_shader_bind(gl_shader_t *shader);
void            gl_shader_push_uniform(gl_shader_t *shader, const char *uniform_name, void *value, u64 value_size, gl_uniform_type type);

void            gl_shader_destroy(const gl_shader_t *shader);


/*--------------------------------------------------------
 // Implementation
--------------------------------------------------------*/
#define GL_SHADER_BIND(pshader) GL_CHECK(glUseProgram((pshader)->id));

#define gl_shader_default_init() gl_shader_from_cstr_init(default_vshader, default_fshader)

static inline void __shader_load_code(gl_shader_t *shader, const char *vs_code, const char *fs_code)
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


static inline void __shader_load_from_file(gl_shader_t *shader, const char *vertex_source_path, const char *fragment_source_path)
{
    if (shader == NULL) eprint("shader argument is null");

    int status;
    char error_log[KB] = {0};

    file_t vsfile = file_init(vertex_source_path);
    const char * vtxfile = file_readall(&vsfile);
    if (vtxfile == NULL) {
        fprintf(stderr, "%s: vertex file returned null\n", __func__);
        exit(1);
    }


    file_t fgfile = file_init(fragment_source_path);
    const char *frgfile = file_readall(&fgfile);
    if (frgfile == NULL) {
        fprintf(stderr, "%s: vertex file returned null\n", __func__);
        exit(1);
    }

    GLuint vertexShader;
    GL_CHECK(vertexShader = glCreateShader(GL_VERTEX_SHADER));
    GL_CHECK(glShaderSource(vertexShader, 1, &vtxfile, NULL));
    GL_CHECK(glCompileShader(vertexShader));
    GL_CHECK(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status));
    if (!status) {
        glGetShaderInfoLog(vertexShader, KB, NULL, error_log);
        eprint("Vertex Error:\n\t%s\n", error_log);
    }
    GL_LOG("Vertex Shader successfully compiled");

    GLuint fragmentShader;
    GL_CHECK(fragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
    GL_CHECK(glShaderSource(fragmentShader, 1, &frgfile, NULL));
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

gl_shader_t  gl_shader_from_file_init(const char *file_vs, const char *file_fs)
{
    if (file_vs == NULL) eprint("file_vs argument is null");
    if (file_fs == NULL) eprint("file_fs arguemnt is null");

    gl_shader_t shader;
    shader.fg_file_path = file_fs;
    shader.vs_file_path = file_vs;

    __shader_load_from_file(&shader, file_vs, file_fs);

    __uniform_meta_data_t *array = (__uniform_meta_data_t *)malloc(MAX_UNIFORM_ARRAY_CAPACITY * sizeof(__uniform_meta_data_t));
    if (array == NULL) eprint("malloc failed");

    shader.uniforms = stack_init((void **)array, MAX_UNIFORM_ARRAY_CAPACITY);

    return shader;
}

gl_shader_t  gl_shader_from_cstr_init(const char *vs_code, const char *fs_code)
{
    if (vs_code == NULL) eprint("vs_code argument is null");
    if (fs_code == NULL) eprint("fs_code arguemnt is null");

    gl_shader_t shader;
    shader.fg_file_path = NULL;
    shader.vs_file_path = NULL;

    __shader_load_code(&shader, vs_code, fs_code);

    __uniform_meta_data_t *array = (__uniform_meta_data_t *)malloc(MAX_UNIFORM_ARRAY_CAPACITY * sizeof(__uniform_meta_data_t));
    if (array == NULL) eprint("malloc failed");

    shader.uniforms = stack_init((void **)array, MAX_UNIFORM_ARRAY_CAPACITY);

    return shader;
}


void gl_shader_uniform_set_ival(gl_shader_t *shader, const char *uniform, int val)
{
    if (shader == NULL) eprint("shader argument is null");
    if (uniform == NULL) eprint("uniform argument is null");

    GL_SHADER_BIND(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");

    GL_CHECK(glUniform1i(location, val));
}

void gl_shader_uniform_set_uival(gl_shader_t *shader, const char *uniform, unsigned int val)
{
    if (shader == NULL) eprint("shader argument is null");
    if (uniform == NULL) eprint("uniform argument is null");

    GL_SHADER_BIND(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform1ui(location, val));
}

void gl_shader_uniform_set_fval(gl_shader_t *shader, const char *uniform, float val)
{
    if (shader == NULL) eprint("shader argument is null");
    if (uniform == NULL) eprint("uniform argument is null");

    GL_SHADER_BIND(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform1f(location, val));
}

void gl_shader_uniform_set_vec3f(gl_shader_t *shader, const char *uniform, vec3f_t val)
{
    if (shader == NULL) eprint("shader argument is null");
    if (uniform == NULL) eprint("uniform argument is null");

    GL_SHADER_BIND(shader);

    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform3f(location, val.cmp[0], val.cmp[1], val.cmp[2]));
}

void gl_shader_uniform_set_vec4f(gl_shader_t *shader, const char *uniform, vec4f_t val)
{
    if (shader == NULL) eprint("shader argument is null");
    if (uniform == NULL) eprint("uniform argument is null");

    GL_SHADER_BIND(shader);

    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform4f(location, val.cmp[0], val.cmp[1], val.cmp[2], val.cmp[3]));
}



inline void gl_shader_destroy(const gl_shader_t *shader)
{
    if (shader == NULL) eprint("shader argument is null");

    free(shader->uniforms.array);

    GL_CHECK(glDeleteProgram(shader->id));
    GL_LOG("gl_shader_t `%i` successfully deleted", shader->id);
}


void gl_shader_bind(gl_shader_t *shader)
{
    if (shader == NULL) eprint("shader argument is null");

    for (int i = 0; i <= shader->uniforms.top; i++)
    {
        __uniform_meta_data_t *uniform = (__uniform_meta_data_t *)shader->uniforms.array + i;
        GL_LOG("Shader `%i` setting uniform `%s`", shader->id, uniform->variable_name);

        switch(uniform->uniform_type)
        {
            case UT_UINT:
                gl_shader_uniform_set_uival(shader, uniform->variable_name, *(u32 *)uniform->data_buffer);
                break;
            case UT_INT:
                gl_shader_uniform_set_ival(shader, uniform->variable_name, *(i32 *)uniform->data_buffer);
                break;
            case UT_FLOAT:
                gl_shader_uniform_set_fval(shader, uniform->variable_name, *(f32 *)uniform->data_buffer);
                break;
            case UT_VEC3F:
                gl_shader_uniform_set_vec3f(shader, uniform->variable_name, *(vec3f_t *)uniform->data_buffer);
                break;
            case UT_VEC4F:
                gl_shader_uniform_set_vec4f(shader, uniform->variable_name, *(vec4f_t *)uniform->data_buffer);
                break;
            default:
                eprint("uniform type not accounted for");
        }
    }
        
}

__uniform_meta_data_t __uniform_meta_data_init(const char *var_name, void *data_ref, const u64 data_size, const gl_uniform_type type)
{
    if (var_name == NULL) eprint("var_name argument is null");
    if (data_ref == NULL) eprint("data_ref argument is null");

    assert(data_size <= MAX_UNIFORM_VALUE_SIZE);

    __uniform_meta_data_t data = {
        .variable_name  = var_name,
        .data_size      = data_size,
        .uniform_type   = type,
    };

    memcpy(data.data_buffer, data_ref, data_size); 

    return data;
}


void gl_shader_push_uniform(gl_shader_t *shader, const char *uniform_name, void *value, u64 value_size, gl_uniform_type type) 
{
    if (shader == NULL) eprint("shader arg is null");    
    if (value == NULL) eprint("value arg is null");

    __uniform_meta_data_t data = __uniform_meta_data_init(
            uniform_name, 
            value, 
            value_size, 
            type);

    stack_push(&shader->uniforms, data);

}



#endif //_GL_SHADER_H_
