#pragma once
#include "poglib/sound/common.h"



audio_music_t       audio_music_init(ma_engine *const engine, arena_t *const arena, const audio_musiclayer_t musiclayers[AUDIO_MUSIC_LAYERS_MAX_COUNT], const u8 layer_count);
void                audio_music_play_layer(audio_music_t *const self, const u32 layer_idx);
void                audio_music_play_all_layers(audio_music_t *const self);
void                audio_music_update(audio_music_t *const self, const f32 dt);
void                audio_music_stop_layer(audio_music_t *const self, const u32 layer_idx);
void                audio_music_destroy(audio_music_t *const self);



#ifndef IGNORE_AUDIO_MUSIC_IMPLEMENTATION

INTERNAL f32 audio_music__internal__db_from_curve(const vec2f_t curve[AUDIO_CURVE_MAX_COUNT], const u8 count, const f32 intensity);

audio_music_t audio_music_init(ma_engine *const engine, arena_t *const arena, const audio_musiclayer_t musiclayers[AUDIO_MUSIC_LAYERS_MAX_COUNT], const u8 layer_count)
{
    audio_music_t music = {0};
    music.layers = slot_init(layer_count, sizeof(audio_musiclayer_t), arena);

    for (u8 layer_idx = 0; layer_idx < layer_count; layer_idx++)
    {
        audio_musiclayer_t layer        = musiclayers[layer_idx];
        layer.internal.sound            = arena_reserve(arena, sizeof(ma_sound));
        layer.volume                    = 0.0f;
        layer.internal.is_active        = false;
        layer.internal.target_volume    = 0.0f;

        const ma_result result = ma_sound_init_from_file(engine, layer.filepath.data, MA_SOUND_FLAG_DECODE, NULL, NULL, layer.internal.sound);
        if (result != MA_SUCCESS) 
            eprint("[audio] music: failed to load `"STR_FMT"`: %d\n", STR_ARG(layer.filepath), result);

        ma_sound_set_looping(layer.internal.sound, MA_TRUE);
        ma_sound_set_volume(layer.internal.sound, 0.0f);
        slot_insert(&music.layers, layer_idx, &layer, sizeof(layer));

        logging("loaded music layer(%i) `"STR_FMT"`", layer_idx, STR_ARG(layer.filepath));
    }
    return music;
}

void audio_music_play_layer(audio_music_t *const self, const u32 layer_idx)
{
    audio_musiclayer_t *const layer = slot_get_value(&self->layers, layer_idx);
    const f32 target_db             = audio_music__internal__db_from_curve(layer->curves.data, layer->curves.count, 100.0f);
    layer->internal.target_volume   = ma_volume_db_to_linear(target_db);
    layer->internal.is_active       = true;

    ma_sound_start(layer->internal.sound);
}

void audio_music_play_all_layers(audio_music_t *const self)
{
    for (u32 idx = 0; idx < self->layers.len; idx++)
    {
        audio_music_play_layer(self, idx);
    }
}

void audio_music_stop_layer(audio_music_t *const self, const u32 layer_idx)
{
    audio_musiclayer_t *const layer = slot_get_value(&self->layers, layer_idx);
    layer->internal.is_active = false;
    layer->internal.target_volume = 0.0f;
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

void audio_music_update(audio_music_t *const self, const f32 dt)
{
     slot_iterator(&self->layers, iter)
    {
        audio_musiclayer_t *const layer = iter;

        const f32 diff = layer->internal.target_volume - layer->volume;
        if (fabsf(diff) > 0.001f)       layer->volume += diff * AUDIO_SMOOTHING_SPEED * dt;
        else                            layer->volume = layer->internal.target_volume;
        if (layer->volume < 0.0f)       layer->volume = 0.0f;

        ma_sound_set_volume(layer->internal.sound, layer->volume);

        if (!layer->internal.is_active
            && layer->volume <= 0.001f
            && ma_sound_is_playing(layer->internal.sound)
        ) {
            ma_sound_stop(layer->internal.sound);
        }
    }
}

INTERNAL f32 audio_music__internal__db_from_curve(const vec2f_t curve[AUDIO_CURVE_MAX_COUNT], const u8 count, const f32 intensity)
{
    if (count == 0)                         return 0.0f;
    if (intensity <= curve[0].x)            return curve[0].y;
    if (intensity >= curve[count - 1].x)    return curve[count - 1].y;

    for (u8 i = 0; i < count - 1; i++) 
    {
        if (intensity >= curve[i].x && intensity <= curve[i + 1].x) {
            const f32 t = (intensity - curve[i].x) / (curve[i + 1].x - curve[i].x);
            return glm_lerp(curve[i].y, curve[i + 1].y, t);
        }
    }
    return curve[count - 1].y;
}


#endif
