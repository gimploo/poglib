#ifndef __GL_COMMON_H__
#define __GL_COMMON_H__

#include "../file.h"
#include "../basic.h"
#include <GL/glew.h>

#define GL_CHECK(cmd) {\
    GLenum err;\
    cmd;\
    err = glGetError();\
    if (err != GL_NO_ERROR) {\
        fprintf(stderr, "[GL_CHECK] (%s): %s -> %s\n", __func__, #cmd, gluErrorString(err));\
        exit(1);\
    }\
}

#endif //__GL_COMMON_H__
