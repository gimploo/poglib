#pragma once
#include "audio_music.h"
#include "poglib/util/assetmanager.h"

/*  music_config.h — Wwise-project-derived music definitions (NO Wwise SDK)
 *
 *  Source: E:\dev\resources\Project Vedanta_theme\Project Vedanta_theme\
 *  Container: Music_Khushi.wwu
 *
 *  Maps 1:1 to Wwise authoring data:
 *    - 4 layers (bed + 3 stems) at 80 BPM
 *    - RTPC Parkour_intensity (0..100) → layer volume curves
 *
 *  Edit this file when curves/assets change in Wwise authoring.
 *  Do NOT parse .wwu / .wproj / SoundBanks at runtime.
 */

static const str_t MUSIC_BASE_PATH = str_lit("res/audio");

static const str_t MUSIC_LAYER_FILES[] = {
    str_lit("music/PV_main-theme_bed_80bpm.wav"),
    str_lit("music/PV_main-theme_layer1_80bpm.wav"),
    str_lit("music/PV_main-theme_layer2_80bpm.wav"),
    str_lit("music/PV_main-theme_layer3_80bpm.wav"),
};
static const u32 MUSIC_LAYER_COUNT = 4;

/*  Volume curves extracted from Music_Khushi.wwu RTPC bindings.
 *  Each curve: pairs of {intensity_0_100, volume_dB}.
 *  Silent = -200 dB (effectively mute in linear).
 *  Curves are piecewise linear interpolated in music__internal__db_from_curve().
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

#ifndef IGNORE_MUSIC_CONFIG_IMPLEMENTATION

bool music_config_load(void)
{
    ma_engine *engine = audio_device_get_engine();
    if (!engine) return false;

    music_audio_info_t audio_info[MUSIC_LAYER_COUNT] = {0};

    for (u32 i = 0; i < MUSIC_LAYER_COUNT; i++) {
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%.*s/%.*s",
            STR_ARG(MUSIC_BASE_PATH), STR_ARG(MUSIC_LAYER_FILES[i]));

        u32 asset_id = assetmanager_load_audio(&global_engine->systems.assets, str(fullpath));
        if (asset_id == INVALID_ASSET_ID) {
            eprint("[audio] music_config: failed to load `%s`\n", fullpath);
            return false;
        }

        const audio_asset_t *asset = assetmanager_get_assetresource(
            &global_engine->systems.assets, ASSET_TYPE_AUDIO, asset_id);
        if (!asset) {
            eprint("[audio] music_config: asset not ready `%s`\n", fullpath);
            return false;
        }

        audio_info[i] = (music_audio_info_t){
            .pcm_data    = asset->pcm_data,
            .frame_count = asset->frame_count,
            .channels    = asset->channels,
            .sample_rate = asset->sample_rate,
        };
    }

    bool ok = music_theme_load(
        audio_info,
        MUSIC_LAYER_COUNT,
        MUSIC_LAYER_CURVES,
        MUSIC_LAYER_CURVE_COUNTS
    );
    if (!ok) return false;

    logging("[audio] music config loaded (asset manager)");
    return true;
}

void music_config_unload(void)
{
    music_theme_unload();
}

#endif
