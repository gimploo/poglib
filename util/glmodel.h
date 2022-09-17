#pragma once
#include <poglib/basic.h>
#define CGLTF_IMPLEMENTATION
#include <poglib/external/cgltf/cgltf.h>

typedef struct glmodel_t {
    const char *filepath;
    cgltf_options options;
    cgltf_data *data;
    cgltf_result result;
} glmodel_t ;


glmodel_t   glmodel_init(const char *filepath);
void        glmodel_destroy(glmodel_t *self);


#ifndef IGNORE_GL_MODEL_IMPLEMENTATION

glmodel_t glmodel_init(const char *filepath)
{
	cgltf_options options;
	memset(&options, 0, sizeof(cgltf_options));
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, filepath, &data);

	if (result == cgltf_result_success)
		result = cgltf_load_buffers(&options, data, filepath);
    else eprint("result error");

	if (result == cgltf_result_success)
		result = cgltf_validate(data);
    else eprint("result error");

    return (glmodel_t ) {
        .filepath   = filepath,
        .options    = options,
        .data       = data,
        .result     = result
    };
}

void glmodel_destroy(glmodel_t *self)
{
	cgltf_free(self->data);
}

#endif
