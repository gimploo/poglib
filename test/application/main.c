#include "../../simple/glrenderer2d.h"
#include "../../application.h"

global glbatch_t batch;

void app_init(application_t *app)
{
}

void app_update(application_t *app)
{
    f32 fps = application_get_fps(app);
    f32 dt = application_get_dt(app);
    char buff[1024] = {0};

    snprintf(buff, sizeof(buff), "FPS: %f | dt: %f", fps, dt);
    printf("%s\n", buff);


}

void app_render(application_t *app)
{
    window_t *win = application_get_whandle(app);
    glrenderer2d_t *renderer = app->game;

}

void app_shutdown(application_t *app)
{
}

int main(void)
{
    window_t win = window_init("Flappy Birds", 700, 800, SDL_INIT_VIDEO);

    application_t game = application_init(&win);

    glshader_t shader = glshader_default_init();
    glrenderer2d_t renderer = glrenderer2d_init(&shader,NULL);
    game.game = &renderer;

    application_run(&game);

    window_destroy(&win);

    return 0;
}
