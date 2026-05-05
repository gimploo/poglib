#pragma once
#define GLEW_NO_GLU
#include <GL/glew.h>
#include <poglib/basic.h>
#include <poglib/math.h>

#ifdef GL_LOG_ENABLE
#   define GL_LOG(FMT, ...) fprintf(stderr, "[OpenGL][LOG]\t " FMT "\n", ##__VA_ARGS__)
#else
#   define GL_LOG(FMT, ...) 
#endif

const char* gl__internal_get_glerror_cstr(const GLenum err) {
    switch (err) {
        case GL_NO_ERROR:                   return "GL_NO_ERROR";
        case GL_INVALID_ENUM:               return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:              return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:          return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY:              return "GL_OUT_OF_MEMORY";
        case GL_STACK_OVERFLOW:             return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:            return "GL_STACK_UNDERFLOW";
        case GL_CONTEXT_LOST:               return "GL_CONTEXT_LOST";
        default:                            return "GL_UNKNOWN_ERROR";
    }}

#define GL_CHECK(CMD) do {\
\
    CMD;\
    GLenum err = glGetError();\
    if (err != GL_NO_ERROR)\
        eprint("[OpenGL][ERROR](%s): %s -> %s\n", __func__, #CMD , gl__internal_get_glerror_cstr(err));\
\
} while(0)
