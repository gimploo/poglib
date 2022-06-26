#pragma once
#include <GL/glew.h>
#include <poglib/basic.h>
#include <poglib/ds/stack.h>
#include <poglib/ds/queue.h>
#include <poglib/file.h>
#include <poglib/math/la.h>
#include <poglib/math/shapes.h>

#ifdef GL_LOG_ENABLE
#   define GL_LOG(FMT, ...) fprintf(stderr, "[OpenGL][LOG]\t " FMT "\n", ##__VA_ARGS__)
#else
#   define GL_LOG(FMT, ...) 
#endif

#define GL_CHECK(CMD) do {\
\
    CMD;\
    GLenum err = glGetError();\
    if (err != GL_NO_ERROR)\
        eprint("[OpenGL][ERROR] (%s): %s -> %s\n", __func__, #CMD, gluErrorString(err));\
\
} while(0)
