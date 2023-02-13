#pragma once
#include "window/sdl_window.h"
#include <SDL2/SDL_audio.h>

typedef struct audio_t {

    char                label[32];
    SDL_AudioSpec       spec;
    u8                  *buffer;
    u32                 buffsize;
    SDL_AudioDeviceID   device;
    u32                 position;

} audio_t ;

audio_t     audio_init(const char *filepath);
void        audio_play(const audio_t *self);
void        audio_change_volume(audio_t *self, const f32 volume);
void        audio_destroy(audio_t *self);

#ifndef IGNORE_AUDIO_IMPLEMENTATION

audio_t audio_init(const char *filepath)
{
    audio_t o = {0};

    assert(filepath);
    assert(strlen(filepath) < sizeof(o.label));

    memcpy(o.label, filepath, strlen(filepath) + 1);

    if (SDL_LoadWAV(filepath, &o.spec, &o.buffer, &o.buffsize) == NULL)
        eprint("Failed to load audio file %s! SDL Error: %s\n", filepath, SDL_GetError());

    o.device = SDL_OpenAudioDevice(
            SDL_GetAudioDeviceName(0,0), 
            0, 
            &o.spec, 
            NULL, 
            /*SDL_AUDIO_ALLOW_ANY_CHANGE);*/
            0);
    if (o.device < 0)
        eprint("Failed to open audio device for `%s` %s\n", filepath, SDL_GetError());

    return o;
}

void audio_play(const audio_t *self)
{
    int success = SDL_QueueAudio(self->device, self->buffer, self->buffsize);
    if (success < 0)
        eprint("SDL Queue Audio ERROR %s\n", SDL_GetError());

    SDL_PauseAudioDevice(self->device, 0);
}

void audio_change_volume(audio_t *self, const f32 volume)
{
    u8 *stream = self->buffer;
    for (int i = 0; i < self->buffsize; i++)
        stream[i] = (u8)(stream[i] * volume);
}

void audio_destroy(audio_t *self)
{
    SDL_CloseAudioDevice(self->device);
    SDL_FreeWAV(self->buffer);

    memset(self, 0, sizeof(audio_t ));
}

#endif
