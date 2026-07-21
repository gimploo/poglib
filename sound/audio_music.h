#pragma once
#include "poglib/sound/common.h"

audio_music_t audio_music_init(const ma_engine *const engine, arena_t *const arena, const audio_musiclayer_t musiclayers[AUDIO_MUSIC_LAYERS_MAX_COUNT], const u8 layer_count)
{
    audio_music_t music = {0};
    music.layers = slot_init(layer_count, sizeof(audio_musiclayer_t), arena);

    for (u8 layer_idx = 0; layer_idx < layer_count; layer_idx++)
    {
        audio_musiclayer_t layer = musiclayers[layer_idx];
        layer.internal.sound = arena_reserve(arena, sizeof(ma_sound));
        layer.filepath = musiclayers[layer_idx].filepath;
        layer.curves = musiclayers[layer_idx].curves;

        const ma_result result = ma_sound_init_from_file(engine, layer.filepath.data, MA_SOUND_FLAG_DECODE, NULL, NULL, layer.internal.sound);
        if (result != MA_SUCCESS) eprint("[audio] music: failed to load `"STR_FMT"`: %d\n", STR_ARG(layer.filepath), result);

        ma_sound_set_looping(layer.internal.sound, MA_TRUE);
        slot_insert(&music.layers, layer_idx, &layer, sizeof(layer));

        logging("loaded music layer(%i) `"STR_FMT"`", layer_idx, STR_ARG(layer.filepath));
    }

    return music;
}

void audio_music_play_all_layers(audio_music_t *const self)
{
    slot_iterator(&self->layers, iter)
    {
        audio_musiclayer_t *const layer = iter;
        ma_sound_start(layer->internal.sound);
    }
}

void audio_music_play_layer(audio_music_t *const self, const u32 layer_idx)
{
    audio_musiclayer_t *const layer = slot_get_value(&self->layers, layer_idx);
    ma_sound_start(layer->internal.sound);
}

void audio_music_stop_layer(audio_music_t *const self, const u32 layer_idx)
{
    audio_musiclayer_t *const layer = slot_get_value(&self->layers, layer_idx);
    ma_sound_stop(layer->internal.sound);
}

void audio_music_destroy(audio_music_t *const self)
{
    slot_iterator(&self->layers, iter)
    {
        audio_musiclayer_t *const layer = iter;
        ma_sound_uninit(layer->internal.sound);
    }
    slot_destroy(&self->layers);
}


