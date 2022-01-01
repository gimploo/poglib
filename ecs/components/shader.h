#pragma once 
#include "../../simple/gl/shader.h"


typedef struct c_shader_t c_shader_t ;


c_shader_t * c_shader_init(const char *vs, const char *fs);
void c_shader_destroy(c_shader_t *);





#ifndef IGNORE_C_SHADER_IMPLEMENTATION


struct c_shader_t {

    glshader_t glshader;

};


c_shader_t * c_shader_init(const char *vs, const char *fs)
{
    c_shader_t *o = (c_shader_t *)calloc(1, sizeof(c_shader_t ));
    *o = (c_shader_t ) {
        .glshader = glshader_from_file_init(vs, fs)
    };

    return o;
}

void c_shader_destroy(c_shader_t *cs)
{
    assert(cs);

    glshader_destroy(&cs->glshader);
}

#endif
