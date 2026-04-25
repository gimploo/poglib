#pragma once 
#include "../../util/assetmanager.h"


typedef struct c_shader_t {

    const char       *label;
    const glshader_t *glshader;

} c_shader_t ;


c_shader_t c_shader(const assetmanager_t *manager, const char *label);


#ifndef IGNORE_C_SHADER_IMPLEMENTATION


c_shader_t c_shader(const assetmanager_t *manager, const char *label)
{
    assert(manager);

    glshader_t *juice = assetmanager_get_shader(manager, label); 
    assert(juice);

    return (c_shader_t ){
        .label = label,
        .glshader = juice
    };
}


#endif
