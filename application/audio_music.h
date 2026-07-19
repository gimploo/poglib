#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>
#include "poglib/external/miniaudio.h"

#define MUSIC_MAX_LAYERS 8

typedef struct music_layer_t music_layer_t;
typedef struct music_theme_t music_theme_t;
typedef vec2f_t music_curve_point_t;

typedef struct {
    const void *pcm_data;
    u64          frame_count;
    u32          channels;
    u32          sample_rate;
} music_audio_info_t;

struct music_layer_t{
    ma_audio_buffer  buffer;
    ma_sound         sound;
    music_curve_point_t curve[8];
    u32              curve_count;
    f32              current_db;
    bool             loaded;
};

struct music_theme_t {
    music_layer_t   layers[MUSIC_MAX_LAYERS];
    u32             layer_count;
    f32             intensity;
    f32             intensity_target;
    f32             smoothing_speed;
    bool            playing;
    bool            initialized;
};

global music_theme_t        global_music = {0};

bool                        music_theme_load(const music_audio_info_t audio_info[], u32 count,
                                             const music_curve_point_t curves[][8], const u32 curve_counts[]);
void                        music_theme_play(void);
void                        music_theme_stop(f32 fade_sec);
void                        music_theme_set_intensity(f32 value_0_100);
void                        music_theme_update(f32 dt);
void                        music_theme_unload(void);
f32                         music_theme_get_intensity(void);

#ifndef IGNORE_AUDIO_MUSIC_IMPLEMENTATION

INTERNAL f32 music__internal__db_from_curve(const music_curve_point_t *curve, u32 count, f32 intensity)
{
    if (count == 0) return 0.0f;
    if (intensity <= curve[0].x) return curve[0].y;
    if (intensity >= curve[count - 1].x) return curve[count - 1].y;

    for (u32 i = 0; i < count - 1; i++) {
        if (intensity >= curve[i].x && intensity <= curve[i + 1].x) {
            f32 t = (intensity - curve[i].x) / (curve[i + 1].x - curve[i].x);
            return curve[i].y + t * (curve[i + 1].y - curve[i].y);
        }
    }
    return curve[count - 1].y;
}

INTERNAL f32 music__internal__db_to_linear(f32 db)
{
    if (db <= -200.0f) return 0.0f;
    return powf(10.0f, db / 20.0f);
}

INTERNAL void music__internal__apply_intensity(music_theme_t *m)
{
    for (u32 i = 0; i < m->layer_count; i++) {
        music_layer_t *layer = &m->layers[i];
        if (!layer->loaded) continue;

        f32 target_db = music__internal__db_from_curve(layer->curve, layer->curve_count, m->intensity);
        layer->current_db = target_db;

        f32 gain = music__internal__db_to_linear(target_db);
        ma_sound_set_volume(&layer->sound, gain);
    }
}

bool music_theme_load(
    const music_audio_info_t audio_info[],
    u32 count,
    const music_curve_point_t curves[][8],
    const u32 curve_counts[])
{
    music_theme_unload();

    if (count > MUSIC_MAX_LAYERS) count = MUSIC_MAX_LAYERS;

    ma_engine *engine = audio_device_get_engine();
    if (!engine) {
        eprint("[audio] music: no engine\n");
        return false;
    }

    global_music.layer_count = count;
    global_music.intensity = 0.0f;
    global_music.intensity_target = 0.0f;
    global_music.smoothing_speed = 5.0f;
    global_music.playing = false;

    for (u32 i = 0; i < count; i++) {
        music_layer_t *layer = &global_music.layers[i];

        layer->curve_count = curve_counts[i];
        memcpy(layer->curve, curves[i], sizeof(music_curve_point_t) * curve_counts[i]);
        layer->current_db = 0.0f;

        if (!audio_info[i].pcm_data || audio_info[i].frame_count == 0) {
            eprint("[audio] music: layer %u has no audio data\n", i);
            layer->loaded = false;
            continue;
        }

        ma_audio_buffer_config buf_cfg = ma_audio_buffer_config_init(
            ma_format_f32,
            audio_info[i].channels,
            audio_info[i].frame_count,
            (void *)audio_info[i].pcm_data,
            NULL);
        buf_cfg.doesCopy = MA_FALSE;

        ma_result result = ma_audio_buffer_init(&buf_cfg, &layer->buffer);
        if (result != MA_SUCCESS) {
            eprint("[audio] music: buffer init failed layer %u: %d\n", i, result);
            layer->loaded = false;
            continue;
        }

        result = ma_sound_init_from_data_source(engine,
            &layer->buffer, 0, NULL, &layer->sound);
        if (result != MA_SUCCESS) {
            eprint("[audio] music: sound init failed layer %u: %d\n", i, result);
            ma_audio_buffer_uninit(&layer->buffer);
            layer->loaded = false;
            continue;
        }

        ma_sound_set_looping(&layer->sound, MA_TRUE);
        layer->loaded = true;
    }

    global_music.initialized = true;
    music__internal__apply_intensity(&global_music);

    logging("[audio] music theme loaded (%u layers)", count);
    return true;
}

void music_theme_play(void)
{
    if (!global_music.initialized) return;
    if (global_music.playing) return;

    for (u32 i = 0; i < global_music.layer_count; i++) {
        if (!global_music.layers[i].loaded) continue;
        ma_sound_start(&global_music.layers[i].sound);
    }
    global_music.playing = true;
}

void music_theme_stop(f32 fade_sec)
{
    if (!global_music.initialized) return;
    if (!global_music.playing) return;

    if (fade_sec <= 0.0f) {
        for (u32 i = 0; i < global_music.layer_count; i++) {
            if (!global_music.layers[i].loaded) continue;
            ma_sound_stop(&global_music.layers[i].sound);
        }
        global_music.playing = false;
        return;
    }

    for (u32 i = 0; i < global_music.layer_count; i++) {
        if (!global_music.layers[i].loaded) continue;
        ma_sound_set_fade_in_milliseconds(&global_music.layers[i].sound,
            -1.0f, 0.0f, (ma_uint64)(fade_sec * 1000.0f));
        ma_sound_stop(&global_music.layers[i].sound);
    }
    global_music.playing = false;
}

void music_theme_set_intensity(f32 value_0_100)
{
    global_music.intensity_target = value_0_100;
}

void music_theme_update(f32 dt)
{
    if (!global_music.initialized || !global_music.playing) return;

    f32 diff = global_music.intensity_target - global_music.intensity;
    if (fabsf(diff) > 0.1f) {
        global_music.intensity += diff * global_music.smoothing_speed * dt;
    } else {
        global_music.intensity = global_music.intensity_target;
    }

    music__internal__apply_intensity(&global_music);
}

void music_theme_unload(void)
{
    for (u32 i = 0; i < global_music.layer_count; i++) {
        if (global_music.layers[i].loaded) {
            ma_sound_uninit(&global_music.layers[i].sound);
            ma_audio_buffer_uninit(&global_music.layers[i].buffer);
            global_music.layers[i].loaded = false;
        }
    }
    global_music.layer_count = 0;
    global_music.playing = false;
    global_music.initialized = false;
}

f32 music_theme_get_intensity(void)
{
    return global_music.intensity;
}

#endif
