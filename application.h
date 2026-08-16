#pragma once
#include "poglib/basic/arena.h"
#include "poglib/basic/common.h"
#include "application/window/sdl_window.h"
#include "application/stopwatch.h"
#include "sound/audio_device.h"
#include "font/glfreetypefont.h"


/*=============================================================================
                            - APPLICATION -
=============================================================================*/

#define APPLICATION_UPDATE_FIXED_TIME_STEP (1.0f / 60.0f)

typedef struct application_t {

    struct {
        char                *title;
        u32                 width;
        u32                 height;
        f32                 aspect_ratio;
        vec4f_t             background_color;
    } window;

    struct {
        const char *base_dir;
    } context;

    //NOTE: this holds data is shared bw all callbacks
    buffer_t                content;

    struct {
        window_t            *window;
        stopwatch_t         *timer;
        glfreetypefont_t    *fontrenderer;
        arena_t            *arena;
    } handle;

    void (*init)(struct application_t *const);
    void (*tick)(struct application_t *const);
    void (*update)(struct application_t *const);
    void (*render)(struct application_t *const, const f32 raw_dt);
    void (*destroy)(struct application_t *const);

} application_t ;


void            application_run(application_t * const app);

#define         application_alloc_content(PAPP, TYPE)\
                    (TYPE *)application__internal__alloc_content((PAPP), sizeof(TYPE))

#define         application_get_content(PAPP)               (void *)(PAPP)->content.raw_data
#define         application_get_window(PAPP)                (PAPP)->handle.window
#define         application_get_fps(PAPP)                   (PAPP)->handle.timer->fps
#define         application_get_fixed_dt(PAPP)              (PAPP)->handle.timer->fixed_dt
#define         application_get_raw_dt(PAPP)                (PAPP)->handle.timer->raw_dt
f32             application_get_tick(const application_t *);
str_t           application_get_absolute_filepath(application_t *app, const char *filepath);





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

f32 application_get_tick(const application_t *const app)
{
    return app->handle.timer->now;
}

void application_run(application_t *const app)
{
#ifdef DEBUG
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

    //NOTE: removed `audio`, reason is that minaudio is used now for audio 
    //also removed `haptic` for now, as i dont know what we would use it for anyway
    const u64 SDL_FLAGS = 
        SDL_INIT_TIMER 
        | SDL_INIT_VIDEO 
        | SDL_INIT_EVENTS 
        | SDL_INIT_JOYSTICK 
        | SDL_INIT_GAMECONTROLLER 
        | SDL_INIT_SENSOR;

    runtimectx_init();

    app->window.aspect_ratio = (f32)app->window.width / (f32)app->window.height;
    app->context.base_dir = __get_base_dir();

    window_t *win = window_init(
            app->window.title, 
            app->window.width, 
            app->window.height, 
            SDL_FLAGS);
    assert(win);

    win->background_color = app->window.background_color;

    logging("Accessing audio device `%s`...", SDL_GetAudioDeviceName(0,0));
    audio_device_init();

    stopwatch_t timer = stopwatch();
    const f32 FIXED_DT = APPLICATION_UPDATE_FIXED_TIME_STEP; //Runs at 60Hz

    app->handle.window= win;
    app->handle.timer = &timer;
    app->handle.fontrenderer = NULL;
    app->handle.arena = arena_init(NULL, 16 * MB);

    // Initialize the content in the application
    printf("[!] APPLICATION INIT!\n");
    app->init(app);

    printf("[!] APPLICATION RUNNING!\n");
    // Update the content in the application
    while(win->is_open)
    {
        stopwatch_update(&timer);

        app->tick(app);

        while(timer.accumulator >= FIXED_DT)
        {
            app->update(app);
            timer.accumulator -= FIXED_DT;
            window_flush_transient_data(win);
        }

        //TODO: this is used to interpolate bw frames like adjusting transforms of an object from one frame moving into the other. 
        //right now we arent using it in anything meaninful, eariler code was using it as raw_dt which is incorrect. To better use 
        //this in systems in place, we could try to bring something that holds this data.
        //
        //const f32 alpha = timer.accumulator / FIXED_DT;

        window_gl_render_begin(win);
            app->render(app, timer.raw_dt);
        window_gl_render_end(win);

        //INFO: Smooth the FPS value so it doesn't flicker
        //0.95 keeps the old value, 0.05 takes the new value
        if (timer.raw_dt > 0.0f) {
            const f32 instantaneous_fps = 1.0f / timer.raw_dt;
            timer.fps = (timer.fps * 0.95f) + (instantaneous_fps * 0.05f);
        }
    }

    printf("[!] APPLICATION SHUTDOWN!\n");
#ifdef DEBUG
    arena_logger_dump_collapsed("arena_map.folded");
#endif
    app->destroy(app);
    SDL_free((char *)app->context.base_dir);

    window_destroy();
    arena_destroy(app->handle.arena);
    audio_device_destroy();
    runtimectx_destroy();


#ifdef DEBUG
    dbg_destroy();
#endif
}

str_t application_get_absolute_filepath(application_t *app, const char *filepath)
{
    ASSERT(app->context.base_dir);

    char scratch[KB] = {0};
    ASSERT((strlen(app->context.base_dir) + strlen(filepath) + 1) <= ARRAY_LEN(scratch));
    const char *base_dir = app->context.base_dir;
    cstr_combine_path(base_dir, filepath, scratch, ARRAY_LEN(scratch));
    return str_init(app->handle.arena, scratch);
}


INTERNAL void * application__internal__alloc_content(application_t *const app, const u64 content_size)
{
    void *const content = arena_reserve(
        app->handle.arena,
        content_size);
    app->content = (buffer_t) {
        .raw_data = content,
        .size = content_size
    };
    return content;
}

#endif 

