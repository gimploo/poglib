#include "../../simple/gl_renderer2d.h"
#include "../../simple/font/gl_ascii_font.h"
#include "../../application.h"

global char buff[1024] = {0};
global glasciifont_t font;
global glbatch_t batch;

void init(application_t *app)
{
    font = glasciifont_init(
            "../../res/ascii_fonts/glyph_atlas.png", 
            16, 6);
}

void update(application_t *app)
{
    f32 fps = application_get_fps(app);
    f32 dt = application_get_dt(app);

    snprintf(buff, sizeof(buff), "FPS: %f | dt: %f", fps, dt);
    batch = glasciifont_batch_text(&font, buff, vec2f(0.0f), 0.1f);
    /*printf("%s\n", buff);*/

}

void render(application_t *app)
{
    window_t *win = application_get_whandle(app);

    window_gl_render_begin(win);
        glrenderer2d_draw_from_batch(&font.textrenderer, &batch);
    window_gl_render_end(win);
}

int main(void)
{
    window_t win = window_init("Flappy Birds", 700, 800, SDL_INIT_VIDEO);

    application_t game = application_init(&win, init, update, render);
    application_run(&game);

    glasciifont_destroy(&font);
    window_destroy(&win);

    return 0;
}
