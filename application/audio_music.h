#pragma once
#include "basic/common.h"
#include "basic/str.h"
#include <math.h>
#include "poglib/external/miniaudio.h"

/*  audio_music.h — Hybrid Wwise-authoring / miniaudio-runtime layered music
 *
 *  Mirrors the Music_Khushi Wwise project structure (NO Wwise SDK):
 *    - 4 synced looping layers (bed + layer1/2/3) at 80 BPM
 *    - RTPC Parkour_intensity (0..100) drives layer volumes via curves
 *    - play / stop / fade
 *
 *  Source Wwise project (authoring only):
 *    E:\dev\resources\Project Vedanta_theme\Project Vedanta_theme\
 *
 *  Curve data extracted from Music_Khushi.wwu RTPC bindings:
 *    - bed:    always audible (0 dB across range)
 *    - layer1/2/3: silent (~-200 dB) until intensity ~45, full by ~60
 *  See music_config.h for actual curve tables.
 */

#define MUSIC_MAX_LAYERS 8

typedef struct {
    f32 x;
    f32 y;
} music_curve_point_t;

typedef struct {
    str_t               filepath;
    ma_sound            sound;
    music_curve_point_t curve[8];
    u32                 curve_count;
    f32                 current_db;
    bool                loaded;
} music_layer_t;

typedef struct {
    music_layer_t   layers[MUSIC_MAX_LAYERS];
    u32             layer_count;
    f32             intensity;
    f32             intensity_target;
    f32             smoothing_speed;
    bool            playing;
    bool            initialized;
} music_theme_t;

bool    music_theme_load(str_t base_path, const str_t layer_files[], u32 count,
                         const music_curve_point_t curves[][8], const u32 curve_counts[]);
void    music_theme_play(void);
void    music_theme_stop(f32 fade_sec);
void    music_theme_set_intensity(f32 value_0_100);
void    music_theme_update(f32 dt);
void    music_theme_unload(void);
f32     music_theme_get_intensity(void);

#ifndef IGNORE_AUDIO_MUSIC_IMPLEMENTATION

global music_theme_t g_music = {0};

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
    str_t base_path,
    const str_t layer_files[],
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

    g_music.layer_count = count;
    g_music.intensity = 0.0f;
    g_music.intensity_target = 0.0f;
    g_music.smoothing_speed = 5.0f;
    g_music.playing = false;

    for (u32 i = 0; i < count; i++) {
        music_layer_t *layer = &g_music.layers[i];

        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%.*s/%.*s",
            STR_ARG(base_path), STR_ARG(layer_files[i]));
        layer->filepath = str(fullpath);

        layer->curve_count = curve_counts[i];
        memcpy(layer->curve, curves[i], sizeof(music_curve_point_t) * curve_counts[i]);
        layer->current_db = 0.0f;

        ma_result result = ma_sound_init_from_file(engine, fullpath,
            MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_DECODE, NULL, NULL, &layer->sound);
        if (result != MA_SUCCESS) {
            eprint("[audio] music: failed to load `%s`: %d\n", fullpath, result);
            layer->loaded = false;
            continue;
        }

        ma_sound_set_looping(&layer->sound, MA_TRUE);
        layer->loaded = true;
    }

    g_music.initialized = true;
    music__internal__apply_intensity(&g_music);

    logging("[audio] music theme loaded (%u layers)", count);
    return true;
}

void music_theme_play(void)
{
    if (!g_music.initialized) return;
    if (g_music.playing) return;

    for (u32 i = 0; i < g_music.layer_count; i++) {
        if (!g_music.layers[i].loaded) continue;
        ma_sound_start(&g_music.layers[i].sound);
    }
    g_music.playing = true;
}

void music_theme_stop(f32 fade_sec)
{
    if (!g_music.initialized) return;
    if (!g_music.playing) return;

    if (fade_sec <= 0.0f) {
        for (u32 i = 0; i < g_music.layer_count; i++) {
            if (!g_music.layers[i].loaded) continue;
            ma_sound_stop(&g_music.layers[i].sound);
        }
        g_music.playing = false;
        return;
    }

    for (u32 i = 0; i < g_music.layer_count; i++) {
        if (!g_music.layers[i].loaded) continue;
        ma_sound_set_fade_in_milliseconds(&g_music.layers[i].sound,
            -1.0f, 0.0f, (ma_uint64)(fade_sec * 1000.0f));
        ma_sound_stop(&g_music.layers[i].sound);
    }
    g_music.playing = false;
}

void music_theme_set_intensity(f32 value_0_100)
{
    g_music.intensity_target = value_0_100;
}

void music_theme_update(f32 dt)
{
    if (!g_music.initialized || !g_music.playing) return;

    f32 diff = g_music.intensity_target - g_music.intensity;
    if (fabsf(diff) > 0.1f) {
        g_music.intensity += diff * g_music.smoothing_speed * dt;
    } else {
        g_music.intensity = g_music.intensity_target;
    }

    music__internal__apply_intensity(&g_music);
}

void music_theme_unload(void)
{
    for (u32 i = 0; i < g_music.layer_count; i++) {
        if (g_music.layers[i].loaded) {
            ma_sound_uninit(&g_music.layers[i].sound);
            g_music.layers[i].loaded = false;
        }
    }
    g_music.layer_count = 0;
    g_music.playing = false;
    g_music.initialized = false;
}

f32 music_theme_get_intensity(void)
{
    return g_music.intensity;
}

#endif
