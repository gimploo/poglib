#pragma once
#include "./common.h"

typedef struct wwise_audio_t wwise_audio_t;

//NOTE: THIS FILE IS UPDATED BY AN EXTERNAL TOOL - 
//IT READS THE WWISE PROJECT AND UPDATE THE VARIABLES BELOW

#define AUDIO_ROOT_DIR "res/audio"


const audio_musiclayer_t WWISE_LOADED_MUSICS[][4] = {

    [0] = {
        [0] = {
            .filepath = str_lit(AUDIO_ROOT_DIR"/music/PV_main-theme_bed_80bpm.wav"),
            .curves = { 
                .count  = 2, 
                .data   = { {0, 0}, {100, 0} }
            },
        },
        [1] = { 
            .filepath = str_lit(AUDIO_ROOT_DIR"/music/PV_main-theme_layer1_80bpm.wav"),
            .curves = {
                .count = 4,
                .data = { {0, -200}, {45, -200}, {60, 0}, {100, 0} }
            },
        },
        [2] = { 
            .filepath = str_lit(AUDIO_ROOT_DIR"/music/PV_main-theme_layer2_80bpm.wav"),
            .curves = {
                .count = 4,
                .data = { {0, -200}, {45, -200}, {60, 0}, {100, 0} },
            },
        },
        [3] = { 
            .filepath = str_lit(AUDIO_ROOT_DIR"/music/PV_main-theme_layer3_80bpm.wav"),
            .curves = {
                .count = 4,
                .data = { {0, -200}, {45, -200}, {60, 0}, {100, 0} }
            }
        },
    }

};

const str_t WWISE_LOADED_SFX[] = 
{
    str_lit(AUDIO_ROOT_DIR"/sfx/Impact_KL_02.wav"),
    str_lit(AUDIO_ROOT_DIR"/sfx/Impact_KL_01.wav"),
    str_lit(AUDIO_ROOT_DIR"/sfx/Impact_KL_03.wav"),
    str_lit(AUDIO_ROOT_DIR"/sfx/Impact_KL_04.wav"),
};


struct wwise_audio_t {
    u32 music_asset_ids[ARRAY_LEN(WWISE_LOADED_MUSICS)];
    u32 sfx_asset_ids[ARRAY_LEN(WWISE_LOADED_SFX)];
};


