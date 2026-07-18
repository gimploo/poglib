#pragma once
#include "audio_music.h"
#include "audio_sfx.h"

/*  music_config.h — Wwise-project-derived music definitions (NO Wwise SDK)
 *
 *  Source: E:\dev\resources\Project Vedanta_theme\Project Vedanta_theme\
 *  Container: Music_Khushi.wwu
 *
 *  Maps 1:1 to Wwise authoring data:
 *    - 4 layers (bed + 3 stems) at 80 BPM
 *    - RTPC Parkour_intensity (0..100) → layer volume curves
 *    - Stinger_impacts set (Impact_KL_01..04)
 *
 *  Edit this file when curves/assets change in Wwise authoring.
 *  Do NOT parse .wwu / .wproj / SoundBanks at runtime.
 */

static const char *MUSIC_BASE_PATH = "res/audio";

static const char *MUSIC_LAYER_FILES[] = {
    "music/PV_main-theme_bed_80bpm.wav",
    "music/PV_main-theme_layer1_80bpm.wav",
    "music/PV_main-theme_layer2_80bpm.wav",
    "music/PV_main-theme_layer3_80bpm.wav",
};
static const u32 MUSIC_LAYER_COUNT = 4;

/*  Volume curves extracted from Music_Khushi.wwu RTPC bindings.
 *  Each curve: pairs of {intensity_0_100, volume_dB}.
 *  Silent = -200 dB (effectively mute in linear).
 *  Curves are piecewise linear interpolated in music__db_from_curve().
 */
static const music_curve_point_t MUSIC_LAYER_CURVES[][8] = {
    /* bed: always audible */
    { {0, 0}, {100, 0} },
    /* layer1: silent until ~45, full by ~60 */
    { {0, -200}, {45, -200}, {60, 0}, {100, 0} },
    /* layer2: same */
    { {0, -200}, {45, -200}, {60, 0}, {100, 0} },
    /* layer3: same */
    { {0, -200}, {45, -200}, {60, 0}, {100, 0} },
};

static const u32 MUSIC_LAYER_CURVE_COUNTS[] = {
    2, 4, 4, 4,
};

static const char *MUSIC_SFX_FILES[] = {
    "sfx/Impact_KL_01.wav",
    "sfx/Impact_KL_02.wav",
    "sfx/Impact_KL_03.wav",
    "sfx/Impact_KL_04.wav",
};
static const u32 MUSIC_SFX_COUNT = 4;

#ifndef IGNORE_MUSIC_CONFIG_IMPLEMENTATION

global u32 g_sfx_impact_set = (u32)-1;

bool music_config_load(void)
{
    ma_engine *engine = audio_device_get_engine();
    if (!engine) return false;

    if (!sfx_system_init()) return false;

    /* Load music layers */
    bool ok = music_theme_load(
        MUSIC_BASE_PATH,
        MUSIC_LAYER_FILES,
        MUSIC_LAYER_COUNT,
        MUSIC_LAYER_CURVES,
        MUSIC_LAYER_CURVE_COUNTS
    );
    if (!ok) return false;

    /* Load impact SFX */
    g_sfx_impact_set = sfx_set_create("impacts");
    for (u32 i = 0; i < MUSIC_SFX_COUNT; i++) {
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", MUSIC_BASE_PATH, MUSIC_SFX_FILES[i]);
        u32 id = sfx_register(MUSIC_SFX_FILES[i], fullpath, engine);
        if (id != (u32)-1) {
            sfx_set_add(g_sfx_impact_set, id);
        }
    }

    logging("[audio] music config loaded (Wwise hybrid)");
    return true;
}

void music_config_play_impact(void)
{
    if (g_sfx_impact_set != (u32)-1) {
        sfx_play_from_set(g_sfx_impact_set);
    }
}

void music_config_unload(void)
{
    music_theme_unload();
    sfx_system_unload();
    g_sfx_impact_set = (u32)-1;
}

#endif
