#ifndef __GL_SHADER_H__
#define __GL_SHADER_H__


#include "internal/gl_common.h"

typedef struct Shader Shader;
struct Shader {
    
    GLuint id;
    const char *vs_file_path;
    const char *fg_file_path;

};





Shader  shader_init(const char *vertexShaderSource, const char *fragmentShaderSource);

void    shader_bind(Shader *shader);

void    shader_set_farr(Shader *shader, const char *uniform, float arr[]);
void    shader_set_fval(Shader *shader, const char *uniform, float val);
void    shader_set_uival(Shader *shader, const char *uniform, unsigned int val);
void    shader_set_ival(Shader *shader, const char *uniform, int val);

Shader  shader_use_default_shader(void); // Debug purpose
void    shader_destroy(Shader *shader);






void shader_set_ival(Shader *shader, const char *uniform, int val)
{
    if (shader == NULL) eprint("shader argument is null");
    if (uniform == NULL) eprint("uniform argument is null");

    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");

    GL_CHECK(glUniform1i(location, val));
}

void shader_set_uival(Shader *shader, const char *uniform, unsigned int val)
{
    if (shader == NULL) eprint("shader argument is null");
    if (uniform == NULL) eprint("uniform argument is null");

    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform1ui(location, val));
}

void shader_set_fval(Shader *shader, const char *uniform, float val)
{
    if (shader == NULL) eprint("shader argument is null");
    if (uniform == NULL) eprint("uniform argument is null");

    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform1f(location, val));
}

void shader_set_farr(Shader *shader, const char *uniform, float arr[])
{
    if (shader == NULL) eprint("shader argument is null");
    if (uniform == NULL) eprint("uniform argument is null");

    int location;
    GL_CHECK(location = glGetUniformLocation(shader->id, uniform));
    if (location == -1) eprint("[ERROR] uniform doesnt exist");
    GL_CHECK(glUniform4f(location, arr[0], arr[1], arr[2], arr[3]));
}



inline void shader_destroy(Shader *shader)
{
    if (shader == NULL) {
        fprintf(stderr, "%s: shader is null\n", __func__);
        exit(1);
    }

    GLuint id = shader->id;
    GL_CHECK(glDeleteProgram(shader->id));
    printf("[INFO]\tShader `%i` successfully deleted\n", id);
}


void shader_bind(Shader *shader)
{
    if (shader == NULL) {
        fprintf(stderr, "%s: shader is null\n", __func__);
        exit(1);
    }
    GL_CHECK(glUseProgram(shader->id));
}



Shader shader_init(const char *vertex_source_path, const char *fragment_source_path)
{
    Shader shader = {0};
    shader.vs_file_path = vertex_source_path;
    shader.fg_file_path = fragment_source_path;

    int status;
    char error_log[KB] = {0};

    File vsfile = file_init(vertex_source_path);
    const char * vtxfile = file_readall(&vsfile);
    if (vtxfile == NULL) {
        fprintf(stderr, "%s: vertex file returned null\n", __func__);
        exit(1);
    }


    File fgfile = file_init(fragment_source_path);
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
    printf("[INFO]\tVertex Shader successfully compiled\n");

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
    printf("[INFO]\tFragment Shader successfully compiled\n");

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

    printf("[POG]\tShader `%i` successfully linked\n", shader.id);

    return shader;
}

Shader simple_use_default_shader(void) 
{

    const char * const vertexshader = 
        "#version 460 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}";

    const char * const fragmentshader = 
        "#version 460 core\n"
        "out vec4 FragColor;\n"
        "\n"
        "uniform vec4 u_color;\n"
        "\n"
        "void main()\n"
        "{\n"
            "FragColor = u_color;\n"
        "}";

    int status;
    char error_log[KB] = {0};

    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexshader, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (!status) {
        glGetShaderInfoLog(vertexShader, KB, NULL, error_log);
        fprintf(stderr, "Vertex Error:\n\t%s\n", error_log);
        exit(1);
    }
    printf("[INFO]\tVertex Shader successfully compiled\n");

    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentshader, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    if (!status) {
        glGetShaderInfoLog(fragmentShader, KB, NULL, error_log);
        fprintf(stderr, "Fragment Error:\n\t%s\n", error_log);
        exit(1);
    }
    printf("[INFO]\tFragment Shader successfully compiled\n");

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

    Shader shader = {0};
    shader.fg_file_path = shader.vs_file_path = NULL;
    shader.id = shaderProgram;

    printf("[POG]\tShader `%i` successfully linked\n", shader.id);

    return shader;
}


#endif //_GL_SHADER_H_
