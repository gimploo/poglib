#pragma once
#include "./common.h"

audio_sfx_t     audio_sfx_init(ma_engine *const engine, arena_t *const arena, const str_t filepath);
void            audio_sfx_play(audio_sfx_t *const self);
void            audio_sfx_destroy(audio_sfx_t *const self);

#ifndef IGNORE_AUDIO_SFX_IMPLEMENTATION

audio_sfx_t audio_sfx_init(ma_engine *const engine, arena_t *const arena, const str_t filepath)
{
    ma_sound *sound = arena_reserve(arena, sizeof(ma_sound));
    ma_result result = ma_sound_init_from_file(engine, filepath.data, MA_SOUND_FLAG_DECODE, NULL, NULL, sound);
    if (result != MA_SUCCESS) {
        eprint("failed to load `"STR_FMT"`: %d\n", STR_ARG(filepath), result);
    }

    logging("loaded audio sfx `"STR_FMT"`", STR_ARG(filepath));

    return (audio_sfx_t) {
        .name = filepath,
        .sound = sound
    };
}

void audio_sfx_play(audio_sfx_t *const self)
{
    ASSERT(self);
    ma_sound_set_volume(self->sound, 1.0f);
    ma_sound_seek_to_pcm_frame(self->sound, 0);
    ma_sound_start(self->sound);
}

void audio_sfx_destroy(audio_sfx_t *const self)
{
    ASSERT(self);
    ma_sound_uninit(self->sound);
}

#endif
