#pragma once
#include <poglib/basic.h>
#define CGLTF_IMPLEMENTATION
#include <poglib/external/cgltf/cgltf.h>
#include <poglib/gfx/glrenderer3d.h>

typedef struct glmodel_t {

    const char      *filepath;
    cgltf_data      *data;

} glmodel_t ;


glmodel_t   glmodel_init(const char *filepath);
glmesh_t    glmodel_get_mesh(const glmodel_t *self);
void        glmodel_destroy(glmodel_t *self);


#ifndef IGNORE_GL_MODEL_IMPLEMENTATION

glmodel_t glmodel_init(const char *filepath)
{
    assert(filepath);

	cgltf_options options;
	memset(&options, 0, sizeof(cgltf_options));
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, filepath, &data);

	if (result == cgltf_result_success)
		result = cgltf_load_buffers(&options, data, filepath);
    else eprint("error cgltf load buffer");

	if (result == cgltf_result_success)
		result = cgltf_validate(data);
    else eprint("error cgltf validation");

    return (glmodel_t ) {
        .filepath   = filepath,
        .data       = data,
    };
}

glmesh_t glmodel_get_mesh(const glmodel_t *self)
{
    assert(self);
    cgltf_data *data = self->data;

    list_t indices, vertices, textures;

    eprint("TODO");
}

void glmodel_destroy(glmodel_t *self)
{
	cgltf_free(self->data);
}

#endif
