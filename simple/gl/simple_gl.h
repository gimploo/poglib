#ifndef __SIMPLE_GL_H_
#define __SIMPLE_GL_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define KB 1024

// comment this line not have opengl used for rendering
#include <GL/glew.h> 

#include "../win/simple_window.h"


const char * const default_vertex_file_path = "/home/gokul/Documents/projects/poglib/simple/gl/default.vert";
const char * const default_fragment_file_path = "/home/gokul/Documents/projects/poglib/simple/gl/default.frag";


GLuint  simple_gl_compile_shader(const char *vertexShaderSource, const char *fragmentShaderSource);
void    simple_gl_render(SimpleWindow *window, float vertices[], size_t vertices_size, GLuint shaderProgram);
void    simple_gl_draw_triangle(float arr[], size_t arr_size);
void    simple_gl_clean_up(GLuint);

size_t _file_get_size(const char *file_path)
{
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "%s: file failed to open\n", __func__);
        exit(1);
    }
    fseek(fp, 0L, SEEK_END);
    size_t size = ftell(fp);
    fclose(fp);
    return size; // Returns the position of the null character 
}

static inline char * _read_file(const char *file_path)
{
    size_t size = _file_get_size(file_path); 
    assert(size > 0);
    char * buffer = (char *)malloc(size);
    if (buffer == NULL) {
        fprintf(stderr, "%s: malloc failed\n", __func__);
        exit(1);
    }
    memset(buffer, 0, size+1);

    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "%s: failed to open file\n", __func__);
        exit(1);
    }

    fread(buffer, size, 1, fp);
    fclose(fp);

    return buffer;
}

static inline GLuint simple_use_default_shader(void) 
{
    return simple_gl_compile_shader(default_vertex_file_path, default_fragment_file_path);
}


GLuint simple_gl_compile_shader(const char *vertex_source_path, const char *fragment_source_path)
{
    int status;
    char error_log[KB] = {0};

    const char *const vtxfile = _read_file(vertex_source_path);
    if (vtxfile == NULL) {
        fprintf(stderr, "%s: vertex file returned null\n", __func__);
        exit(1);
    }


    const char *const frgfile = _read_file(fragment_source_path);
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

    return shaderProgram;
}

void simple_gl_draw_triangle(float arr[], size_t arr_size)
{

    GLuint vertex_buffer_obj;
    glGenBuffers(1, &vertex_buffer_obj);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_obj);
    glBufferData(GL_ARRAY_BUFFER, arr_size, arr, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    

}

void simple_gl_render(SimpleWindow *window, float vertices[], size_t vertices_size, GLuint shaderProgram)
{
    if (window == NULL) {
        fprintf(stderr, "%s: window argument is null\n", __func__);
        exit(1);
    }

    // Draws the background
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draws a triangle
    glUseProgram(shaderProgram);
    simple_gl_draw_triangle(vertices, vertices_size);

    SDL_GL_SwapWindow(window->window_handle);

}


void simple_gl_cleanup(GLuint prog)
{
    glDeleteProgram(prog);
}


#endif
