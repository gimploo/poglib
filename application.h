#pragma once
#include "basic.h"
#include "gfx/glrenderer3d.h"
#include "gfx/glrenderer2d.h"
#if defined(WINDOW_GLFW)
#   pragma message("GLFW NOT FULLY IMPLEMENTED, REQUIRES ADDITIONAL WORK")
#   include "application/window/glfw_window.h"
#elif defined(WINDOW_SDL)
#   include "application/window/sdl_window.h"
#else
#   pragma message("WINDOW IMPLEMENTATION NOT SPECIFIED - RUNNING SDL VERSION (DEFAULT)")
#   include "application/window/sdl_window.h"
#endif

#include "application/stopwatch.h"
#include "application/sound.h"
#include "font/glfreetypefont.h"


/*=============================================================================
                            - APPLICATION -
=============================================================================*/

#define state_t u8

typedef struct application_t {

    struct {
        char                *title;
        u32                 width;
        u32                 height;
        f32                 aspect_ratio;
        u32                 fps_limit;
        vec4f_t             background_color;
    } window;

    struct {
        const char *base_dir;
    } context;

    struct {
        //NOTE: Add content related stuff in here
        const u64           size;
    } content;

    state_t state;

    struct {
        window_t            *window;
        stopwatch_t         *timer;
        glfreetypefont_t    *fontrenderer;
        void                *content;
    } handle;

    void (*init)(struct application_t *);
    void (*update)(struct application_t *);
    void (*render)(struct application_t *);
    void (*destroy)(struct application_t *);

} application_t ;


void            application_pass_content(application_t *app, const void *content);
void            application_run(application_t *app);

#define         application_set_font(PAPP, FONT)            (PAPP)->handle->fontrenderer = FONT

#define         application_get_game(PAPP)                  (PAPP)->handle.content
#define         application_get_content(PAPP)               (PAPP)->handle.content
#define         application_get_window(PAPP)                (PAPP)->handle.window
#define         application_get_dt(PAPP)                    (PAPP)->handle.timer->dt
#define         application_get_fps(PAPP)                   (PAPP)->handle.timer->fps
f32             application_get_tick(const application_t *);

#define         application_update_state(PAPP, STATE)       (PAPP)->state = STATE


/*---------------------------------------------------------------------------*/


#ifndef IGNORE_APPLICATION_IMPLEMENTATION

const char *__get_base_dir()
{
    char *exe_dir = SDL_GetBasePath();

#ifdef _WIN64
    char delimter = '\\';
#else
    char delimter = '/';
#endif

    u32 starting_len = strlen(exe_dir);
    ASSERT(starting_len > 0);
    starting_len = exe_dir[starting_len - 1] == delimter ? starting_len - 1 : starting_len;

    for (u32 i = starting_len - 1; i >= 0; i--)
    {
        if (exe_dir[i] == delimter) {
            break;
        }
        exe_dir[i] = '\0';
    }

    return exe_dir;
}

f32 application_get_tick(const application_t *app)
{
    return app->handle.timer->__now;
}

void application_pass_content(application_t *app, const void *content)
{
    assert(content);

    app->handle.content = calloc(1, app->content.size);
    assert(app->handle.content);
    memcpy(app->handle.content, content, app->content.size);
}

void application_run(application_t *app)
{
#ifdef DEBUG
#if defined(_WIN64)
    SetUnhandledExceptionFilter(TopLevelExceptionHandler);
#endif
    dbg_init();
#endif

    if (app == NULL)                eprint("application argument is null");
    if (!app->window.title)         eprint("application title is missing ");
    if (app->window.width <= 0)     eprint("provide a proper width to the application");
    if (app->window.height <= 0)    eprint("provide a proper height to the application");
    if (!app->init)                 eprint("application init funciton is missing");
    if (!app->update)               eprint("application update function is missing");
    if (!app->render)               eprint("application render function is missing");
    if (!app->destroy)              eprint("application shutdown function is missing");

    u64 flags = 0;
#if defined(WINDOW_SDL)
    flags = SDL_INIT_EVERYTHING;
#endif

    app->window.aspect_ratio = (f32)app->window.width / (f32)app->window.height;
    app->context.base_dir = __get_base_dir();

    window_t * win = window_init(
            app->window.title, 
            app->window.width, 
            app->window.height, 
            flags);
    assert(win);

    win->background_color = app->window.background_color;

    logging("Accessing audio device `%s`...", SDL_GetAudioDeviceName(0,0));

    stopwatch_t timer = stopwatch();

    app->handle.window= win;
    app->handle.timer = &timer;
    app->handle.fontrenderer = NULL;

    // Initialize the content in the application
    printf("[!] APPLICATION INIT!\n");
    app->init(app);

    // states
    state_t st_current  = app->state, 
            st_previous = 0; 

    const f32 cap_dt = 1000.0f/(f32)app->window.fps_limit;

    printf("[!] APPLICATION RUNNING!\n");
    // Update the content in the application
    while(win->is_open)
    {
        timer.__last   = timer.__now;
        timer.__now    = stopwatch_get_tick();
        timer.dt = (timer.__now - timer.__last) / 1000.0f;

        if (timer.dt > 0.25f) timer.dt = 0.25f;

        timer.accumulator += timer.dt;

        while(timer.accumulator >= timer.dt && timer.dt != 0.0f)
        {
            st_previous = st_current;
            application_update_state(app, st_current);
            app->update(app);
            timer.accumulator -= timer.dt;
        }

        const f32 alpha = timer.accumulator / timer.dt;
        state_t state   = st_current * alpha + st_previous * (1.0 - alpha);
        application_update_state(app, state);

        window_gl_render_begin(win);
            app->render(app);
        window_gl_render_end(win);

        timer.fps = 1.0f / timer.dt;
        stopwatch_delay(floor(cap_dt - timer.dt));
    }

    printf("[!] APPLICATION SHUTDOWN!\n");
    app->destroy(app);
    SDL_free((char *)app->context.base_dir);

    window_destroy();

    free(app->handle.content);
    app->handle.content = NULL;

#ifdef DEBUG
    dbg_destroy();
#endif
}

#endif 
