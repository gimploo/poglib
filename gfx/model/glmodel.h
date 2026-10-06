#pragma once
#include <poglib/basic.h>
#include <poglib/external/cgltf/cgltf.h>
#include <poglib/external/extensions/cgltf.h>

typedef struct {
    cgltf_data *data;
} model_t;

model_t     model_load_level_from_file_init(const str_t filepath, arena_t *const arena);
void        model_destroy(model_t *const self);

model_t model_load_level_from_file_init(const str_t filepath, arena_t *const arena)
{
    cgltf_data *data_buffer     = NULL;
    const cgltf_options options = {0};
    file_t file = file_init(filepath.data, "rb");
    {
        ASSERT(file.size);

        u8 *buffer = arena_reserve(arena, file.size);
        {
            file_readbytes(&file, buffer, file.size);
            const cgltf_result result = cgltf_parse(&options, buffer, file.size, &data_buffer);
            if (result != cgltf_result_success) {
                eprint("unable to load "STR_FMT" | ERROR: %s", STR_ARG(filepath), CGLTF_RESULT_CSTR_LOOKUP[result]);
            }

            //TODO: cgltf_memory_options update this to have it use the arena instead of malloc
            const cgltf_result load_result = cgltf_load_buffers(&options, data_buffer, filepath.data);
            if (load_result != cgltf_result_success) {
                eprint("failed to load buffers for "STR_FMT" | ERROR: %s", STR_ARG(filepath), CGLTF_RESULT_CSTR_LOOKUP[load_result]);
            }
        }
    }
    file_destroy(&file);

    logging("Loaded model file `"STR_FMT"`", STR_ARG(filepath));

    return (model_t) {
        .data = data_buffer
    };
}

void model_destroy(model_t *const self)
{
    cgltf_free(self->data);
}




