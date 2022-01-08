#ifndef __INTERNAL_COMMON_H__
#define __INTERNAL_COMMON_H__

#include <GL/glew.h>

#include "../../basic.h"
#include "../../ds/stack.h"
#include "../../ds/queue.h"
#include "../../file.h"
#include "../../math/la.h"

#ifdef GL_LOG_ENABLE
#   define GL_LOG(fmt, ...) fprintf(stderr, "[LOG]\t " fmt "\n", ##__VA_ARGS__)
#else
#   define GL_LOG(fmt, ...) 
#endif

#define GL_CHECK(CMD) do {\
\
    CMD;\
    GLenum err = glGetError();\
    if (err != GL_NO_ERROR) {\
\
        fprintf(stderr, "[GL_CHECK] (%s): %s -> %s\n", __func__, #CMD, gluErrorString(err));\
        exit(1);\
\
    }\
\
} while(0)

#endif //__INTERNAL_COMMON_H__
