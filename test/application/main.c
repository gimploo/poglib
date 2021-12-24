#include "../../simple/glrenderer2d.h"
#include "../../application.h"

global char buff[1024] = {0};
global glbatch_t batch;
global glrenderer2d_t renderer;

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
        /*glrenderer2d_draw_from_batch(&renderer, &batch);*/
    window_gl_render_end(win);
}

int main(void)
{
    window_t win = window_init("Flappy Birds", 700, 800, SDL_INIT_VIDEO);

    application_t game = application_init(&win);
    application_run(&game);

    window_destroy(&win);

    return 0;
}
