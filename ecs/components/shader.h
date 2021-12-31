#pragma once 
#include "../../simple/gl/shader.h"


typedef struct c_shader_t c_shader_t ;

c_shader_t c_shader_init(const char *vs, const char *fs);
void c_shader_destroy(c_shader_t *);





#ifndef IGNORE_C_SHADER_IMPLEMENTATION


struct c_shader_t {

    glshader_t glshader;

};


c_shader_t c_shader_init(const char *vs, const char *fs)
{
    return (c_shader_t ) {
        .glshader = glshader_from_file_init(vs, fs)
    };
}

void c_shader_destroy(c_shader_t *cs)
{
    assert(cs);

    glshader_destroy(&cs->glshader);
}

#endif
