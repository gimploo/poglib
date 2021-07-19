#ifndef _SHADER_H_
#define _SHADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>


// Includes OpenGL
#include <GL/glew.h>
#include "../file.h"


typedef struct Shader Shader;
struct Shader {
    
    GLuint id;
    const char *vs_file_path;
    const char *fg_file_path;

};

#define GL_CHECK(cmd) {         \
    GLenum err;                 \
    cmd;                        \
    err = glGetError();         \
    if (err != GL_NO_ERROR) {   \
        fprintf(stderr, "[GL] %s: %s\n", __func__, gluErrorString(err));\
        exit(1);\
    }\
}





Shader  shader_init(const char *vertexShaderSource, const char *fragmentShaderSource);

void    shader_use(Shader *shader);

void    shader_set_farr(Shader *shader, const char *uniform, float arr[]);
void    shader_set_fval(Shader *shader, const char *uniform, float val);
void    shader_set_uival(Shader *shader, const char *uniform, unsigned int val);
void    shader_set_ival(Shader *shader, const char *uniform, int val);

void    shader_destroy(Shader *shader);






void shader_set_ival(Shader *shader, const char *uniform, int val)
{
    GL_CHECK(glUniform1i(glGetUniformLocation(shader->id, uniform), val));
}

void shader_set_uival(Shader *shader, const char *uniform, unsigned int val)
{
    GL_CHECK(glUniform1ui(glGetUniformLocation(shader->id, uniform), val));
}

void shader_set_fval(Shader *shader, const char *uniform, float val)
{
    GL_CHECK(glUniform1f(glGetUniformLocation(shader->id, uniform), val));
}

void shader_set_farr(Shader *shader, const char *uniform, float arr[])
{
    GL_CHECK(glUniform4f(glGetUniformLocation(shader->id, uniform), arr[0], arr[1], arr[2], arr[3]));
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


void shader_use(Shader *shader)
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


#endif //_SHADER_H_
