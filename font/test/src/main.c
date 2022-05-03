#include <poglib/font/glbitmapfont.h>
#include <poglib/font/glfreetypefont.h>
#include <poglib/application.h>


int main(void)
{    
    dbg_init();
    window_t * win = window_init("main.c", 700, 800, SDL_INIT_VIDEO);
    window_set_background(win, COLOR_BLACK);

    glfreetypefont_t font = glfreetypefont_init("../../res/ttf_fonts/Roboto-Medium.ttf", 42);

    const char *text[] = 
    {
        "BRUH",
        "dioderant",
        "pepega",
        "*()@#$%#$!@#:<>?:-->"
    };

    glbatch_t sbatch = glbatch_init(KB, glquad_t );
    glbatch_t dbatch = glbatch_init(KB, glquad_t );

    glfreetypefont_add_text_to_batch(&font, &sbatch, "JUICE", (vec2f_t ){0.6f,-0.6f},  COLOR_CYAN);
    glframebuffer_t fbo = glframebuffer_init(700, 800);

    int i = 0;
    while(win->is_open)
    {
        window_update_user_input(win);

        window_gl_render_begin(win);
        glfreetypefont_add_text_to_batch(&font, &dbatch, text[++i % 4], (vec2f_t ){-0.6f, 0.6f}, COLOR_RED);
        glfreetypefont_add_text_to_batch(&font, &dbatch,text[++i % 2], (vec2f_t ){0.0f, 0.0f}, COLOR_BLUE);
        glfreetypefont_add_text_to_batch(&font, &dbatch,text[++i % 3], (vec2f_t ){-0.6f, -0.6f}, COLOR_GREEN);

        glframebuffer_begin_texture(&fbo);
            glfreetypefont_draw(&font, &sbatch);
            glfreetypefont_draw(&font, &dbatch);
        glframebuffer_end_texture(&fbo);

        glbatch_clear(&dbatch);
        glshader_t shader = glshader_default_init();
        glrenderer2d_t r2d = {
            .shader = &shader,
            .texture = &fbo.texture2d,
        };
        const glquad_t fbo_quad = glquad(
                                quadf((vec3f_t ){-1.0f, 1.0f, 1.0f}, 2.0f, 2.0f), 
                                vec4f(1.0f), 
                                quadf((vec3f_t ){0.0f, 1.0f, 1.0f}, 1.0f, 1.0f), 0);
        glrenderer2d_draw_quad(&r2d, fbo_quad);

        window_gl_render_end(win);

    }

    glbatch_destroy(&sbatch);
    glbatch_destroy(&dbatch);

    glfreetypefont_destroy(&font);
    window_destroy();
    dbg_destroy();

    return 0;

}

int oldmain(void)
{
    window_t *win = window_init("main.c", 700, 800, SDL_INIT_VIDEO);

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

    while(win->is_open)
    {
        window_update_user_input(win);

        window_gl_render_begin(win);

            glbitmapfont_draw(&font);

        window_gl_render_end(win);

    }

    glbitmapfont_destroy(&font);
    window_destroy();

    return 0;
}
