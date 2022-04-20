#include "../../gl_renderer2d.h"
#include "../../window.h"

void stuff (void *arg)
{    
    quadf_t rectangle = {
        0.0f, 0.0f,  
        0.5f, 0.0f,  
        0.5f, 0.5f,  
        0.0f, 0.5f,  
    };
    const gl_quad_t qua = gl_quad(rectangle, (vec3f_t ) {0.0f, 1.0f, 0.0f}, quadf(0));
    gl_shader_t shader = gl_shader_default_init();
    gl_renderer2d_t renderer = gl_renderer2d_init(&shader, NULL);
    gl_renderer2d_draw_quad(&renderer, qua);
    gl_renderer2d_destroy(&renderer);

}

int main(void)
{
    window_t window = window_init("TEST", 1080, 920, SDL_INIT_VIDEO);
    window_t *sub_window = window_sub_window_init(&window, "SUB", 1080/2, 920/2, SDL_INIT_VIDEO);

    window_set_background(&window, (vec4f_t) {1.0f, 0.0f, 0.0f, 1.0f});
    window_set_background(sub_window, (vec4f_t) {0.0f, 0.0f, 1.0f, 1.0f});

    window_while_is_open(&window)
    {
        window_render_stuff(&window, stuff, NULL);
        window_sub_window_render_stuff(sub_window, stuff, NULL);
    }

    window_destroy(&window);

    return 0;
}
