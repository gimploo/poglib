#define GL_LOG_ENABLE
#include "../../../gl_renderer.h"
#include "../../../window.h"


const char * const test_default_vshader = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 v_pos;\n"
    "\n"
    "void main()\n"
    "{\n"
        "gl_Position = vec4(v_pos, 1.0f);\n"
    "}";

const char * const test_default_fshader = 
    "#version 330 core\n"
    "\n"
    "uniform vec4 u_color;\n"
    "uniform vec4 u_texcoord;\n"
    "uniform int u_texid;\n"
    "\n"
    "out vec4 FragColor;\n"
    "\n"
    "void main()\n"
    "{\n"
        "FragColor = u_color;\n"
    "}";


int main(void)
{
    window_t window = window_init(1024, 720, SDL_INIT_VIDEO);
    gl_shader_t shader = gl_shader_from_cstr_init(test_default_vshader, test_default_fshader);
    vec4f_t color = vec4f(1.0f);
    int id = 10;
    vec3f_t tex_coord = vec3f(2.0f);
    gl_shader_push_uniform(&shader, "u_color", &color, sizeof(color), UT_VEC4F);   
    gl_shader_push_uniform(&shader, "u_texcoord", &tex_coord, sizeof(tex_coord), UT_VEC3F);   
    gl_shader_push_uniform(&shader, "u_texid", &id, sizeof(id), UT_INT);   

    for_i_in_stack(&shader.uniforms)
    {
        __uniform_meta_data_t *data = ((__uniform_meta_data_t *)shader.uniforms.array + i);

        switch(data->uniform_type)
        {
            case UT_UINT:
                break;
            case UT_INT: {

                u32 *value = (u32 *)data->data_buffer;
                printf("%i\n", *value);

            } break;
            case UT_VEC3F: {

                vec3f_t *value = (vec3f_t *)data->data_buffer;
                printf(VEC3F_FMT"\n", VEC3F_ARG(value));

            } break;
            case UT_VEC4F: {

                vec4f_t *value = (vec4f_t *)data->data_buffer;
                printf(VEC4F_FMT"\n", VEC4F_ARG(value));

            } break;
            default:
                eprint("uniform type not accounted for");
        }
    }

    gl_shader_bind(&shader);

    while (window.is_open)
    {
        __window_update_user_input(&window);
        if (window.keyboard_handler.key == SDLK_q)
            window.is_open = false;
    }

    gl_shader_destroy(&shader);

    return 0;
}
