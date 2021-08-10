#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "../gl_ascii_font.h"
#include "../../window.h"

#define uint32 u_int32_t 

void loop(void *arg)
{
    gl_ascii_font_handler_t *font = (gl_ascii_font_handler_t *)arg;

    gl_ascii_font_render_text(font, "Gokul", (vec2f){ -1.0f, 1.0f });
}

int main(void)
{
    uint32 FLAGS = SDL_INIT_VIDEO;
    Window window = window_init(1080, 920, FLAGS);

    gl_ascii_font_handler_t font = gl_ascii_font_init(
            "../res/glyph_atlas.png", 16, 6);

    while (window.is_open)
    {
        window_process_user_input(&window);
        window_render(&window, loop, &font); 
    }

    gl_ascii_font_destroy(&font);

    window_destroy(&window);

    return 0;
}
