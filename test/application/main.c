#include "../../simple/gl_renderer2d.h"
#include "../../application.h"

window_t win;
application_t game;
vec4f_t color, inc_color;
char buff[1024];

f32 dt, fps;

void init(void *arg)
{
    color = vec4f(0.0f);

    inc_color = (vec4f_t ) {
        .cmp[X] = 0.01f,
        .cmp[Y] = 0.0f,
        .cmp[Z] = 0.02f,
    };
}

void update(void *arg)
{
    fps = application_get_fps(&game);
    dt = application_get_dt(&game);

    snprintf(buff, sizeof(buff), "FPS: %f | dt: %f", fps, dt);
    SDL_Log("%s\n", buff);

    window_set_background(&win, color);
    color = vec4f_add(color, inc_color);
    color = (vec4f_t ){ 
        .cmp[X] = fmod(color.cmp[X], 1.0f),
        .cmp[Y] = fmod(color.cmp[Y], 1.0f),
        .cmp[Z] = fmod(color.cmp[Z], 1.0f),
        .cmp[W] = 1.0f
    }; 
}

void render(void *arg)
{
    window_gl_render_begin(&win);
    window_gl_render_end(&win);
}

int main(void)
{
    win = window_init("Flappy Birds", 700, 800, SDL_INIT_VIDEO);
    game = application_init(&win, init, update, render);

    application_run(&game);
}
