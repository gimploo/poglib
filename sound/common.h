#pragma once
#include <poglib/basic.h>
#include <poglib/math.h>
#define MA_NO_DSOUND
#define MINIAUDIO_IMPLEMENTATION
#include <poglib/external/miniaudio.h>

#define AUDIO_MUSIC_LAYERS_MAX_COUNT    16
#define AUDIO_CURVE_MAX_COUNT           8
#define AUDIO_SMOOTHING_SPEED           5.f

typedef struct audio_device_t       audio_device_t;
typedef struct audio_musiclayer_t   audio_musiclayer_t;
typedef struct audio_music_t        audio_music_t;
typedef struct audio_sfx_t          audio_sfx_t;

struct audio_device_t {
    ma_engine_config config;
    bool is_initialized;
};

struct audio_musiclayer_t {
    struct {
        ma_sound    *sound;
    } internal;
    str_t       filepath;
    struct {
        u8      count;
        vec2f_t data[AUDIO_CURVE_MAX_COUNT];
    } curves;
    f32         intensity;
};

struct audio_music_t
{
    slot_t layers;
};

struct audio_sfx_t {
    str_t       name;
    ma_sound    *sound;
};

