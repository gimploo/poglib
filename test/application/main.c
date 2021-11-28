#include "../../simple/gl_renderer2d.h"
#include "../../simple/font/gl_ascii_font.h"
#include "../../application.h"

global vec4f_t color, inc_color;
global char buff[1024];
global gl_ascii_font_t font;

void init(application_t *app)
{
    font = gl_ascii_font_init(
            "../../res/ascii_fonts/charmap-oldschool_black.png", 
            17, 7);
}

void update(application_t *app)
{
    f32 fps = application_get_fps(app);
    f32 dt = application_get_dt(app);

    snprintf(buff, sizeof(buff), "FPS: %f | dt: %f", fps, dt);
    /*printf("%s\n", buff);*/

}

void render(application_t *app)
{
    window_t *win = application_get_whandle(app);

    window_gl_render_begin(win);
        gl_ascii_font_render_text(&font, buff, vec2f(0), 0.5f); 
    window_gl_render_end(win);
}

int main(void)
{
    window_t win = window_init("Flappy Birds", 700, 800, SDL_INIT_VIDEO);

    application_t game = application_init(&win);
    game = (application_t ){
        .init = init,
        .update = update,
        .render = render
    };
    application_run(&game);

    gl_ascii_font_destroy(&font);
    window_destroy(&win);

    return 0;
}
