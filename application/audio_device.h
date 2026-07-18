#pragma once
#include "basic/common.h"

typedef struct audio_device_t {
    bool is_initialized;
} audio_device_t;

bool        audio_device_init(void);
void        audio_device_shutdown(void);
audio_device_t *audio_device_get(void);

#ifndef IGNORE_AUDIO_DEVICE_IMPLEMENTATION

#ifdef _WIN64
#define MA_NO_WASAPI
#endif

#define MINIAUDIO_IMPLEMENTATION
#include "poglib/external/miniaudio.h"

global audio_device_t g_audio_device = {0};
global ma_engine      g_ma_engine    = {0};

bool audio_device_init(void)
{
    if (g_audio_device.is_initialized) return true;

    ma_engine_config cfg = ma_engine_config_init();
    cfg.channels = 2;
    cfg.sampleRate = 48000;

    ma_result result = ma_engine_init(&cfg, &g_ma_engine);
    if (result != MA_SUCCESS) {
        eprint("Failed to initialize miniaudio engine: %d\n", result);
        return false;
    }

    g_audio_device.is_initialized = true;
    logging("[audio] miniaudio engine initialized (2ch, 48kHz)");
    return true;
}

void audio_device_shutdown(void)
{
    if (!g_audio_device.is_initialized) return;

    ma_engine_uninit(&g_ma_engine);
    g_audio_device.is_initialized = false;
    logging("[audio] miniaudio engine shutdown");
}

audio_device_t *audio_device_get(void)
{
    return &g_audio_device;
}

ma_engine *audio_device_get_engine(void)
{
    return &g_ma_engine;
}

#endif
