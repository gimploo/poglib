#ifndef __INTERNAL_COMMON_H__
#define __INTERNAL_COMMON_H__

#include <GL/glew.h>

#include "../../basic.h"
#include "../../ds/stack.h"
#include "../../file.h"
#include "../../math/la.h"

#ifdef GL_LOG_ENABLE
#   define GL_LOG(fmt, ...) fprintf(stderr, "[LOG]\t " fmt "\n", ##__VA_ARGS__)
#else
#   define GL_LOG(fmt, ...) 
#endif

#define GL_CHECK(cmd) {\
    GLenum err;\
    cmd;\
    err = glGetError();\
    if (err != GL_NO_ERROR) {\
        fprintf(stderr, "[GL_CHECK] (%s): %s -> %s\n", __func__, #cmd, gluErrorString(err));\
        exit(1);\
    }\
}

#endif //__INTERNAL_COMMON_H__
