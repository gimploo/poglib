#ifndef __GL_INTERNAL_COMMON_H__
#define __GL_INTERNAL_COMMON_H__

#include <GL/glew.h>

#include "../../../image/bitmap.h"
#include "../../../file.h"
#include "../../../basic.h"

#define GL_CHECK(cmd) {\
    GLenum err;\
    cmd;\
    err = glGetError();\
    if (err != GL_NO_ERROR) {\
        fprintf(stderr, "[GL_CHECK] (%s): %s -> %s\n", __func__, #cmd, gluErrorString(err));\
        exit(1);\
    }\
}

#endif //__GL_INTERNAL_COMMON_H__
