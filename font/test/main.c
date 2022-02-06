#include "../../basic.h"
#include "../glbitmapfont.h"
#include "../glfreetypefont.h"
#include "../../simple/window.h"


int main(void)
{    
    dbg_init();
    window_t win = window_init("main.c", 700, 800, SDL_INIT_VIDEO);

    glfreetypefont_t font = glfreetypefont_init("../../res/ttf_fonts/Roboto-Medium.ttf", 42);

    const char *text[] = 
    {
        "Bruh",
        "dioderant",
        "pepega",
        "*()@#$%#$!@#:<>?:-->"
    };

    glfreetypefont_set_text(&font, text[0], (vec2f_t ){-1.0f, 1.0f}, COLOR_RED);
    glfreetypefont_set_text(&font, text[2], (vec2f_t ){-0.5f, 0.5f}, COLOR_BLUE);
    glfreetypefont_set_text(&font, text[1], (vec2f_t ){-0.5f, -0.5f}, COLOR_GREEN);
    glfreetypefont_set_text(&font, text[3], (vec2f_t ){-0.75f,-0.75},  COLOR_CYAN);


    while(win.is_open)
    {
        window_update_user_input(&win);

        window_gl_render_begin(&win);

            glfreetypefont_draw(&font);

        window_gl_render_end(&win);

    }

    glfreetypefont_destroy(&font);
    window_destroy(&win);
    dbg_destroy();

    return 0;

}

int oldmain(void)
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

    glbitmapfont_set_text(&font, text[0], (vec2f_t ){-1.0f, 1.0f} , 0.2f );
    glbitmapfont_set_text(&font, text[2], (vec2f_t ){-0.5f, 0.5f} , 0.2f );
    glbitmapfont_set_text(&font, text[1], (vec2f_t ){-0.5f, -0.5f}, 0.2f );
    glbitmapfont_set_text(&font, text[3], (vec2f_t ){-0.75f,-0.75}, 0.2f );

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
