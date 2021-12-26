#include "../glbitmapfont.h"
#include "../../simple/window.h"


int main(void)
{
    window_t win = window_init("main.c", 700, 800, SDL_INIT_VIDEO);

    glbitmapfont_t font = glbitmapfont_init("../../res/ascii_fonts/glyph_atlas.png", 16, 6);

    const char *text01 = "BRUH";

    while(win.is_open)
    {
        window_update_user_input(&win);

        window_gl_render_begin(&win);

            glbitmapfont_render_text(&font, text01, (vec2f_t ){-1.0f, 1.0f}, 0.25f);

        window_gl_render_end(&win);

    }

    glbitmapfont_destroy(&font);
    window_destroy(&win);

    return 0;
}
