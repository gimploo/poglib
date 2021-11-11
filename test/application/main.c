#include "../../simple/gl_renderer2d.h"
#include "../../application.h"

global vec4f_t color, inc_color;
global char buff[1024];

void init(application_t *app)
{
}

void update(application_t *app)
{
    f32 fps = application_get_fps(app);
    f32 dt = application_get_dt(app);

    snprintf(buff, sizeof(buff), "FPS: %f | dt: %f", fps, dt);
    printf("%s\n", buff);

}

void render(application_t *app)
{
    window_t *win = application_get_whandle(app);

    window_gl_render_begin(win);

    window_gl_render_end(win);
}

int main(void)
{
    window_t win = window_init("Flappy Birds", 700, 800, SDL_INIT_VIDEO);
    application_t game = application_init(&win, (const void *)init, (const void *)update,(const void *)render);

    application_run(&game);
}
