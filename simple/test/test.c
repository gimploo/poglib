#include <stdio.h>
#include <stdlib.h>

#include "../gl/simple_gl.h"

#define uint32 u_int32_t 

int main(void)
{
    uint32 FLAGS = SDL_INIT_VIDEO;
    GLuint shader;
    SimpleWindow window = window_init(FLAGS);
    float vertices[] = {    
    -0.5f, -0.5f, 0.0f, 
     0.5f, -0.5f, 0.0f, 
     0.0f,  0.5f, 0.0f  
    };                      


    while(window.is_window_open)
    {
        window_event_process_user_input(&window);

        shader = simple_use_default_shader();
        simple_gl_render(&window, vertices, sizeof(vertices), shader);

    }

    window_destroy(&window);

    return 0;
}
