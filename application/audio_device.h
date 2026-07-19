#pragma once
#define MA_NO_DSOUND
#define MINIAUDIO_IMPLEMENTATION
#include <poglib/external/miniaudio.h>
#include <poglib/basic.h>

typedef struct audio_device_t audio_device_t;

struct audio_device_t {
    bool is_initialized;
};

global audio_device_t       global_audio_device = {0};
global ma_engine            global_audio_engine = {0};

bool                        audio_device_init(void);
audio_device_t *            audio_device_get(void);
void                        audio_device_destroy(void);


#ifndef IGNORE_AUDIO_DEVICE_IMPLEMENTATION

bool audio_device_init(void)
{
    if (global_audio_device.is_initialized) return true;

    ma_engine_config cfg = ma_engine_config_init();
    cfg.channels    = 2;
    cfg.sampleRate  = 48000;

    ma_result result = ma_engine_init(&cfg, &global_audio_engine);
    if (result != MA_SUCCESS) {
        eprint("Failed to initialize miniaudio engine: %d\n", result);
        return false;
    }

    global_audio_device.is_initialized = true;
    logging("[audio] miniaudio engine initialized (2ch, 48kHz)");
    return true;
}

void audio_device_destroy(void)
{
    if (!global_audio_device.is_initialized) return;

    ma_engine_uninit(&global_audio_engine);
    global_audio_device.is_initialized = false;
    logging("[audio] miniaudio engine shutdown");
}

audio_device_t * audio_device_get(void)
{
    return &global_audio_device;
}

ma_engine *audio_device_get_engine(void)
{
    return &global_audio_engine;
}

#endif
