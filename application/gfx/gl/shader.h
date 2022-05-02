#ifndef __GL_SHADER_H__
#define __GL_SHADER_H__

/*=======================================================
 // OpenGL shader handling library
=======================================================*/

//TODO: Get rid of the whole uniform system implementation and keep it
// raw 

#include <string.h>
#include <poglib/application/gfx/gl/common.h>

#define MAX_UNIFORM_ARRAY_CAPACITY 10
#define MAX_UNIFORM_VALUE_SIZE (sizeof(matrixf_t ))

const char * const default_vshader = 
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

const char * const default_fshader = 
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


typedef struct glshader_t {

    GLuint      id;
    const char  *vs_file_path;
    const char  *fg_file_path;
    queue_t     uniforms;

} glshader_t;


typedef enum glshader_uniform_type {

    UT_u32 = 0,
    UT_i32,
    UT_f32,

    UT_vec2f_t ,
    UT_vec3f_t,
    UT_vec4f_t,

    UT_matrix4f_t,

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

//NOTE:(macro)  glshader_default_init(void) --> glshader_t

glshader_t     glshader_from_file_init(const char *file_vs, const char *file_fs);
glshader_t     glshader_from_cstr_init(const char *vs_code, const char *fs_code);

void            glshader_bind(glshader_t *shader);
#define         glshader_set_uniform(PSHADER, UNIFORM_NAME, VALUE, TYPE) __impl_glshader_set_uniform((PSHADER), (UNIFORM_NAME), &(VALUE), sizeof(VALUE), UT_##TYPE, sizeof(TYPE)) 

void            glshader_destroy(const glshader_t *shader);


/*--------------------------------------------------------
 // Implementation
--------------------------------------------------------*/
#define GL_SHADER_BIND(pshader) GL_CHECK(glUseProgram((pshader)->id));

#define glshader_default_init() glshader_from_cstr_init(default_vshader, default_fshader)

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

    char vs_code[KB]; 
    char fs_code[KB]; 

    file_t vs_file = file_init(vertex_source_path);
        file_readall(&vs_file, vs_code, sizeof(vs_code));
    file_destroy(&vs_file);

    file_t fg_file = file_init(fragment_source_path);
        file_readall(&fg_file, fs_code, sizeof(fs_code));
    file_destroy(&fg_file);

    __shader_load_code(shader, vs_code, fs_code);

    GL_LOG("Shader `%i` successfully linked", shader->id);

}

glshader_t glshader_from_file_init(const char *file_vs, const char *file_fs)
{
    if (file_vs == NULL) eprint("file_vs argument is null");
    if (file_fs == NULL) eprint("file_fs arguemnt is null");

    glshader_t shader;
    shader.fg_file_path = file_fs;
    shader.vs_file_path = file_vs;

    __shader_load_from_file(&shader, file_vs, file_fs);

    shader.uniforms = queue_init(MAX_UNIFORM_ARRAY_CAPACITY, __uniform_meta_data_t );

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

    shader.uniforms = queue_init(MAX_UNIFORM_ARRAY_CAPACITY, __uniform_meta_data_t );

    return shader;
}


void __glshader_uniform_set_ival(const glshader_t *shader, const char *uniform, int val)
{
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");

    GL_CHECK(glUniform1i(location, val));
}

void __glshader_uniform_set_uival(const glshader_t *shader, const char *uniform, unsigned int val)
{
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform1ui(location, val));
}

void __glshader_uniform_set_fval(const glshader_t *shader, const char *uniform, float val)
{
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform1f(location, val));
}

void __glshader_uniform_set_vec3f(const glshader_t *shader, const char *uniform, vec3f_t val)
{
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform3f(location, val.cmp[0], val.cmp[1], val.cmp[2]));
}

void __glshader_uniform_set_vec4f(const glshader_t *shader, const char *uniform, vec4f_t val)
{
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform4f(location, val.cmp[0], val.cmp[1], val.cmp[2], val.cmp[3]));
}

void __glshader_uniform_set_vec2f(const glshader_t *shader, const char *uniform, vec2f_t val)
{
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform2f(location, val.cmp[0], val.cmp[1]));
}

void __glshader_uniform_set_matrix4f(const glshader_t *shader, const char *uniform, matrixf_t val)
{
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");

    vec4f_t matrix[4];
    memcpy(matrix, val.buffer, sizeof(matrix));
    GLfloat *value = (GLfloat *)&matrix;

    GL_CHECK(glUniformMatrix4fv(location, 1, GL_TRUE, value));
}



void glshader_destroy(const glshader_t *shader)
{
    if (shader == NULL) eprint("shader argument is null");

    GL_CHECK(glDeleteProgram(shader->id));

    queue_t *queue =(queue_t *) &shader->uniforms;
    queue_destroy(queue);

    GL_LOG("Shader `%i` successfully deleted", shader->id);
}



__uniform_meta_data_t __uniform_meta_data_init(const char *var_name, void *data_ref, const u64 data_size, const gl_uniform_type type)
{
    if (var_name == NULL) eprint("var_name argument is null");
    if (data_ref == NULL) eprint("data_ref argument is null");

    assert(data_size != MAX_UNIFORM_VALUE_SIZE);

    __uniform_meta_data_t data = {
        .variable_name  = var_name,
        .data_size      = data_size,
        .uniform_type   = type,
    };

    memcpy(data.data_buffer, data_ref, data_size); 

    return data;
}


void __impl_glshader_set_uniform(glshader_t *shader, const char *uniform_name, void *value_ref, u64 value_size, gl_uniform_type type, u64 type_size) 
{
    if (shader == NULL) eprint("shader arg is null");    
    if (value_ref == NULL) eprint("value ref is null");

    assert(type_size == value_size);

    __uniform_meta_data_t data = __uniform_meta_data_init(
            uniform_name, 
            value_ref, 
            value_size, 
            type);

    queue_put(&shader->uniforms, data);
}

void __glshader_get_uniform(glshader_t *shader)
{
    queue_get(&shader->uniforms);
}

void glshader_bind(glshader_t *shader)
{
    if (shader == NULL) eprint("shader argument is null");

    GL_SHADER_BIND(shader);

    queue_t *queue = &shader->uniforms;
    queue_iterator(queue, iter)
    {
        __uniform_meta_data_t *uniform = (__uniform_meta_data_t *)iter;
        GL_LOG("Shader `%i` setting uniform `%s`", shader->id, uniform->variable_name);

        switch(uniform->uniform_type)
        {
            case UT_u32:
                __glshader_uniform_set_uival(shader, uniform->variable_name, *(u32 *)uniform->data_buffer);
            break;

            case UT_i32:
                __glshader_uniform_set_ival(shader, uniform->variable_name, *(i32 *)uniform->data_buffer);
            break;

            case UT_f32:
                __glshader_uniform_set_fval(shader, uniform->variable_name, *(f32 *)uniform->data_buffer);
            break;

            case UT_vec2f_t:
                __glshader_uniform_set_vec2f(shader, uniform->variable_name, *(vec2f_t *)uniform->data_buffer);
            break;

            case UT_vec3f_t:
                __glshader_uniform_set_vec3f(shader, uniform->variable_name, *(vec3f_t *)uniform->data_buffer);
            break;

            case UT_vec4f_t:
                __glshader_uniform_set_vec4f(shader, uniform->variable_name, *(vec4f_t *)uniform->data_buffer);
            break;

            case UT_matrix4f_t:
                __glshader_uniform_set_matrix4f(shader, uniform->variable_name, *(matrixf_t *)uniform->data_buffer);
            break;

            case UT_COUNT:
            break;
        }
        __glshader_get_uniform(shader);
    }
        
}



#endif //_GL_SHADER_H_
