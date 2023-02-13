#define WINDOW_SDL
#include <poglib/application.h>

#define WINDOW_WIDTH    400
#define WINDOW_HEIGHT   500

typedef struct content_t {
    
    audio_t wavs[2];

} content_t ;

void sound_init(application_t *app) 
{
    content_t cont = {
        .wavs = {
            audio_init("res/StarWars3.wav"),
            audio_init("res/PinkPanther30.wav"),
        }
    };

    application_pass_content(app, &cont);
}

void sound_update(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);

    window_update_user_input(win);

    if (window_keyboard_is_key_pressed(win, SDLK_q))
    {
        printf("playing %s\n", c->wavs[0].label);
        audio_play(&c->wavs[0]);
    }
    if (window_keyboard_is_key_pressed(win, SDLK_w))
    {
        printf("playing %s\n", c->wavs[1].label);
        audio_play(&c->wavs[1]);
    }

}

void sound_render(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);
}

void sound_destroy(application_t *app) 
{
    window_t *win = application_get_window(app);
    content_t *c = application_get_content(app);

    for (int i = 0; i < ARRAY_LEN(c->wavs); i++)
        audio_destroy(&c->wavs[i]);
}

int main(void)
{
    application_t app = {
        .window = {
            .title = "sound_test",
            .width = WINDOW_WIDTH,
            .height = WINDOW_HEIGHT,
            .aspect_ratio = (f32)WINDOW_WIDTH / (f32)WINDOW_HEIGHT,
            .fps_limit = 60,
            .background_color = COLOR_RED
        },   
        .content = {
            .size = sizeof(content_t )
        },
        .init       = sound_init,
        .update     = sound_update,
        .render     = sound_render,
        .destroy    = sound_destroy
    };

    application_run(&app);

    return 0;
}

