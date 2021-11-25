#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "../../../dbg/dbg.h"
#include "../gl_ascii_font.h"
#include "../../window.h"

dbg_t debug = {0};

#define uint32 u_int32_t 

void loop(void *arg)
{
    gl_ascii_font_t *font = (gl_ascii_font_t *)arg;

    gl_ascii_font_render_text(font, "329374~!#*&*", (vec2f_t) {-1.0f, 1.0f}, 0.1f);
    gl_ascii_font_render_text(font, "Bruh", (vec2f_t) { 0.0f,0.0f }, 0.1f);
}

int main(void)
{
    dbg_init(&debug, "./tmp.txt");

    uint32 FLAGS = SDL_INIT_VIDEO;
    window_t window = window_init("test.c", 1080, 920, FLAGS);

    gl_ascii_font_t font = gl_ascii_font_init(
            "../res/glyph_atlas.png", 16, 6);

    while (window.is_open)
    {
        window_render_stuff(&window, loop, &font); 
    }

    gl_ascii_font_destroy(&font);

    window_destroy(&window);

    dbg_destroy();

    return 0;
}
