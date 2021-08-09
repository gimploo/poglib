#ifndef __GL_SHADER_H__
#define __GL_SHADER_H__

/*=======================================================
 // OpenGL shader handling library
=======================================================*/

#include "_common.h"

typedef struct gl_shader_t {

    GLuint id;
    const char *vs_file_path;
    const char *fg_file_path;

} gl_shader_t;


const char * const default_vshader = 
    "#version 460 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}";

const char * const default_fshader = 
    "#version 460 core\n"
    "out vec4 FragColor;\n"
    "\n"
    "uniform vec4 u_color;\n"
    "\n"
    "void main()\n"
    "{\n"
        "FragColor = u_color;\n"
    "}";

    


/*-----------------------------------------------------
 // Declarations
-----------------------------------------------------*/

//NOTE:(macro)  shader_default_init(void) --> gl_shader_t
gl_shader_t     shader_init(const char *file_vs, const char *file_fs);
gl_shader_t     shader_load_code(const char *vertex_source_code, const char *fragment_source_code);

void            shader_bind(gl_shader_t *shader);

void            shader_set_farr(gl_shader_t *shader, const char *uniform, float arr[]);
void            shader_set_fval(gl_shader_t *shader, const char *uniform, float val);
void            shader_set_uival(gl_shader_t *shader, const char *uniform, unsigned int val);
void            shader_set_ival(gl_shader_t *shader, const char *uniform, int val);

void            shader_destroy(gl_shader_t *shader);


/*--------------------------------------------------------
 // Implementation
--------------------------------------------------------*/

#define shader_default_init() shader_load_code(default_vshader, default_fshader)

void shader_set_ival(gl_shader_t *shader, const char *uniform, int val)
{
    if (shader == NULL) eprint("shader argument is null");
    if (uniform == NULL) eprint("uniform argument is null");

    shader_bind(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");

    GL_CHECK(glUniform1i(location, val));
}

void shader_set_uival(gl_shader_t *shader, const char *uniform, unsigned int val)
{
    if (shader == NULL) eprint("shader argument is null");
    if (uniform == NULL) eprint("uniform argument is null");

    shader_bind(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform1ui(location, val));
}

void shader_set_fval(gl_shader_t *shader, const char *uniform, float val)
{
    if (shader == NULL) eprint("shader argument is null");
    if (uniform == NULL) eprint("uniform argument is null");

    shader_bind(shader);
    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform1f(location, val));
}

void shader_set_farr(gl_shader_t *shader, const char *uniform, float arr[])
{
    if (shader == NULL) eprint("shader argument is null");
    if (uniform == NULL) eprint("uniform argument is null");

    shader_bind(shader);

    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform4f(location, arr[0], arr[1], arr[2], arr[3]));
}



inline void shader_destroy(gl_shader_t *shader)
{
    if (shader == NULL) {
        fprintf(stderr, "%s: shader is null\n", __func__);
        exit(1);
    }


    GLuint id = shader->id;
    GL_CHECK(glDeleteProgram(shader->id));
    GL_LOG("gl_shader_t `%i` successfully deleted", id);
}


void shader_bind(gl_shader_t *shader)
{
    if (shader == NULL) {
        fprintf(stderr, "%s: shader is null\n", __func__);
        exit(1);
    }
    GL_CHECK(glUseProgram(shader->id));
}

gl_shader_t shader_load_code(const char *vertex_source_code, const char *fragment_source_code)
{
    gl_shader_t shader = {0};
    int status;
    char error_log[KB] = {0};

    const char * vtxfile = vertex_source_code;
    if (vtxfile == NULL) {
        fprintf(stderr, "%s: vertex file returned null\n", __func__);
        exit(1);
    }

    const char *frgfile = fragment_source_code;
    if (frgfile == NULL) {
        fprintf(stderr, "%s: vertex file returned null\n", __func__);
        exit(1);
    }

    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vtxfile, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (!status) {
        glGetShaderInfoLog(vertexShader, KB, NULL, error_log);
        fprintf(stderr, "Vertex Error:\n\t%s\n", error_log);
        exit(1);
    }
   GL_LOG("Vertex Shader successfully compiled");

    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &frgfile, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    if (!status) {
        glGetShaderInfoLog(fragmentShader, KB, NULL, error_log);
        fprintf(stderr, "Fragment Error:\n\t%s\n", error_log);
        exit(1);
    }
   GL_LOG("Fragment Shader successfully compiled");

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if(!status) {
        glGetProgramInfoLog(shaderProgram, KB, NULL, error_log);
        fprintf(stderr, "Error: %s\n", error_log);
        exit(1);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    shader.id = shaderProgram;

   GL_LOG("Shader `%i` successfully linked", shader.id);

    return shader;
}



gl_shader_t shader_init(const char *vertex_source_path, const char *fragment_source_path)
{
    gl_shader_t shader = {0};
    shader.vs_file_path = vertex_source_path;
    shader.fg_file_path = fragment_source_path;

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
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vtxfile, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (!status) {
        glGetShaderInfoLog(vertexShader, KB, NULL, error_log);
        fprintf(stderr, "Vertex Error:\n\t%s\n", error_log);
        exit(1);
    }
   GL_LOG("Vertex Shader successfully compiled");

    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &frgfile, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    if (!status) {
        glGetShaderInfoLog(fragmentShader, KB, NULL, error_log);
        fprintf(stderr, "Fragment Error:\n\t%s\n", error_log);
        exit(1);
    }
   GL_LOG("Fragment Shader successfully compiled");

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if(!status) {
        glGetProgramInfoLog(shaderProgram, KB, NULL, error_log);
        fprintf(stderr, "Error: %s\n", error_log);
        exit(1);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    shader.id = shaderProgram;

   GL_LOG("Shader `%i` successfully linked", shader.id);

    return shader;
}



#endif //_GL_SHADER_H_
