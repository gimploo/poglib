#include "../cgltf/cgltf.h"
#include <poglib/basic.h>

cgltf_accessor *cgltf_primitive_find_attribute(const cgltf_primitive *const primitive, const cgltf_attribute_type type, const u32 index)
{
    for (u8 idx = 0; idx < primitive->attributes_count; idx++)
        if (primitive->attributes[idx].type == type && primitive->attributes[idx].index == index) 
            return primitive->attributes[idx].data;

    //eprint("CGLTF Attribute type provided doesnt exist in given attributes array");
    return NULL;
}
