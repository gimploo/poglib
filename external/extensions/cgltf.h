#include "../cgltf/cgltf.h"
#include <poglib/basic.h>

cgltf_accessor *cgltf_primitive_find_attribute(cgltf_primitive *const primitive, const cgltf_attribute_type type)
{
    for (u8 idx = 0; idx < primitive->attributes_count; idx++)
        if (primitive->attributes[idx].type == type) 
            return primitive->attributes[idx].data;

    eprint("CGLTF Attribute type provided doesnt exist in given attributes array");
    return NULL;
}
