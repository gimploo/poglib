#include "../glbitmapfont.h"
#include "../../simple/window.h"


int main(void)
{
    window_t win = window_init("main.c", 700, 800, SDL_INIT_VIDEO);

    glbitmapfont_t font = glbitmapfont_init("../../res/ascii_fonts/glyph_atlas.png", 16, 6);

    const char *text[] = 
    {
        "Bruh",
        "dio",
        "pepega",
        "12345"
    };

    glbitmapfont_set_text(&font, text[0], (vec2f_t ){-1.0f, 1.0f});
    glbitmapfont_set_text(&font, text[2], (vec2f_t ){-0.5f, 0.5f});
    glbitmapfont_set_text(&font, text[1], (vec2f_t ){-0.5f, -0.5f});
    glbitmapfont_set_text(&font, text[3], (vec2f_t ){-0.75f,-0.75});

    while(win.is_open)
    {
        window_update_user_input(&win);

        window_gl_render_begin(&win);

            glbitmapfont_draw(&font);

        window_gl_render_end(&win);

    }

    glbitmapfont_destroy(&font);
    window_destroy(&win);

    return 0;
}
